#include "./NES_NoiseChannel.h"

void NES_NoiseChannel::feedback() {
    bool b0 = shiftRegister & 0x01;
    bool b1 = mode ? ((shiftRegister >> 6) & 0x01) : ((shiftRegister >> 1) & 0x01);

    bool f = b0 ^ b1;

    shiftRegister >>= 1;
    shiftRegister |= ((+f) << 14);
}

void NES_NoiseChannel::write(uint16_t addr, uint8_t data) {
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
            if (enabled) lengthCounter.counter = NES_LENGTH_TABLE[data >> 3];

            envelopeStart = true;
            break;
    }
}

void NES_NoiseChannel::clockTimer() {
    if (timer <= 0)
        timer = period;
    else
        timer--;
}

void NES_NoiseChannel::clockLength() {
    if (!lengthCounter.halt && lengthCounter.counter > 0)
        lengthCounter.counter--;
}

void NES_NoiseChannel::clockEnvelope() {
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

uint8_t NES_NoiseChannel::output() const {
    if (lengthCounter.counter == 0 || (shiftRegister & 0x01) == 1) return 0;

    return volume();
}