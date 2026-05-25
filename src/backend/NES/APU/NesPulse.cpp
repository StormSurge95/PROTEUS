#include "../shared/NES_PCH.hpp"
#include "./NesPulse.hpp"

using namespace NS_NES;

void PulseChannel::clockTimer() {
    // when the pulse channel outputs a clock signal, we increment our duty table position
    if (timer <= 0) {
        timer = period;
        dutyStep = (dutyStep + 1) & 7;
    } else {
        timer--;
    }
}

void PulseChannel::clockLength() {
    // length counter only decrements if NOT halted and greater than 0
    if (!lengthCounter.halt && lengthCounter.counter > 0)
        lengthCounter.counter--;
}

void PulseChannel::clockEnvelope() {
    if (envelope.start) {
        // if `start` flag is set, we simply restart the envelope
        envelope.start = false;
        envelope.decay = 15;
        envelope.divider = envelope.period;
    } else {
        if (envelope.divider <= 0) {
            envelope.divider = envelope.period;
            // when envelope divider reaches zero, we clock decay value
            if (envelope.decay > 0)
                envelope.decay--;
            else if (lengthCounter.halt)
                // otherwise, if length counter is halted, decay is reloaded to 15
                envelope.decay = 15;
        } else
            envelope.divider--;
    }
}

void PulseChannel::clockSweep() {
    if (sweep.reload) { // if reload flag is set, we reload
        sweep.divider = sweep.period;
        sweep.reload = false;
    } else if (sweep.divider == 0) { // otherwise, we clock the sweep's divider
        sweep.divider = sweep.period;
        // if sweep is enabled, and shift is greater than 0, we sweep the pulse frequency
        if (sweep.enabled && sweep.shift > 0) {
            // base changes is calculated by shifting the timer value
            int change = period >> sweep.shift;
            if (sweep.negate) { // when negating...
                if (isPulse1) // pulse one uses ones' complement
                    period += (-change - 1);
                else // pulse two uses two's complement
                    period -= change;
            } else // otherwise, we simply add the change
                period += change;
        }
    } else
        sweep.divider--;
}

void PulseChannel::write(u16 addr, u8 data) {
    switch (addr) {
        case 0x4000: // pulse 1 control
        case 0x4004: // pulse 2 control
            // side effects: duty cycle is changed, but sequencer's current position isn't affected
            dutyMode = (data >> 6) & 0x03;
            lengthCounter.halt = ((data >> 5) & 0x01) > 0;
            envelope.constVol = ((data >> 4) & 0x01) > 0;
            envelope.period = data & 0x0F;
            break;
        case 0x4001: // pulse 1 sweep
        case 0x4005: // pulse 2 sweep
            sweep.enabled = ((data >> 7) & 0x01) > 0;
            sweep.period = (data >> 4) & 0x07;
            sweep.negate = ((data >> 3) & 0x01) > 0;
            sweep.shift = data & 0x07;
            sweep.reload = true;
            break;
        case 0x4002: // pulse 1 timer low
        case 0x4006: // pulse 2 timer low
            period = (period & 0xFF00) | data;
            break;
        case 0x4003: // pulse 1 timer high & lcl
        case 0x4007: // pulse 2 timer high & lcl
            period = (period & 0x00FF) | (((u16)data & 0x07) << 8);
            if (enabled) lengthCounter.counter = LENGTH_TABLE[data >> 3];
            // side effects:
            //      sequencer is immediately restarted at first value of current sequence
            //      envelope is restarted
            //      period divider is NOT reset
            dutyStep = 0;
            envelope.start = true;
            break;
    }
}

u8 PulseChannel::output() const {
    // if pulse channel output is not available, return silence (0)
    if (lengthCounter.counter == 0 ||
        period < 8 ||
        !DUTY_TABLE[dutyMode][dutyStep])
        return 0x00;
    // otherwise, return current volume
    return volume();
}