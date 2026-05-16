#pragma once

#include "../shared/NES_PCH.h"

namespace NES_NS {
    class NoiseChannel {
            // Allow Debugger class to access all private members of the NoiseChannel class
            friend class Debugger;
        private:
            // enabled flag
            bool enabled = false;
            // mode flag
            bool mode = false;

            // current value of the timer
            u16 timer = 0x0000;
            // reload value of the timer
            u16 period = GetRateNoise(REGION::NTSC, 0);

            // linear feedback shift register
            u16 shiftRegister = 0x01;

            // length counter struct variable
            LengthCounter lengthCounter;

            // volume envelope struct variable
            VolumeEnvelope envelope;

            // calculate and process feedback for the LFSR
            void feedback();

            // determine and provide the sample value of the channel
            inline u8 volume() const { return envelope.constVol ? envelope.period : envelope.decay; }
        public:
            // default constructor
            NoiseChannel() = default;
            // default destructor
            ~NoiseChannel() = default;

            // clock the main channel timer
            void clockTimer();
            // clock the length counter
            void clockLength();
            // clock the volume envelope
            void clockEnvelope();

            // write data to the various registers of the channel
            void write(u16 addr, u8 data);

            // channel sample/volume getter
            u8 output() const;

            // channel status getter; returns whether length counter is greater than 0
            inline u8 status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
            // enable flag setter; if not enabled, length counter is halted at zero
            inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
    };
}