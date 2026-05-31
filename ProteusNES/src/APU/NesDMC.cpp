#include <NesDMC.h>

using namespace NS_NES;

void DMC_Channel::write(u16 addr, u8 data) {
    switch (addr) {
        case 0x4010:
            irq_enable = ((data >> 7) & 0x01) > 0;
            if (!irq_enable) pending_irq = false;
            loop_flag = ((data >> 6) & 0x01) > 0;
            //rate_index = data & 0x0F;
            timerPeriod = GetRateDMC(ConsoleRegion::NTSC, data & 0x0F);
            break;
        case 0x4011:
            outputLevel = data & 0x7F;
            // TODO: immediate DMC load
            break;
        case 0x4012:
            sampleAddrReload = 0xC000 + ((u16)data << 6);
            break;
        case 0x4013:
            sampleLengthReload = ((u16)data << 4) + 1;
            break;
    }
}

void DMC_Channel::enable() {
    enabled = true;
    if (bytesRemaining == 0) {
        currentAddr = sampleAddrReload;
        bytesRemaining = sampleLengthReload;
    }
}

void DMC_Channel::disable() {
    bytesRemaining = 0;
}

void DMC_Channel::clockTimer() {
    if (timerCounter > 0) {
        timerCounter--;
    } else {
        timerCounter = timerPeriod;
        clockShiftRegister();
    }
}

void DMC_Channel::clockShiftRegister() {
    if (!silence) {
        if ((shiftReg & 0x01) > 0) {
            if (outputLevel <= 125)
                outputLevel += 2;
        } else {
            if (outputLevel >= 2)
                outputLevel -= 2;
        }
    }

    shiftReg >>= 1;
    bitsRemaining--;

    if (bitsRemaining == 0) {
        bitsRemaining = 8;

        if (sampleBufferFull) {
            shiftReg = sampleBuffer;
            sampleBufferFull = false;
            silence = false;
        } else {
            silence = true;
        }

        bufferNeeded = true;
    }
}

void DMC_Channel::onByteFetch(u8 byte) {
    sampleBuffer = byte;
    sampleBufferFull = true;
    bufferNeeded = false;

    if (currentAddr == 0xFFFF)
        currentAddr = 0x8000;
    else
        currentAddr++;

    if (bytesRemaining > 0)
        bytesRemaining--;

    if (bytesRemaining == 0) {
        if (loop_flag) {
            currentAddr = sampleAddrReload;
            bytesRemaining = sampleLengthReload;
        } else {
            // TODO
        }
    }
}