#pragma once

#include "../../../Proteus/src/shared/CorePCH.h"

namespace NS_GBA {
    static const u32 BIOS_ROM_SIZE = 16'384;
    static const u32 WRAM_SIZE = 294'912;
    static const u32 VRAM_SIZE = 98'304;
    static const u32 OAM_SIZE = 1'024;
    static const u32 PALETTE_RAM_SIZE = 1'024;
    static const u32 GBA_SCREEN_WIDTH = 240;
    static const u32 GBA_SCREEN_HEIGHT = 160;
    static const u8 BACKGROUND_LAYERS = 4;
    static const u16 DOTS_PER_LINE = 308;
    static const u16 LINES_PER_FRAME = 228;
    static const u32 BASE_HEADER_SIZE = 192;

    // timebase
    static const u32 SYSTEM_CLOCK = 16'777'216; // why u64?
    static const u32 GBA_TICKS_PER_DOT = 4;
    static const u32 GBA_TICKS_PER_LINE = static_cast<u64>(DOTS_PER_LINE) * GBA_TICKS_PER_DOT; // 1232 ticks
    static const u32 GBA_TICKS_PER_FRAME = GBA_TICKS_PER_LINE * LINES_PER_FRAME; // 280,896 ticks
    static const u32 LEGACY_NORMAL_TICKS_PER_CPU_CLOCK = 4;
    static const u32 LEGACY_DOUBLE_TICKS_PER_CPU_CLOCK = 2;
    static const u32 LEGACY_TICKS_PER_PPU_DOT = 4;
    static const double GBA_FPS = static_cast<double>(SYSTEM_CLOCK) / static_cast<double>(GBA_TICKS_PER_FRAME); // 
    static const double MILLIS_PER_FRAME = 1000.0 / GBA_FPS;
}