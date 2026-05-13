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

        virtual string** GetStateCPU(u8& numRegs) const = 0;
        /**
        * @brief Outputs the current status flags of the processor.
        * @param status The number value holding all of the flags to be checked.
        * @return A string representation of each flag using its single-letter
        *      mnemonic (capital for `set`, lowercase for `clear`)
        */
        virtual string GetFlags(int) const = 0;
        virtual string* GetStateRAM(u64& numLines) const = 0;
        virtual string* GetDisassembly() const = 0;
        virtual void ScanInstructions(vector<u64>& list) const = 0;

        virtual string** GetStatePPU(u8& numRegs) const = 0;
        virtual vector<u32> GetPaletteColors() = 0;
        virtual vector<u32> GetPatternTable(int) = 0;
        virtual vector<u32> GetNameTables(int) = 0;

        virtual string** GetStateAPU(u8& numRegs) const = 0;
        virtual vector<u32> GetPulse1() = 0;
        virtual vector<u32> GetPulse2() = 0;
        virtual vector<u32> GetTriangle() = 0;
        virtual vector<u32> GetNoise() = 0;
        virtual vector<u32> GetDMC() = 0;
};