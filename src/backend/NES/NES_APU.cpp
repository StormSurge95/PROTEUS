#include "./NES_APU.h"

#pragma region SweepUnit
void SweepUnit::onWrite(uint8_t data) {
    enabled = !!((data >> 7) & 0x01);
    div.reload = ((data >> 4) & 0x07);
    negate = !!((data >> 3) & 0x01);
    shift = (data & 0x07);
    reload = true;
}

void SweepUnit::updateTarget() {
    int16_t t = channel->timer;
    int16_t change = t >> shift;
    if (negate) target = period - change - (channel->isPulse1 ? 1 : 0);
    else target = period + change;

    if (target < 0) target = 0;
}

void SweepUnit::onHalfFrame() {
    if (div.counter == 0 && enabled && shift != 0) {
        if (!mute()) channel->timer = period;
        div.counter = div.reload;
    }

    div.counter--;
}
#pragma endregion

#pragma region VolumeEnvelope
void VolumeEnvelope::onWrite(uint16_t addr, uint8_t data) {
    switch (addr) {
        case 0x4000:
        case 0x4004:
        case 0x400C:
            loop = !!((data >> 5) & 0x01);
            div.reload = (data & 0x0F);
            return;
        case 0x4003:
        case 0x4007:
        case 0x400F:
            start = true;
            return;
    }
}

void VolumeEnvelope::clock() {
    if (start) {
        start = false;
        div.counter = div.reload;
        decayCounter = 15;
    } else {
        if (div.counter == 0) {
            div.counter = div.reload;
            if (decayCounter != 0)
                decayCounter--;
            else if (loop)
                decayCounter = 15;
        } else div.counter--;
    }
}
#pragma endregion

#pragma region LengthCounter & LinearCounter
// TODO: add other channels
void LengthCounter::onWrite(uint16_t addr, uint8_t data) {
    switch (addr) {
        case 0x4015:
            enabled = !!((data >> channel->which()) & 0x01);
            return;
        case 0x4000:
            if (channel->which() == PULSE1) halted = !!((data >> 5) & 0x01);
            return;
        case 0x4004:
            if (channel->which() == PULSE2) halted = !!((data >> 5) & 0x01);
            return;
        case 0x4008:
            if (channel->which() == TRIANGLE) halted = !!((data >> 7) & 0x01);
            return;
        case 0x400C:
            if (channel->which() == NOISE) halted = !!((data >> 5) & 0x01);
            return;
        case 0x4003:
            if (channel->which() == PULSE1) load((data >> 3) & 0x1F);
            return;
        case 0x4007:
            if (channel->which() == PULSE2) load((data >> 3) & 0x1F);
            return;
        case 0x400B:
            if (channel->which() == TRIANGLE) load((data >> 3) & 0x1F);
            return;
        case 0x400F:
            if (channel->which() == NOISE) load((data >> 3) & 0x1F);
            return;
    }
}

void LengthCounter::clock() {
    if (enabled) {
        if (counter > 0 && !halted)
            counter--;
    } else
        counter = 0;
}

void LinearCounter::onWrite(uint16_t addr, uint8_t data) {
    if (addr == 0x4008) {
        enabled = !!((data >> 7) & 0x01);
        init = (data & 0x7F);
    }
}

// TODO: complete
void LinearCounter::clock() {
}
#pragma endregion

#pragma region PulseChannel
PulseChannel::PulseChannel(bool isP1) {
    sweep = new SweepUnit(this);
    envelope = new VolumeEnvelope(this);
    counter = new LengthCounter(this);
    isPulse1 = isP1;
}

PulseChannel::~PulseChannel() {
    delete sweep;
    delete envelope;
    delete counter;
}

void PulseChannel::onWrite(uint16_t addr, uint8_t data) {
    if (addr == 0x4015) {
        counter->onWrite(addr, data);
        return;
    }
    switch (which()) {
        case PULSE1:
            switch (addr) {
                case 0x4000:
                    dutyCycle = ((data >> 6) & 0x03);
                    constVol = !!((data >> 4) & 0x01);
                    envelope->onWrite(addr, data);
                    counter->onWrite(addr, data);
                    return;
                case 0x4001: return sweep->onWrite(data);
                case 0x4002:
                    timer = (timer & 0x0700) | data;
                    return;
                case 0x4003:
                    timer = (timer & 0x00FF) | ((uint16_t)(data & 0x07) << 8);
                    envelope->onWrite(addr, data);
                    counter->onWrite(addr, data);
                    return;
            }
            break;
        case PULSE2:
            switch (addr) {
                case 0x4004:
                    dutyCycle = ((data >> 6) & 0x03);
                    constVol = !!((data >> 4) & 0x01);
                    envelope->onWrite(addr, data);
                    counter->onWrite(addr, data);
                    return;
                case 0x4005: return sweep->onWrite(data);
                case 0x4006:
                    timer = (timer & 0x0700) | data;
                    return;
                case 0x4007:
                    timer = (timer & 0x00FF) | ((uint16_t)(data & 0x07) << 8);
                    envelope->onWrite(addr, data);
                    counter->onWrite(addr, data);
                    return;
            }
            break;
    }
}

void PulseChannel::clock() {
    if (timer == 0) {
        if (dutyIndex == 0) dutyIndex = 7;
        else dutyIndex--;
    } else timer--;
}

void PulseChannel::onHalfFrame() {
    counter->clock();
    sweep->clock();
}

float PulseChannel::sample() const {
    if (!sequencer() || sweep->mute() || counter->counter == 0 || timer < 8)
        return 0.0f;

    if (constVol) return envelope->div.reload;
    else return envelope->decayCounter;
}
#pragma endregion

#pragma region TriangleChannel
#pragma endregion

#pragma region NoiseChannel
#pragma endregion

#pragma region DMCChannel
#pragma endregion

#pragma region FrameCounter
void FrameCounter::onWrite(uint16_t addr, uint8_t data) {
    if (addr == 0x4017) {
        use5step = !!((data >> 7) & 0x01);
        inhibitIRQ = !!((data >> 6) & 0x01);
    }
}

void FrameCounter::clock() {
    bool isEnd = use5step ? cycle == q4_5 : cycle == q4_4;
    bool isHalf = (cycle == q2 || isEnd);
    bool isQuarter = (cycle == q1 || cycle == q3 || isHalf);

    if (isQuarter) onQuarterFrame();
    if (isHalf) onHalfFrame();
    if (isEnd) cycle = 0;

    if (!use5step && cycle == 29828 && !inhibitIRQ)
        apu->irqRequest = true;
}

void FrameCounter::onQuarterFrame() {
    apu->channels->pulse1->onQuarterFrame();
    apu->channels->pulse2->onQuarterFrame();
}

void FrameCounter::onHalfFrame() {
    apu->channels->pulse1->onHalfFrame();
    apu->channels->pulse2->onHalfFrame();
}
#pragma endregion

#pragma region NES_APU
NES_APU::NES_APU() {
    channels = new CHANNELS();
    counter = new FrameCounter(this);
}

NES_APU::CHANNELS::CHANNELS() {
    pulse1 = new PulseChannel(true);
    pulse2 = new PulseChannel(false);
}

NES_APU::CHANNELS::~CHANNELS() {
    delete pulse1;
    delete pulse2;
}

NES_APU::~NES_APU() {
    // channel deletion handled in struct deconstructor
    delete counter;
}

void NES_APU::reset() {

}

uint8_t NES_APU::read(uint16_t addr, bool readonly) {
    if (addr == 0x4015) {
        // TODO: include status of other channels
        return (
            0b11111100 |
            ((channels->pulse2->counter->counter > 0) << 1) |
            (channels->pulse1->counter->counter > 0)
        );
    }
    return 0x00;
}

// TODO: include other channels
void NES_APU::write(uint16_t addr, uint8_t data) {
    switch (addr) {
        case 0x4000: case 0x4001: case 0x4002: case 0x4003:
            return channels->pulse1->onWrite(addr, data);
        case 0x4004: case 0x4005: case 0x4006: case 0x4007:
            return channels->pulse2->onWrite(addr, data);
        case 0x4015:
            channels->pulse1->onWrite(addr, data);
            channels->pulse2->onWrite(addr, data);
            return;
        case 0x4017:
            counter->onWrite(addr, data);
            return;
    }
}

void NES_APU::clock() {
    counter->clock();

    sample();

    if (sampleBuffer.size() >= CYCLES_PER_SAMPLE)
        sampleReady = true;
}

void NES_APU::sample() {
    float p = channels->pulse1->sample() + channels->pulse2->sample();
    float pOut = (p == 0.0f ? 0.0f : 95.88f / ((8128.0f / p) + 100.0f));

    float output = pOut;

    sampleBuffer.push_back(output);
}

float NES_APU::getSample() {
    float sum = 0.0f;
    size_t num = 0;

    while (this->sampleBuffer.size() > 0) {
        sum += sampleBuffer.back();
        sampleBuffer.pop_back();
        num++;
    }

    return sum / num;
}
#pragma endregion