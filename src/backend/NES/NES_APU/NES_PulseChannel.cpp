#include "./NES_PulseChannel.h"

void NES_PulseChannel::clockTimer() {
    if (timer <= 0) {
        timer = period;
        dutyStep = (dutyStep + 1) & 7;
    } else {
        timer--;
    }
}

void NES_PulseChannel::clockLength() {
    if (!lengthCounter.halt && lengthCounter.counter > 0)
        lengthCounter.counter--;
}

void NES_PulseChannel::clockEnvelope() {
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

void NES_PulseChannel::clockSweep() {
    if (sweepReload) {
        sweepDivider = sweepPeriod;
        sweepReload = false;
    } else if (sweepDivider == 0) {
        sweepDivider = sweepPeriod;

        if (sweepEnabled && sweepShift > 0) {
            int change = period >> sweepShift;

            if (sweepNegate) {
                if (isPulse1) period -= (change - 1);
                else period -= change;
            } else
                period += change;
        }
    } else
        sweepDivider--;
}

void NES_PulseChannel::write(uint16_t addr, uint8_t data) {
    switch (addr) {
        case 0x4000:
        case 0x4004:
            dutyMode = (data >> 6) & 0x03;
            lengthCounter.halt = ((data >> 5) & 0x01) > 0;
            constantVolume = ((data >> 4) & 0x01) > 0;
            envelopePeriod = data & 0x0F;
            break;
        case 0x4001:
        case 0x4005:
            sweepEnabled = ((data >> 7) & 0x01) > 0;
            sweepPeriod = (data >> 4) & 0x07;
            sweepNegate = ((data >> 3) & 0x01) > 0;
            sweepShift = data & 0x07;
            sweepReload = true;
            break;
        case 0x4002:
        case 0x4006:
            period = (period & 0xFF00) | data;
            break;
        case 0x4003:
        case 0x4007:
            period = (period & 0x00FF) | (((uint16_t)data & 0x07) << 8);
            if (enabled) lengthCounter.counter = NES_LENGTH_TABLE[data >> 3];

            dutyStep = 0;
            envelopeStart = true;
            break;
    }
}

uint8_t NES_PulseChannel::output() const {
    if (lengthCounter.counter == 0 ||
        period < 8 ||
        !NES_DUTY_TABLE[dutyMode][dutyStep])
        return 0x00;

    return volume();
}