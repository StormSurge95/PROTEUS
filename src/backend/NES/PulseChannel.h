#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class PulseChannel {
        private:
            bool isPulse1 = false;
            bool enabled = false;

            u16 timer = 0x0000;
            u16 period = 0x0000;

            u8 dutyMode = 0x00;
            u8 dutyStep = 0x00;

            LengthCounter lengthCounter;

            bool envelopeStart = false;
            bool constantVolume = false;
            u8 envelopeDivider = 0x00;
            u8 envelopePeriod = 0x00;
            u8 decayLevel = 0x0F;

            bool sweepEnabled = false;
            u8 sweepDivider = 0x00;
            u8 sweepPeriod = 0x00;
            bool sweepNegate = false;
            u8 sweepShift = 0x00;
            bool sweepReload = false;

            inline u8 volume() const { return constantVolume ? envelopePeriod : decayLevel; }

        public:
            PulseChannel(bool p1): isPulse1(p1) {}
            ~PulseChannel() = default;

            void clockTimer();
            void clockLength();
            void clockEnvelope();
            void clockSweep();

            void write(u16 addr, u8 data);

            u8 output() const;

            inline u8 status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
            inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
    };
}