#include "./NES_APU.h"
#include "./NES_DMCChannel.h"

NES_DMCChannel::NES_DMCChannel(NES_APU* p) : apu(p) {}

void NES_DMCChannel::write(uint16_t addr, uint8_t data) {
    switch (addr) {
        case 0x4010:
            interrupt = ((data >> 7) & 0x01) > 0;
            loop = ((data >> 6) & 0x01) > 0;
            period = RATE_TABLE[data & 0x0F];
            break;
        case 0x4011:
            outputLevel = data & 0x7F;
            break;
        case 0x4012:
            sampleAddress = 0xC000 + ((uint16_t)data << 6);
            break;
        case 0x4013:
            sampleLength = ((uint16_t)data << 4) + 1;
            break;
    }
}

void NES_DMCChannel::fetchByte() {
    sampleBuffer = apu->bus->read(currentAddr++);
    bytesRemaining--;
    if (bytesRemaining == 0) {
        if (loop) {
            bytesRemaining = sampleLength;
            currentAddr = sampleAddress;
        }
    }
}

void NES_DMCChannel::clockTimer() {
    if (timer > 0) {
        timer--;
    } else {
        timer = period;
        clockShifter();
    }
}

void NES_DMCChannel::clockShifter() {
    if (bitsRemaining > 0)
        shifter >>= 1;
    else {
        shifter = sampleBuffer;
        fetchByte();
    }
}