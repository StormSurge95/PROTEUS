#pragma once

#include "../shared/NES_PCH.hpp"

namespace NS_NES {
    class NoiseChannel {
            /// @brief Allow Debugger class to access all private members of the NoiseChannel class
            friend class Debugger;
        private:
            /// @brief enabled flag
            bool enabled = false;
            /// @brief mode flag
            bool mode = false;

            /// @brief current value of the timer
            u16 timer = 0x0000;
            /// @brief reload value of the timer
            u16 period = GetRateNoise(ConsoleRegion::NTSC, 0);

            /// @brief linear feedback shift register
            u16 shiftRegister = 0x01;

            /// @brief length counter struct variable
            LengthCounter lengthCounter;

            /// @brief volume envelope struct variable
            VolumeEnvelope envelope;

            /// @brief calculate and process feedback for the LFSR
            void feedback();

            /// @brief determine and provide the sample value of the channel
            inline u8 volume() const { return envelope.constVol ? envelope.period : envelope.decay; }
        public:
            /// @brief default constructor
            NoiseChannel() = default;
            /// @brief default destructor
            ~NoiseChannel() = default;

            /// @brief clock the main channel timer
            void clockTimer();
            /// @brief clock the length counter
            void clockLength();
            /// @brief clock the volume envelope
            void clockEnvelope();

            /// @brief write data to the various registers of the channel
            void write(u16 addr, u8 data);

            /// @brief channel sample/volume getter
            u8 output() const;

            /// @brief channel status getter; returns whether length counter is greater than 0
            inline u8 status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
            /// @brief enabled flag setter; if not enabled, length counter is halted at zero
            inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
    };
}