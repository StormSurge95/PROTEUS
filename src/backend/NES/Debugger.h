#pragma once

#include "../SHARED/IDebugger.h"
#include "./NES.h"

namespace NES_NS {
    class Debugger : public IDebugger {
        private:
            bool enabled = false;
            sptr<BUS> bus = nullptr;
            sptr<CPU> cpu = nullptr;
            sptr<PPU> ppu = nullptr;
            vector<u16> instAddrs;

            u32 debugPalette[4] = { 0xFF000000, 0xFF323232, 0xFF646464, 0xFFFFFFFF };
        public:
            Debugger() = default;
            Debugger(sptr<NES>);
            ~Debugger() { Clear(); }

            void Toggle(bool enable) override { enabled = enabled; }
            void StepInstruction() override;
            void StepCycle() override;
            bool IsEnabled() const override { return enabled; }
            void Clear();

            string GetStateCPU() override;
            string GetDisassembly() override;
            string GetStateRAM() override;
            string GetFlags(int) override;

            void ScanInstructions(u16, bool) override;

            vector<u32> GetPaletteColors() override;
            vector<u32> GetPatternTable(int) override;

            // TODO: figure out APU debug stuff
    };
}