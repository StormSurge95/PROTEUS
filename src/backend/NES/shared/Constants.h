#pragma once

// TODO: COLLECT OTHER NECESSARY CONSTANTS FOR VARIOUS NES SYSTEMS

namespace NES_NS {
    /// @brief CPU clock rate for North American/Japanese NES consoles
    static constexpr u32 CLOCK_RATE_NTSC = 1789773;
    /// @brief CPU clock rate for European NES consoles
    static constexpr u32 CLOCK_RATE_PAL = 1662607;
    /// @brief CPU clock rate for Russian bootleg NES consoles
    static constexpr u32 CLOCK_RATE_DENDY = 1773448;

    /// @brief Total PPU cycles per scanline
    static constexpr u32 CYCLES_PER_LINE = 341;

    /// @brief NTSC Screen Width
    static constexpr u32 SCREEN_WIDTH = 256;
    /// @brief NTSC Screen Height
    static constexpr u32 SCREEN_HEIGHT = 240;
    /// @brief NTSC Total pixels per frame.
    static constexpr u32 PIXEL_COUNT = SCREEN_WIDTH * SCREEN_HEIGHT;

    /// @brief Pulse Channel Duty Table
    static const u8 DUTY_TABLE[4][8] = {
        {0,0,0,0,0,0,0,1},
        {0,0,0,0,0,0,1,1},
        {0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,0,0}
    };

    /// @brief APU Length Counter lookup table
    static const u8 LENGTH_TABLE[32] = {
         10, 254, 20,  2, 40,  4, 80,  6,
        160,   8, 60, 10, 14, 12, 26, 14,
         12,  16, 24, 18, 48, 20, 96, 22,
        192,  24, 72, 26, 16, 28, 32, 30
    };

    /// @brief Triangle Channel Linear Sequence table
    static const u8 LINEAR_SEQUENCE[32] = {
        15, 14, 13, 12, 11, 10,  9,  8,
         7,  6,  5,  4,  3,  2,  1,  0,
         0,  1,  2,  3,  4,  5,  6,  7,
         8,  9, 10, 11, 12, 13, 14, 15
    };

    static const u16 NOISE_RATES_NTSC[16] = {
        4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
    };
    static const u16 NOISE_RATES_PAL[16] = {
        4, 8, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708,  944, 1890, 3778
    };

    static u16 GetRateNoise(REGION system, u8 index) {
        switch (system) {
            default:
            case REGION::NTSC:
                return NOISE_RATES_NTSC[index];
            case REGION::PAL:
                return NOISE_RATES_PAL[index];
        }
    }

    /// @brief DMC Channel Rate lookup table
    static const u16 DMC_RATES_NTSC[16] = {
        428, 380, 340, 320,
        286, 254, 226, 214,
        190, 160, 142, 128,
        106,  84,  72,  54
    };
    static const u16 DMC_RATES_PAL[16] = {
        398, 354, 316, 298,
        276, 236, 210, 198,
        176, 148, 132, 118,
         98,  78,  66,  50
    };

    static u16 GetRateDMC(REGION system, u8 index) {
        switch (system) {
            default:
            case REGION::NTSC:
                return DMC_RATES_NTSC[index];
            case REGION::PAL:
                return DMC_RATES_PAL[index];
        }
    }

    /**
     * @brief Helper function to get CPU clock rate based on console region.
     * @param system The region to get the clock rate for.
     * @return The region-specific clock rate
     */
    static u32 GetClockRate(REGION system) {
        switch (system) {
            default:
            case REGION::NTSC: return CLOCK_RATE_NTSC;
            case REGION::PAL: return CLOCK_RATE_PAL;
            case REGION::DENDY: return CLOCK_RATE_DENDY;
        }
    }

    /**
     * @brief Helper function to get audio sample rate for emulation based on console region.
     * @param system The region to get the audio sample rate for.
     * @return The region-specific audio sample rate.
     */
    static double GetAudioRate(REGION system) {
        return GetClockRate(system) / 44100.0;
    }
}