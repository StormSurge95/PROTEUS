#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class NoiseChannel {
        private:
            bool enabled = false;
            bool mode = false;

            u16 timer = 0x0000;
            u16 period = 0x0000;

            u16 shiftRegister = 0x01;

            LengthCounter lengthCounter;

            bool envelopeStart = false;
            bool constantVolume = false;
            u8 envelopeDivider = 0x00;
            u8 envelopePeriod = 0x00;
            u8 decayLevel = 0x0F;

            inline static const u16 PERIOD_TABLE[16] = {
                4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
            };

            void feedback();

            inline u8 volume() const { return constantVolume ? envelopePeriod : decayLevel; }
        public:
            NoiseChannel() = default;
            ~NoiseChannel() = default;

            void clockTimer();
            void clockLength();
            void clockEnvelope();

            void write(u16 addr, u8 data);

            u8 output() const;

            inline u8 status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
            inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
    };
}