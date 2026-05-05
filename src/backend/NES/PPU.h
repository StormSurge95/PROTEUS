#pragma once

#include "./NES_PCH.h"

#include "./Gamepak.h"
#include "./CPU.h"

namespace NES_NS {
    class PPU : public IDevice<u8, u16> {
        friend class Debugger;
        public:
            u16 scanline = 0;
            u16 cycle = 0;
            bool nmiRequested = false;
            bool frameComplete = false;

            PPU() = default;
            ~PPU() = default;

            /*
                Primary scheduling function of the PPU.
                Determines what operations need to be performed based on
                which pixel of the screen we are on.
            */
            void clock();

            /*
                Reset function of the PPU.
                Handles and performs any operations/changes necessary to
                reset the PPU to a known and predetermined state.
            */
            void reset();

            /*
                Performs intra-device read operations on the various
                registers that are visible to other devices for reading.
            */
            u8 read(u16, bool = false) override;
            u8 getOAMADDR() const { return OAMADDR; };

            /*
                Performs intra-device write operations on the various
                registers that are visible to other devices for writing.
            */
            void write(u16, u8) override;

            /*
                Collects and returns OAM byte data from memory.
                If `i` is provided, then the requested byte is
                returned; otherwise, the byte pointed to by 
                `OAMADDR` is returned.

                `i`: any integer value from 0-255.
            */
            u8 readOAMByte(int = -1) const;

            /*
                Writes a provided byte of data to the requested
                location within OAM memory.

                `i`: integer value 0-255 representing where to write
                `b`: actual byte of data to write
            */
            void writeOAMByte(u8, u8);

            const u32* getFrameBuffer() const { return frameBuffer.data(); }
            void connectCART(sptr<Gamepak>& c) { cart = c; }
            void connectCPU(sptr<CPU>& c) { cpu = c; }
        private:
            bool nmiOutput = false;
            bool nmiOutputPrev = false;
            bool isRendering = false;
            bool oddFrame = false;
            bool initComplete = false;
            bool suppressVBL = false;
            bool suppressNMI = false;

            /*
                Performs read operations by reading from VRAM on
                the PPU and/or CHR-ROM/CHR-RAM on the cartridge.
            */
            u8 ppuRead(u16, bool = false);

            /*
                Performs write operations by writing to VRAM on
                the PPU and/or CHR-ROM/CHR-RAM on the cartridge.
            */
            void ppuWrite(u16, u8);

            /*
                Fakes the bit decay of the analogue NES system PPU
            */
            void bitDecay();
            void updateCounters(u8);
            u8 decayCounters[8] = { 20, 20, 20, 20, 20, 20, 20, 20 };

            wptr<Gamepak> cart;
            wptr<CPU> cpu;
            array<u32, 61440> frameBuffer{ 0 };
            array<u32, 64> masterPalette = {
                /* 0x00 */ 0xff626262,
                /* 0x01 */ 0xff902001,
                /* 0x02 */ 0xffa00b24,
                /* 0x03 */ 0xff900047,
                /* 0x04 */ 0xff620060,
                /* 0x05 */ 0xff24006a,
                /* 0x06 */ 0xff001160,
                /* 0x07 */ 0xff002747,
                /* 0x08 */ 0xff003c24,
                /* 0x09 */ 0xff004a01,
                /* 0x0a */ 0xff004f00,
                /* 0x0b */ 0xff244700,
                /* 0x0c */ 0xff623600,
                /* 0x0d */ 0xff000000,
                /* 0x0e */ 0xff000000,
                /* 0x0f */ 0xff000000,
                /* 0x10 */ 0xffababab,
                /* 0x11 */ 0xffe1561f,
                /* 0x12 */ 0xffff394d,
                /* 0x13 */ 0xffef237e,
                /* 0x14 */ 0xffb71ba3,
                /* 0x15 */ 0xff6422b4,
                /* 0x16 */ 0xff0e37ac,
                /* 0x17 */ 0xff00558c,
                /* 0x18 */ 0xff00725e,
                /* 0x19 */ 0xff00882d,
                /* 0x1a */ 0xff009007,
                /* 0x1b */ 0xff478900,
                /* 0x1c */ 0xff9d7300,
                /* 0x1d */ 0xff000000,
                /* 0x1e */ 0xff000000,
                /* 0x1f */ 0xff000000,
                /* 0x20 */ 0xffffffff,
                /* 0x21 */ 0xffffac67,
                /* 0x22 */ 0xffff8d95,
                /* 0x23 */ 0xffff75c8,
                /* 0x24 */ 0xffff6af2,
                /* 0x25 */ 0xffc56fff,
                /* 0x26 */ 0xff6a83ff,
                /* 0x27 */ 0xff1fa0e6,
                /* 0x28 */ 0xff00bfb8,
                /* 0x29 */ 0xff01d885,
                /* 0x2a */ 0xff35e35b,
                /* 0x2b */ 0xff88de45,
                /* 0x2c */ 0xffe3ca49,
                /* 0x2d */ 0xff4e4e4e,
                /* 0x2e */ 0xff000000,
                /* 0x2f */ 0xff000000,
                /* 0x30 */ 0xffffffff,
                /* 0x31 */ 0xffffe0bf,
                /* 0x32 */ 0xffffd3d1,
                /* 0x33 */ 0xffffc9e6,
                /* 0x34 */ 0xffffc3f7,
                /* 0x35 */ 0xffeec4ff,
                /* 0x36 */ 0xffc9cbff,
                /* 0x37 */ 0xffa9d7f7,
                /* 0x38 */ 0xff97e3e6,
                /* 0x39 */ 0xff97eed1,
                /* 0x3a */ 0xffa9f3bf,
                /* 0x3b */ 0xffc9f2b5,
                /* 0x3c */ 0xffeeebb5,
                /* 0x3d */ 0xffb8b8b8,
                /* 0x3e */ 0xff000000,
                /* 0x3f */ 0xff000000
            };
            array<u8, 4096> nametables{ 0 };
            array<u8, 32> palettes{ 0 };

            /*
                Performs Pre - Render Scanline operations
                This scanline is mostly just for "priming" our rendering pipelines
                so that we can be prepared to begin submitting pixels as soon as
                the frame starts.
                This includes background AND sprite pipelines.
            */
            void onPreRenderLine();
            void onVisibleLine();
            void onStartVBlankLine();

            // 76543210
            // I-sBSVNN
            // |------->NMI
            //  |------>Master/Slave (unused)
            //   |----->Sprite size (1: 8x16; 0: 8x8)
            //    |---->Background Pattern Base (1: 0x1000; 0: 0x0000)
            //     |--->Sprite Pattern Base (1: 0x1000; 0x0000) [ONLY FOR 8X8 MODE]
            //      |-->VRAM Increment (1: 32; 0: 1)
            //       ||>Nametable Base (0: 0x2000; 1: 0x2400; 2: 0x2800; 3: 0x2C00)
            enum CONTROL {
                NAMETABLE_BASE,
                VRAM_INCREMENT = 2,
                SPRITE_PATTERN_ADDR,
                BACKGROUND_PATTERN_ADDR,
                SPRITE_SIZE,
                NMI_ENABLED = 7
            };
            u8 getControlData(CONTROL which) const;
            inline u16 getNametableBase() const { return 0x2000 + (getControlData(NAMETABLE_BASE) * 0x400); }
            inline u8 getVRAMIncrement() const { return (getControlData(VRAM_INCREMENT) ? 32 : 1); }
            inline u16 getSpritePatternTableAddr8x8() const { return (getControlData(SPRITE_PATTERN_ADDR) ? 0x1000 : 0x0000); }
            inline u16 getBackgroundPatternTableAddr() const { return (getControlData(BACKGROUND_PATTERN_ADDR) ? 0x1000 : 0x0000); }
            inline u8 getSpriteHeight() const { return (getControlData(SPRITE_SIZE) ? 16 : 8); }
            inline bool getNMIEnabled() const { return !!(getControlData(NMI_ENABLED)); }

            // 76543210
            // bgrSBlLG
            // |------->emphasize blue in rendered pixels
            //  |------>emphasize green in rendered pixels
            //   |----->emphasize red in rendered pixels
            //    |---->enable sprite rendering
            //     |--->enable background rendering
            //      |-->enable sprites in first 8 pixels of screen
            //       |->enable background in first 8 pixels of screen
            //        |>enable greyscale/monochrome mode
            enum MASK {
                GREYSCALE,
                ENABLE_BACKGROUND_LEFT,
                ENABLE_SPRITES_LEFT,
                ENABLE_BACKGROUND,
                ENABLE_SPRITES,
                EMPHASIZE_RED,
                EMPHASIZE_GREEN,
                EMPHASIZE_BLUE
            };
            u8 getMaskData(MASK which) const { return ((PPUMASK >> which) & 0x01); }
            inline bool getGreyscale() const { return !!(getMaskData(GREYSCALE)); }
            inline bool renderBackgroundLeft() const { return !!(getMaskData(ENABLE_BACKGROUND_LEFT)); }
            inline bool renderSpritesLeft() const { return !!(getMaskData(ENABLE_SPRITES_LEFT)); }
            inline bool renderBackground() const { return !!(getMaskData(ENABLE_BACKGROUND)); }
            inline bool renderSprites() const { return !!(getMaskData(ENABLE_SPRITES)); }
            u32 applyEmphasis(u32 color) const;

            // 76543210
            // VZO-----
            // |------->VBlank flag
            //  |------>Sprite-Zero-Hit flag
            //   |----->Sprite-Overflow flag
            //    |||||>UNUSED/OPENBUS
            enum STATUS {
                SPRITE_OVERFLOW = 0x05,
                SPRITE_ZERO_HIT,
                VBLANK
            };
            bool getStatusData(STATUS which) const { return ((PPUSTATUS >> which) & 0x01); }
            void setStatusData(STATUS which, bool v);
            inline bool spritesOverflowed() const { return getStatusData(SPRITE_OVERFLOW); }
            inline void spritesOverflowed(bool v) { setStatusData(SPRITE_OVERFLOW, v); }
            inline bool spriteZeroHit() const { return getStatusData(SPRITE_ZERO_HIT); }
            inline void spriteZeroHit(bool v) { setStatusData(SPRITE_ZERO_HIT, v); }
            inline bool inVBlank() const { return getStatusData(VBLANK); }
            inline void inVBlank(bool v) { setStatusData(VBLANK, v); if (!v) nmiRequested = false; }

            // REGISTERS
            u8 PPUCTRL = 0x00;     // $2000 write
            u8 PPUMASK = 0x00;     // $2001 write
            u8 PPUSTATUS = 0xA0;   // $2002 read
            u8 OAMADDR = 0x00;     // $2003 write
            u8 OAMDATA = 0x00;     // $2004 read/write
            u8 PPUSCROLL = 0x00;   // $2005 write
            u8 PPUADDR = 0x00;     // $2006 write
            u8 PPUDATA = 0x00;     // $2007 read/write
            u8 OAMDMA = 0x00;      // $4014 write
            u8 dataBuffer = 0x00;
            u8 ppuBus = 0x00;

            u16 v = 0x0000;  // during rendering, used for scroll position; outside rendering, used as current VRAM address
            u16 t = 0x0000;  // during rendering, specifies starting coarse-x scroll for next scanline and starting y scroll for screen; outside rendering, holds scroll or VRAM before transferring it to v
            u8 x = 0x00;  // fine-x position of current scroll, used during rendering alongside v
            bool w = false; // write-latch for PPUSCROLL/PPUADDR; clears on read of PPUSTATUS

            inline bool renderingEnabled() const { return ((renderBackground() || renderSprites()) && !inVBlank()); }

            u8 nextNametableByte = 0x00;
            u8 nextAttributeByte = 0x00;
            u8 nextPatternByteLo = 0x00;
            u8 nextPatternByteHi = 0x00;

            u16 bgPatternAddr = 0x0000;
            u16 patternShiftLo = 0x0000;
            u16 patternShiftHi = 0x0000;
            u16 attributeShiftLo = 0x0000;
            u16 attributeShiftHi = 0x0000;

            inline void copyHorizontalBits() { if (renderingEnabled()) v = (v & ~0b0000010000011111) | (t & 0b0000010000011111); }
            inline void copyVerticalBits() { if (renderingEnabled()) v = (v & ~0b0111101111100000) | (t & 0b0111101111100000); }
            inline u8 coarseX() const { return v & 0x1F; }
            inline u8 coarseY() const { return ((v & 0x03E0) >> 5); }
            inline u8 fineY() const { return ((v & 0x7000) >> 12); }

            std::array<std::array<u8, 4>, 64> primaryOAM{ 0 };  // max of 64 sprites per game
            std::array<std::array<u8, 4>, 8> secondaryOAM{ 0 }; // max of 8 sprites per scanline

            enum SPRITE_ATTR {
                PALETTE,
                PRIORITY,
                XFLIP,
                YFLIP
            };
            inline u8 readSpriteAttr(SPRITE_ATTR which, u8 attr) {
                switch (which) {
                    case PALETTE:
                        return (attr & 0x03);
                    case PRIORITY:
                        return ((attr >> 5) & 0x01);
                    case XFLIP:
                        return ((attr >> 6) & 0x01);
                    case YFLIP:
                        return ((attr >> 7) & 0x01);
                }
                return 0x00;
            }
            inline u8 getSpritePalette(u8 attr) { return readSpriteAttr(PALETTE, attr); }
            inline bool spriteAboveBackground(u8 attr) { return readSpriteAttr(PRIORITY, attr) == 0; }
            inline bool flipX(u8 attr) { return readSpriteAttr(XFLIP, attr) > 0; }
            inline bool flipY(u8 attr) { return readSpriteAttr(YFLIP, attr) > 0; }

            void backgroundPipeline();

            void fetchBGNametableByte();
            void fetchBGAttributeByte();

            void fetchBGPatternByteLo();
            void fetchBGPatternByteHi();

            void incrementCoarseX();
            void incrementFineY();

            void loadBackgroundShifters();

            void shiftBackgroundShifters();

            /*
                Performs the necessary calculations to determine the values
                related to the background pixel at the current dot.
            */
            void getBackgroundPixel(u8& pixel, u8& attr) const;

            /*
                Performs comparisons and processing necessary to determine how
                the current pixel should be rendered (ie. via background or via
                sprite).
            */
            void renderPixel();

            /*
                Reinitializes secondaryOAM and clears it in preparation for
                the next round of sprite evaluation(s). The NES expects a
                "cleared" byte to be equal to 0xFF; so on every EVEN cycle,
                we set the corresponding byte in secondaryOAM to 0xFF.
            */
            void initSecondaryOAM();

            u8 spriteIndex = 0;
            u8 n = 0x00;
            u8 m = 0x00;
            u8 oamLatch = 0x00;
            u8 spritesOnScanline = 0x00;
            u8 oamIndex = 0x00;
            u8 byteIndex = 0x00;
            bool evaluating = false;
            bool spriteInRange = false;
            bool copying = false;
            bool sprite0InRange = false;
            bool overflow = false;
            bool allSpriteEvalsComplete = false;
            u16 spritePatternAddr = 0x0000;

            u8 sprTileIndex = 0x00;
            u8 sprAttributes = 0x00;
            u8 sprXPosition = 0x00;
            u8 sprPatternLo = 0x00;
            u8 sprPatternHi = 0x00;

            struct ActiveSprite {
                u8 patternLo = 0x00;
                u8 patternHi = 0x00;
                u8 attr = 0x00;
                u8 xCounter = 0x00;

                ActiveSprite() = default;
                ActiveSprite(u8 pl, u8 ph, u8 a, u8 x) :
                    patternLo(pl), patternHi(ph), attr(a), xCounter(x) {}
            };

            vector<ActiveSprite> activeSprites;
            vector<ActiveSprite> nextSprites;

            /*
                Evaluates one sprite each cycle during cycles 65-256
                This functions tests sprite position compared to scanline to ensure
                that the sprite actually appears on the scanline. If it does,
                then we copy its data into secondaryOAM for later testing to determine
                whether it will be printed during any particular cycle.
                Each scanline can only support up to 8 sprites. If more than 8 are
                found, then the spriteOverflow flag within PPUSTATUS is set.
            */
            void spriteEval();

            /*
                Fetches various data for the sprite being processed. Each
                sprite takes a total of 8 cycles to fetch all necessary data.
                This allows the process to perfectly fit in cycles 257-320.
            */
            void spriteFetch();

            /*
                Helper function to calculate address of sprite pattern table.
            */
            void calcSPRPatternAddr(u8 index, u8 id, u8 y);

            /*
                Performs the necessary calculations to determine the values
                related to the sprite pixel (if there is one) at the current dot.
            */
            u8 getSpritePixel(u8& pixel, u8& attr);
    };
}