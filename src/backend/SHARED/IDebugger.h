#pragma once

#include "../BackendPCH.h"

/**
 * @interface IDebugger IDebugger.h "backend/SHARED/IDebugger.h"
 * @brief "Manager" interface class implemented by each of the various console debuggers.
 * @details
 * Includes various functions required for interaction with the frontend.
 */
class IDebugger {
    public:
        /// @brief enabled flag for the debugger
        bool enabled = false;

        /// @brief default virtual constructor
        virtual ~IDebugger() = default;
        /// @brief toggles the enabled state of the debugger
        void Toggle(bool set) { enabled = set; }
        /// @brief required for debuggers to step forward by a single cpu instruction
        virtual void StepInstruction() = 0;
        /// @brief required for debuggers to step forward by a single cpu cycle
        virtual void StepCycle() = 0;
        /// @brief returns the current enabled state of the debugger
        virtual bool IsEnabled() const { return enabled; }
        /// @brief required to retrieve the current state of the console CPU
        virtual string** GetStateCPU(u8& numRegs) const = 0;
        /// @brief required to retrieve the current state of the console CPU Status register/flags
        virtual string GetFlags(int) const = 0;
        /// @brief required to retrieve the current state fo the console CPU RAM
        virtual string* GetStateRAM(u64& numLines) const = 0;
        /// @brief required to retrieve the current instruction disassembly
        virtual string* GetDisassembly() const = 0;
        /// @brief required to retrieve the list of instruction addresses to use for disassembly
        virtual void ScanInstructions(array<u64, 25>& list) const = 0;
        /// @brief required to retrieve the current state of the console PPU
        virtual string** GetStatePPU(u8& numRegs) const = 0;
        /// @brief required to retrieve the current set of palette colors in use
        virtual vector<u32> GetPaletteColors() = 0;
        /// @brief required to retreive the set of pattern table tiles for debug rendering
        virtual vector<u32> GetPatternTable(int) = 0;
        /// @brief required to retrieve the currently constructed nametables for debug rendering
        virtual vector<u32> GetNameTables(int) = 0;
        /// @brief required to retrieve the current state of the console APU
        virtual string** GetStateAPU(u8& numRegs) const = 0;
};