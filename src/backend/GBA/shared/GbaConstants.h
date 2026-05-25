#pragma once

namespace NS_GBA {
    static const u32 BIOS_ROM_SIZE = 16384;
    static const u32 WRAM_SIZE = 294912;
    static const u32 VRAM_SIZE = 98304;
    static const u32 OAM_SIZE = 1024;
    static const u32 PALETTE_RAM_SIZE = 1024;
    static const u32 SCREEN_WIDTH = 240;
    static const u32 SCREEN_HEIGHT = 160;
    static const u8 BACKGROUND_LAYERS = 4;
    static const u16 DOTS_PER_LINE = 308;
    static const u16 LINES_PER_FRAME = 228;
    static const u32 SYSTEM_CLOCK = 1677216;
    static const u32 BASE_HEADER_SIZE = 192;
}