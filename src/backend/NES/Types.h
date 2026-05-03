#pragma once

namespace NES_NS {
    class NES;
    class Gamepak;
    class Controller;
    class BUS;
    class CPU;
    class PPU;
    class APU;
    class Debugger;
    class Mapper;

    enum class MIRROR {
        HARDWARE,
        HORIZONTAL,
        VERTICAL,
        FOUR_SCREEN,
        ONE_SCREEN_LO,
        ONE_SCREEN_HI,
        DIAGONAL,
        L_SHAPED,
        THREE_SCREEN_VERTICAL,
        THREE_SCREEN_HORIZONTAL,
        ONE_SCREEN_FIXED
    };

    struct Header {
        char name[4] = { 0 };
        u8 prgChunks = 0;
        u8 chrChunks = 0;
        u8 flags6 = 0;
        u8 flags7 = 0;
        u8 prgRamSize = 0;
        u8 tvSystem1 = 0;
        u8 tvSystem2 = 0;
        char padding[5] = { 0 };
    };

    enum class BUTTONS {
        BUTTON_A,
        BUTTON_B,
        BUTTON_SELECT,
        BUTTON_START,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_RIGHT
    };

    struct ADDR {
        u8 lo = 0x00;
        u8 hi = 0x00;

        ADDR() = default;
        ADDR(u8 l, u8 h) : lo(l), hi(h) {}
        ADDR(u16 a) {
            lo = a & 0xFF;
            hi = ((a >> 8) & 0xFF);
        }
        ADDR(const ADDR& other) {
            if (this != &other) {
                lo = other.lo;
                hi = other.hi;
            }
        }

        ADDR& operator=(u16 val) {
            lo = val & 0xFF;
            hi = ((val >> 8) & 0xFF);
            return *this;
        }
        ADDR& operator++() { // prefix
            if (lo == 0xFF) {
                lo = 0x00;
                if (hi == 0xFF) {
                    hi = 0x00;
                } else hi++;
            } else lo++;
            return *this;
        }
        ADDR operator++(int) { // postfix
            ADDR temp = *this;
            ++(*this);
            return temp;
        }
        ADDR& operator--() {
            if (lo == 0x00) {
                lo = 0xFF;
                if (hi == 0x00) {
                    hi = 0xFF;
                } else hi--;
            } else lo--;
            return *this;
        }
        ADDR operator--(int) {
            ADDR temp = *this;
            --(*this);
            return temp;
        }
        ADDR& operator+(u16 val) {
            add(val);
            return *this;
        }
        ADDR& operator+=(u16 val) {
            add(val);
            return *this;
        }
        bool operator==(const ADDR& other) {
            return value() == other.value();
        }
        bool operator==(const u16& val) {
            return value() == val;
        }
        bool operator!=(const ADDR& other) {
            return value() != other.value();
        }
        bool operator!=(const u16& val) {
            return value() != val;
        }
        bool operator>(const ADDR& other) {
            return value() > other.value();
        }
        bool operator>(const u16& val) {
            return value() > val;
        }
        bool operator<(const ADDR& other) {
            return value() < other.value();
        }
        bool operator<(const u16& val) {
            return value() < val;
        }
        bool operator>=(const ADDR& other) {
            return value() >= other.value();
        }
        bool operator>=(const u16& val) {
            return value() >= val;
        }
        bool operator<=(const ADDR& other) {
            return value() <= other.value();
        }
        bool operator<=(const u16& val) {
            return value() <= val;
        }
        operator u16() {
            return value();
        }

        void add(u16 val) {
            u8 l = val & 0xFF;
            u8 h = (val >> 8) & 0xFF;
            if (0xFF - lo < l)
                hi++;
            lo += l;
            hi += h;
        }
        void add_s(u8 val) {
            if (((val >> 7) & 0x01) == 0) { // positive
                if (val > (0xFF - lo)) hi++;
                lo += val;
            } else { // negative
                val = ~val;
                val++;
                if (val > lo) hi--;
                lo -= val;
            }
        }
        void sub(u16 val) {
            u8 l = val & 0xFF;
            u8 h = (val >> 8) & 0xFF;
        }

        u16 value() const {
            return (((u16)hi << 8) | lo);
        }
    };

    enum class FLAGS {
        C = 0x01,
        Z = 0x02,
        I = 0x04,
        D = 0x08,
        B = 0x10,
        U = 0x20,
        V = 0x40,
        N = 0x80
    };

    struct INST {
        string name;
        u8 bytes = 0;

        void (CPU::* address)(void) = nullptr;
        void (CPU::* operate)(void) = nullptr;
    };

    struct SpriteUnit {
        u8 y = 0x00;
        u8 tile = 0x00;
        u8 attr = 0x00;
        u8 x = 0x00;

        u8 patternLo = 0x00;
        u8 patternHi = 0x00;

        u8 xCounter = 0x00;
    };

    struct LengthCounter {
        bool halt = false;
        u8 counter = 0x00;
        u8 reloadVal = 0x00;
    };

    struct VolumeEnvelope {
        bool start = false;
        bool loop = false;
        bool constVol = false;
        u8 divider;
        u8 counter;
        u8 volume;
    };

    enum class SYSTEM_TYPE {
        NTSC,
        PAL,
        FAMICOM,
        DENDY,
        VS_SYSTEM,
        PLAYCHOICE,
        FDS,
        FNS,
        UNKNOWN
    };

    struct HASH_INFO {
        u32 CRC = 0;
        u32 PRG_CRC = 0;
        u32 PRG_CHR_CRC = 0;
    };

    enum class VS_SYSTEM_TYPE {
        DEFAULT,
        RBI_BASEBALL_PROTECTION,
        TKO_BOXING_PROTECTION,
        SUPER_XEVIOUS_PROTECTION,
        ICE_CLIMBER_PROTECTION,
        VS_DUAL_SYSTEM,
        RAID_ON_BUNGELING_BAY_PROTECTION
    };

    enum class INPUT_TYPE {
        UNSPECIFIED,
        STANDARD_CONTROLLERS,
        FOUR_SCORE,
        FOUR_PLAYER_ADAPTER,
        VS_SYSTEM,
        VS_SYSTEM_SWAPPED,
        VS_SYSTEM_SWAP_AB,
        VS_ZAPPER,
        ZAPPER,
        TWO_ZAPPERS,
        BANDAI_HYPERSHOT,
        POWER_PAD_SIDE_A,
        POWER_PAD_SIDE_B,
        FAMILY_TRAINER_SIDE_A,
        FAMILY_TRAINER_SIDE_B,
        ARKANOID_CONTROLLER_NES,
        ARKANOID_CONTROLLER_FAMICOM,
        DOUBLE_ARKANOID_CONTROLLER,
        KONAMI_HYPERSHOT,
        PACHINKO_CONTROLLER,
        EXCITING_BOXING,
        JISSEN_MAHJONG,
        PARTY_TAP,
        OEKA_KIDS_TABLET,
        BARCODE_BATTLER,
        MIRACLE_PIANO,
        POKKUN_MOGURAA,
        TOP_RIDER,
        DOUBLE_FISTED,
        FAMICOM_3D_SYSTEM,
        DOREMIKKO_KEYBOARD,
        ROB,
        FAMICOM_DATA_RECORDER,
        TURBO_FILE,
        BATTLE_BOX,
        FAMILY_BASIC_KEYBOARD,
        PEC_586_KEYBOARD,
        BIT_79_KEYBOARD,
        SUBOR_KEYBOARD,
        SUBOR_KEYBOARD_MOUSE1,
        SUBOR_KEYBOARD_MOUSE2,
        SNES_MOUSE,
        GENERIC_MULTICART,
        SNES_CONTROLLERS,
        RACERMATE_BICYCLE,
        UFORCE,
        LAST_ENTRY
    };

    enum class PPU_MODEL {
        PPU2C02,
        PPU2C03,
        PPU2C04A,
        PPU2C04B,
        PPU2C04C,
        PPU2C04D,
        PPU2C05A,
        PPU2C05B,
        PPU2C05C,
        PPU2C05D,
        PPU2C05E
    };

    enum class AUDIO_CHANNEL {
        SQUARE1,
        SQUARE2,
        TRIANGLE,
        NOISE,
        DMC,
        FDS,
        MMC5,
        VRC6,
        VRC7,
        NAMCO_163,
        SUNSOFT_5B
    };
}