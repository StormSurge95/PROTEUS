#pragma once

#include "./shared/GbaPCH.h"

namespace NS_GBA {
    class Arm7Tdmi;
    class Sm83;
    class GbaBus;
    class GbaPPU;
    class GbaAPU;
    class GbaGamepak;
    class GbaTimers;
    class GbaDMA;
    class GbaInterrupts;
    class GbaSerial;

    class GBA : public IConsole {
        public:
            GBA();
            ~GBA();

            // lifecycle functions
            bool poweron() override;
            void reset() override;
            bool shutdown() override;

            // timing functions
            void clock() override;
            void clockFrame() override;
            void clockMaster();

            bool loadROM(const string& path) override;

            // frontend-comm functions
            const u32* getFrameBuffer() const override;
            void collectAudio(vector<float>&) override;
            size_t buttonCount() const override { return 10; } // TODO: verify
            void update(u8, const bool*) override;
            const int SCREEN_WIDTH() const override { return GBA_SCREEN_WIDTH; }
            const int SCREEN_HEIGHT() const override { return GBA_SCREEN_HEIGHT; }

            // cpu-test functions
            void initSST(SingleStateTest::State) override;
            void runSST() override;
            bool checkSST(SingleStateTest::State, string&) override;

        private:
            u64 masterCycle = 0;
            bool powered = false;

            GB_Version hardwareMode = GB_Version::GBA;

            /// @brief The 32-bit ARM7TDMI CPU of the GBA console
            // uptr<Arm7Tdmi> armCpu;
            /// @brief The 8-bit SM83 CPU of the CGB/DMG console
            // uptr<Sm83> legCpu;

            /// @brief Abstraction of the GBA data/communication bus
            // uptr<GbaBus> bus;
            /// @brief Abstraction of the GBA PPU video-controller
            // uptr<GbaPPU> ppu;
            /// @brief Abstraction of the GBA APU audio-controller
            // uptr<GbaAPU> apu;
            /// @brief Abstraction of the GBA Game Pak cartridge
            // uptr<GbaGamepak> pak;

            // uptr<GbaTimers> timers;
            // uptr<GbaDMA> dma;
            // uptr<GbaInterrupts> interrupts;
            // uptr<GbaSerial> serial;

            void clockARM7cycle();
            void clockSM83cycle();
    };
}