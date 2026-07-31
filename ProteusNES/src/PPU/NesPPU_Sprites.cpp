#include "NesPPU.h"
#include "../PAK/Mappers/NesMapper.h"
#include "../shared/NesProfiles.h"

using namespace NS_NES;

u8 PPU::getSpritePixel(u8& pixel, u8& attr) {
    bool found = false;
    u8 tempPixel = 0x00;
    u8 tempAttr = 0x00;
    u8 tempIndex = 0xFF;

    for (u8 i = 0; i < activeSprites.size(); i++) {
        ActiveSprite* spr = &activeSprites[i];
        if (spr->xCounter > 0) {
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

    u8 data = primaryOAM[s][b];

    if (b == 2) data &= 0xE3;

    return data;
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

    // nesdev says that the internal OAM bus is forced to $FF during dots 1-64
    oamLatch = 0xFF;

    oamAddr2 = static_cast<u8>((cycle - 1) >> 1);

    // decide whether we even need to run during this cycle
    u8 index = (cycle - 1);
    if (index % 8 != 0) return;

    // we need to run; decide which sprite's OAM data to clear
    u8 sprite = index / 8;
    secondaryOAM[sprite].fill(0xFF);
}

void PPU::beginSpriteEval() {
    n = OAMADDR >> 2;
    m = OAMADDR & 0x03;
    byteIndex = 0;
    spritesOnScanline = 0;
    oamAddr2 = 0;
    evalMode = EvalMode::SearchY;
}

bool PPU::spriteInRange(u8 y) const {
    u16 tgt = static_cast<u8>(scanline);
    u16 top = u16(y);
    return (tgt >= top && tgt < (top + getSpriteHeight()));
}

void PPU::spriteEvalRead() {
    const u8 evalAddr = static_cast<u8>((n << 2) | m);

    switch (evalMode) {
        case EvalMode::SearchY:
            // oamLatch = primaryOAM[n][0];
            oamLatch = readOAMByte(evalAddr);
            return;
        case EvalMode::CopyBytes:
            // oamLatch = primaryOAM[n][byteIndex];
            oamLatch = readOAMByte(
                static_cast<u8>(evalAddr + byteIndex)
            );
            return;
        case EvalMode::OverflowScan:
            //oamLatch = primaryOAM[n][m];
            oamLatch = readOAMByte(evalAddr);
            return;
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
        case EvalMode::SearchY: {
            if (spriteInRange(oamLatch)) {
                secondaryOAM[spritesOnScanline][0] = oamLatch;
                oamAddr2 = static_cast<u8>((oamAddr2 + 1) & 0x1F);
                // "cycle == 66" refers to the first `spriteEvalWrite`
                // cycle after beginning sprite evaluation; and, therefore,
                // refers to the processing of the first sprite within secondary OAM.
                if (cycle == 66) sprite0HitOnNextScanline = true;
                byteIndex = 1;
                evalMode = EvalMode::CopyBytes;
            } else {
                n++;
                if (n >= 64) evalMode = EvalMode::Done;
            }
            break;
        }
        case EvalMode::CopyBytes:
            secondaryOAM[spritesOnScanline][byteIndex] = oamLatch;
            oamAddr2 = static_cast<u8>((oamAddr2 + 1) & 0x1F);
            if (byteIndex < 3) byteIndex++;
            else {
                secondaryOAM[spritesOnScanline][4] = n;
                spritesOnScanline++;
                n++;
                byteIndex = 0;
                if (n >= 64) evalMode = EvalMode::Done;
                else if (spritesOnScanline < 8) evalMode = EvalMode::SearchY;
                else {
                    m = 0;
                    evalMode = EvalMode::OverflowScan;
                }
            }
            break;
        case EvalMode::OverflowScan:
            if (spriteInRange(oamLatch)) {
                if (!spritesOverflowed() && !pendingSOS) pendingSOS = true;
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
    if (cycle == 65) {
        beginSpriteEval();
        sprite0HitOnNextScanline = false;
    }

    if (evalMode == EvalMode::Done) return;

    if (cycle & 0x01)
        spriteEvalRead();
    else
        spriteEvalWrite();
}

void PPU::calcSPRPatternAddr(u8 index, u8 id, u8 y) {
    u16 sprFineY = static_cast<u8>(scanline) - y;

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
    // "OAMADDR is set to 0 during each of ticks 257—320 (the sprite tile loading interval) of the pre-render and visible scanlines."
    OAMADDR = 0x00;

    if (cycle == 257) {
        if (logSL0 && scanline == GetScanlinesPerFrame(*region) - 1) {
            printf(
                "[SL0] PRE257 next=%u this=%u "
                "sec=%02X,%02X,%02X,%02X idx=%02X\n",
                sprite0HitOnNextScanline,
                sprite0HitOnThisScanline,
                secondaryOAM[0][0],
                secondaryOAM[0][1],
                secondaryOAM[0][2],
                secondaryOAM[0][3],
                secondaryOAM[0][4]
            );
        }

        oamAddr2 = 0;

        if (scanline != GetScanlinesPerFrame(*region) - 1) {
            sprite0HitOnThisScanline = sprite0HitOnNextScanline;
            sprite0HitOnNextScanline = false;
        }
    }

    u8 sprite = (cycle - 257) / 8;
    u8 step = (cycle - 257) % 8;

    const u8 oamByte = step < 4 ? step : 3;

    oamLatch = secondaryOAM[sprite][oamByte];

    if (oamByte == 2) oamLatch &= 0xE3;

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
            sprFetchValid = secondaryOAM[sprite][4] != 0xFF && spriteInRange(secondaryOAM[sprite][0]);

            if (!sprFetchValid) {
                if (getSpriteHeight() == 8) {
                    spritePatternAddr = getSpritePatternTableAddr8x8() + (0xFF * 16);
                } else {
                    spritePatternAddr = (((0xFF & 0x01) ? 0x1000 : 0x0000) | ((0xFF & ~0x01) << 4));
                }
            } else {
                calcSPRPatternAddr(
                    sprite,
                    sprTileIndex,
                    secondaryOAM[sprite][0]
                );
            }

            sprPatternLo = ppuRead(spritePatternAddr, false);

            if (logSL0 && scanline == GetScanlinesPerFrame(*region) - 1 && sprite == 0) {
                printf(
                    "[SL0] FETCH valid=%u line=%u y=%02X fine=%u "
                    "tile=%02X addr=%04X lo=%02X next=%u this=%u\n",
                    sprFetchValid,
                    static_cast<u8>(scanline),
                    secondaryOAM[0][0],
                    static_cast<u8>(scanline) - secondaryOAM[0][0],
                    sprTileIndex,
                    spritePatternAddr,
                    sprPatternLo,
                    sprite0HitOnNextScanline,
                    sprite0HitOnThisScanline
                );
            }
            break;
        case 6:
            sprPatternHi = ppuRead(spritePatternAddr + 8, false);
            break;
        case 7:
            activeSprites[sprite] = ActiveSprite(
                sprFetchValid ? sprPatternLo : 0x00,
                sprFetchValid ? sprPatternHi : 0x00,
                sprAttributes,
                sprXPosition,
                sprFetchValid ? secondaryOAM[sprite][4] : 0xFF
            );
            break;

    }

    if (step <= 2 || step == 7) oamAddr2 = static_cast<u8>((oamAddr2 + 1) & 0x1F);
}

void PPU::applyOamCorruption() {
    const u8 row = oamCorruptionRow & 0x1F;
    const u8 dst = static_cast<u8>(row << 3);
    
    array<u8, 8> source{};

    // preserve the complete source row before overwriting anything
    for (u8 i = 0; i < 8; i++) {
        source[i] = readOAMByte(i);
    }

    for (u8 i = 0; i < 8; i++) {
        writeOAMByte(
            static_cast<u8>(dst + i),
            source[i]
        );
    }

    secondaryOAM[row >> 2][row & 0x03] = secondaryOAM[0][0];

    oamCorruptionPending = false;
}

void PPU::clockSpriteCounters() {
    for (ActiveSprite& spr : activeSprites) {
        if (spr.xCounter > 0) spr.xCounter--;
    }
}