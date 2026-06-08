#pragma once

#include "./SharedPCH.h"

struct PaletteData {
    vector<u32> colors = {};
    vector<u8> indices = {};
    u8 colorsPerPalette = 4;
    u8 paletteCount = 8;
    u8 bgPaletteCount = 4;
    u8 spritePaletteOffset = 4;
};

struct EventViewerDisplaySize {
    u32 width = 0;
    u32 height = 0;
};

struct EventViewerCategoryConfig {
    bool visible = true;
    u32 color = 0xFFFFFFFF;
};

struct EventFilter {
    const char* label;
    bool show;
};

namespace DebugEventFlags  {
    enum : u32 {
        NONE = 0,

        // ACCESS FLAGS
        READ = 1 << 0,
        WRITE = 1 << 1,
        EXECUTE = 1 << 2,

        // SUBSYSTEM & DOMAIN FLAGS
        MEMORY = 1 << 3,
        MAPPER = 1 << 4,
        VIDEO = 1 << 5,
        AUDIO = 1 << 6,
        INPUT = 1 << 7,
        DMA = 1 << 8,
        INTERRUPT = 1 << 9,

        // VIEWER & UI STATE FLAGS
        PREVIOUS_FRAME = 1 << 10,
        SYNTHETIC = 1 << 11,
        HIGHLIGHTED = 1 << 12,
        ERROR = 1 << 13,

        HAS_ADDRESS = 1 << 14,
        HAS_VALUE = 1 << 15,
        HAS_DETAILS = 1 << 16,

        ALL = 0xFF'FF'FF'FF
    };
}

struct EventViewerConfig {
    bool autoRefresh = false;
    bool showPreviousFrame = false;
    map<u32, EventFilter> eventFilters = {
        { DebugEventFlags::READ, { "READ", false } },
        { DebugEventFlags::WRITE, { "WRITE", false } },
        { DebugEventFlags::MEMORY, { "MEMORY", false } },
        { DebugEventFlags::MAPPER, { "MAPPER", false } },
        { DebugEventFlags::VIDEO, { "VIDEO", false } },
        { DebugEventFlags::AUDIO, { "AUDIO", false } },
        { DebugEventFlags::INPUT, { "INPUT", false } },
        { DebugEventFlags::DMA, { "DMA", false } },
        { DebugEventFlags::INTERRUPT, { "INTERRUPT", false } },
        { DebugEventFlags::SYNTHETIC, {"SYNTHETIC", false } }
    };
};

static const u32 DebugEvent_ReadVideo =
    DebugEventFlags::READ |
    DebugEventFlags::VIDEO |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_WriteVideo =
    DebugEventFlags::WRITE |
    DebugEventFlags::VIDEO |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_ReadAudio =
    DebugEventFlags::READ |
    DebugEventFlags::AUDIO |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_WriteAudio =
    DebugEventFlags::WRITE |
    DebugEventFlags::AUDIO |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_ReadMapper =
    DebugEventFlags::MAPPER |
    DebugEventFlags::READ |
    DebugEventFlags::MEMORY |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_WriteMapper =
    DebugEventFlags::MAPPER |
    DebugEventFlags::WRITE |
    DebugEventFlags::MEMORY |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_ReadInput =
    DebugEventFlags::READ |
    DebugEventFlags::INPUT |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_WriteInput =
    DebugEventFlags::WRITE |
    DebugEventFlags::INPUT |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_ReadDma =
    DebugEventFlags::DMA |
    DebugEventFlags::READ |
    DebugEventFlags::MEMORY |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_WriteDma =
    DebugEventFlags::DMA |
    DebugEventFlags::WRITE |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_VALUE |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_Interrupt =
    DebugEventFlags::INTERRUPT |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_Breakpoint =
    DebugEventFlags::EXECUTE |
    DebugEventFlags::SYNTHETIC |
    DebugEventFlags::HAS_ADDRESS |
    DebugEventFlags::HAS_DETAILS;
static const u32 DebugEvent_SpriteZeroHit = DebugEventFlags::VIDEO;

struct DebugEventRecord {
    u16 scanline = 0;
    u16 cycle = 0;
    u32 address = 0;
    u32 value = 0;
    u32 color = 0xFFFFFFFF;
    u32 flags = 0;
    string type = "";
    string details = "";


    bool hasAddress() const {
        return (flags & DebugEventFlags::HAS_ADDRESS) != 0;
    }
    bool hasValue() const {
        return (flags & DebugEventFlags::HAS_VALUE) != 0;
    }
    bool hasDetails() const {
        return (flags & DebugEventFlags::HAS_DETAILS) != 0;
    }
    bool isNull() const {
        return flags == DebugEventFlags::NONE;
    }
};

const static DebugEventRecord NullEvent = {
    .flags = DebugEventFlags::NONE
};

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

        virtual const PaletteData GetPaletteData() const = 0;

        /**
         * @brief Get the list of indices for the currently loaded palette colors within PPU VRAM
         * @return A vector list of indices in the range 0-63 (inclusive); one for each color 
         */
        virtual vector<u8> GetPaletteIndices() const = 0;

        /**
         * @brief Get the list of currently loaded palette colors within PPU VRAM
         * @return A vector list containing the currently loaded palette colors as unsigned integers
         */
        virtual vector<u32> GetPaletteColors(const vector<u8>& indices) const = 0;

        /**
         * @brief Get a specified Pattern Table from ROM memory
         * @param id The index of the requested pattern table
         * @return A vector list containing each of the pixels for the requested pattern table
         * @todo should there be another parameter to hold the "pitch" of the returned pattern table?
         */
        virtual vector<u32> GetPatternTable(int, int) = 0;

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

        virtual vector<u32> GetSprites() const = 0;

        /**
         * @brief Get the values of various header-defined portions of the gamepak
         * @return A vector of string arrays in the format `{ <data name>, <data value> }`
         */
        virtual vector<array<string, 2>> GetPakHeader() const = 0;
        /**
         * @brief Get the values of various mapper-defined portions of the gamepak
         * @return a vector of string arrays in the format `{ "<data name>", "<data value>" }`
         */
        virtual vector<array<string, 2>> GetPakMapper() const = 0;

        virtual EventViewerDisplaySize GetEventViewerDisplaySize() const = 0;
        virtual void SetEventViewerConfig(const EventViewerConfig& cfg) = 0;
        virtual const EventViewerConfig& GetEventViewerConfig() const = 0;
        virtual const vector<u32>& GetEventViewerPixels() const = 0;
        virtual const vector<DebugEventRecord>& GetEventViewerEvents() const = 0;
        virtual const DebugEventRecord& GetEventAt(u16 scanline, u16 cycle) const = 0;
        virtual void TakeEventViewerSnapshot(bool forAutoRefresh) = 0;
};

#define IDEBUGGER_CONTRACT_VERSION 1