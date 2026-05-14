#include "./NES_PCH.h"
#include "./BUS.h"
#include "./APU.h"
#include "./DMC.h"

using namespace NES_NS;

void DMC_Channel::write(u16 addr, u8 data) {
    switch (addr) {
        case 0x4010:
            /* Flags/Rate Register (write)
             * 7..bit..0
             * IL--.RRRR
             * ||   ++++-> Rate index - used to set the rate of output level changes
             * |+--------> Loop flag
             * +---------> IRQ enabled flag; if cleared, the interrupt flag is cleared
             */
            // set irq flag
            irqEnabled = ((data >> 7) & 0x01) > 0;
            // clear interrupt if necessary
            if (!irqEnabled) interrupt = false;
            // set loop flag
            loop = ((data >> 6) & 0x01) > 0;
            // set new pariod
            period = DMC_RATES[data & 0x0F];
            return;
        case 0x4011:
            /*
             * Direct Load Register(write)
             * 7..bit..0
             * -DDD.DDDD
             *  +++.++++-> The DMC output level is set to D, an unsigned value. If the timer is outputting a clock at the same time, the output level is occasionally not changed properly
             */
            outputLevel = data & 0x7F;
            return;
        case 0x4012:
            /*
             * Sample Address Register (write)
             * 7..bit..0
             * AAAA.AAAA
             * ++++.++++-> Sample address = %11AAAAAA.AA000000 = $C000 + (A << 6)
             */
            sampleAddr = 0xC000 + ((u16)data << 6);
            return;
        case 0x4013:
            /*
             * Sample Length Register (write)
             * 7..bit..0
             * LLLL.LLLL
             * ++++.++++-> Sample length = %0000LLLL.LLLL0001 = (L << 4) + 1
             */
            sampleLength = ((u16)data << 4) + 1;
            return;
    }
}

void DMC_Channel::clockTimer() {
    // timer outputs a clock signal when timer = 0
    if (timer <= 0) {
        // timer is automatically restarted
        timer = period;
        // if silence flag is clear, output level changes based on bit 0 of the shift register
        if (!silent) {
            // if bit is 1 (and output <= 125), add 2
            if ((shifter & 0x01) == 1 && outputLevel <= 125) outputLevel += 2;
            // if bit is 0 (and output >= 2) sub 2
            else if ((shifter & 0x01) == 0 && outputLevel >= 2) outputLevel -= 2;
        }
        // clock the right shifter
        clockShifter();
    } else timer--;
}

void DMC_Channel::clockShifter() {
    // shifter is a "right shifter"; so we shift right by one.
    shifter >>= 1;
    // decrement bits remaining
    bitsRemaining--;
    // start a new cycle when we run out of bits
    if (bitsRemaining == 0) {
        newOutputCycle();
    }
}

void DMC_Channel::newOutputCycle() {
    // bits remaining counter is loaded with 8
    bitsRemaining = 8;
    // if sample buffer is empty, silence flag is set
    if (noSample)
        silent = true;
    // otherwise:
    else {
        // silence flag is cleared
        silent = false;
        // sample buffer emptied into shift register
        shifter = sampleBuffer;
        // update helper vars to trigger DMCDMA
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