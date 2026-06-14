#pragma once

#include "./NesPCH.h"
#include "./NesEventSink.h"
#include "../NES.h"

namespace NS_NES {
    /**
     * @class NesDebugger NesDebugger.h "./NesDebugger.h"
     * @implements INesDebugger
     * @brief This is meant to be the main debugging tool of the NES emulator.
     */
    class NesDebugger : public IDebugger, public NesEventSink {
        private:
            /// @brief reference to the station
            NES* nes = nullptr;

            EventViewerConfig evConfig = {};
            vector<u32> evPixelsSnapshot = {};
            vector<DebugEventRecord> evEventsSnapshot = {};
            vector<DebugEventRecord> evrActiveFrame = {};
            vector<DebugEventRecord> evrLastFrame = {};
            vector<DebugEventRecord> evrPrevFrame = {};
            bool evSnapshotValid = false;
            enum class EventType {
                PPU_READ, PPU_WRITE, APU_READ, APU_WRITE,
                MAPPER_READ, MAPPER_WRITE, CONTROLLER_READ,
                CONTROLLER_WRITE, DMC_READ, OAM_READ, OAM_START,
                INTERRUPT_IRQ, INTERRUPT_NMI, ZERO_HIT, BREAKPOINT
            };
            array<u32, 15> defaultEventColors = {
                0xFF00DFFF, // PPU_READ
                0xFFFFA000, // PPU_WRITE
                0xFF60E060, // APU_READ
                0xFF90FF40, // APU_WRITE
                0xFF40A0FF, // MAPPER_READ
                0xFF2080FF, // MAPPER_WRITE
                0xFFFF40D0, // CONTROLLER_READ
                0xFFFF70F0, // CONTROLLER_WRITE
                0xFF40FF40, // DMC_READ
                0xFFE060A0, // OAM_READ
                0xFFFF60C0, // OAM_START
                0xFF4040FF, // INTERRUPT_IRQ
                0xFF6060FF, // INTERRUPT_NMI
                0xFFFFFFFF, // ZERO_HIT
                0xFF00BFFF  // BREAKPOINT
            };

            DebugTraceConfig traceCfg = {};
            vector<TraceRecord> traceRecords = {};
            string traceTextBuffer = {};
            ofstream traceFile;
            u64 traceFrame = 0;

            /**
             * @brief Helper function for decoding instructions
             * @param addr The address of the instruction opcode
             * @return String representation of the instruction
             */
            string DisassembleInstruction(u16 addr) const;
        public:
            /**
             * @brief explicit constructor
             * @param n reference to the NES station this debugger is attached to
             */
            NesDebugger(NES* n);
            /**
             * @brief explicit destructor; simply calls clear in order to detach the NES reference
             */
            ~NesDebugger() = default;

            void Init() override;

            /// @brief Performs one entire CPU instruction within the emulator.
            void StepInstruction() override;
            /// @brief Performs one single CPU cycle within the emulator
            void StepCycle() override;
            /// @brief Getter for the enabled flag
            bool IsEnabled() const override { return enabled; }

            #pragma region TRACE /// @brief Trace-Logging-related debugging methods; WIP
            bool TraceEnabled() const override { return traceCfg.enabled; }
            void PushTraceRecord(const TraceRecord& rec);
            void AppendFormattedTraceLine(string& out, const TraceRecord& rec) const;
            u16 ResolveEventDetailID(const DebugEventRecord& ev) const;
            void SetTraceConfig(const DebugTraceConfig& cfg) override { traceCfg = cfg; traceCfg.enabled = false; }
            const DebugTraceConfig& GetTraceConfig() const override { return traceCfg; }
            bool BeginTrace() override;
            void EndTrace() override;
            void FlushTrace() override;
            void OnInstructionExecute(u16 pc, u8 oc, u8 a, u8 x, u8 y, u8 sp, u8 status, u64 cycle) override;
            #pragma endregion
            #pragma region EVENT /// @brief Event-Viewer-related debugging methods; WIP
            EventViewerDisplaySize GetEventViewerDisplaySize() const override;
            void SetEventViewerConfig(const EventViewerConfig& cfg) override;
            const EventViewerConfig& GetEventViewerConfig() const override;
            const vector<u32>& GetEventViewerPixels() const override;
            const vector<DebugEventRecord>& GetEventViewerEvents() const override;
            const DebugEventRecord& GetEventAt(u16, u16) const override;
            void TakeEventViewerSnapshot(bool forAutoRefresh) override;
            void PushEventRecord(DebugEventRecord ev);
            void OnPpuRegisterRead(u16 addr, u8 data) override;
            void OnPpuRegisterWrite(u16 addr, u8 data) override;
            void OnApuRegisterRead(u16 addr, u8 data) override;
            void OnApuRegisterWrite(u16 addr, u8 data) override;
            void OnMapperRegisterRead(string details, u16 addr, u8 data) override;
            void OnMapperRegisterWrite(string details, u16 addr, u8 data) override;
            void OnControllerRead(u8 player, u16 addr, u8 data) override;
            void OnControllerWrite(u8 player, u16 addr, u8 data) override;
            void OnDmcDmaRead(string details, u16 addr, u8 data) override;
            void OnOamDmaRead(u16 addr, u8 data) override;
            void OnOamDmaStart(u8 data) override;
            void OnInterrupt(INTERRUPT_EVENT type) override;
            void OnSpriteZeroHit() override;
            void OnMarkedBreakpoint(string details) override;
            void OnFrameComplete() override;
            u32 GetEventColor(EventType type);
            #pragma endregion
            #pragma region CPU /// @brief CPU-related debugging methods; fully functional, but subject to change
            /**
             * @brief Acquires and formats the current state of the CPU as a two-dimensional array of strings
             * @return A vector of 3-string arrays, with one entry per CPU register/value.
             */
            vector<array<string, 3>> GetStateCPU() const override;
            /**
             * @brief Helper function to aquire and format the status flags of the CPU as a string.
             * @param reg Register value that contains all of the flags to be formatted.
             * @return A single string value representing each of the flags as a whole, formatted based on which flags are set/clear.
             */
            string GetFlags(int reg) const override;
            /**
             * @brief Acquires and formats the current state of the CPU RAM (i.e. work-RAM).
             * @param [out] numLines Reference to a `u64` variable to contain the number of bytes of RAM to be processed
             * @return A dynamically allocated pointer to the array of strings representing RAM contents.
             */
            vector<string> GetStateRAM() const override;
            /**
             * @brief Copies the CPU instruction history and then scans through the following memory to fill the supplied vector with addresses.
             * @param [out] list Reference to the vector to fill with instruction opcode addresses
             */
            void ScanInstructions(array<u64, 25>& list) const override;
            /**
             * @brief Acquires and formats a list of 25 CPU instructions.
             * @return A dynamically allocated pointer to the array of strings representing CPU instructions
             * @details
             * The first 12 instructions are disassembeled based on history acquired directly from the CPU.
             * Instruction #13 is the current instruction within the CPU.
             * The last 12 instructions are disassembled based only on instruction byte sizes.
             * In other words, branches and jumps are not actually processed, and are only disassembled
             * @todo This might be improved by simply having a history of instructions within the debugger;
             * that way we don't have to disassemble 25 instructions EVERY frame during application playback.
             */
            vector<string> GetDisassembly() const override;
            #pragma endregion
            #pragma region PPU /// @brief PPU-related debugging methods; fully functional, but subject to change
            /**
             * @brief Acquires and formats the current state of the PPU as a two-dimensional array of strings
             * @return A vector of 4-string arrays, with one entry per PPU register/value
             */
            vector<array<string, 4>> GetStatePPU() const override;
            /**
             * @brief Acquires the list of palette colors used by the ROM
             * @return A vector of the palette colors for easy processing/rendering
             */
            vector<u32> GetPaletteColors(const vector<u8>& indices) const override;
            /**
             * @brief Acquires the list of indices within palette RAM that relate to the corresponding colors within `GetPaletteColor`
             * @return A vector of index values.
             */
            vector<u8> GetPaletteIndices() const override;
            /**
             * 
             */
            const PaletteData GetPaletteData() const override;
            /**
             * @brief Acquires a pattern table used by the ROM to render backgrounds/sprites
             * @param id The id number of the pattern table
             * @return A vector containing pixel data for the full pattern table referred to by `id`
             */
            vector<u32> GetPatternTable(int tableID, int paletteID) override;
            /**
             * @brief Acquires a specified nametable currently in use for displaying ROM background(s)
             * @param id The id number of the nametable to get
             * @return A vector containing the constructed pixel data of the requested nametable
             */
            vector<u32> GetNameTable(int id) override;
            /**
             * @brief Acquires the set of sprites defined within the primary OAM of PPU memory.
             * @details Collects the data within `PPU::primaryOAM` and then uses it to construct each sprite.
             *          Upon constructing a sprite, copies the constructed pixels into a pre-sized vector such
             *          that the contents of the vector resemble a single frame consisting of an 8x8 grid of
             *          all constructed sprites.
             * @return A single vector containing the constructed pixels of all sprites.
             */
            vector<u32> GetSprites() const override;
            #pragma endregion
            #pragma region APU /// @brief APU-related debugging methods; currenly mostly unimplemented
            /**
             * @brief Acquires and formats the current state of the APU as a two-dimensional array of strings
             * @return A vector of 4-string arrays, with one entry per APU register/value
             */
            vector<array<string, 4>> GetStateAPU() const override;
            /**
             * @brief Acquires the sample data of the Pulse1 APU channel
             * @return A vector of `u32` entries relating to the various samples produced by the channel
             */
            vector<u32> GetPulse1();
            /**
             * @brief Acquires the sample data of the Pulse2 APU channel
             * @return A vector of `u32` entries relating to the various samples produced by the channel
             */
            vector<u32> GetPulse2();
            /**
             * @brief Acquires the sample data of the Triangle APU channel
             * @return A vector of `u32` entries relating to the various samples produced by the channel
             */
            vector<u32> GetTriangle();
            /**
             * @brief Acquires the sample data of the Noise APU channel
             * @return A vector of `u32` entries relating to the various samples produced by the channel
             */
            vector<u32> GetNoise();
            /**
             * @brief Acquires the sample data of the DMC APU channel
             * @return A vector of `u32` entries relating to the various samples produced by the channel
             */
            vector<u32> GetDMC();
            #pragma endregion
            #pragma region PAK /// @brief PAK-related debugging methods; currently limited to just acquiring header-defined information
            /**
             * @brief Acquires and formats the information defined within the Gamepak Header as a 2D array of strings
             * @return A vector of 2-string arrays, with one entry per Header-defined value
             */
            vector<array<string, 2>> GetPakHeader() const override;
            vector<array<string, 2>> GetPakMapper() const override;
            #pragma endregion
    };
}