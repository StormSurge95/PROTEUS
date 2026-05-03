#include "./NoiseChannel.h"

using namespace NES_NS;

void NoiseChannel::feedback() {
    bool b0 = shiftRegister & 0x01;
    bool b1 = mode ? ((shiftRegister >> 6) & 0x01) : ((shiftRegister >> 1) & 0x01);

    bool f = b0 ^ b1;

    shiftRegister >>= 1;
    shiftRegister |= ((+f) << 14);
}

void NoiseChannel::write(u16 addr, u8 data) {
    switch (addr) {
        case 0x400C:
            lengthCounter.halt = ((data >> 5) & 0x01) > 0;
            constantVolume = ((data >> 4) & 0x01);
            envelopePeriod = data & 0x0F;
            break;
        case 0x400E:
            mode = ((data >> 7) & 0x01) > 0;
            period = PERIOD_TABLE[data & 0x0F];
            break;
        case 0x400F:
            if (enabled) lengthCounter.counter = LENGTH_TABLE[data >> 3];

            envelopeStart = true;
            break;
    }
}

void NoiseChannel::clockTimer() {
    if (timer <= 0) {
        timer = period;
        feedback();
    } else
        timer--;
}

void NoiseChannel::clockLength() {
    if (!lengthCounter.halt && lengthCounter.counter > 0)
        lengthCounter.counter--;
}

void NoiseChannel::clockEnvelope() {
    if (envelopeStart) {
        envelopeStart = false;
        decayLevel = 15;
        envelopeDivider = envelopePeriod;
    } else {
        if (envelopeDivider <= 0) {
            envelopeDivider = envelopePeriod;

            if (decayLevel > 0)
                decayLevel--;
            else if (lengthCounter.halt)
                decayLevel = 15;
        } else
            envelopeDivider--;
    }
}

u8 NoiseChannel::output() const {
    if (lengthCounter.counter == 0 || (shiftRegister & 0x01) == 1) return 0;

    return volume();
}