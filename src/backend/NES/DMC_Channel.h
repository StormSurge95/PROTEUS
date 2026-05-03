#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class DMC_Channel {
        private:
            sptr<APU> apu = nullptr;
            bool irqEnabled = false;
            bool silent = false;
            bool noSample = false;

            bool loop = false;

            u16 sampleAddr = 0x0000;
            u16 currAddr = 0x0000;
            u16 sampleLength = 0x0000;
            u16 bytesRemaining = 0x0000;

            u8 sampleBuffer = 0x00;
            u8 shifter = 0x00;

            u16 period = 0x0000;
            u16 timer = 0x0000;

            u8 bitsRemaining = 0x00;
            u8 outputLevel = 0x00;

            void clockShifter();
            void newOutputCycle();
        public:
            bool enabled = false;
            bool interrupt = false;

            DMC_Channel(APU* p) : apu(p) {}
            ~DMC_Channel() = default;

            void write(u16, u8);

            void clockTimer();

            u8 output() const;

            void fetchSample(bool);

            inline u8 status() const { return (bytesRemaining > 0 ? 0x01 : 0x00); }
    };
}