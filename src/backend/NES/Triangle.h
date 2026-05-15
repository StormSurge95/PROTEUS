#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class TriangleChannel {
            // Allow Debugger class to access all private members of the Triangle class
            friend class Debugger;
        private:
            // enabled flag
            bool enabled = false;

            // current timer value
            u16 timer = 0x0000;
            // timer reload value
            u16 period = 0x0000;

            // Length Counter struct variable
            LengthCounter lengthCounter;

            // current linear length counter value
            u8 linearCounter = 0x00;
            // linear length counter reload value
            u8 linearPeriod = 0x00;
            // linear length counter reload flag
            bool linearReload = false;

            // the current step of the linear sequence
            u8 step = 0x00;
        public:
            // default constructor
            TriangleChannel() = default;
            // default destructor
            ~TriangleChannel() = default;

            // clocks the main timer
            void clockTimer();
            // clocks the REGULAR length counter
            void clockLength();
            // clocks the LINEAR length counter
            void clockLinear();

            // handles write operations for the registers within this channel
            void write(u16 addr, u8 data);

            // produces and returns an audio sample byte
            u8 output() const;

            // returns the current status of this channel
            inline u8 status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
            // helper function to (dis)enable this channel and acknowledge any side-effects
            inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
    };
}