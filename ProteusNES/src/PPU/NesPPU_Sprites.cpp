#include "NesPPU.h"
#include "../PAK/Mappers/NesMapper.h"

using namespace NS_NES;

u8 PPU::getSpritePixel(u8& pixel, u8& attr) {
    bool found = false;
    u8 tempPixel = 0x00;
    u8 tempAttr = 0x00;
    u8 tempIndex = 0xFF;

    for (u8 i = 0; i < activeSprites.size(); i++) {
        ActiveSprite* spr = &activeSprites[i];
        if (spr->xCounter > 0) {
            spr->xCounter--;
            continue;
        }

        u8 p = 0;

        if (flipX(spr->attr)) {
            u8 p0 = spr->patternLo & 0x01;
            u8 p1 = spr->patternHi & 0x01;

            p = (p1 << 1) | p0;

            spr->patternLo >>= 1;
            spr->patternHi >>= 1;
        } else {
            u8 p0 = (spr->patternLo & 0x80) >> 7;
            u8 p1 = (spr->patternHi & 0x80) >> 7;

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

u8 PPU::readOAMByte(int i) const {
    if (i < 0 || i > 255) i = OAMADDR;

    u8 s = i / 4;
    u8 b = i % 4;

    return primaryOAM[s][b];
}

void PPU::writeOAMByte(u8 i, u8 b) {
    u8 s = i / 4;
    u8 o = i % 4;

    primaryOAM[s][o] = b;
}

void PPU::initSecondaryOAM() {
    // called from cycle 1 to 64 (inclusive)
    // cycle - 1 gives us indexes 0-63 (inclusive)
    // index / 8 gives us sprites 0-7
    // rather than doing one byte per cycle and missing one byte from
    // each sprite, this allows us to 'clear' the index byte as well
    u8 index = (cycle - 1);
    u8 sprite = index / 8;
    if (index % 8 == 0)
        secondaryOAM[sprite].fill(0xFF);
}

void PPU::beginSpriteEval() {
    n = 0;
    m = 0;
    byteIndex = 0;
    spritesOnScanline = 0;
    evalMode = EvalMode::SearchY;
    spritesOverflowed(false);
}

bool PPU::spriteInRange(u8 y) const {
    u16 tgt = scanline + 1;
    u16 top = u16(y) + 1;
    return (tgt >= top && tgt < (top + getSpriteHeight()));
}

void PPU::spriteEvalRead() {
    switch (evalMode) {
        case EvalMode::SearchY:
            oamLatch = primaryOAM[n][0];
            break;
        case EvalMode::CopyBytes:
            oamLatch = primaryOAM[n][byteIndex];
            break;
        case EvalMode::OverflowScan:
            oamLatch = primaryOAM[n][m];
        case EvalMode::Done:
        default: return;
    }
}

void PPU::advanceOverflowMiss() {
    n++;
    m = (m + 1) & 3;
}

void PPU::advanceOverflowHit() {
    m++;
    if (m >= 4) {
        m = 0;
        n++;
    }
}

void PPU::spriteEvalWrite() {
    switch (evalMode) {
        case EvalMode::SearchY:
            if (spriteInRange(oamLatch)) {
                secondaryOAM[spritesOnScanline][0] = oamLatch;
                if (n == 0) sprite0HitOnNextScanline = true;
                byteIndex = 1;
                evalMode = EvalMode::CopyBytes;
            } else {
                n++;
                if (n >= 64) evalMode = EvalMode::Done;
            }
            break;
        case EvalMode::CopyBytes:
            secondaryOAM[spritesOnScanline][byteIndex] = oamLatch;
            if (byteIndex < 3) byteIndex++;
            else {
                secondaryOAM[spritesOnScanline][4] = n;
                spritesOnScanline++;
                n++;
                byteIndex = 0;
            }
            if (n >= 64) evalMode = EvalMode::Done;
            else if (spritesOnScanline < 8) evalMode = EvalMode::SearchY;
            else {
                m = 0;
                evalMode = EvalMode::OverflowScan;
            }
            break;
        case EvalMode::OverflowScan:
            if (spriteInRange(oamLatch)) {
                spritesOverflowed(true);
                advanceOverflowHit();
            } else {
                advanceOverflowMiss();
            }
            if (n >= 64) evalMode = EvalMode::Done;
            break;
        case EvalMode::Done:
        default: return;
    }
}

void PPU::spriteEval() {
    if (cycle == 65) beginSpriteEval();

    if (evalMode == EvalMode::Done) return;

    bool oddCycle = cycle & 0x01;
    if (oddCycle)
        spriteEvalRead();
    else
        spriteEvalWrite();
}

void PPU::calcSPRPatternAddr(u8 index, u8 id, u8 y) {
    u16 sprFineY = scanline - y;

    if (getSpriteHeight() == 8) {
        if (flipY(secondaryOAM[index][2]))
            sprFineY = 7 - sprFineY;

        spritePatternAddr =
            getSpritePatternTableAddr8x8() +
            (u16(id) * 16) +
            sprFineY;
    } else {
        if (flipY(secondaryOAM[index][2]))
            sprFineY = 15 - sprFineY;

        u16 table = (id & 0x01) * 0x1000;
        u16 tileIndex = id & 0xFE;

        if (sprFineY >= 8) {
            tileIndex += 1;
            sprFineY -= 8;
        }
        spritePatternAddr = (table + (tileIndex * 16) + sprFineY);
    }
}

void PPU::spriteFetch() {
    u8 sprite = (cycle - 257) / 8;

    u8 step = (cycle - 257) % 8;
    switch (step) {
        case 0:
        case 1:
            ppuRead((0x2000 | (v & 0x0FFF)), false);
            break;
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
            if (secondaryOAM[sprite][4] == 0xFF) {
                if (getSpriteHeight() == 8) {
                    spritePatternAddr = getSpritePatternTableAddr8x8() + (0xFF * 16);
                } else {
                    spritePatternAddr = (((0xFF & 1) ? 0x1000 : 0x0000) | ((0xFF & ~1) << 4));
                }
            } else {
                calcSPRPatternAddr(sprite, sprTileIndex, secondaryOAM[sprite][0]);
            }
            sprPatternLo = ppuRead(spritePatternAddr, false);
            break;
        case 6:
            sprPatternHi = ppuRead(spritePatternAddr + 8, false);
            break;
        case 7:
            activeSprites[sprite] = ActiveSprite(sprPatternLo, sprPatternHi, sprAttributes, sprXPosition, secondaryOAM[sprite][4]);
            break;

    }

    if (cycle == 320) {
        sprite0HitOnThisScanline = sprite0HitOnNextScanline;
        sprite0HitOnNextScanline = false;
    }
}