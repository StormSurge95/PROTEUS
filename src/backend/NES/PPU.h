#pragma once

#include "./NES_PCH.h"

#include "./Gamepak.h"
#include "./CPU.h"

namespace NES_NS {
    class PPU : public IDevice<u8, u16> {
            // allow Debugger class to access all private members of the PPU class
            friend class Debugger;
        public:
            // current scanline of current frame
            u16 scanline = 0;
            // current dot/pixel of current scanline
            u16 cycle = 0;
            // public nmi flag as produced by ppu
            bool nmiRequested = false;
            // frame complete flag
            bool frameComplete = false;
            // suppress nmi flag
            bool suppressNMI = false;

            // default constructor
            PPU() = default;
            // default destructor
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
            // returns the value of the OAMADDR register
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

            // returns a pointer to the pixel data produced by the PPU
            const u32* getFrameBuffer() const { return frameBuffer.data(); }
            // connects a cartridge/gamepak/ROM to the ppu (mainly for oamdma)
            void connectCART(sptr<Gamepak>& c) { cart = c; }
            // connects the cpu to the ppu
            void connectCPU(sptr<CPU>& c) { cpu = c; }
        private:
            // current nmi output value
            bool nmiOutput = false;
            // previous nmi output value
            bool nmiOutputPrev = false;
            // odd frame flag
            bool oddFrame = false;
            // suppress vbl flag
            bool suppressVBL = false;

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

            // Fakes the bit decay of the analogue NES system PPU
            void bitDecay();
            // Updates the counters used for bit decay
            void updateCounters(u8);
            // actual decay counters for ppu bus bit decay
            u8 decayCounters[8] = { 20, 20, 20, 20, 20, 20, 20, 20 };

            // reference to the gamepak
            wptr<Gamepak> cart;
            // reference to the cpu
            wptr<CPU> cpu;
            // array to hold the processed and assembled frame pixels
            array<u32, 61440> frameBuffer{ 0 };
            // NES master color palette in ABGR format
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
            // array to hold the tile indexes of the nametables within vram
            array<u8, 4096> nametables{ 0 };
            // array to hold the various master palette indexes within vram
            array<u8, 32> palettes{ 0 };

            // Performs Pre-Render Scanline operations
            void onPreRenderLine();
            // Performs Visible Scanline operations
            void onVisibleLine();
            // Performs Start-of-VBlank Scanline operations
            void onStartVBlankLine();

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
            enum CONTROL {
                NAMETABLE_BASE,
                VRAM_INCREMENT = 2,
                SPRITE_PATTERN_ADDR,
                BACKGROUND_PATTERN_ADDR,
                SPRITE_SIZE,
                MAIN_SECOND,
                NMI_ENABLED
            };
            // Helper function to return the requested bit-flag value from the PPUCTRL register
            u8 getControlData(CONTROL which) const;
            // Gets the nametable base address based on PPUCTRL values
            inline u16 getNametableBase() const { return 0x2000 + (getControlData(NAMETABLE_BASE) * 0x400); }
            // Gets the current VRAM increment amount based on PPUCTRL values
            inline u8 getVRAMIncrement() const { return (getControlData(VRAM_INCREMENT) ? 32 : 1); }
            // Gets the Sprite Pattern Table base address based on PPUCTRL values; NOTE: THIS IS ONLY VALID FOR 8x8 SPRITES
            inline u16 getSpritePatternTableAddr8x8() const { return (getControlData(SPRITE_PATTERN_ADDR) ? 0x1000 : 0x0000); }
            // Gets the Background Pattern Table base address based on PPUCTRL values
            inline u16 getBackgroundPatternTableAddr() const { return (getControlData(BACKGROUND_PATTERN_ADDR) ? 0x1000 : 0x0000); }
            // Gets the current Sprite mode (8x8 or 8x16) we are in based on PPUCTRL values
            inline u8 getSpriteHeight() const { return (getControlData(SPRITE_SIZE) ? 16 : 8); }
            // Gets whether or not NMI is currently enabled based on PPUCTRL values
            inline bool getNMIEnabled() const { return !!(getControlData(NMI_ENABLED)); }

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
            enum MASK {
                GRAYSCALE,
                ENABLE_BACKGROUND_LEFT,
                ENABLE_SPRITES_LEFT,
                ENABLE_BACKGROUND,
                ENABLE_SPRITES,
                EMPHASIZE_RED,
                EMPHASIZE_GREEN,
                EMPHASIZE_BLUE
            };
            // Helper function to return the requested bit-flag value from the PPUMASK register
            u8 getMaskData(MASK which) const { return ((PPUMASK >> which) & 0x01); }
            // Gets whether or not rendering should be monochromatic.
            inline bool getGreyscale() const { return !!(getMaskData(GRAYSCALE)); }
            // Gets whether or not the first column of background tiles should be rendered.
            inline bool renderBackgroundLeft() const { return !!(getMaskData(ENABLE_BACKGROUND_LEFT)); }
            // Gets whether or not any sprites should be rendered within the first column of tiles.
            inline bool renderSpritesLeft() const { return !!(getMaskData(ENABLE_SPRITES_LEFT)); }
            // Gets whether or not background should be rendered for this frame
            inline bool renderBackground() const { return !!(getMaskData(ENABLE_BACKGROUND)); }
            // Gets whether or not sprites should be rendered for this frame
            inline bool renderSprites() const { return !!(getMaskData(ENABLE_SPRITES)); }
            // helper function to apply R/G/B color emphasis based on their individual bit-flag values
            u32 applyEmphasis(u32 color) const;

            /*
                PPU Status Register flags
                7..bit..0
                VZO- ----
                ||+------->Sprite-Overflow flag
                |+-------->Sprite-Zero-Hit flag
                +--------->VBlank flag
            */
            enum STATUS {
                SPRITE_OVERFLOW = 0x05,
                SPRITE_ZERO_HIT,
                VBLANK
            };
            // Helper function to GET the requested bit-flag value from teh PPU Status register
            bool getStatusData(STATUS which) const { return ((PPUSTATUS >> which) & 0x01); }
            // Helper function to SET the requested bit-flag value from the PPU Status register
            void setStatusData(STATUS which, bool v);
            // Gets/Sets whether sprite overflow has occurred on the current scanline.
            inline bool spritesOverflowed() const { return getStatusData(SPRITE_OVERFLOW); } // GETTER
            inline void spritesOverflowed(bool v) { setStatusData(SPRITE_OVERFLOW, v); } // SETTER
            // Gets/Sets whether sprite zero hit has occurred on the current scanline.
            inline bool spriteZeroHit() const { return getStatusData(SPRITE_ZERO_HIT); } // GETTER
            inline void spriteZeroHit(bool v) { setStatusData(SPRITE_ZERO_HIT, v); } // SETTER
            // Gets/Sets whether or not we are currently in VBlank status.
            inline bool inVBlank() const { return getStatusData(VBLANK); } // GETTER
            inline void inVBlank(bool v) { setStatusData(VBLANK, v); } // SETTER

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

            u8 dataBuffer = 0x00; // helper variable for emulating the PPUDATA read quirk
            u8 ppuBus = 0x00; // helper variable for emulating the PPU's open-bus behavior

            u16 v = 0x0000;  // during rendering, used for scroll position; outside rendering, used as current VRAM address
            u16 t = 0x0000;  // during rendering, specifies starting coarse-x scroll for next scanline and starting y scroll for screen; outside rendering, holds scroll or VRAM before transferring it to v
            u8 x = 0x00;  // fine-x position of current scroll, used during rendering alongside v
            bool w = false; // write-latch for PPUSCROLL/PPUADDR; clears on read of PPUSTATUS

            // Helper function for determining whether or not ANY form of rendering is enabled at any given point.
            inline bool renderingEnabled() const { return ((renderBackground() || renderSprites()) && !inVBlank()); }

            // the next nametable byte for use during the background pipeline process
            u8 nextNametableByte = 0x00;
            // the next attribute table byte for use during the background pipeline process
            u8 nextAttributeByte = 0x00;
            // the byte value used for the LOW bit during pattern table processing of the background pipeline process
            u8 nextPatternByteLo = 0x00;
            // the byte value used for the HIGH bit during pattern table processing of the background pipeline process
            u8 nextPatternByteHi = 0x00;

            // the address of the background pattern table tile information byte
            u16 bgPatternAddr = 0x0000;
            // low byte pattern table shift register
            u16 patternShiftLo = 0x0000;
            // high byte pattern table shift register
            u16 patternShiftHi = 0x0000;
            // low byte attribute table shift register
            u16 attributeShiftLo = 0x0000;
            // high byte attribute table shift register
            u16 attributeShiftHi = 0x0000;

            // helper function for copying horizontal-scroll related bits from v to t
            inline void copyHorizontalBits() { if (renderingEnabled()) v = (v & ~0b0000010000011111) | (t & 0b0000010000011111); }
            // helper function for copying vertical-scroll related bits from v to t
            inline void copyVerticalBits() { if (renderingEnabled()) v = (v & ~0b0111101111100000) | (t & 0b0111101111100000); }
            // helper function to obtain the current Tile-wise horizontal-scroll
            inline u8 coarseX() const { return v & 0x1F; }
            // helper function to obtain the current Tile-wise vertical-scroll
            inline u8 coarseY() const { return ((v & 0x03E0) >> 5); }
            // helper function to obtain the current pixel-wise vertical-scroll
            inline u8 fineY() const { return ((v & 0x7000) >> 12); }

            // container for Primary OAM data; i.e. the pixel data for each of the 64 possible sprites within the game
            std::array<std::array<u8, 4>, 64> primaryOAM{ 0 };
            // container for Secondary OAM data; i.e. the pixel data for each of the 8 possible sprites on the next scanline
            std::array<std::array<u8, 4>, 8> secondaryOAM{ 0 };

            /*
                Sprite Attribute byte flags
                7..bit..0
                YXP- --pp
                |||    ++-> index into the current palette selections for sprite coloring
                ||+-------> whether this sprite has priority over the background (0 = higher priority)
                |+--------> whether this sprite should be horizontally flipped
                +---------> whether this sprite should be vertically flipped
            */
            enum SPRITE_ATTR {
                PALETTE,
                PRIORITY,
                XFLIP,
                YFLIP
            };
            // helper function for reading sprite attribute byte flags
            u8 readSpriteAttr(SPRITE_ATTR which, u8 attr);
            // returns the palette index of the current sprite
            inline u8 getSpritePalette(u8 attr) { return readSpriteAttr(PALETTE, attr); }
            // returns whether or not the current sprite should be rendered above the background
            inline bool spriteAboveBackground(u8 attr) { return readSpriteAttr(PRIORITY, attr) == 0; }
            // returns whether or not the current sprite should be h-flipped
            inline bool flipX(u8 attr) { return readSpriteAttr(XFLIP, attr) > 0; }
            // returns whether or not the current sprite should be v-flipped
            inline bool flipY(u8 attr) { return readSpriteAttr(YFLIP, attr) > 0; }

            // helper function to encompass the entirety of the NES PPU background pipeline
            void backgroundPipeline();

            // fetches the next nametable byte
            void fetchBGNametableByte();
            // fetches the next attribute byte
            void fetchBGAttributeByte();

            // fetches the next LOW pattern byte
            void fetchBGPatternByteLo();
            // fetches the next HIGH pattern byte
            void fetchBGPatternByteHi();

            // helper function to increment the Coarse X scroll value
            void incrementCoarseX();
            // helper function to increment the Fine Y scroll value
            void incrementFineY();

            // loads the background shift registers with the related previously fetched bytes
            void loadBackgroundShifters();
            // shifts the background shift registers to access the next relevant bit
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

            /*
                priority index of a sprite
                index of zero would cause 'sprite zero hit'
            */
            u8 spriteIndex = 0;
            // helper variable for sprite evaluation
            u8 n = 0x00;
            // helper variable for sprite evaluation
            u8 m = 0x00;
            // ???
            u8 oamLatch = 0x00;
            // current sprites evaluated to be present on the current scanline
            u8 spritesOnScanline = 0x00;
            // index into oam of the SPRITE being processed
            u8 oamIndex = 0x00;
            // index into the SPRITE being processed; i.e. y-pos/tile-index/attr/x-pos
            u8 byteIndex = 0x00;
            // address of the sprite's pattern tile
            u16 spritePatternAddr = 0x0000;

            // sprite's tile index (duh)
            u8 sprTileIndex = 0x00;
            // sprite's attribute byte
            u8 sprAttributes = 0x00;
            // sprite's x-pos value (duh)
            u8 sprXPosition = 0x00;
            // sprite's LOW pattern byte (duh)
            u8 sprPatternLo = 0x00;
            // sprite's HIGH pattern byte (duh)
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

            // container for the sprites to be placed on the CURRENT scanline
            vector<ActiveSprite> activeSprites;
            // container for the sprites to be placed on the NEXT scanline
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