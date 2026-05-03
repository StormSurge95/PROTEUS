#pragma once

#include "../BackendPCH.h"

enum class InfoMode {
    CPU, PPU, APU, INPUT
};

class IDebugger {
    public:
        virtual ~IDebugger() = default;
        virtual void Toggle(bool) = 0;
        virtual void StepInstruction() = 0;
        virtual void StepCycle() = 0;
        virtual bool IsEnabled() const = 0;
        virtual string GetStateCPU() = 0;
        virtual string GetDisassembly() = 0;
        virtual string GetStateRAM() = 0;
        virtual void ScanInstructions(u16, bool) = 0;
        virtual vector<u32> GetPaletteColors() = 0;
        virtual vector<u32> GetPatternTable(int) = 0;
};