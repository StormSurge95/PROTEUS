#include "./NES_TriangleChannel.h"

void NES_TriangleChannel::write(uint16_t addr, uint8_t data) {
    switch (addr) {
        case 0x4008:
            lengthCounter.halt = ((data >> 7) & 0x01) > 0;
            linearPeriod = data & 0x7F;
            break;
        case 0x400A:
            period = (period & 0xFF00) | data;
            break;
        case 0x400B:
            period = (period & 0x00FF) | (((uint16_t)data & 0x07) << 8);
            if (enabled) lengthCounter.counter = NES_LENGTH_TABLE[data >> 3];

            linearReload = true;
    }
}

void NES_TriangleChannel::clockTimer() {
    if (timer <= 0) {
        timer = period;
        if (lengthCounter.counter != 0 && linearCounter != 0) step++;
        if (step >= 32) step = 0x00;
    } else
        timer--;
}

void NES_TriangleChannel::clockLength() {
    if (!lengthCounter.halt && lengthCounter.counter > 0)
        lengthCounter.counter--;
}

void NES_TriangleChannel::clockLinear() {
    if (linearReload) {
        linearCounter = linearPeriod;
        if (!lengthCounter.halt) linearReload = false;
    } else if (linearCounter > 0) {
        linearCounter--;
    }
}

uint8_t NES_TriangleChannel::output() const {
    return SEQUENCE[step];
}