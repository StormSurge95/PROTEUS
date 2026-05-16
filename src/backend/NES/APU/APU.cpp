#include "./APU.h"

using namespace NES_NS;

APU::APU() : HPF1(90.0f, 44100.0f), HPF2(440.0f, 44100.0f), LPF(14000.0f, 44100.0f) {
    pulse1 = make_unique<PulseChannel>(true);
    pulse2 = make_unique<PulseChannel>(false);
    triangle = make_unique<TriangleChannel>();
    noise = make_unique<NoiseChannel>();
    dmc = make_unique<DMC_Channel>(this);
}

u8 APU::read(u16 addr, bool readonly) {
    if (addr == 0x4015) {
        apuBus = read4015();
    }
    return apuBus;
}

void APU::write(u16 addr, u8 data) {
    apuBus = data;
    // pulse 1 channels
    if (addr >= 0x4000 && addr <= 0x4003) return pulse1->write(addr, data);
    // pulse 2 channels
    if (addr >= 0x4004 && addr <= 0x4007) return pulse2->write(addr, data);
    // triangle channels
    if (addr >= 0x4008 && addr <= 0x400B) return triangle->write(addr, data);
    // noise channels
    if (addr >= 0x400C && addr <= 0x400F) return noise->write(addr, data);
    // dmc channels
    if (addr >= 0x4010 && addr <= 0x4013) return dmc->write(addr, data);
    // control register
    if (addr == 0x4015) return write4015(data);
    // frame counter register
    if (addr == 0x4017) return write4017(data);
}

void APU::clock() {
    // reset cycle if necessary
    if (pendingReset && masterCycle == resetAt) {
        cycle = 0;
        pendingReset = false;
    }
    // clock pulse channels every other CPU cycle
    if ((masterCycle & 0x01) == 1) {
        pulse1->clockTimer();
        pulse2->clockTimer();
    }
    // clock triangle/noise/dmc channels every CPU cycle
    /// @todo implement dmc clocking
    triangle->clockTimer();
    noise->clockTimer();

    // clock frame counter sequence every CPU cycle
    clockFrameCounter();

    // generate new sample every CPU cycle (these are downsampled to 44100 Hz later)
    generateSample();
}

u8 APU::read4015() {
    // each channel's status() function returns a full 8bit value with the status in bit0
    // so we can simply acquire these values and use bitshifts and bit-OR to create the
    // final data to be returned.
    u8 p1 = pulse1->status();
    u8 p2 = pulse2->status() << 1;
    u8 t = triangle->status() << 2;
    u8 n = noise->status() << 3;
    u8 d = dmc->status() << 4;
    // bit 5 is open bus
    u8 u = apuBus & 0x20;
    u8 f = (irqRequested ? 0x40 : 0x00);
    // reading the frame counter interrupt flag clears it
    irqRequested = false;
    // reading the dmc interrupt flag DOES NOT clear it
    u8 i = dmc->interrupt << 7;
    return p1 | p2 | t | n | d | u | f | i;
}

void APU::write4015(u8 data) {
    // enable the various channels based on the value written
    pulse1->enable(((data >> 0) & 0x01) > 0);
    pulse2->enable(((data >> 1) & 0x01) > 0);
    triangle->enable(((data >> 2) & 0x01) > 0);
    noise->enable(((data >> 3) & 0x01) > 0);
    /// @todo enabling dmc should trigger an automatic load
    dmc->enabled = (((data >> 4) & 0x01) > 0);
}

void APU::write4017(u8 data) {
    // change various flags based on the value written
    use5step = ((data >> 7) & 0x01) > 0;
    inhibitIRQ = ((data >> 6) & 0x01) > 0;
    // clear IRQ flag if IRQ is inhibited
    if (inhibitIRQ) {
        irqRequested = false;
    }
    // writing to 4017 triggers a delayed FrameCounter reset
    if ((masterCycle & 0x01) == 1) {
        // odd cycle = apu cycle
        resetAt = masterCycle + 3;
        pendingReset = true;
    } else {
        // even cycle = cpu cycle
        resetAt = masterCycle + 4;
        pendingReset = true;
    }
    // setting 5-step mode triggers immediate quarter/half frame signals
    if (use5step) {
        quarterFrame();
        halfFrame();
    }
}

void APU::clockFrameCounter() {
    masterCycle++;
    cycle++;

    switch (cycle) {
        case 7457:
        case 22371: // quarter frames
            quarterFrame();
            break;
        case 14913: // half frame
            quarterFrame();
            halfFrame();
            break;
        case 29828: // irq trigger (only during 4-step with irq enabled)
            if (!use5step && !inhibitIRQ)
                irqRequested = true;
            break;
        case 29829: // end frame (4-step)
            if (!use5step) {
                quarterFrame();
                halfFrame();
                cycle = 0;
            }
            break;
        case 37281: // end frame (5-step)
            quarterFrame();
            halfFrame();
            cycle = 0;
            break;
    }
}

void APU::quarterFrame() {
    // quarter frames clock all Volume Envelopes and
    // the Triangle channel's Linear Counter
    pulse1->clockEnvelope();
    pulse2->clockEnvelope();
    triangle->clockLinear();
    noise->clockEnvelope();
}

void APU::halfFrame() {
    // half frames clock all Length Counters and
    // the Pulse channels' Sweep units.
    pulse1->clockLength();
    pulse2->clockLength();
    pulse1->clockSweep();
    pulse2->clockSweep();
    triangle->clockLength();
    noise->clockLength();
}

void APU::generateSample() {
    cycleAccumulator += 1.0;

    if (cycleAccumulator >= CYCLES_PER_SAMPLE) {
        cycleAccumulator -= CYCLES_PER_SAMPLE;

        u8 p1 = pulse1->output();
        u8 p2 = pulse2->output();
        u8 t = triangle->output();
        u8 n = noise->output();
        u8 d = 0x00;// dmc->output();

        sampleBuffer.push_back(mixSamples(p1, p2, t, n, d));
    }
}

void APU::collectSamples(vector<float>& out) {
    // take all samples currenly in the APU's sampleBuffer and insert them
    // at the end of the provided vector of audio samples from the console.
    out.insert(out.end(), sampleBuffer.begin(), sampleBuffer.end());
    // clear the APU's sampleBuffer so that we don't have repeated samples.
    sampleBuffer.clear();
}

float APU::mixSamples(u8 p1, u8 p2, u8 t, u8 n, u8 d) {
    // perform linear approximation of the NES's audio mixing
    float p = ((p1 + p2) * 1.0f);
    float pulseOut = 0.00752f * p;
    float tndOut = (0.00851f * t) + (0.00494f * n) + (0.00335f * d);
    float sample = pulseOut + tndOut;

    HPF1.process(sample);
    HPF2.process(sample);
    LPF.process(sample);

    return sample;
}