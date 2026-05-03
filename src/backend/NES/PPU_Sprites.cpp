#include "PPU.h"

using namespace NES_NS;

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
    if (cycle % 2 == 0) { // 2, 4, 6, 8, ... 60, 62, 64
        u8 index = cycle / 2; // 1, 2, 3, 4, ... 30, 31, 32
        u8 sprite = (index - 1) / 4; // 0, 1, 2, 3, 4, 5, 6, 7
        u8 byte = (index - 1) % 4; // 0, 1, 2, 3

        secondaryOAM[sprite][byte] = 0xFF;
    }
}

void PPU::spriteEval() {
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
        u8 h = getSpriteHeight();
        u8 tgt = scanline + 1;
        u8 top = oamLatch + 1;

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

void PPU::calcSPRPatternAddr(u8 index, u8 id, u8 y) {
    u16 sprFineY = (scanline + 1) - y - 1;

    if (getSpriteHeight() == 8) {
        if (flipY(secondaryOAM[index][2]))
            sprFineY = 7 - sprFineY;

        spritePatternAddr = (
            getSpritePatternTableAddr8x8() +
            ((u16)id * 16) +
            sprFineY
            );
    } else {
        if (flipY(secondaryOAM[index][2]))
            sprFineY = 15 - sprFineY;

        u16 table = ((u16)id & 1) * 0x1000;
        u16 tileIndex = (u16)id & 0xFE;

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