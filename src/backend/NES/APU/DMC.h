#pragma once

#include "../shared/NES_PCH.h"

namespace NES_NS {
    /// @class DMC_Channel DMC_Channel.h "./DMC_Channel.h"
    /// @brief Class representing the Delta Modulated Channel of the NES APU
    class DMC_Channel {
            // Allow Debugger class to access all private members of the DMC_Channel class
            friend class Debugger;
        private:
            /// @brief reference to the parent APU object
            APU* apu = nullptr;
            /// @brief irq enabled flag (DMC IRQ only)
            bool irqEnabled = false;
            /// @brief silent flag; used to determine whether the output pin sends a value or silence
            bool silent = false;
            /// @brief no sample flag; used to determine whether a dmc reload is necessary
            bool noSample = false;
            /// @brief loop flag; if set, we will simply restart from the beginning of the sample rather than request a reload
            bool loop = false;

            /// @brief starting address of current sample being played back
            u16 sampleAddr = 0x0000;
            /// @brief address of current byte of current sample
            u16 currAddr = 0x0000;
            /// @brief total number of bytes in current sample
            u16 sampleLength = 0x0000;
            /// @brief unplayed bytes left in current sample
            u16 bytesRemaining = 0x0000;

            /// @brief register to hold the next sample byte to be used for playback
            u8 sampleBuffer = 0x00;
            /// @brief shift register holding the current byte being processed
            u8 shifter = 0x00;
            /// @brief number of bits left before shift register is "empty"
            u8 bitsRemaining = 0x00;

            /// @brief reload value of channel timer
            u16 period = 1;
            /// @brief current value of channel timer
            u16 timer = 0x0000;

            /// @brief current sound volume to output on sample request
            u8 outputLevel = 0x00;

            /// @brief clocks the shift register
            void clockShifter();
            /// @brief starts a new byte within the shift register and clears `silence` flag if necessary
            void newOutputCycle();
        public:
            /// @brief enabled flag
            bool enabled = false;
            /// @brief interrupt flag (DMC IRQ interrupt only)
            bool interrupt = false;

            /**
             * @brief Explicit constructor
             * @param p Reference to the parent APU object
             */
            DMC_Channel(APU* p) : apu(p) {}
            /// @brief default destructor
            ~DMC_Channel() = default;

            u16 getSampleAddr() const { return sampleAddr; }
            u16 getCurrAddr() {
                u16 ret = currAddr;
                if (currAddr == 0xFFFF)
                    currAddr = 0x8000;
                else currAddr++;
                return ret;
            }
            void setSampleByte(u8 data) { sampleBuffer = data; }

            /**
             * @brief Data write operation for the DMC channel register(s)
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void write(u16 addr, u8 data);

            /// @brief Clocks the timer of the channel and performs necessary operations based on result
            void clockTimer();

            /**
             * @brief Provides an output sample
             * @return current `outputLevel` value, or zero if `silent` flag is set
             */
            inline u8 output() const { return silent ? 0x00 : outputLevel; }

            /**
             * @brief Performs the DMCDMA byte fetch based on cycle parity
             * @param first If set, collects the byte into `sampleBuffer` and increments address;
             * otherwise, modifies the various flags and observers for sample logic
             */
            void fetchSample(bool first);

            /**
             * @brief Getter for DMC channel status
             * @return whether or not there are unplayed bytes remaining within the sample
             */
            inline bool status() const { return bytesRemaining > 0; }
    };
}