#include "./Triangle.hpp"

using namespace NES_NS;

void TriangleChannel::write(u16 addr, u8 data) {
    switch (addr) {
        case 0x4008: // linear counter setup
            lengthCounter.halt = ((data >> 7) & 0x01) > 0;
            linearPeriod = data & 0x7F;
            break;
        case 0x400A: // timer low
            period = (period & 0xFF00) | data;
            break;
        case 0x400B: // timer high & lcl
            period = (period & 0x00FF) | (((u16)data & 0x07) << 8);
            if (enabled) lengthCounter.counter = LENGTH_TABLE[data >> 3];
            // side effect: sets linear counter reload flag
            linearReload = true;
    }
}

void TriangleChannel::clockTimer() {
    if (timer <= 0) {
        timer = period;
        // only increment step if BOTH counters are active
        if (lengthCounter.counter != 0 && linearCounter != 0) step++;
        // step wraps from 31 to 0
        if (step >= 32) step = 0x00;
    } else
        timer--;
}

void TriangleChannel::clockLength() {
    // ditto other length counters
    if (!lengthCounter.halt && lengthCounter.counter > 0)
        lengthCounter.counter--;
}

void TriangleChannel::clockLinear() {
    if (linearReload) {
        // if reload flag is set, we reload the linear counter
        linearCounter = linearPeriod;
        // clear reload flag ONLY if length counter is NOT halted
        if (!lengthCounter.halt) linearReload = false;
    } else if (linearCounter > 0) {
        linearCounter--;
    }
}

u8 TriangleChannel::output() const {
    // triangle channel cannot be "silenced" like other channels
    // instead, it will always output it's current volume
    return LINEAR_SEQUENCE[step];
}