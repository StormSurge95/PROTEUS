#include <NesNoise.h>

using namespace NS_NES;

void NoiseChannel::feedback() {
    // noise channel calculates feedback using two bits from the shift register.
    bool b0 = shiftRegister & 0x01; // first bit is always bit 0
    // if mode flag is set, second bit is bit 6; otherwise, second bit is bit 1
    bool b1 = mode ? ((shiftRegister >> 6) & 0x01) : ((shiftRegister >> 1) & 0x01);

    bool f = b0 ^ b1; // feedback is calculated via eor operation

    // shift the regsiter to the right
    shiftRegister >>= 1;
    // input new bit as bit #14
    shiftRegister |= ((+f) << 14);
}

void NoiseChannel::write(u16 addr, u8 data) {
    switch (addr) {
        case 0x400C: // length counter & envelope control
            lengthCounter.halt = ((data >> 5) & 0x01) > 0;
            envelope.constVol = ((data >> 4) & 0x01);
            envelope.period = data & 0x0F;
            break;
        case 0x400E: // mode & period control
            mode = ((data >> 7) & 0x01) > 0;
            // TODO: get period value based on region
            period = GetRateNoise(ConsoleRegion::NTSC, data & 0x0F);
            break;
        case 0x400F: // length counter load
            if (enabled) lengthCounter.counter = LENGTH_TABLE[data >> 3];
            // writing to $400F has a side effect of restarting the volume envelope
            envelope.start = true;
            break;
    }
}

void NoiseChannel::clockTimer() {
    // timer outputs a clock signal at 0
    if (timer <= 0) {
        // when clock signal happens, calculate feedback
        timer = period;
        feedback();
    } else
        timer--;
}

void NoiseChannel::clockLength() {
    // length counter is only decremented if it is greater than 0 and NOT halted
    if (!lengthCounter.halt && lengthCounter.counter > 0)
        lengthCounter.counter--;
}

void NoiseChannel::clockEnvelope() {
    if (envelope.start) { // if `start` is set, then we restart the envelope
        envelope.start = false;
        envelope.decay = 15;
        envelope.divider = envelope.period;
    } else { // otherwise, it is simply clocked
        if (envelope.divider <= 0) {
            envelope.divider = envelope.period;
            // when envelope divider reaches 0, we clock the decay value
            if (envelope.decay > 0)
                // if we still have volume, we decrement
                envelope.decay--;
            else if (lengthCounter.halt)
                // otherwise, if length counter is halted, decay is reloaded to 15
                envelope.decay = 15;
        } else
            envelope.divider--;
    }
}

u8 NoiseChannel::output() const {
    // if noise channel is not availabe, return silence (0)
    if (lengthCounter.counter == 0 || (shiftRegister & 0x01) == 1) return 0;
    // otherwise, return current volume
    return volume();
}