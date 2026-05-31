#pragma once

#include "SharedPCH.h"

/**
 * @interface IDebugger
 * @brief Debugging interface for console cores
 * @details Provides debugging capabilities for console emulation.
 *          Debuggers are OPTIONAL - not all cores need to implement.
 */
class IDebugger {
    public:
        /// @brief enabled flag for the debugger
        bool enabled = false;

        /// @brief default virtual constructor
        virtual ~IDebugger() = default;

        /// @brief enables the debugger
        void Enable() { enabled = true; }

        /// @brief disabled the debugger
        void Disable() { enabled = false; }

        /// @brief step ROM execution forward by a full instruction
        virtual void StepInstruction() = 0;

        /// @brief step ROM execution forward by a single CPU cycle
        virtual void StepCycle() = 0;

        /// @brief returns the current enabled state of the debugger
        virtual bool IsEnabled() const { return enabled; }

        /**
         * @brief Get the state of the CPU
         * @return a vector of string arrays in the format `{ <register name>, <string/decimal value>, <hex value (if relevant)> }`
         */
        virtual vector<array<string, 3>> GetStateCPU() const = 0;

        /**
         * @brief Get the individual state of each of the CPU status flags
         * @param status A number containing the bit-wise flags of the CPU status
         * @return a string containing a `'1'` for each set flag and a `'0'` for each clear flag; separated by spaces
         */
        virtual string GetFlags(int) const = 0;

        /**
         * @brief Get the state of the RAM
         * @details takes each byte of ram and appends its hex value to one string and its char value to another,
         * then for each set of 16 bytes, composes a string containing `"<ram hex vals> <ram char vals>"` and
         * pushes the final string into a vector for later return.
         * @return A vector containing a composed string for each 16-byte line of RAM
         */
        virtual vector<string> GetStateRAM() const = 0;

        /**
         * @brief Get a disassembly of the surrounding CPU instructions based on the current PC value
         * @return A vector containing a string for each of the surrounding cpu instructions
         * @todo should there be a concrete instruction limit?
         */
        virtual vector<string> GetDisassembly() const = 0;

        /**
         * @brief Acquire a list of the 25 surrounding instruction addresses
         * @note The console CPU is responsible for keeping track of the 12 most recent instructions.
         * Then, we simply add on an entry for the current PC value and then scan through the following
         * addresses for the next few instructions.
         * @param list a reference to an array to hold the list of instruction addresses
         */
        virtual void ScanInstructions(array<u64, 25>& list) const = 0;

        /**
         * @brief Get the state of the PPU
         * @return A vector of string arrays in the format `{ <register address(es)>, <register name>, <string/decimal value>, <hex value (if relevant)> }`
         */
        virtual vector<array<string, 4>> GetStatePPU() const = 0;

        /**
         * @brief Get the list of currently loaded palette colors within PPU VRAM
         * @return A vector list containing the currently loaded palette colors as unsigned integers
         */
        virtual vector<u32> GetPaletteColors() = 0;

        /**
         * @brief Get a specified Pattern Table from ROM memory
         * @param id The index of the requested pattern table
         * @return A vector list containing each of the pixels for the requested pattern table
         * @todo should there be another parameter to hold the "pitch" of the returned pattern table?
         */
        virtual vector<u32> GetPatternTable(int) = 0;

        /**
         * @brief Get a specified Nametable from ROM memory
         * @param id The index of the requested nametable.
         * @return A vector list containing each of the pixels for the requested nametable.
         * @todo should there be another parameter to hold the "pitch" of the returned nametable?
         */
        virtual vector<u32> GetNameTable(int) = 0;

        /**
         * @brief Get the state of the APU
         * @return A vector of string arrays in the format `{ <register address(es)>, <register name>, <string/decimal value>, <hex value (if relevant)> }`
         */
        virtual vector<array<string, 4>> GetStateAPU() const = 0;

        /**
         * @brief Pause program execution on the next frame
         * @todo should we pause at the start of the next frame or the end?
         */
        virtual void BreakOnNextFrame() {}

        /**
         * @brief Pause program execution upon reaching a specified condition
         * @param condition a string representation of the condition to be satisfied
         */
        virtual void BreakOnCondition(const string&) {}

        /**
         * @brief Tell whether we are expecting program execution to be paused at some particular point.
         * @return true of a breakpoint is set
         */
        virtual bool IsBreakpointSet() const { return false; }
};

#define IDEBUGGER_CONTRACT_VERSION 1