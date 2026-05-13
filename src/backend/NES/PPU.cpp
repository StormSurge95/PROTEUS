#include "./Mappers/Mapper.h"
#include "./PPU.h"

using namespace NES_NS;

u8 PPU::read(u16 addr, bool readonly) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        addr &= 0x0007;

        switch (addr) {
            case 0x02: // PPUSTATUS
                ppuBus = (PPUSTATUS & 0xE0) | (ppuBus & 0x1F);
                if (!readonly) {
                    w = false;
                    if (scanline == 241) {
                        if (cycle <= 1) {
                            suppressVBL = true;
                            suppressNMI = false;
                        } else if (cycle == 2) {
                            suppressVBL = false;
                            suppressNMI = true;
                        } else {
                            suppressVBL = suppressNMI = false;
                        }
                    }
                    inVBlank(false);
                    updateCounters(0xE0);
                    nmiOutput = inVBlank() && getNMIEnabled();
                }
                break;
            case 0x04:
                ppuBus = readOAMByte(OAMADDR);
                if (!readonly) updateCounters(0xFF);
                break;
            case 0x07: // PPUDATA
                {
                    u8 ret = 0x00;

                    u16 addr = v & 0x3FFF;
                    u8 data = ppuRead(addr);

                    if (addr >= 0x3F00) {
                        ret = data;
                        if (!readonly) dataBuffer = ppuRead(addr - 0x1000);
                    } else {
                        ret = dataBuffer;
                        if (!readonly) dataBuffer = data;
                    }
                    if (readonly) return ret;
                    v = (v + getVRAMIncrement()) & 0x3FFF;
                    ppuBus = ret;
                    updateCounters(0xFF);
                }
                break;
        }
    }

    return ppuBus;
}

void PPU::write(u16 addr, u8 data) {
    if (addr >= 0x2000 && addr <= 0x3FFF) {
        ppuBus = data;
        updateCounters(0xFF);
        addr &= 0x0007;

        switch (addr) {
            case 0x00:
                { // PPUCTRL
                    bool prevEnabled = getNMIEnabled();
                    PPUCTRL = data;
                    t = ((t & 0xF3FF) | ((u16)(data & 0x03) << 10));

                    if (inVBlank() && getNMIEnabled())
                        nmiRequested = true;

                    //nmiOutput = inVBlank() && getNMIEnabled();
                    //if (!nmiOutputPrev && !prevEnabled && nmiOutput)
                    //    nmiRequested = true;

                    //nmiOutputPrev = nmiOutput;
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
                    t |= ((u16)(data >> 3) & 0x1F);

                    w = true;
                } else { // second write
                    t &= 0x0C1F;

                    t |= ((u16)(data & 0x07) << 12);
                    t |= ((u16)(data & 0xF8) << 2);

                    w = false;
                }
                return;
            case 0x06: // PPUADDR
                if (!w) { // first write
                    t &= 0x00FF;
                    t |= ((u16)(data & 0x3F) << 8);

                    w = true;
                } else { // second write
                    t &= 0x7F00;
                    t |= ((u16)data);

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

u8 PPU::ppuRead(u16 addr, bool readonly) {
    addr &= 0x3FFF;
    u8 ret = 0x00;
    
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        ret = cart.lock()->mapper->ppuRead(addr, readonly);
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        u16 A = addr & 0x03FF;
        u16 B = A + 0x0400;
        u16 C = B + 0x0400;
        u16 D = C + 0x0400;
        switch (cart.lock()->getMirror()) {
            case MIRROR::VERTICAL:
                if (addr <= 0x03FF) ret = nametables[A];
                else if (addr <= 0x07FF) ret = nametables[B];
                else if (addr <= 0x0BFF) ret = nametables[A];
                else ret = nametables[B];
                break;
            case MIRROR::ONE_SCREEN_LO:
                ret = nametables[A];
                break;
            case MIRROR::ONE_SCREEN_HI:
                ret = nametables[B];
                break;
            case MIRROR::HORIZONTAL:
                if (addr <= 0x03FF) ret = nametables[A];
                else if (addr <= 0x07FF) ret = nametables[A];
                else if (addr <= 0x0BFF) ret = nametables[B];
                else ret = nametables[B];
                break;
            case MIRROR::FOUR_SCREEN:
                if (addr <= 0x03FF) ret = nametables[A];
                else if (addr <= 0x07FF) ret = nametables[B];
                else if (addr <= 0x0BFF) ret = nametables[C];
                else ret = nametables[D];
                break;
        }
    } else {
        addr &= 0x1F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;

        u8 p = (palettes[addr] & 0x3F) | (ppuBus & ~0x3F);

        if (getGreyscale()) p &= 0xF0;

        ret = p;
    }

    return ret;
}

void PPU::ppuWrite(u16 addr, u8 data) {
    if (addr >= 0x0000 && addr <= 0x1FFF)
        cart.lock()->mapper->ppuWrite(addr, data);
    else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        u16 A = addr & 0x03FF;
        u16 B = A + 0x0400;
        u16 C = B + 0x0400;
        u16 D = C + 0x0400;
        switch (cart.lock()->getMirror()) {
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

void PPU::clock() {
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
            for (int x = 0; x < 8; x++) {
                decayCounters[x]--;
                if (decayCounters[x] == 0) ppuBus &= ~(0x01 << x);
            }
        }
    }
}

void PPU::onPreRenderLine() {
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

void PPU::onVisibleLine() {
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

void PPU::onStartVBlankLine() {
    if (cycle == 1) {
        if (!suppressVBL) {
            bool prev = inVBlank();
            inVBlank(true);
            if (!prev && inVBlank() && getNMIEnabled() && !suppressNMI) {
                nmiRequested = true;
            } else suppressNMI = false;
        }
        suppressVBL = false;
    }
}

void PPU::incrementCoarseX() {
    if (renderingEnabled()) {
        if ((v & 0x001F) == 31) {
            v &= ~0x001F;
            v ^= 0x0400;
        } else {
            v++;
        }
    }
}

void PPU::incrementFineY() {
    if (renderingEnabled()) {
        if ((v & 0x7000) != 0x7000) {
            v += 0x1000;
        } else {
            v &= ~0x7000;
            u8 y = coarseY();
            if (y == 29) {
                y = 0;
                v ^= 0x0800;
            } else if (y == 31) {
                y = 0;
            } else {
                y++;
            }
            v = (v & ~0x03E0) | ((u16)y << 5);
        }
    }
}

void PPU::renderPixel() {
    u16 paletteAddr = 0x3F00;
    bool sprSource = false;

    u8 bgPixel = 0;
    u8 bgAttr = 0;
    u8 sprPixel = 0;
    u8 sprAttr = 0;
    u8 sprIndex = 0;
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

    u8 finalPixel;
    u8 finalAttr;

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

    u8 index = ppuRead(paletteAddr, false);

    frameBuffer[(size_t)scanline * 256 + ((size_t)cycle - 1)] = masterPalette[index & 0x3F];
}

u8 PPU::getControlData(CONTROL which) const {
    if (which == NAMETABLE_BASE) return PPUCTRL & 0x03;
    else return ((PPUCTRL >> which) & 0x01);
}

void PPU::setStatusData(STATUS which, bool v) {
    if (v) {
        PPUSTATUS |= (0x01 << which);
    } else {
        PPUSTATUS &= (~(0x01 << which));
    }
}

void PPU::reset() {
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

u32 PPU::applyEmphasis(u32 color) const {
    u32 r = (color >> 0) & 0xFF;
    u32 g = (color >> 8) & 0xFF;
    u32 b = (color >> 16) & 0xFF;

    if (this->getGreyscale())
        r = g = b = (u32)std::floor((r + g + b) / 3);

    bool eR = this->getMaskData(EMPHASIZE_RED);
    bool eG = this->getMaskData(EMPHASIZE_GREEN);
    bool eB = this->getMaskData(EMPHASIZE_BLUE);

    if (eR || eG || eB) {
        bool eA = eR && eG && eB;

        if (eA || !eR)
            r = (u32)std::floor(r * 0.75);
        if (eA || !eG)
            g = (u32)std::floor(g * 0.75);
        if (eA || !eB)
            b = (u32)std::floor(b * 0.75);
    }

    return 0xFF000000 | (r << 0) | (g << 8) | (b << 16);
}

void PPU::updateCounters(u8 bits) {
    for (int x = 0; x < 8; x++) {
        if (((bits >> x) & 0x01) > 0) decayCounters[x] = 20;
    }
}