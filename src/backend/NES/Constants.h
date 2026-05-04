#pragma once

// TODO: COLLECT OTHER NECESSARY CONSTANTS FOR VARIOUS NES SYSTEMS

namespace NES_NS {
    static constexpr u32 CLOCK_RATE_NTSC = 1789773;
    static constexpr u32 CLOCK_RATE_PAL = 1662607;
    static constexpr u32 CLOCK_RATE_DENDY = 1773448;

    static constexpr u32 CYCLES_PER_LINE = 341;

    static constexpr u32 SCREEN_WIDTH = 256;
    static constexpr u32 SCREEN_HEIGHT = 240;
    static constexpr u32 PIXEL_COUNT = SCREEN_WIDTH * SCREEN_HEIGHT;

    static const u8 DUTY_TABLE[4][8] = {
        {0,0,0,0,0,0,0,1},
        {0,0,0,0,0,0,1,1},
        {0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,0,0}
    };

    static const u8 LENGTH_TABLE[32] = {
        10, 254, 20,  2, 40,  4, 80,  6,
        160, 8, 60, 10, 14, 12, 26, 14,
        12, 16, 24, 18, 48, 20, 96, 22,
        192,24, 72, 26, 16, 28, 32, 30
    };

    static const u8 LINEAR_SEQUENCE[32] = {
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };

    static const u16 DMC_RATES[16] = {
        428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
    };

    static u32 GetClockRate(REGION system) {
        switch (system) {
            default:
            case REGION::NTSC: return CLOCK_RATE_NTSC;
            case REGION::PAL: return CLOCK_RATE_PAL;
            case REGION::DENDY: return CLOCK_RATE_DENDY;
        }
    }
}