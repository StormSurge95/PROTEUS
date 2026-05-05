#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class APU : IDevice<u8, u16> {
        friend class Debugger;
        public:
            sptr<BUS> bus;
            bool irqRequested = false;

            APU();
            ~APU();

            inline void connectBUS(sptr<BUS> b) { bus = b; }

            u8 read(u16, bool = false) override;
            void write(u16, u8) override;

            void clock();

            void dmcFetch(bool);

            void collectSamples(vector<float>& out);

        private:
            u64 masterCycle = 0x0000000000000000;
            u16 cycle = 0x0000;
            u64 resetAt = 0x0000000000000000;
            u8 apuBus = 0x00;
            bool pendingReset = false;

            bool use5step = false;
            bool inhibitIRQ = false;

            const double CYCLES_PER_SAMPLE = 1789773.0 / 44100.0;
            double cycleAccumulator = 0.0;
            vector<float> sampleBuffer;

            uptr<PulseChannel> pulse1 = nullptr;
            uptr<PulseChannel> pulse2 = nullptr;
            uptr<TriangleChannel> triangle = nullptr;
            uptr<NoiseChannel> noise = nullptr;
            uptr<DMC_Channel> dmc = nullptr;

            void clockFrameCounter();

            void quarterFrame();
            void halfFrame();

            void generateSample();

            void cycleReset();

            u8 read4015();
            void write4015(u8 data);
            void write4017(u8 data);

            float mixSamples(u8 pulse1, u8 pulse2, u8 triangle, u8 noise, u8 dmc);
    };
}