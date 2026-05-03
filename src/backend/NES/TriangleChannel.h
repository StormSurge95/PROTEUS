#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class TriangleChannel {
        private:
            bool enabled = false;

            u16 timer = 0x0000;
            u16 period = 0x0000;

            LengthCounter lengthCounter;

            u16 linearCounter = 0x0000;
            u16 linearPeriod = 0x0000;
            bool linearReload = false;

            u8 step = 0x00;
        public:
            TriangleChannel() = default;
            ~TriangleChannel() = default;

            void clockTimer();
            void clockLength();
            void clockLinear();

            void write(u16 addr, u8 data);

            u8 output() const;

            inline u8 status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
            inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
    };
}