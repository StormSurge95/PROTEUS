#include "./Noise.h"

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
            envelope.constVol = ((data >> 4) & 0x01);
            envelope.period = data & 0x0F;
            break;
        case 0x400E:
            mode = ((data >> 7) & 0x01) > 0;
            printf("Noise Period: %d -> ", period);
            // TODO: get period value based on region
            period = GetRateNoise(REGION::NTSC, data & 0x0F);
            printf("%d\n", period);
            break;
        case 0x400F:
            if (enabled) lengthCounter.counter = LENGTH_TABLE[data >> 3];

            envelope.start = true;
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
    if (envelope.start) {
        envelope.start = false;
        envelope.decay = 15;
        envelope.divider = envelope.period;
    } else {
        if (envelope.divider <= 0) {
            envelope.divider = envelope.period;

            if (envelope.decay > 0)
                envelope.decay--;
            else if (lengthCounter.halt)
                envelope.decay = 15;
        } else
            envelope.divider--;
    }
}

u8 NoiseChannel::output() const {
    if (lengthCounter.counter == 0 || (shiftRegister & 0x01) == 1) return 0;

    return volume();
}