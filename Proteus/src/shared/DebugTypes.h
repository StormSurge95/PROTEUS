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
#pragma region EVENT
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
#pragma endregion
#pragma region TRACE
enum class DebugTraceMode : u32 {
    NONE            = 0,
    INSTRUCTIONS    = 1 << 0,
    EVENTS          = 1 << 1,
    ALL             = INSTRUCTIONS | EVENTS
};
inline bool HasTraceMode(DebugTraceMode value, DebugTraceMode flag) {
    return (static_cast<u32>(value) & static_cast<u32>(flag)) != 0;
}
enum class TraceKind : u8 {
    Instruction,
    Event
};
struct TraceRecord {
    TraceKind kind = TraceKind::Instruction;

    u64 frame = 0;
    u64 cpuCycle = 0;
    u16 scanline = 0;
    u16 dot = 0;

    u16 pc = 0;
    u16 address = 0;
    u8 opcode = 0;
    u8 value = 0;
    u8 a = 0;
    u8 x = 0;
    u8 y = 0;
    u8 sp = 0;
    u8 status = 0;

    u32 flags = 0;
    u16 detailID = 0;
};
struct DebugTraceConfig {
    bool enabled = false;
    bool append = false;
    bool flushEveryFrame = true;
    bool traceToFile = false;
    bool traceToConsole = false;
    size_t flushThresholdRecords = 8192;
    DebugTraceMode mode = DebugTraceMode::INSTRUCTIONS;
    path filePath = {};
};
#pragma endregion