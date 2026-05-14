#pragma once

#include "./NES_PCH.h"

namespace NES_NS {

    class PulseChannel {
            // Allow Debugger class to access all private members of the PulseChannel class
            friend class Debugger;
        private:
            // flag for whether this is Pulse1 or Pulse2
            bool isPulse1 = false;
            // enabled flag
            bool enabled = false;

            // current timer value
            u16 timer = 0x0000;
            // timer reload value
            u16 period = 0x0000;

            // current duty sequence
            u8 dutyMode = 0x00;
            // current duty index
            u8 dutyStep = 0x00;

            // length counter struct variable
            LengthCounter lengthCounter;

            // volume envelope struct variable
            VolumeEnvelope envelope;

            // sweep unit struct variable
            SweepUnit sweep;

            // getter function for the current volume of the channel
            inline u8 volume() const { return envelope.constVol ? envelope.period : envelope.decay; }

        public:
            // explicit constructor
            PulseChannel(bool p1): isPulse1(p1) {}
            // default destructor
            ~PulseChannel() = default;

            // clocks the main timer of the channel
            void clockTimer();
            // clocks the length counter of the channel
            void clockLength();
            // clocks the volume envelope of the channel
            void clockEnvelope();
            // clocks the sweep unit of the channel
            void clockSweep();

            // handles the write operations for the registers connected to this channel
            void write(u16 addr, u8 data);

            // produces and returns an audio sample byte
            u8 output() const;

            // returns the current status of the channel
            inline u8 status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
            // helper function to handle (dis)enabling of the channel and related side effects
            inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
    };
}