#pragma once

#include "../shared/NesPCH.h"

namespace NS_NES {
    class DMC_Channel {
            // Allow Debugger class to access all private members of the DMC_Channel class
            friend class NesDebugger;
        private:
            bool irq_enable = false;
            bool loop_flag = true;
            bool enabled = false;
            //u8 rate_index = 0;
            u8 outputLevel = 0;
            u16 sampleAddrReload = 0x0000;
            u16 sampleLengthReload = 0x0000;
            u16 currentAddr = 0x0000;
            u16 bytesRemaining = 0;

            u16 timerPeriod = 0x0000;
            u16 timerCounter = 0x0000;

            u8 shiftReg = 0x00;
            u8 bitsRemaining = 8;

            u8 sampleBuffer = 0x00;
            bool sampleBufferFull = false;
            bool silence = true;

            void clockShiftRegister();

        public:
            bool pending_irq = false;
            bool bufferNeeded = false;

            DMC_Channel() = default;
            ~DMC_Channel() = default;

            void write(u16 addr, u8 data);

            void enable();
            void disable();

            void clockTimer();

            void onByteFetch(u8 byte);

            inline bool needsByteFetch() const { return !sampleBufferFull && bytesRemaining > 0; }

            inline u16 getCurrentAddr() const { return currentAddr; }

            inline u8 status() const { return bytesRemaining > 0 ? 1 : 0; }

            inline u8 output() const { return outputLevel; }
    };
}