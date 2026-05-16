#pragma once

/**
 * @brief Namespace containing the various classes/enums/structs/etc related to the NES
 */
namespace NES_NS {
    /**
     * @brief The "master" class of the NES console.
     */
    class NES;

    /**
     * @brief The game cartridge (ROM) to be played.
     */
    class Gamepak;

    /**
     * @brief The input method to be emulated.
     */
    class Controller;

    /**
     * @brief The "brain" of the NES console.
     */
    class CPU;

    /**
     * @brief The "painter" of the NES console.
     */
    class PPU;

    /**
     * @brief The "orchestrator" of the NES console's sound.
     * @note Relies heavily on the various channel classes.
     */
    class APU;

    /**
     * @brief The Square Wave channel of the NES console.
     */
    class PulseChannel;

    /**
     * @brief The Triangle Wave channel of the NES console.
     */
    class TriangleChannel;

    /**
     * @brief The Digital Noise channel of the NES console. 
     */
    class NoiseChannel;

    /**
     * @brief The Delta Modulation channel of the NES console.
     */
    class DMC_Channel;

    /**
     * @brief A helper class for implementing Debug Mode within the NES emulator. 
     */
    class Debugger;

    /**
     * @brief Base NES Mapper class.
     */
    class Mapper;

    /**
     * @brief Enumeration of the various Mirroring arrangements within NES ROMs
     */
    enum class MIRROR {
        HARDWARE,
        HORIZONTAL,
        VERTICAL,
        FOUR_SCREEN,
        ONE_SCREEN_LO,
        ONE_SCREEN_HI
    };

    /**
     * @brief The header structure for NES ROM image files.
     * 
     * @details
     * This structure can be in one of several formats; however,
     * the only formats fully supported within this emulator are
     * iNES and NES 2.0 (iNES 0.7/archaic iNES support is accidental)
     */
    struct Header {
        /**
         * @brief The first four bytes (00-03) of the NES ROM header are
         * meant to be an identification string composed of "NES<EOF>"
         */
        char name[4] = { 0 };
        /**
         * @brief Byte 04 represents the LSB of the PRG-ROM size value.
         */
        u8 byte4 = 0;
        /**
         * @brief Byte 05 represents the LSB of the CHR-ROM size value.
         */
        u8 byte5 = 0;
        /**
         * @brief Byte 06 contains various flags related to the ROM.
         * 
         * @details
         * NNNN FTBM
         * 
         * - M: Hard-wired nametable layout (0: vertical; 1: horizontal)
         * 
         * - B: "Battery" and other non-volatile memory (0: not present; 1: present)
         * 
         * - T: 512-byte Trainer (0: not present; 1: present)
         * 
         * - F: Alternative nametables (0: no; 1: yes)
         * 
         * - N: Mapper Number (D3..D0)
         */
        u8 byte6 = 0;
        /**
         * @brief Byte 07 contains various flags retlated to the ROM.
         * @details
         * NNNN IITT
         * 
         * - T: Console Type (00: NES/Famicom; 01: Vs. System; 10: Playchoice 10; 11: Extended Console)
         * 
         * - I: NES header file structure identifier (00: iNES; 01: archaic iNES; 10: NES 2.0; 11: iNES 0.7 or before)
         * 
         * - N: Mapper Number (D7..D4)
         * @note For I = 00 (iNES): bytes 12-15 shoudl all be 0
         * @note For I = 10 (NES 2.0): total size calculation should not exceed size of ROM file
         */
        u8 byte7 = 0;
        /**
         * @brief Byte 08 contains the MSB bits of the mapper number and the submapper number.
         * @details
         * SSSS NNNN
         * 
         * - N: Mapper Number (D11..D8)
         * 
         * - S: Submapper Number
         */
        u8 byte8 = 0;
        /**
         * @brief Byte 09 contains the MSB of both the PRG-ROM and CHR-ROM size values.
         * @details
         * CCCC PPPP
         * 
         * - P: PRG-ROM size MSB
         * 
         * - C: CHR-ROM size MSB
         */
        u8 byte9 = 0;
        /**
         * @brief Byte 10 contains the PRG-RAM/EEPROM size values.
         * 
         * @details
         * pppp PPPP
         * 
         * - P: PRG-RAM (volatile) shift count
         * 
         * - p: PRG-NVRAM/EEPROM (non-volatile) shift count.
         * 
         * @note
         * A shift count of zero is read as no RAM present.
         * 
         * A non-zero shift count is read as a number of 64B chunks
         * 
         * i.e. A shift count of 7 means 8192 bytes of PRG-(NV)RAM.
         */
        u8 byteA = 0;
        /**
         * @brief Byte 11 contains the CHR-RAM size values.
         * 
         * @details
         * cccc CCCC
         * 
         * - C: CHR-RAM (volatile) shift count
         * 
         * - c: CHR-NVRAM (non-volatile) shift count
         * 
         * @note
         * A shift count of zero is read as no RAM present.
         * 
         * A non-zero shift count is read as a number of 64B chunks
         * 
         * i.e. A shift count of 7 means 8192 bytes of CHR-(NV)RAM.
         */
        u8 byteB = 0;
        /**
         * @brief Byte 12 represents the CPU/PPU timing (i.e. the console region)
         * 
         * @details
         * ---- --VV
         * 
         * - V: CPU/PPU timing mode (00: RP2C02 [NTSC]; 01: RP2C07 [PAL]; 10: Multi-region; 11: UA6538 [Dendy])
         */
        u8 byteC = 0;
        /**
         * @brief Byte 13 represents the type of NES system based on the value of T in byte 07.
         * @see byte7
         * 
         * @details
         * If T = 1: Vs. System Type
         * 
         * MMMM PPPP
         * 
         * P: Vs. PPU Type ID
         * 
         * M: Vs. Hardware Type ID
         * 
         * If T = 3: Extended Console Type
         * 
         * ---- CCCC
         * 
         * C: Extended Console Type ID
         */
        u8 byteD = 0;
        /**
         * @brief Byte 14 represents the number of miscellaneous ROMs present
         * 
         * @details
         * ---- --RR
         * 
         * R: Number of miscellaneous ROMs present.
         */
        u8 byteE = 0;
        /**
         * @brief Byte 15 represents the default expansion device needed for this ROM
         * 
         * @details
         * -DDD DDDD
         * 
         * D: Default Expansion Device ID
         */
        u8 byteF = 0;
    };

    /**
     * @brief Enumeration of the various NES Controller buttons
     */
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

    /**
     * @brief Helper structure for modifying and processing 16 bit values
     * and the individual bytes they are composed of.
     */
    struct ADDR {
        /**
         * @brief The "low" byte of the value.
         */
        u8 lo = 0x00;
        /**
         * @brief The "high" byte of the value.
         */
        u8 hi = 0x00;

        /**
         * @brief Default constructor uses default arguments; setting the value to `0`
         */
        ADDR() = default;
        /**
         * @brief Byte-wise constructor sets each byte individually
         * @param l The "low" byte
         * @param h The "high" byte
         */
        ADDR(u8 l, u8 h) : lo(l), hi(h) {}
        /**
         * @brief Constructor to set individual bytes based on a full 16-bit value
         * @param a The 16-bit value to "separate" into individual bytes
         */
        ADDR(u16 a) {
            lo = a & 0xFF;
            hi = ((a >> 8) & 0xFF);
        }
        /**
         * @brief Copy constructor for ease.
         * @param other 
         */
        ADDR(const ADDR& other) {
            if (this != &other) {
                lo = other.lo;
                hi = other.hi;
            }
        }

        /**
         * @brief Assignment operator using provided 16-bit value.
         * @param val The 16-bit value to use for changing this ADDR.
         * @return Reference to this ADDR; for chaining.
         */
        ADDR& operator=(u16 val) {
            lo = val & 0xFF;
            hi = ((val >> 8) & 0xFF);
            return *this;
        }
        /**
         * @brief Pre-Increment operator; properly accounts for overflow.
         * @return Reference to this ADDR; for chaining.
         */
        ADDR& operator++() { // prefix
            add(1);
            return *this;
        }
        /**
         * @brief Post-Increment operator; properly accounts for overflow.
         * @return The value of this ADDR before incrementing.
         */
        ADDR operator++(int) { // postfix
            ADDR temp = *this;
            add(1);
            return temp;
        }
        /**
         * @brief Pre-Decrement operator; properly accounts for underflow.
         * @return Reference to this ADDR; for chaining.
         */
        ADDR& operator--() {
            if (lo == 0x00) {
                lo = 0xFF;
                if (hi == 0x00) {
                    hi = 0xFF;
                } else hi--;
            } else lo--;
            return *this;
        }
        /**
         * @brief Post-Decrement operator; properly accounts for underflow.
         * @return The value of this ADDR before decrementing.
         */
        ADDR operator--(int) {
            ADDR temp = *this;
            --(*this);
            return temp;
        }
        /**
         * @brief Addition Operator; properly accounts for overflow
         * @param val The 16-bit value to add
         * @return Reference to this ADDR; for chaining
         */
        ADDR& operator+(u16 val) {
            add(val);
            return *this;
        }
        /**
         * @brief Addition-Assignment operator; properly accounts for overflow
         * @param val The 16-bit value to add
         * @return Reference to this ADDR; for chaining
         */
        ADDR& operator+=(u16 val) {
            add(val);
            return *this;
        }
        /**
         * @brief EQ operator
         * @param other The other ADDR to use for comparison.
         * @return THIS == OTHER
         */
        bool operator==(const ADDR& other) {
            return value() == other.value();
        }
        /**
         * @brief EQ operator
         * @param val The 16-bit value to use for comparison.
         * @return THIS == VAL
         */
        bool operator==(const u16& val) {
            return value() == val;
        }
        /**
         * @brief NE operator
         * @param other The other ADDR to use for comparison.
         * @return THIS != OTHER
         */
        bool operator!=(const ADDR& other) {
            return value() != other.value();
        }
        /**
         * @brief NE operator
         * @param val The 16-bit value to use for comparison.
         * @return THIS != VAL
         */
        bool operator!=(const u16& val) {
            return value() != val;
        }
        /**
         * @brief GT operator
         * @param other The other ADDR to use for comparison.
         * @return THIS > OTHER
         */
        bool operator>(const ADDR& other) {
            return value() > other.value();
        }
        /**
         * @brief GT operator
         * @param val The 16-bit value to use for comparison.
         * @return THIS > VAL
         */
        bool operator>(const u16& val) {
            return value() > val;
        }
        /**
         * @brief LT operator
         * @param other The other ADDR to use for comparison.
         * @return THIS < OTHER
         */
        bool operator<(const ADDR& other) {
            return value() < other.value();
        }
        /**
         * @brief LT operator
         * @param val The 16-bit value to use for comparison.
         * @return THIS < VAL
         */
        bool operator<(const u16& val) {
            return value() < val;
        }
        /**
         * @brief GTE operator
         * @param other The other ADDR to use for comparison.
         * @return THIS >= OTHER
         */
        bool operator>=(const ADDR& other) {
            return value() >= other.value();
        }
        /**
         * @brief GTE operator
         * @param val The 16-bit value to use for comparison.
         * @return THIS >= VAL
         */
        bool operator>=(const u16& val) {
            return value() >= val;
        }
        /**
         * @brief LTE operator
         * @param other The other ADDR to use for comparison.
         * @return THIS <= OTHER
         */
        bool operator<=(const ADDR& other) {
            return value() <= other.value();
        }
        /**
         * @brief LTE operator
         * @param val The 16-bit value to use for comparison.
         * @return THIS <= VAL
         */
        bool operator<=(const u16& val) {
            return value() <= val;
        }
        /**
         * @brief Implicit casting operator
         */
        operator u16() const {
            return value();
        }

        /**
         * @brief Helper function for performing addition.
         * @note val is assumed to be unsigned.
         * @param val The value to add to THIS.
         */
        void add(u16 val) {
            u8 l = val & 0xFF;
            u8 h = (val >> 8) & 0xFF;
            if (0xFF - lo < l)
                hi++;
            lo += l;
            hi += h;
        }
        /**
         * @brief Helper function for performing subtraction.
         * @note val is assumed to be unsigned.
         * @param val The value to subtract from THIS.
         */
        void sub(u16 val) {
            u8 l = val & 0xFF;
            u8 h = (val >> 8) & 0xFF;
            if (lo < l)
                hi--;
            lo -= l;
            hi -= h;
        }
        /**
         * @brief Helper function for performing addition with a signed value.
         * @param val The value to add to (or subtract from) THIS.
         */
        void add_s(u8 val) {
            if (((val >> 7) & 0x01) == 0) { // positive
                add(val);
            } else { // negative
                val = ~val;
                val++;
                sub(val);
            }
        }
        /**
         * @brief Helper function for converting ADDR to an unsigned short.
         * @return The 16-bit value represented by THIS.
         */
        u16 value() const {
            return (((u16)hi << 8) | lo);
        }
    };

    /**
     * @brief Enumerator of the various status flags of the NES CPU
     */
    enum class FLAGS : unsigned char {
        C = 0x01,   /// Carry Flag
        Z = 0x02,   /// Zero Flag
        I = 0x04,   /// (inhibit) Interrupt Flag
        D = 0x08,   /// Decimal Flag
        B = 0x10,   /// (software) Break Flag
        U = 0x20,   /// Unused Flag
        V = 0x40,   /// Overflow Flag
        N = 0x80    /// Negative Flag
    };

    /**
     * @brief Helper structure for CPU Instructions
     */
    struct INST {
        /**
         * @brief The name of the instruction.
         */
        string name;
        /**
         * @brief The byte-length of the instruction.
         */
        u8 bytes = 0;

        /**
         * @brief Function pointer to the addressing implementation of the instruction.
         */
        void (CPU::* address)(void) = nullptr;
        /**
         * @brief Function pointer to the operation implementation of the instruction.
         */
        void (CPU::* operate)(void) = nullptr;
    };

    /**
     * @brief Helper structure for processing sprite data within the NES PPU.
     */
    struct SpriteUnit {
        /**
         * @brief The Y-Coordinate of the sprite.
         */
        u8 y = 0x00;
        /**
         * @brief The tile index of the sprite.
         */
        u8 tile = 0x00;
        /**
         * @brief The attribute flags of the sprite.
         */
        u8 attr = 0x00;
        /**
         * @brief The X-Coordinate of the sprite.
         */
        u8 x = 0x00;

        /**
         * @brief The "low" byte of the address of the pattern byte of the sprite.
         */
        u8 patternLo = 0x00;
        /**
         * @brief The "high" byte of the address of the pattern byte of the sprite.
         */
        u8 patternHi = 0x00;

        /**
         * @brief Helper variable to determine when to actually process the sprite.
         */
        u8 xCounter = 0x00;
    };

    /**
     * @brief Helper structure for processing sprite data within the NES PPU.
     */
    struct ActiveSprite {
        u8 patternLo = 0x00;
        u8 patternHi = 0x00;
        u8 attr = 0x00;
        u8 xCounter = 0x00;

        ActiveSprite() = default;
        ActiveSprite(u8 pl, u8 ph, u8 a, u8 x) :
            patternLo(pl), patternHi(ph), attr(a), xCounter(x) {}
    };

    /**
     * @brief Helper structure for the various Length Counters within NES APU channels.
     */
    struct LengthCounter {
        bool halt = false;      /// @brief Halt flag
        u8 counter = 0x00;      /// @brief Current value of the length counter.
        u8 reloadVal = 0x00;    /// @brief Value to reload length counter with upon reaching zero.
    };

    /**
     * @brief Helper structure for the various Volume Envelopes within NES APU channels.
     */
    struct VolumeEnvelope {
        bool loop = false;      /// @brief Loop flag
        bool start = false;     /// @brief Start flag
        bool constVol = false;  /// @brief Constant Volume flag
        u8 divider = 0x00;      /// @brief Current envelope counter value.
        u8 period = 0x00;       /// @brief Value to reload counter to upon reaching zero.
        u8 decay = 0x0F;        /// @brief Current Decay Level value.
    };

    /**
     * @brief Helper structure for the Sweep Units within the two NES APU Pulse channels.
     */
    struct SweepUnit {
        bool enabled = false;   /// @brief Enabled flag
        bool negate = false;    /// @brief Negate flag
        bool reload = false;    /// @brief Reload flag
        u8 divider = 0x00;      /// @brief Current sweep unit divider value.
        u8 period = 0x00;       /// @brief Divider reload value
        u8 shift = 0x00;        /// @brief Sweep Unit's shift amount
    };

    struct HighPassFilter {
        float alpha = 0.0f;
        float prevInput = 0.0f;
        float prevOutput = 0.0f;

        HighPassFilter(float cutoff, float rate) {
            float dt = 1.0f / rate;
            float rc = 1.0f / (2.0f * 3.1415927f * cutoff);
            alpha = rc / (rc + dt);
        }

        void process(float& input) {
            float output = alpha * (prevOutput + input - prevInput);

            prevInput = input;
            prevOutput = output;

            input = output;
        }
    };

    struct LowPassFilter {
        float alpha = 0.0f;
        float prevOutput = 0.0f;

        LowPassFilter(float cutoff, float rate) {
            float dt = 1.0f / rate;
            float rc = 1.0f / (2.0f * 3.1415927f * cutoff);
            alpha = dt / (rc + dt);
        }

        void process(float& input) {
            input = alpha * input + (1.0f - alpha) * prevOutput;
            prevOutput = input;
        }
    };

    /**
     * @brief The console region the ROM was developed for.
     */
    enum class REGION {
        NTSC,
        PAL,
        MULTI,
        DENDY
    };

    /**
     * @brief  Enumeration of the various Vs. System Hardware types.
     */
    enum class VS_HARDWARE_TYPE {
        VS_UNISYSTEM,
        VSU_RBI_BASEBALL_PROTECTION,
        VSU_TKO_BOXING_PROTECTION,
        VSU_SUPER_XEVIOUS_PROTECTION,
        VSU_ICE_CLIMBER_PROTECTION,
        VS_DUAL_SYSTEM,
        VSD_RAID_ON_BUNGELING_BAY_PROTECTION
    };

    /**
     * @brief Enumeration of the various Vs. System PPU types.
     */
    enum class VS_PPU_TYPE {
        PPU2C02_2C03,
        PPU2C04A = 2,
        PPU2C04B,
        PPU2C04C,
        PPU2C04D,
        PPU2C05A = 8,
        PPU2C05B,
        PPU2C05C,
        PPU2C05D
    };

    /**
     * @brief Enumeration of the various possible values of "Default Expansion Device"
     */
    enum class EXPANSION_DEVICE {
        UNSPECIFIED,
        STANDARD_CONTROLLERS,
        FOUR_SCORE,
        FOUR_PLAYER_ADAPTER,
        VS_SYSTEM_4016,
        VS_SYSTEM_4017,
        VS_ZAPPER = 7,
        ZAPPER_4017,
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
        ROB_GYROMITE,
        FAMICOM_DATA_RECORDER,
        TURBO_FILE,
        BATTLE_BOX,
        FAMILY_BASIC_KEYBOARD,
        PEC_KEYBOARD,
        BIT_79_KEYBOARD,
        SUBOR_KEYBOARD,
        SUBOR_KEYBOARD_MACROWINNER_MOUSE,
        SUBOR_KEYBOARD_SUBOR_MOUSE_4016,
        SNES_MOUSE_4016,
        GENERIC_MULTICART,
        SNES_CONTROLLERS,
        RACERMATE_BICYCLE,
        UFORCE,
        ROB_STACK_UP,
        CITY_PATROLMAN,
        CASSETTE_INTERFACE,
        STANDARD_SWAPPED,
        EXCALIBUR_SUDOKU,
        ABL_PINBALL,
        CASINO_BUTTONS,
        KEDA_KEYBOARD,
        SUBOR_KEYBOARD_SUBOR_MOUSE_4017,
        PORT_TEST_CONTROLLER,
        BANDAI_MGP_BUTTONS,
        VENOM_DANCE_MAT,
        LG_TV_REMOTE,
        FAMICOM_NETWORK_CONTROLLER,
        KING_FISHING,
        CROAKY_KARAOKE,
        KINGWON_KEYBOARD,
        ZECHENG_KEYBOARD,
        SUBOR_KEYBOARD_LPS2_MOUSE_4017,
        PS2_KEYBOARD_UM6578_PS2_MOUSE_4017,
        PS2_MOUSE_UM6578,
        YUXING_MOUSE_4016,
        SUBOR_KEYBOARD_YUXING_MOUSE_4016,
        GIGGLE_TV_PUMP,
        SUBOR_KEYBOARD_RPS2_MOUSE_4017,
        MAGICAL_COOKING,
        SNES_MOUSE_4017,
        ZAPPER_4016,
        ARKANOID_PROTOTYPE,
        TV_MAHJONG_CONTROLLER,
        MAHJONG_GEKITOU_CONTROLLER,
        SUBOR_KEYBOARD_XPS2_MOUSE_4017,
        IBM_KEYBOARD,
        SUBOR_KEYBOARD_MEGABOOK_MOUSE,
        TOTAL_DEVICES
    };

    /**
     * @brief Enumeration of the various possible "Extended Console" types.
     */
    enum class EXTENDED_CONSOLE_TYPE {
        FAMICLONE_DECIMAL = 3,
        NES_EPSM,
        VR_VT01,
        VR_VT02,
        VR_VT03,
        VR_VT09,
        VR_VT32,
        VR_VT369,
        UMC_UM6578,
        FNS
    };

    enum class DMA_TYPE {
        NONE,
        OAM_DMA,
        DMC_LOAD,
        DMC_RELOAD
    };

    /*
        PPU Control Register flags
        7..bit..0
        I-sB SVNN
        |||| ||++->Nametable Base (0: 0x2000; 1: 0x2400; 2: 0x2800; 3: 0x2C00)
        |||| |+--->VRAM Increment (1: 32; 0: 1)
        |||| +---->Sprite Pattern Base (1: 0x1000; 0x0000) [ONLY FOR 8X8 MODE]
        |||+------>Background Pattern Base (1: 0x1000; 0: 0x0000)
        ||+------->Sprite size (1: 8x16; 0: 8x8)
        |+-------->Master/Slave (unused)
        +--------->NMI
    */
    enum class CONTROL {
        NAMETABLE_BASE,
        VRAM_INCREMENT = 2,
        SPRITE_PATTERN_ADDR,
        BACKGROUND_PATTERN_ADDR,
        SPRITE_SIZE,
        MAIN_SECOND,
        NMI_ENABLED
    };

    /*
        PPU Mask Register flags
        7..bit..0
        bgrS BlLG
        |||| |||+->enable greyscale/monochrome mode
        |||| ||+-->enable background in first 8 pixels of screen
        |||| |+--->enable sprites in first 8 pixels of screen
        |||| +---->enable background rendering
        |||+----->enable sprite rendering
        ||+------>emphasize red in rendered pixels
        |+------->emphasize green in rendered pixels
        +-------->emphasize blue in rendered pixels
    */
    enum class MASK {
        GRAYSCALE,
        ENABLE_BACKGROUND_LEFT,
        ENABLE_SPRITES_LEFT,
        ENABLE_BACKGROUND,
        ENABLE_SPRITES,
        EMPHASIZE_RED,
        EMPHASIZE_GREEN,
        EMPHASIZE_BLUE
    };

    /*
        PPU Status Register flags
        7..bit..0
        VZO- ----
        ||+------->Sprite-Overflow flag
        |+-------->Sprite-Zero-Hit flag
        +--------->VBlank flag
    */
    enum class STATUS {
        SPRITE_OVERFLOW = 0x05,
        SPRITE_ZERO_HIT,
        VBLANK
    };
}