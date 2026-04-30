#pragma once

#include <cstdint>
#include <vector>

enum class InfoMode {
    CPU, PPU, APU, INPUT
};

class IDebugger {
    public:
        virtual ~IDebugger() = default;
        virtual void Toggle(bool enabled) = 0;
        virtual void StepInstruction() = 0;
        virtual void StepCycle() = 0;
        virtual bool IsEnabled() const = 0;
        virtual std::string GetStateCPU() = 0;
        virtual std::string GetDisassembly() = 0;
        virtual std::string GetStateRAM() = 0;
        virtual void ScanInstructions(uint16_t, bool) = 0;
        virtual std::vector<uint32_t> GetPaletteColors() = 0;
        virtual std::vector<uint32_t> GetPatternTable(int) = 0;
};