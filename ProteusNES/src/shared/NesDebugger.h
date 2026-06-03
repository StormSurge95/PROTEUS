#pragma once

#include "./NesPCH.h"
#include "../NES.h"

namespace NS_NES {
    /**
     * @class NesDebugger NesDebugger.h "./NesDebugger.h"
     * @implements INesDebugger
     * @brief This is meant to be the main debugging tool of the NES emulator.
     */
    class NesDebugger : public IDebugger {
        private:
            /// @brief enabled flag
            bool enabled = false;
            /// @brief reference to the station
            NES* nes = nullptr;

            /**
             * @brief Helper function for decoding instructions
             * @param addr The address of the instruction opcode
             * @return String representation of the instruction
             */
            string DisassembleInstruction(u16 addr) const;

            /// @brief Palette of debug colors for rendering the ROM pattern table(s)
            u32 debugPalette[4] = { 0xFF000000, 0xFF323232, 0xFF646464, 0xFFFFFFFF };
        public:
            /**
             * @brief explicit constructor
             * @param n reference to the NES station this debugger is attached to
             */
            NesDebugger(NES* n);
            /**
             * @brief explicit destructor; simply calls clear in order to detach the NES reference
             */
            ~NesDebugger() { Clear(); }

            /// @brief Performs one entire CPU instruction within the emulator.
            void StepInstruction() override;
            /// @brief Performs one single CPU cycle within the emulator
            void StepCycle() override;
            /// @brief Getter for the enabled flag
            bool IsEnabled() const override { return enabled; }
            /// @brief clears the NES station reference
            void Clear();

            /// @section CPU
            /// @brief CPU-related debugging methods; fully functional, but subject to change
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

            /// @section PPU
            /// @brief PPU-related debugging methods; currently in progress and subject to change
            /**
             * @brief Acquires and formats the current state of the PPU as a two-dimensional array of strings
             * @return A vector of 4-string arrays, with one entry per PPU register/value
             */
            vector<array<string, 4>> GetStatePPU() const override;
            /**
             * @brief Acquires the various palette colors used by the ROM
             * @return A vector of the palette colors for easy processing/rendering
             */
            vector<u32> GetPaletteColors() override;
            /**
             * @brief Acquires a pattern table used by the ROM to render backgrounds/sprites
             * @param id The id number of the pattern table
             * @return A vector containing pixel data for the full pattern table referred to by `id`
             */
            vector<u32> GetPatternTable(int id) override;
            /**
             * @brief Acquires a specified nametable currently in use for displaying ROM background(s)
             * @param id The id number of the nametable to get
             * @return A vector containing the constructed pixel data of the requested nametable
             */
            vector<u32> GetNameTable(int id) override;

            /// @section APU
            /// @brief APU-related debugging methods; currenly unimplemented
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

            /// @section PAK
            /// @section PAK-related debuggin methods; currently limited to just acquiring header-defined information
            /**
             * @brief Acquires and formats the information defined within the Gamepak Header as a 2D array of strings
             * @return A vector of 2-string arrays, with one entry per Header-defined value
             */
            vector<array<string, 2>> GetPakHeader() const override;
    };
}