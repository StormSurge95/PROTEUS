#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "../../../core/IDevice.h"
#include "../NES_CART.h"
#include "../NES_CPU/NES_CPU.h"

struct SpriteUnit {
    uint8_t y = 0x00;
    uint8_t tile = 0x00;
    uint8_t attr = 0x00;
    uint8_t x = 0x00;

    uint8_t patternLo = 0x00;
    uint8_t patternHi = 0x00;

    uint8_t xCounter = 0x00;
};

class NES_PPU : public IDevice<uint8_t, uint16_t> {
    public:
        uint16_t scanline = 0;
        uint16_t cycle = 0;
        bool nmiRequested = false;
        bool frameComplete = false;

        NES_PPU() = default;
        ~NES_PPU() = default;

        void clock();
        void reset();

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        uint8_t readOAMByte(int i = -1) const;
        void writeOAMByte(uint8_t i, uint8_t b);

        const uint32_t* getFrameBuffer() const { return frameBuffer.data(); }
        void connectCART(std::shared_ptr<NES_CART> cart) { this->cart = cart; }
        void connectCPU(std::shared_ptr<NES_CPU> cpu) { this->cpu = cpu; }
    private:
        bool nmiOutput = false;
        bool nmiOutputPrev = false;
        bool isRendering = false;
        bool oddFrame = false;
        bool initComplete = false;
        bool suppressVBL = false;
        bool suppressNMI = false;
        uint64_t frameCounter = 0;

        uint8_t ppuRead(uint16_t addr, bool readonly = false);
        void ppuWrite(uint16_t addr, uint8_t data);

        std::shared_ptr<NES_CART> cart = nullptr;
        std::shared_ptr<NES_CPU> cpu = nullptr;
        std::array<uint32_t, 61440> frameBuffer{ 0 };
        std::array<uint32_t, 64> masterPalette = {
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
        std::array<uint8_t, 4096> nametables{ 0 };
        std::array<uint8_t, 32> palettes{ 0 };

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
        uint8_t getControlData(CONTROL which) const;
        inline uint8_t getNametableBase() const { return 0x2000 + (this->getControlData(NAMETABLE_BASE) * 0x400); }
        inline uint8_t getVRAMIncrement() const { return (this->getControlData(VRAM_INCREMENT) ? 32 : 1); }
        inline uint16_t getSpritePatternTableAddr8x8() const { return (this->getControlData(SPRITE_PATTERN_ADDR) ? 0x1000 : 0x0000); }
        inline uint16_t getBackgroundPatternTableAddr() const { return (this->getControlData(BACKGROUND_PATTERN_ADDR) ? 0x1000 : 0x0000); }
        inline uint8_t getSpriteHeight() const { return (this->getControlData(SPRITE_SIZE) ? 16 : 8); }
        inline bool getNMIEnabled() const { return !!(this->getControlData(NMI_ENABLED)); }

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
        uint8_t getMaskData(MASK which) const { return ((this->PPUMASK >> which) & 0x01); }
        inline bool getGreyscale() const { return !!(this->getMaskData(GREYSCALE)); }
        inline bool renderBackgroundLeft() const { return !!(this->getMaskData(ENABLE_BACKGROUND_LEFT)); }
        inline bool renderSpritesLeft() const { return !!(this->getMaskData(ENABLE_SPRITES_LEFT)); }
        inline bool renderBackground() const { return !!(this->getMaskData(ENABLE_BACKGROUND)); }
        inline bool renderSprites() const { return !!(this->getMaskData(ENABLE_SPRITES)); }
        uint32_t applyEmphasis(uint32_t color) const;

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
        bool getStatusData(STATUS which) const { return ((this->PPUSTATUS >> which) & 0x01); }
        void setStatusData(STATUS which, bool v);
        inline bool spritesOverflowed() const { return this->getStatusData(SPRITE_OVERFLOW); }
        inline void spritesOverflowed(bool v) { this->setStatusData(SPRITE_OVERFLOW, v); }
        inline bool spriteZeroHit() const { return this->getStatusData(SPRITE_ZERO_HIT); }
        inline void spriteZeroHit(bool v) { this->setStatusData(SPRITE_ZERO_HIT, v); }
        inline bool inVBlank() const { return this->getStatusData(VBLANK); }
        inline void inVBlank(bool v) { this->setStatusData(VBLANK, v); }

        // REGISTERS
        uint8_t PPUCTRL = 0x00;     // $2000 write
        uint8_t PPUMASK = 0x00;     // $2001 write
        uint8_t PPUSTATUS = 0xA0;   // $2002 read
        uint8_t OAMADDR = 0x00;     // $2003 write
        uint8_t OAMDATA = 0x00;     // $2004 read/write
        uint8_t PPUSCROLL = 0x00;   // $2005 write
        uint8_t PPUADDR = 0x00;     // $2006 write
        uint8_t PPUDATA = 0x00;     // $2007 read/write
        uint8_t OAMDMA = 0x00;      // $4014 write
        uint8_t dataBuffer = 0x00;
        uint8_t ppuBus = 0x00;

        uint16_t v = 0x0000;  // during rendering, used for scroll position; outside rendering, used as current VRAM address
        uint16_t t = 0x0000;  // during rendering, specifies starting coarse-x scroll for next scanline and starting y scroll for screen; outside rendering, holds scroll or VRAM before transferring it to v
        uint8_t x = 0x00;  // fine-x position of current scroll, used during rendering alongside v
        bool w = false; // write-latch for PPUSCROLL/PPUADDR; clears on read os PPUSTATUS

        inline bool renderingEnabled() const { return ((this->renderBackground() || this->renderSprites()) && !this->inVBlank()); }

        uint8_t nextNametableByte = 0x00;
        uint8_t nextAttributeByte = 0x00;
        uint8_t nextPatternByteLo = 0x00;
        uint8_t nextPatternByteHi = 0x00;

        uint16_t bgPatternAddr = 0x0000;
        uint16_t patternShiftLo = 0x0000;
        uint16_t patternShiftHi = 0x0000;
        uint16_t attributeShiftLo = 0x0000;
        uint16_t attributeShiftHi = 0x0000;

        inline void copyHorizontalBits() { if (this->renderingEnabled()) this->v = (this->v & ~0b0000010000011111) | (this->t & 0b0000010000011111); }
        inline void copyVerticalBits() { if (this->renderingEnabled()) this->v = (this->v & ~0b0111101111100000) | (this->t & 0b0111101111100000); }
        inline uint8_t coarseX() const { return this->v & 0x1F; }
        inline uint8_t coarseY() const { return ((this->v & 0x03E0) >> 5); }
        inline uint8_t fineY() const { return ((this->v & 0x7000) >> 12); }

        uint8_t primaryOAM[64][4] = { 0 };  // max of 64 sprites per game
        uint8_t secondaryOAM[8][4] = { 0 }; // max of 8 sprites per scanline

        enum SPRITE_ATTR {
            PALETTE,
            PRIORITY,
            XFLIP,
            YFLIP
        };
        inline uint8_t readSpriteAttr(SPRITE_ATTR which, uint8_t attr) {
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
        }
        inline uint8_t getSpritePalette(uint8_t attr) { return this->readSpriteAttr(PALETTE, attr); }
        inline bool spriteAboveBackground(uint8_t attr) { return this->readSpriteAttr(PRIORITY, attr) == 0; }
        inline bool flipX(uint8_t attr) { return this->readSpriteAttr(XFLIP, attr) > 0; }
        inline bool flipY(uint8_t attr) { return this->readSpriteAttr(YFLIP, attr) > 0; }

        void backgroundPipeline();

        void fetchBGNametableByte();
        void fetchBGAttributeByte();

        void fetchBGPatternByteLo();
        void fetchBGPatternByteHi();

        void incrementCoarseX();
        void incrementFineY();

        void loadBackgroundShifters();

        void shiftBackgroundShifters();

        void getBackgroundPixel(uint8_t& pixel, uint8_t& attr) const;
        void renderPixel();

        void initSecondaryOAM();

        uint8_t spriteIndex = 0;
        uint8_t n = 0x00;
        uint8_t m = 0x00;
        uint8_t oamLatch = 0x00;
        uint8_t spritesOnScanline = 0x00;
        uint8_t oamIndex = 0x00;
        uint8_t byteIndex = 0x00;
        bool evaluating = false;
        bool spriteInRange = false;
        bool copying = false;
        bool sprite0InRange = false;
        bool overflow = false;
        bool allSpriteEvalsComplete = false;
        uint16_t spritePatternAddr = 0x0000;

        uint8_t sprTileIndex = 0x00;
        uint8_t sprAttributes = 0x00;
        uint8_t sprXPosition = 0x00;
        uint8_t sprPatternLo = 0x00;
        uint8_t sprPatternHi = 0x00;

        struct ActiveSprite {
            uint8_t patternLo = 0x00;
            uint8_t patternHi = 0x00;
            uint8_t attr = 0x00;
            uint8_t xCounter = 0x00;

            ActiveSprite() = default;
            ActiveSprite(uint8_t pl, uint8_t ph, uint8_t a, uint8_t x) :
                patternLo(pl), patternHi(ph), attr(a), xCounter(x) {}
        };

        std::vector<ActiveSprite> activeSprites;
        std::vector<ActiveSprite> nextSprites;

        void spriteEval();
        void spriteFetch();

        void calcSPRPatternAddr(uint8_t index, uint8_t id, uint8_t y);

        uint8_t getSpritePixel(uint8_t& pixel, uint8_t& attr);
};