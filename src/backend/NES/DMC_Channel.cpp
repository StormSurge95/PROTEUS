#include "./NES_PCH.h"
#include "./BUS.h"
#include "./APU.h"
#include "./DMC_Channel.h"

using namespace NES_NS;

void DMC_Channel::write(u16 addr, u8 data) {
    switch (addr) {
        case 0x4010: // flags and rate
            irqEnabled = ((data >> 7) & 0x01) > 0;
            if (!irqEnabled) interrupt = false;
            loop = ((data >> 6) & 0x01) > 0;
            period = DMC_RATES[data & 0x0F];
            return;
        case 0x4011: // direct load
            outputLevel = data & 0x7F;
            return;
        case 0x4012: // sample addr
            sampleAddr = 0xC000 + ((u16)data << 6);
            return;
        case 0x4013: // sample length
            sampleLength = ((u16)data << 4) + 1;
            return;
    }
}

void DMC_Channel::clockTimer() {
    if (timer <= 0) {
        timer = period;
        if (!silent) {
            if ((shifter & 0x01) == 1 && outputLevel <= 125) outputLevel += 2;
            else if ((shifter & 0x01) == 0 && outputLevel >= 2) outputLevel -= 2;
        }
        clockShifter();
    } else timer--;
}

void DMC_Channel::clockShifter() {
    shifter >>= 1;
    bitsRemaining--;
    if (bitsRemaining == 0) {
        newOutputCycle();
    }
}

void DMC_Channel::newOutputCycle() {
    bitsRemaining = 8;
    if (noSample)
        silent = true;
    else {
        silent = false;
        shifter = sampleBuffer;
        noSample = true;
        apu->bus.lock()->dmcActive = true;
    }
}

void DMC_Channel::fetchSample(bool first) {
    if (first) {
        sampleBuffer = apu->bus.lock()->read(currAddr);
        if (currAddr == 0xFFFF) currAddr = 0x8000;
        else currAddr++;
    } else {
        bytesRemaining--;
        if (bytesRemaining == 0) {
            if (loop) {
                currAddr = sampleAddr;
                bytesRemaining = sampleLength;
            } else if (irqEnabled) interrupt = true;
        }
        apu->bus.lock()->dmcActive = false;
        apu->bus.lock()->dmaDummy = true;
    }
}