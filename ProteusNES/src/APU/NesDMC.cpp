#include "../shared/NesPCH.h"
#include "../CPU/NesCPU.h"
#include "./NesAPU.h"
#include "./NesDMC.h"
#include "../shared/NesProfiles.h"

using namespace NS_NES;

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
            if (!irqEnabled)
                apu->cpu.lock()->setIrqLine_DMC(interrupt = false);
            // set loop flag
            loop = ((data >> 6) & 0x01) > 0;
            // set new pariod
            period = GetDmcRate(*region, data & 0x0F);
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
    if (timer == 0) {
        // timer is automatically restarted
        timer = period - 1;
        // if silence flag is clear, output level changes based on bit 0 of the shift register
        if (!silent) {
            bool up = ((shifter & 0x01) == 1);
            // if bit is 1 (and output <= 125), add 2
            if (up && outputLevel <= 125) outputLevel += 2;
            // if bit is 0 (and output >= 2) sub 2
            else if (!up && outputLevel >= 2) outputLevel -= 2;
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
        if (bytesRemaining > 0 && !dmcStartPending) {
            apu->cpu.lock()->requestDmcDma(currAddr, false);
        }
    }
}

void DMC_Channel::onByteFetch(u8 byte) {
    sampleBuffer = byte;
    noSample = false;
    if (currAddr == 0xFFFF)
        currAddr = 0x8000;
    else
        currAddr++;
    
    if (bytesRemaining == 0) return;
    
    bytesRemaining--;
    if (bytesRemaining == 0) {
        if (loop) {
            currAddr = sampleAddr;
            bytesRemaining = sampleLength;
        } else if (irqEnabled) {
            apu->cpu.lock()->setIrqLine_DMC(interrupt = true);
            if (eventSink) eventSink->OnInterrupt(INTERRUPT_EVENT::IRQ_REQ_DMC);
        }
    }
}

void DMC_Channel::enable() {
    enabled = true;

    if (bytesRemaining != 0) return;

    currAddr = sampleAddr;
    bytesRemaining = sampleLength;

    if (bytesRemaining == 0) return;

    sptr<CPU> cpup = apu->cpu.lock();

    dmcStartPending = dmcStartDelayArmed = true;
    
    dmcStartDelay = cpup->isGetCycle() ? 3 : 2;
}

void DMC_Channel::clockDmcStart() {
    if (dmcDisablePending) {
        if (dmcDisableDelayArmed) {
            dmcDisableDelayArmed = false;
        } else if (--dmcDisableDelay == 0) {
            dmcDisablePending = false;
            bytesRemaining = 0;

            apu->cpu.lock()->stopDmcDma();
        }
    }

    if (!dmcStartPending) return;

    if (dmcStartDelayArmed) {
        dmcStartDelayArmed = false;
        return;
    }

    if (--dmcStartDelay != 0) return;

    dmcStartPending = false;

    if (noSample && bytesRemaining > 0) {
        sptr<CPU> cpup = apu->cpu.lock();
        cpup->requestDmcDma(currAddr, true);
    }
}

void DMC_Channel::disable() {
    enabled = false;

    sptr<CPU> cpup = apu->cpu.lock();

    if (!dmcDisablePending) {
        dmcDisablePending = dmcDisableDelayArmed = true;

        // The disable becomes visible at the appropriate following
        // DMC/APU phase rather than immediately on the $4015 write.
        dmcDisableDelay = cpup->isGetCycle() ? 3 : 2;
    }

    // A $4015 write still clears the DMC IRQ immediately.
    cpup->setIrqLine_DMC(interrupt = false);
}

void DMC_Channel::init(ConsoleRegion* r) {
    region = r;
    irqEnabled = loop = enabled = interrupt = false;
    noSample = silent = true;
    sampleAddr = currAddr = sampleLength = bytesRemaining = 0x0000;
    sampleBuffer = shifter = outputLevel = 0x00;
    bitsRemaining = 8;
    period = (r == nullptr ? 0 : GetDmcRate(*region, 0));
    timer = 0;
    dmcStartDelay = 0;
    dmcStartDelayArmed = dmcStartPending = false;
    dmcDisableDelay = 0;
    dmcDisableDelayArmed = dmcDisablePending = false;
}

void DMC_Channel::reset() {
    enabled = false;
    interrupt = false;
    irqEnabled = false;

    silent = true;
    noSample = true;

    currAddr = sampleAddr;
    bytesRemaining = 0;

    sampleBuffer = 0;
    shifter = 0;
    bitsRemaining = 8;

    timer = 0;
    
    dmcStartDelay = 0;
    dmcStartDelayArmed = dmcStartPending = false;
    
    dmcDisableDelay = 0;
    dmcDisableDelayArmed = dmcDisablePending = false;

    apu->cpu.lock()->setIrqLine_DMC(false);
}