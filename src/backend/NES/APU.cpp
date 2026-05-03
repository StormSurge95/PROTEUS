#include "./APU.h"

using namespace NES_NS;

APU::APU() {
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

u8 APU::read4015() {
    u8 p1 = pulse1->status();
    u8 p2 = pulse2->status() << 1;
    u8 t = triangle->status() << 2;
    u8 n = noise->status() << 3;
    u8 d = dmc->status() << 4;
    u8 u = apuBus & 0x20;
    u8 f = (irqRequested ? 0x40 : 0x00);
    irqRequested = false;
    u8 i = dmc->interrupt << 7;
    return p1 | p2 | t | n | d | u | f | i;
}

void APU::write(u16 addr, u8 data) {
    apuBus = data;
    if (addr >= 0x4000 && addr <= 0x4003)
        return pulse1->write(addr, data);
    if (addr >= 0x4004 && addr <= 0x4007) {
        return pulse2->write(addr, data);
    }
    if (addr >= 0x4008 && addr <= 0x400B) {
        return triangle->write(addr, data);
    }
    if (addr >= 0x400C && addr <= 0x400F) {
        return noise->write(addr, data);
    }
    if (addr >= 0x4010 && addr <= 0x4013) {
        return dmc->write(addr, data);
    }
    if (addr == 0x4015) {
        return write4015(data);
    }
    if (addr == 0x4017) {
        return write4017(data);
    }
}

void APU::write4015(u8 data) {
    pulse1->enable(((data >> 0) & 0x01) > 0);
    pulse2->enable(((data >> 1) & 0x01) > 0);
    triangle->enable(((data >> 2) & 0x01) > 0);
    noise->enable(((data >> 3) & 0x01) > 0);
    dmc->enabled = (((data >> 4) & 0x01) > 0);
}

void APU::write4017(u8 data) {
    use5step = ((data >> 7) & 0x01) > 0;
    inhibitIRQ = ((data >> 6) & 0x01) > 0;
    if (inhibitIRQ) {
        irqRequested = false;
    }
    if ((masterCycle & 0x01) == 1) {
        // odd cycle = apu cycle
        resetAt = masterCycle + 3;
        pendingReset = true;
    } else {
        // even cycle = cpu cycle
        resetAt = masterCycle + 4;
        pendingReset = true;
    }
    if (use5step) {
        quarterFrame();
        halfFrame();
    }
}

void APU::cycleReset() {
    if (pendingReset && masterCycle == resetAt) {
        cycle = 0;
        pendingReset = false;
    }
}

void APU::clock() {
    cycleReset();
    if ((masterCycle & 0x01) == 1) {
        pulse1->clockTimer();
        pulse2->clockTimer();
    }
    triangle->clockTimer();
    noise->clockTimer();
    clockFrameCounter();

    generateSample();
}

void APU::clockFrameCounter() {
    masterCycle++;
    cycle++;

    switch (cycle) {
        case 7457:
        case 22371:
            quarterFrame();
            break;
        case 14913:
            quarterFrame();
            halfFrame();
            break;
        case 29828:
            if (!use5step && !inhibitIRQ)
                irqRequested = true;
            break;
        case 29829:
            if (!use5step) {
                quarterFrame();
                halfFrame();
                cycle = 0;
            }
            break;
        case 37281:
            quarterFrame();
            halfFrame();
            cycle = 0;
            break;
    }
}

void APU::quarterFrame() {
    pulse1->clockEnvelope();
    pulse2->clockEnvelope();
    triangle->clockLinear();
    noise->clockEnvelope();
}

void APU::halfFrame() {
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
        u8 d = dmc->output();

        sampleBuffer.push_back(mixSamples(p1, p2, t, n, d));
    }
}

void APU::collectSamples(vector<float>& out) {
    out.insert(out.end(), sampleBuffer.begin(), sampleBuffer.end());
    sampleBuffer.clear();
}

float APU::mixSamples(u8 p1, u8 p2, u8 t, u8 n, u8 d) {
    float p = ((p1 + p2) * 1.0f);
    float pulseOut = 0.00752f * p;
    float tndOut = (0.00851f * t) + (0.00494f * n) + (0.00335f * d);
    return pulseOut + tndOut;
}

void APU::dmcFetch(bool first) { dmc->fetchSample(first); }