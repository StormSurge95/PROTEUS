#pragma once

#include "../SHARED/IDebugger.h"
#include "./NES.h"

namespace NES_NS {
    class Debugger : public IDebugger {
        private:
            bool enabled = false;
            sptr<NES> nes = nullptr;

            string DisassembleInstruction(u16 addr) const;

            u32 debugPalette[4] = { 0xFF000000, 0xFF323232, 0xFF646464, 0xFFFFFFFF };
        public:
            Debugger() = default;
            Debugger(sptr<NES>);
            ~Debugger() { Clear(); }

            void Toggle(bool enable) override { enabled = enable; }
            void StepInstruction() override;
            void StepCycle() override;
            bool IsEnabled() const override { return enabled; }
            void Clear();

            string** GetStateCPU(u8& numRegs) const override;
            string** GetStatePPU(u8& numRegs) const override;
            string* GetDisassembly() const override;
            string* GetStateRAM(u64& numLines) const override;
            string GetFlags(int) const override;

            void ScanInstructions(vector<u64>& list) const override;

            vector<u32> GetPaletteColors() override;
            vector<u32> GetPatternTable(int) override;

            // TODO: figure out APU debug stuff
    };
}