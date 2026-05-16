#include "./PPU.h"

using namespace NES_NS;

void PPU::shiftBackgroundShifters() {
    patternShiftLo <<= 1;
    patternShiftHi <<= 1;

    attributeShiftLo <<= 1;
    attributeShiftHi <<= 1;
}

void PPU::loadBackgroundShifters() {
    patternShiftLo = (patternShiftLo & 0xFF00) | nextPatternByteLo;
    patternShiftHi = (patternShiftHi & 0xFF00) | nextPatternByteHi;

    attributeShiftLo = (attributeShiftLo & 0xFF00) | ((nextAttributeByte & 0b01) ? 0xFF : 0x00);
    attributeShiftHi = (attributeShiftHi & 0xFF00) | ((nextAttributeByte & 0b10) ? 0xFF : 0x00);
}

void PPU::fetchBGNametableByte() {
    u16 addr = (0x2000 | (v & 0x0FFF));
    nextNametableByte = ppuRead(addr, false);
}

void PPU::fetchBGAttributeByte() {
    u16 addr = 0x23C0 |
        (v & 0x0C00) |
        ((v >> 4) & 0x38) |
        ((v >> 2) & 0x07);
    nextAttributeByte = ppuRead(addr, false);

    if (coarseY() & 0x02) nextAttributeByte >>= 4;
    if (coarseX() & 0x02) nextAttributeByte >>= 2;

    nextAttributeByte &= 0x03;
}

void PPU::fetchBGPatternByteLo() {
    u16 addr = getBackgroundPatternTableAddr() |
        (((u16)nextNametableByte) << 4) |
        ((u16)fineY());
    nextPatternByteLo = ppuRead(addr, false);
}

void PPU::fetchBGPatternByteHi() {
    u16 addr = getBackgroundPatternTableAddr() |
        (((u16)nextNametableByte) << 4) |
        ((u16)fineY());
    nextPatternByteHi = ppuRead(addr + 8, false);
}

void PPU::backgroundPipeline() {
    if ((cycle > 1 && cycle <= 257) || (cycle > 321 && cycle <= 337))
        shiftBackgroundShifters();

    if (cycle <= 256 || cycle <= 336) {
        switch (cycle & 0x07) {
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
}

void PPU::getBackgroundPixel(u8& pixel, u8& attr) const {
    u16 mux = 0x8000 >> x;

    u8 p0 = (patternShiftLo & mux) > 0;
    u8 p1 = (patternShiftHi & mux) > 0;

    pixel = (p1 << 1) | p0;

    u8 a0 = (attributeShiftLo & mux) > 0;
    u8 a1 = (attributeShiftHi & mux) > 0;

    attr = (a1 << 1) | a0;
}