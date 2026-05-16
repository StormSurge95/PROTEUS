#include "./NES_PCH.h"
#include "./Pulse.h"

using namespace NES_NS;

void PulseChannel::clockTimer() {
    if (timer <= 0) {
        timer = period;
        dutyStep = (dutyStep + 1) & 7;
    } else {
        timer--;
    }
}

void PulseChannel::clockLength() {
    if (!lengthCounter.halt && lengthCounter.counter > 0)
        lengthCounter.counter--;
}

void PulseChannel::clockEnvelope() {
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

void PulseChannel::clockSweep() {
    if (sweep.reload) {
        sweep.divider = sweep.period;
        sweep.reload = false;
    } else if (sweep.divider == 0) {
        sweep.divider = sweep.period;

        if (sweep.enabled && sweep.shift > 0) {
            int change = period >> sweep.shift;

            if (sweep.negate) {
                if (isPulse1) period -= (change - 1);
                else period -= change;
            } else
                period += change;
        }
    } else
        sweep.divider--;
}

void PulseChannel::write(u16 addr, u8 data) {
    switch (addr) {
        case 0x4000:
        case 0x4004:
            dutyMode = (data >> 6) & 0x03;
            lengthCounter.halt = ((data >> 5) & 0x01) > 0;
            envelope.constVol = ((data >> 4) & 0x01) > 0;
            envelope.period = data & 0x0F;
            break;
        case 0x4001:
        case 0x4005:
            sweep.enabled = ((data >> 7) & 0x01) > 0;
            sweep.period = (data >> 4) & 0x07;
            sweep.negate = ((data >> 3) & 0x01) > 0;
            sweep.shift = data & 0x07;
            sweep.reload = true;
            break;
        case 0x4002:
        case 0x4006:
            period = (period & 0xFF00) | data;
            break;
        case 0x4003:
        case 0x4007:
            period = (period & 0x00FF) | (((u16)data & 0x07) << 8);
            if (enabled) lengthCounter.counter = LENGTH_TABLE[data >> 3];

            dutyStep = 0;
            envelope.start = true;
            break;
    }
}

u8 PulseChannel::output() const {
    if (lengthCounter.counter == 0 ||
        period < 8 ||
        !DUTY_TABLE[dutyMode][dutyStep])
        return 0x00;

    return volume();
}