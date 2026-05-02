#include "../Mappers/Mapper.h"
#include "NES_PPU.h"

uint8_t NES_PPU::read(uint16_t addr, bool readonly) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        addr &= 0x0007;

        switch (addr) {
            case 0x02: // PPUSTATUS
                ppuBus = (PPUSTATUS & 0xE0) | (ppuBus & 0x1F);
                if (!readonly) {
                    if (scanline == 241) {
                        if (cycle <= 1) suppressVBL = true; else suppressVBL = false;
                    }
                    inVBlank(false);
                    w = false;

                    nmiOutput = inVBlank() && getNMIEnabled();
                }
                break;
            case 0x04:
                ppuBus = readOAMByte(OAMADDR);
                break;
            case 0x07: // PPUDATA
                {
                    uint8_t ret = 0x00;

                    uint16_t addr = v & 0x3FFF;
                    uint8_t data = ppuRead(addr);

                    if (addr >= 0x3F00) {
                        ret = data;
                        dataBuffer = ppuRead(addr - 0x1000);
                    } else {
                        ret = dataBuffer;
                        dataBuffer = data;
                    }

                    v = (v + getVRAMIncrement()) & 0x3FFF;

                    ppuBus = ret;
                }
                break;
        }
    }

    return ppuBus;
}

void NES_PPU::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        ppuBus = data;
        addr &= 0x0007;

        switch (addr) {
            case 0x00:
                { // PPUCTRL
                    bool prevEnabled = getNMIEnabled();
                    PPUCTRL = data;
                    t = ((t & 0xF3FF) | ((uint16_t)(data & 0x03) << 10));

                    nmiOutput = inVBlank() && getNMIEnabled();

                    if (!nmiOutputPrev && !prevEnabled && nmiOutput)
                        nmiRequested = true;

                    nmiOutputPrev = nmiOutput;
                    return;
                }
            case 0x01: // PPUMASK
                PPUMASK = data;
                return;
            case 0x03: // OAMADDR
                OAMADDR = data;
                return;
            case 0x04: // OAMDATA
                writeOAMByte(OAMADDR, data);
                OAMADDR++;
                return;
            case 0x05: // PPUSCROLL
                PPUSCROLL = data;
                if (!w) { // first write
                    x = (data & 0x07);

                    t &= ~0x001F;
                    t |= ((uint16_t)(data >> 3) & 0x1F);

                    w = true;
                } else { // second write
                    t &= 0x0C1F;

                    t |= ((uint16_t)(data & 0x07) << 12);
                    t |= ((uint16_t)(data & 0xF8) << 2);

                    w = false;
                }
                return;
            case 0x06: // PPUADDR
                if (!w) { // first write
                    t &= 0x00FF;
                    t |= ((uint16_t)(data & 0x3F) << 8);

                    w = true;
                } else { // second write
                    t &= 0x7F00;
                    t |= ((uint16_t)data);

                    v = t & 0x3FFF;

                    w = false;
                }
                return;
            case 0x07: // PPUDATA
                PPUDATA = data;
                ppuWrite(v, PPUDATA);
                v = (v + getVRAMIncrement()) & 0x3FFF;
                return;
        }
    }
}

uint8_t NES_PPU::ppuRead(uint16_t addr, bool readonly) {
    if (addr >= 0x0000 && addr <= 0x1FFF)
        return cart->mapper->ppuRead(addr, readonly);
    else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        uint16_t A = addr & 0x03FF;
        uint16_t B = A + 0x0400;
        uint16_t C = B + 0x0400;
        uint16_t D = C + 0x0400;
        switch (cart->getMirror()) {
            case MIRROR::VERTICAL:
                if (addr <= 0x03FF) return nametables[A];
                if (addr <= 0x07FF) return nametables[B];
                if (addr <= 0x0BFF) return nametables[A];
                return nametables[B];
            case MIRROR::ONE_SCREEN_LO:
                return nametables[A];
            case MIRROR::ONE_SCREEN_HI:
                return nametables[B];
            case MIRROR::HORIZONTAL:
                if (addr <= 0x03FF) return nametables[A];
                if (addr <= 0x07FF) return nametables[A];
                if (addr <= 0x0BFF) return nametables[B];
                return nametables[B];
            case MIRROR::FOUR_SCREEN:
                if (addr <= 0x03FF) return nametables[A];
                if (addr <= 0x07FF) return nametables[B];
                if (addr <= 0x0BFF) return nametables[C];
                return nametables[D];
        }
    } else if (addr <= 0x3FFF) {
        addr &= 0x1F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;
        return palettes[addr];
    }

    return 0x00;
}

void NES_PPU::ppuWrite(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0x1FFF)
        cart->mapper->ppuWrite(addr, data);
    else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        uint16_t A = addr & 0x03FF;
        uint16_t B = A + 0x0400;
        uint16_t C = B + 0x0400;
        uint16_t D = C + 0x0400;
        switch (cart->getMirror()) {
            case MIRROR::VERTICAL:
                if (addr <= 0x03FF) {
                    nametables[A] = data;
                    return;
                }
                if (addr <= 0x07FF) {
                    nametables[B] = data;
                    return;
                }
                if (addr <= 0x0BFF) {
                    nametables[A] = data;
                    return;
                }
                nametables[B] = data;
                return;
            case MIRROR::ONE_SCREEN_LO:
                nametables[A] = data;
                return;
            case MIRROR::ONE_SCREEN_HI:
                nametables[B] = data;
                return;
            case MIRROR::HORIZONTAL:
                if (addr <= 0x03FF) {
                    nametables[A] = data;
                    return;
                }
                if (addr <= 0x07FF) {
                    nametables[A] = data;
                    return;
                }
                if (addr <= 0x0BFF) {
                    nametables[B] = data;
                    return;
                }
                nametables[B] = data;
                return;
            case MIRROR::FOUR_SCREEN:
                if (addr <= 0x03FF) {
                    nametables[A] = data;
                    return;
                }
                if (addr <= 0x07FF) {
                    nametables[B] = data;
                    return;
                }
                if (addr <= 0x0BFF) {
                    nametables[C] = data;
                    return;
                }
                nametables[D] = data;
                return;
        }
    } else if (addr <= 0x3FFF) {
        addr &= 0x1F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;
        palettes[addr] = data;
    }
}

void NES_PPU::clock() {
    if (cycle == 0) {
        activeSprites = nextSprites;
    }
    if (scanline == 261) { // we are on the pre-render scanline
        onPreRenderLine();
        // handle odd-frame cycle-skip here
        if (renderingEnabled() && cycle == 339 && oddFrame) {
            cycle++;
        }
    } else if (scanline >= 0 && scanline <= 239) { // we are on a visible scanline
        onVisibleLine();
    } else if (scanline == 241) { // we are on the first scanline within VBlank
        onStartVBlankLine();
    }

    // increment cycle count
    cycle++;

    if (cycle >= 341) { // if we have done all pixels on this, scanline...
        cycle = 0;
        scanline++; // ...increment scanline count

        if (scanline >= 262) { // if we have done all scanlines on this frame...
            scanline = 0;
            frameComplete = true; // ...mark frame as complete (so console can render it) and...
            oddFrame = !oddFrame; // ...alternate whether this frame # was odd or even
        }
    }
}

void NES_PPU::onPreRenderLine() {
    if (cycle == 1) { // if we're on cycle 1, we clear our status flags.
        inVBlank(false);
        spriteZeroHit(false);
        spritesOverflowed(false);
    }

    if (renderingEnabled()) {
        if ((cycle >= 2 && cycle <= 257) || (cycle >= 322 && cycle <= 337)) {
            shiftBackgroundShifters();
        }

        if ((cycle >= 1 && cycle <= 256) || (cycle >= 321 && cycle <= 336)) {
            backgroundPipeline();
        }

        if (cycle == 256) incrementFineY();
        if (cycle == 257) copyHorizontalBits();

        if (cycle >= 257 && cycle <= 320) {
            spriteFetch();
        }

        if (cycle >= 280 && cycle <= 304) {
            copyVerticalBits();
        }

        // dummy nametable fetch on 337
        // dummy nametable fetch on 339
    }
}

void NES_PPU::onVisibleLine() {
    if (renderingEnabled()) {
        if (cycle >= 1 && cycle <= 64) {
            initSecondaryOAM();
        }

        if (cycle >= 65 && cycle <= 256) {
            spriteEval();
        }

        if (cycle >= 257 && cycle <= 320) {
            spriteFetch();
        }

        if ((cycle >= 2 && cycle <= 257) || (cycle >= 322 && cycle <= 337)) {
            shiftBackgroundShifters();
        }

        if ((cycle >= 1 && cycle <= 256) || (cycle >= 321 && cycle <= 336)) {
            backgroundPipeline();
        }

        if (cycle == 256) incrementFineY();
        if (cycle == 257) copyHorizontalBits();

        if (cycle >= 1 && cycle <= 256) {
            renderPixel();
        }
    }
}

void NES_PPU::onStartVBlankLine() {
    if (cycle == 1 && !suppressVBL) {
        bool prev = inVBlank();
        inVBlank(true);
        if (!prev && inVBlank() && getNMIEnabled()) {
            nmiRequested = true;
        }
    } else suppressVBL = false;
}

void NES_PPU::backgroundPipeline() {
    switch (cycle % 8) {
        case 1:
            loadBackgroundShifters();
            fetchBGNametableByte();
            break;
        case 3:
            fetchBGAttributeByte();
            break;
        case 5:
            fetchBGPatternByteLo();
            break;
        case 7:
            fetchBGPatternByteHi();
            break;
        case 0:
            incrementCoarseX();
            break;
        default:
            break;
    }
}

void NES_PPU::incrementCoarseX() {
    if (renderingEnabled()) {
        if ((v & 0x001F) == 31) {
            v &= ~0x001F;
            v ^= 0x0400;
        } else {
            v++;
        }
    }
}

void NES_PPU::incrementFineY() {
    if (renderingEnabled()) {
        if ((v & 0x7000) != 0x7000) {
            v += 0x1000;
        } else {
            v &= ~0x7000;
            uint8_t y = coarseY();
            if (y == 29) {
                y = 0;
                v ^= 0x0800;
            } else if (y == 31) {
                y = 0;
            } else {
                y++;
            }
            v = (v & ~0x03E0) | ((uint16_t)y << 5);
        }
    }
}

void NES_PPU::shiftBackgroundShifters() {
    if (renderBackground()) {
        patternShiftLo <<= 1;
        patternShiftHi <<= 1;

        attributeShiftLo <<= 1;
        attributeShiftHi <<= 1;
    }
}

void NES_PPU::loadBackgroundShifters() {
    patternShiftLo = (patternShiftLo & 0xFF00) | nextPatternByteLo;
    patternShiftHi = (patternShiftHi & 0xFF00) | nextPatternByteHi;

    attributeShiftLo = (attributeShiftLo & 0xFF00) | ((nextAttributeByte & 0b01) ? 0xFF : 0x00);
    attributeShiftHi = (attributeShiftHi & 0xFF00) | ((nextAttributeByte & 0b10) ? 0xFF : 0x00);
}

void NES_PPU::fetchBGNametableByte() {
    uint16_t addr = (0x2000 | (v & 0x0FFF));
    nextNametableByte = ppuRead(addr, false);
}

void NES_PPU::fetchBGAttributeByte() {
    uint16_t addr = 0x23C0 |
        (v & 0x0C00) |
        ((v >> 4) & 0x38) |
        ((v >> 2) & 0x07);
    nextAttributeByte = ppuRead(addr, false);

    if (coarseY() & 0x02) nextAttributeByte >>= 4;
    if (coarseX() & 0x02) nextAttributeByte >>= 2;

    nextAttributeByte &= 0x03;
}

void NES_PPU::fetchBGPatternByteLo() {
    uint16_t addr = getBackgroundPatternTableAddr() |
        (((uint16_t)nextNametableByte) << 4) |
        ((uint16_t)fineY());
    nextPatternByteLo = ppuRead(addr, false);
}

void NES_PPU::fetchBGPatternByteHi() {
    uint16_t addr = getBackgroundPatternTableAddr() |
        (((uint16_t)nextNametableByte) << 4) |
        ((uint16_t)fineY());
    nextPatternByteHi = ppuRead(addr + 8, false);
}

void NES_PPU::getBackgroundPixel(uint8_t& pixel, uint8_t& attr) const {
    uint16_t mux = 0x8000 >> x;

    uint8_t p0 = (patternShiftLo & mux) > 0;
    uint8_t p1 = (patternShiftHi & mux) > 0;

    pixel = (p1 << 1) | p0;

    uint8_t a0 = (attributeShiftLo & mux) > 0;
    uint8_t a1 = (attributeShiftHi & mux) > 0;

    attr = (a1 << 1) | a0;
}

uint8_t NES_PPU::getSpritePixel(uint8_t& pixel, uint8_t& attr) {
    bool found = false;
    uint8_t tempPixel = 0x00;
    uint8_t tempAttr = 0x00;
    uint8_t tempIndex = 0xFF;

    for (uint8_t i = 0; i < activeSprites.size(); i++) {
        ActiveSprite* spr = &activeSprites[i];
        if (spr->xCounter > 0) {
            spr->xCounter--;
            continue;
        }

        uint8_t p = 0;

        if (flipX(spr->attr)) {
            uint8_t p0 = spr->patternLo & 0x01;
            uint8_t p1 = spr->patternHi & 0x01;

            p = (p1 << 1) | p0;

            spr->patternLo >>= 1;
            spr->patternHi >>= 1;
        } else {
            uint8_t p0 = (spr->patternLo & 0x80) >> 7;
            uint8_t p1 = (spr->patternHi & 0x80) >> 7;

            p = (p1 << 1) | p0;

            spr->patternLo <<= 1;
            spr->patternHi <<= 1;
        }

        if (p != 0 && !found) {
            tempPixel = p;
            tempAttr = spr->attr;
            tempIndex = i;
            found = true;
        }
    }

    pixel = tempPixel;
    attr = tempAttr;

    return tempIndex;
}

void NES_PPU::renderPixel() {
    uint16_t paletteAddr = 0x3F00;
    bool sprSource = false;

    uint8_t bgPixel = 0;
    uint8_t bgAttr = 0;
    uint8_t sprPixel = 0;
    uint8_t sprAttr = 0;
    uint8_t sprIndex = 0;
    if (renderingEnabled()) {
        getBackgroundPixel(bgPixel, bgAttr);
        sprIndex = getSpritePixel(sprPixel, sprAttr);
    }

    if (!renderBackground()) {
        bgPixel = 0;
        bgAttr = 0;
    }

    if (!renderSprites()) {
        sprPixel = 0;
        sprAttr = 0;
    }

    if ((cycle - 1) >= 0 && (cycle - 1) < 8) {
        if (!renderBackgroundLeft()) {
            bgPixel = 0;
            bgAttr = 0;
        }
        if (!renderSpritesLeft()) {
            sprPixel = 0;
            sprAttr = 0;
        }
    }

    uint8_t finalPixel;
    uint8_t finalAttr;

    if (bgPixel == 0 && sprPixel == 0) {
        finalPixel = 0;
        finalAttr = 0;
    } else if (bgPixel == 0) {
        finalPixel = sprPixel;
        finalAttr = sprAttr & 0x03;
        sprSource = true;
    } else if (sprPixel == 0) {
        finalPixel = bgPixel;
        finalAttr = bgAttr;
    } else {
        // handle sprite 0 hit
        if (sprIndex == 0 && cycle < 255)
            spriteZeroHit(true);

        if (spriteAboveBackground(sprAttr)) {
            finalPixel = sprPixel;
            finalAttr = sprAttr & 0x03;
            sprSource = true;
        } else {
            finalPixel = bgPixel;
            finalAttr = bgAttr;
        }
    }

    if (finalPixel != 0) paletteAddr += ((finalAttr << 2) + finalPixel);
    if (sprSource) paletteAddr += 0x10;

    uint8_t index = ppuRead(paletteAddr, false);

    frameBuffer[(size_t)scanline * 256 + ((size_t)cycle - 1)] = masterPalette[index & 0x3F];
}

uint8_t NES_PPU::readOAMByte(int i) const {
    if (i < 0 || i > 255) i = OAMADDR;

    uint8_t s = i / 4;
    uint8_t b = i % 4;

    return primaryOAM[s][b];
}

void NES_PPU::writeOAMByte(uint8_t i, uint8_t b) {
    uint8_t s = i / 4;
    uint8_t o = i % 4;

    primaryOAM[s][o] = b;
}

uint8_t NES_PPU::getControlData(CONTROL which) const {
    if (which == NAMETABLE_BASE) return PPUCTRL & 0x03;
    else return ((PPUCTRL >> which) & 0x01);
}

void NES_PPU::setStatusData(STATUS which, bool v) {
    if (v) {
        PPUSTATUS |= (0x01 << which);
    } else {
        PPUSTATUS &= (~(0x01 << which));
    }
}

void NES_PPU::initSecondaryOAM() {
    if (cycle % 2 == 0) { // 2, 4, 6, 8, ... 60, 62, 64
        uint8_t index = cycle / 2; // 1, 2, 3, 4, ... 30, 31, 32
        uint8_t sprite = (index - 1) / 4; // 0, 1, 2, 3, 4, 5, 6, 7
        uint8_t byte = (index - 1) % 4; // 0, 1, 2, 3

        secondaryOAM[sprite][byte] = 0xFF;
    }
}

void NES_PPU::spriteEval() {
    if (cycle == 65) {
        spriteIndex = 0;
        byteIndex = 0;
        spritesOnScanline = 0;
        nextSprites.clear();
        spritesOverflowed(false);
    }

    if (spriteIndex >= 64 || spritesOverflowed()) return;

    bool oddCycle = cycle % 2 == 1;

    // if odd cycle, read byte from primary OAM
    if (oddCycle) {
        oamLatch = primaryOAM[spriteIndex][0];
    } // if even cycle, evalute sprite via previously read byte
    else {
        // check if we're in range
        uint8_t h = getSpriteHeight();
        uint8_t tgt = scanline + 1;
        uint8_t top = oamLatch + 1;

        if (tgt >= top && tgt < (top + h)) {
            // sprite is in range; if we have room, copy data to sOAM
            if (spritesOnScanline < 8) {
                secondaryOAM[spritesOnScanline][0] = primaryOAM[spriteIndex][0];
                secondaryOAM[spritesOnScanline][1] = primaryOAM[spriteIndex][1];
                secondaryOAM[spritesOnScanline][2] = primaryOAM[spriteIndex][2];
                secondaryOAM[spritesOnScanline][3] = primaryOAM[spriteIndex][3];
                spritesOnScanline++;
            } else {
                spritesOverflowed(true);
            }
        }

        spriteIndex++;
    }
}

void NES_PPU::calcSPRPatternAddr(uint8_t index, uint8_t id, uint8_t y) {
    uint16_t sprFineY = (scanline + 1) - y - 1;

    if (getSpriteHeight() == 8) {
        if (flipY(secondaryOAM[index][2]))
            sprFineY = 7 - sprFineY;

        spritePatternAddr = (
            getSpritePatternTableAddr8x8() +
            ((uint16_t)id * 16) +
            sprFineY
            );
    } else {
        if (flipY(secondaryOAM[index][2]))
            sprFineY = 15 - sprFineY;

        uint16_t table = ((uint16_t)id & 1) * 0x1000;
        uint16_t tileIndex = (uint16_t)id & 0xFE;

        if (sprFineY >= 8) {
            tileIndex += 1;
            sprFineY -= 8;
        }
        spritePatternAddr = (table + (tileIndex * 16) + sprFineY);
    }
}

void NES_PPU::spriteFetch() {
    uint8_t sprite = (cycle - 257) / 8;

    uint8_t step = (cycle - 257) % 8;
    switch (step) {
        case 2:
            sprTileIndex = secondaryOAM[sprite][1];
            break;
        case 3:
            sprAttributes = secondaryOAM[sprite][2];
            break;
        case 4:
            sprXPosition = secondaryOAM[sprite][3];
            break;
        case 5:
            calcSPRPatternAddr(sprite, sprTileIndex, secondaryOAM[sprite][0]);
            sprPatternLo = ppuRead(spritePatternAddr, false);
            break;
        case 6:
            sprPatternHi = ppuRead(spritePatternAddr + 8, false);
            break;
        case 7:
            nextSprites.push_back(ActiveSprite(sprPatternLo, sprPatternHi, sprAttributes, sprXPosition));
            break;

    }
}

void NES_PPU::reset() {
    oddFrame = false;
    w = false;
    PPUSCROLL = 0x00;
    PPUCTRL = 0x00;
    PPUMASK = 0x00;
    PPUDATA = 0x00;
    scanline = 0;
    cycle = 0;
    v = 0x0000;
    t = 0x0000;
    dataBuffer = 0x00;
    OAMADDR = 0x00;
    spriteIndex = 0;
    byteIndex = 0;
    spritesOnScanline = 0;
    suppressVBL = false;
    suppressNMI = false;
    nmiRequested = false;
    nmiOutput = false;
    nmiOutputPrev = false;
    activeSprites.clear();
    nextSprites.clear();
    for (auto& a : primaryOAM) {
        a.fill(0xFF);
    }
    for (auto& a : secondaryOAM) {
        a.fill(0xFF);
    }
}

uint32_t NES_PPU::applyEmphasis(uint32_t color) const {
    uint32_t r = (color >> 0) & 0xFF;
    uint32_t g = (color >> 8) & 0xFF;
    uint32_t b = (color >> 16) & 0xFF;

    if (this->getGreyscale())
        r = g = b = (uint32_t)std::floor((r + g + b) / 3);

    bool eR = this->getMaskData(EMPHASIZE_RED);
    bool eG = this->getMaskData(EMPHASIZE_GREEN);
    bool eB = this->getMaskData(EMPHASIZE_BLUE);

    if (eR || eG || eB) {
        bool eA = eR && eG && eB;

        if (eA || !eR)
            r = (uint32_t)std::floor(r * 0.75);
        if (eA || !eG)
            g = (uint32_t)std::floor(g * 0.75);
        if (eA || !eB)
            b = (uint32_t)std::floor(b * 0.75);
    }

    return 0xFF000000 | (r << 0) | (g << 8) | (b << 16);
}