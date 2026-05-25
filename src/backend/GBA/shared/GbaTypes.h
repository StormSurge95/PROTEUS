#pragma once

namespace NS_GBA {
    enum class CpuMode {
        ARM,    // ARM7TDMI 32bit RISC CPU, 16.78MHz, 32bit opcodes (GBA)
        THUMB,  // ARM7TDMI 32bit RISC CPU, 16.78MHz, 16bit opcodes (GBA)
        CGB,    // Z80/8080-style 8bit CPU, 4.2MHz or 8.4MHz (CGB compatibility)
        CMG     // Z80/8080-style 8bit CPU, 4.2MHz (monochrome gameboy compatibility)
    };

    enum class BackgroundType {
        TILEMAP,
        BITMAP
    };

    enum class BackgroundColors {
        SINGLE_PALETTE, // one palette of 256 colors
        MULTI_PALETTE,  // 16 palettes of 16 colors each
        UNLIMITED       // "32768 colors" ???
    };

    enum class ObjectColors {
        SINGLE_PALETTE, // one palette of 256 colors
        MULTI_PALETTE   // 16 palettes of 16 colors each
    };

    enum class ObjectMode {
        NORMAL,
        SEMI_TRANSPARENT,
        OBJ_WINDOW,
        PROHIBITED
    };

    enum class ObjectSize {
        // "12 types (in range 8x8 up to 64x64 dots)"
        SQUARE8,
        TALL8,
        FAT8,
        SQUARE16,
        TALL16,
        FAT16,
        SQUARE32,
        TALL32,
        FAT32,
        SQUARE64,
        TALL64,
        FAT64
    };

    enum class ObjectShape {
        SQUARE,
        HORIZONTAL,
        VERTICAL,
        PROHIBITED
    };

    /**
     @brief Structure for GBA Sprite Objects
     @note OBJ Mode
     The OBJ Mode may be Normal, Semi-Transparent, or OBJ Window
     - Semi-Transparent means that the OBJ is used as 'Alpha Blending 1st Target' (regardless of BLDCNT register)
     - OBJ Window means that the OBJ is not displayed; instead, dots with non-zero color are used as mask for the
          OBJ Window (see DISPCNT and WINOUT for details).
     
     @note OBJ Tile Number
     There are two situations which may divide the amount of available tiles by two (by four if both situations apply):
     1. When using the 256 Colors/1 Palette mode, only each second tile may be used, the lower bit of the tile
          number should be zero (in 2-dimensional mapping mode, the bit is completely ignored).
     2. When using BG Mode 3-5 (Bitmap Modes), only tile numbers 512-1023 may be used. That is because lower
          16K of OBJ memory are used for BG. Attempts to use tiles 0-511 are ignored (not displayed).
     
     @note Priority
     In the case that the 'Priority relative to BG' is the same as the priority of one of the background layers, then the
     OBJ becomes higher priority and is displayed on top of that BG layer.
     Caution: Take care not to mess up BG Priority and OBJ priority. For example, the following would cause garbage
     to be displayed:
          OBJ No. 0 with Priority relative to BG=1    ;hi OBJ prio, lo BG prio
          OBJ No. 1 with Priority relative to BG=0    ;lo OBJ prio, hi BG prio
     That is, OBJ0 is always having priority above OBJ1-127, so assigning a lower BG priority to OBJ0 than for
     OBJ1-127 would be a bad idea.
     **/
    struct OBJ {
        /**
         * @brief OBJ Attribute 0 (R/W)
         * @details
         * Bit      Expl.
         * 0-7      Y-Coordinate            (0..255)
         * 8        Rotation/Scaling Flag   (0=Off, 1=On)
         * When Rotoation/Scaling used:
         *   9      Double-Size Flag        (0=Normal, 1=Double)
         * When Rotation/Scaling NOT used:
         *   9      OBJ Disable             (0=Normal, 1=Not displayed)
         * 10-11    OBJ Mode                (0=Normal, 1=Semi-Transparent, 2=OBJ Window, 3=Prohibited)
         * 12       OBJ Mosaic              (0=Off, 1=On)
         * 13       Colors/Palettes         (0=16/16, 1=256/1)
         * 14-15    OBJ Shape               (0=Square, 1=Horizontal, 2=Vertical, 3=Prohibited)
         */
        u16 attr0;

        /**
         * @brief OBJ Attribute 1 (R/W)
         * @details
         * Bit      Expl.
         * 0-8      X-Coordinate            (0..511)
         * When Rotation/Scaling used:
         *   9-13   Rotation/Scaling Paremeter Selection (0-31)
         * When Rotation/Scaling NOT used:
         *   9-11   Not used
         *   12     Horizontal Flip         (0=Normal, 1=Mirrored)
         *   13     Vertical Flip           (0=Normal, 1=Mirrored)
         * 14-15    OBJ Size                (0..3, depends on OBJ Shape, see Attr 0)
         *      Size    Square  Horizontal  Vertical
         *      0       8x8     16x8        8x16
         *      1       16x16   32x8        8x32
         *      2       32x32   32x16       16x32
         *      3       64x64   64x32       32x64
         */
        u16 attr1;

        /**
         * @brief OBJ Attribute 2 (R/W)
         * @details
         * Bit      Expl.
         * 0-9      Character Name              (0..1023=Tile Number)
         * 10-11    Priority relative to BG     (0..3; 0=Highest)
         * 12-15    Palette Number              (0..15; Not used in 256 color/1 palette mode)
         */
        u16 attr2;

        // attr 0
        u8 GetYCoord() const { return (attr0 & 0xFF); }
        bool RotateScale() const { return ((attr0 >> 8) & 0x01) > 0; }
        bool DoubleSize() const { return RotateScale() ? ((attr0 >> 9) & 0x01) > 0 : false; }
        bool Disabled() const { return RotateScale() ? false : ((attr0 >> 9) & 0x01) > 0; }
        ObjectMode Mode() const { return ObjectMode((attr0 >> 10) & 0x03); }
        bool Mosaic() const { return ((attr0 >> 12) & 0x01) > 0; }
        ObjectColors Colors() const { return ObjectColors((attr0 >> 13) & 0x01); }
        ObjectShape Shape() const { return ObjectShape((attr0 >> 14) & 0x03); }

        // attr 1
        u8 GetXCoord() const { return (attr1 & 0xFF); }
        u8 GetRotateScaleParam() const { return RotateScale() ? ((attr1 >> 9) & 0x1F) : 0xFF; }
        bool FlipX() const { return RotateScale() ? false : ((attr1 >> 12) & 0x01) > 0; }
        bool FlipY() const { return RotateScale() ? false : ((attr1 >> 13) & 0x01) > 0; }
        ObjectSize Size() const { return ObjectSize(((u8)Shape() << 2) | ((attr1 >> 14) & 0x03)); } // TODO: Test and confirm this

        // attr 2
        u16 Name() const { return (attr2 & 0x03FF); }
        u8 PriorityToBG() const { return ((attr2 >> 10) & 0x03); }
        u8 PaletteNumber() const { return (Colors() == ObjectColors::SINGLE_PALETTE ? 0 : ((attr2 >> 12) & 0x0F)); }
    };

    enum class Button {
        BUTTON_A,
        BUTTON_B,
        SELECT,
        START,
        RIGHT,
        LEFT,
        UP,
        DOWN,
        BUTTON_R,
        BUTTON_L,
        IRQ_ENABLE = 14,
        IRQ_CONDITION
    };

    /**
     * @brief Nintendo GBA ROM header
     * @details
     * The first 192 bytes at $08000000-$080000BF in ROM are used as cartridge header.
     * The same header is also used for Multiboot images at $02000000-$020000BF
     */
    struct GbaHeader {
        // standard header entries
        u32 EntryPointROM = 0x00000000; // 32bit ARM branch opcode
        u8 Logo[156] = {};              // compressed bitmap, REQUIRED
        u8 GameTitle[12] = {};          // uppercase ascii, max 12 characters
        u8 GameCode[4] = {};            // uppercase ascii, 4 characters
        u8 MakerCode[2] = {};           // uppercase ascii, 2 characters
        u8 FixedValue = 0x96;           // must be $96; REQUIRED
        u8 MainUnitCode = 0x00;         // $00 for current GBA models
        u8 DeviceType = 0x00;           // usually $00; bit7=DACS/debug related
        u8 ReservedArea1[7] = { 0x00 }; // should be zero filled
        u8 SoftwareVersion = 0x00;      // usually $00
        u8 ComplementCheck = 0x00;      // header checksum; REQUIRED
        u16 ReservedArea2 = 0x0000;     // should be zero filled
        // additional multiboot header entries
        u32 EntryPointRAM = 0x00000000; // 32bit ARM branch opcode
        u8 BootMode = 0x00;             // init as $00; BIOS OVERWRITES THIS VALUE
        u8 SlaveID = 0x00;              // init as $00; BIOS OVERWRITES THIS VALUE
        u8 Unused[26] = { 0x00 };       // seems to be unused
        u32 EntryPointJOY = 0x00000000; // 32bit ARM branch opcode
    };
}