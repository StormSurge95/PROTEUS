#pragma once

#include "./NesPCH.h"
#include "./NesTypes.h"
#include "./NesConstants.h"

namespace NS_NES {
    struct ApuFrameSequencerProfile {
        array<u32, 3> quarter4{};
        array<u32, 2> half4{};
        u32 irqSet4a = 0;
        u32 irqSet4b = 0;
        u32 reset4 = 0;

        array<u32, 3> quarter5{};
        array<u32, 2> half5{};
        u32 reset5 = 0;
    };
    
    inline constexpr ApuFrameSequencerProfile NTSC_APU_PROFILE{
        .quarter4 = { 7457, 14913, 22371 },
        .half4 = { 14913, 29829 },
        .irqSet4a = 29828,
        .irqSet4b = 29829,
        .reset4 = 29829,

        .quarter5 = { 7457, 14913, 22371 },
        .half5 = { 14913, 37281 },
        .reset5 = 37281
    };

    inline constexpr ApuFrameSequencerProfile PAL_APU_PROFILE{
        .quarter4 = { 8313, 16626, 24939 },
        .half4 = { 16626, 33252 },
        .irqSet4a = 33252,
        .irqSet4b = 33253,
        .reset4 = 33253,

        .quarter5 = { 8313, 16626, 24939 },
        .half5 = { 16626, 41565 },
        .reset5 = 41565
    };

    struct NesTimingProfile {
        ConsoleRegion region = ConsoleRegion::NTSC;

        const u32 masterClockHz;
        const u32 cpuClockHz;
        const u32 ppuClockHz;

        const u8 cpuDiv;
        const u8 ppuDiv;

        const double millisPerFrame;
        const u16 scanlinesPerFrame;
        const bool oddFrameSkip;

        const double cyclesPerSample;

        const u16* noisePeriods = nullptr;
        const u16* dmcPeriods = nullptr;

        const ApuFrameSequencerProfile& apu;
    };

    inline constexpr NesTimingProfile NTSC_PROFILE{
        .region = ConsoleRegion::NTSC,
        .masterClockHz = 21477272,
        .cpuClockHz = 1789773,
        .ppuClockHz = 5369318,
        .cpuDiv = 12,
        .ppuDiv = 4,
        .millisPerFrame = 1.0 / 60.0988,
        .scanlinesPerFrame = 262,
        .oddFrameSkip = true,
        .cyclesPerSample = 1789773.0 / 44100.0,
        .noisePeriods = NOISE_RATES_NTSC,
        .dmcPeriods = DMC_RATES_NTSC,
        .apu = NTSC_APU_PROFILE
    };

    inline constexpr NesTimingProfile PAL_PROFILE{
        .region = ConsoleRegion::PAL,
        .masterClockHz = 26601712,
        .cpuClockHz = 1662607,
        .ppuClockHz = 5320342,
        .cpuDiv = 16,
        .ppuDiv = 5,
        .millisPerFrame = 1.0 / 50.0070,
        .scanlinesPerFrame = 312,
        .oddFrameSkip = false,
        .cyclesPerSample = 1662607.0 / 44100.0,
        .noisePeriods = NOISE_RATES_PAL,
        .dmcPeriods = DMC_RATES_PAL,
        .apu = PAL_APU_PROFILE
    };

    inline constexpr NesTimingProfile DENDY_PROFILE{
        .region = ConsoleRegion::DENDY,
        .masterClockHz = 26601712,
        .cpuClockHz = 1773448,
        .ppuClockHz = 5320342,
        .cpuDiv = 15,
        .ppuDiv = 5,
        .millisPerFrame = 1.0 / 50.0070,
        .scanlinesPerFrame = 312,
        .oddFrameSkip = false,
        .cyclesPerSample = 1773448.0 / 44100.0,
        .noisePeriods = NOISE_RATES_NTSC,
        .dmcPeriods = DMC_RATES_NTSC,
        .apu = NTSC_APU_PROFILE
    };

    inline constexpr const NesTimingProfile& GetTimingProfile(ConsoleRegion region) {
        switch (region) {
            case ConsoleRegion::PAL:
                return PAL_PROFILE;
            case ConsoleRegion::DENDY:
                return DENDY_PROFILE;
            case ConsoleRegion::MULTI:
                /**
                 * Multi-region ROMs will default to NTSC until user-configurable
                 * runtime options exist in the host and are passed to the core.
                 * TODO: get 'er done
                 */
                return NTSC_PROFILE;
            case ConsoleRegion::NTSC:
            default:
                return NTSC_PROFILE;
        }
    }

    inline constexpr ConsoleRegion ResolveConsoleRegion(ConsoleRegion region) {
        return region == ConsoleRegion::MULTI ? ConsoleRegion::NTSC : region;
    }

    static u32 GetMasterClockRate(ConsoleRegion region) {
        return GetTimingProfile(region).masterClockHz;
    }

    static u32 GetCpuClockRate(ConsoleRegion region) {
        return GetTimingProfile(region).cpuClockHz;
    }

    static u32 GetPpuClockRate(ConsoleRegion region) {
        return GetTimingProfile(region).ppuClockHz;
    }

    static u32 GetCpuClockDiv(ConsoleRegion region) {
        return GetTimingProfile(region).cpuDiv;
    }

    static u32 GetPpuClockDiv(ConsoleRegion region) {
        return GetTimingProfile(region).ppuDiv;
    }

    static u32 GetDotsPerScanline() {
        return 341;
    }

    static u32 GetScanlinesPerFrame(ConsoleRegion region) {
        return GetTimingProfile(region).scanlinesPerFrame;
    }

    static double GetMillisPerFrame(ConsoleRegion region) {
        return GetTimingProfile(region).millisPerFrame;
    }

    static bool AllowOddFrameSkip(ConsoleRegion region) {
        return GetTimingProfile(region).oddFrameSkip;
    }

    static double GetAudioRate(ConsoleRegion region) {
        return GetTimingProfile(region).cyclesPerSample;
    }

    static u16 GetNoiseRate(ConsoleRegion region, u8 index) {
        return GetTimingProfile(region).noisePeriods[index];
    }

    static u16 GetDmcRate(ConsoleRegion region, u8 index) {
        return GetTimingProfile(region).dmcPeriods[index];
    }
    
    inline constexpr const ApuFrameSequencerProfile& GetApuProfile(ConsoleRegion region) {
        return GetTimingProfile(region).apu;
    }

    static bool IsQuarterFrame(ConsoleRegion region, u32 cycle, bool fourStep) {
        const ApuFrameSequencerProfile& apu = GetApuProfile(region);
        if (fourStep) {
            for (const u32& c : apu.quarter4) {
                if (c == cycle) return true;
            }
        } else {
            for (const u32& c : apu.quarter5) {
                if (c == cycle) return true;
            }
        }
        return false;
    }

    static bool IsHalfFrame(ConsoleRegion region, u32 cycle, bool fourStep) {
        const ApuFrameSequencerProfile& apu = GetApuProfile(region);
        if (fourStep) {
            for (const u32& c : apu.half4) {
                if (c == cycle) return true;
            }
        } else {
            for (const u32& c : apu.half5) {
                if (c == cycle) return true;
            }
        }
        return false;
    }

    static bool IsResetFrame(ConsoleRegion region, u32 cycle, bool fourStep) {
        if (fourStep)
            return cycle == GetTimingProfile(region).apu.reset4;
        else
            return cycle == GetTimingProfile(region).apu.reset5;
    }

    static u32 GetIrqSetA(ConsoleRegion region) {
        return GetTimingProfile(region).apu.irqSet4a;
    }

    static u32 GetIrqSetB(ConsoleRegion region) {
        return GetTimingProfile(region).apu.irqSet4b;
    }
}