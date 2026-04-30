#pragma once

#include <vector>

#include "../../../core/IDevice.h"
#include "../NES_BUS.h"
#include "./NES_PulseChannel.h"
#include "./NES_TriangleChannel.h"
#include "./NES_NoiseChannel.h"
#include "./NES_DMCChannel.h"

class NES_APU : IDevice<uint8_t, uint16_t> {
    friend class NES_DBG;
    public:
        std::shared_ptr<NES_BUS> bus;
        bool irqRequested = false;

        NES_APU() = default;
        ~NES_APU() = default;

        inline void connectBUS(std::shared_ptr<NES_BUS> b) { bus = b; }

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        void clock();

        void collectSamples(std::vector<float>& out);

    private:

        uint64_t masterCycle = 0x0000;
        uint16_t cycle = 0x0000;
        uint64_t resetAt = 0x0000;
        uint8_t apuBus = 0x00;
        bool pendingReset = false;

        bool use5step = false;
        bool inhibitIRQ = false;

        const double CYCLES_PER_SAMPLE = 1789773.0 / 44100.0;
        double cycleAccumulator = 0.0;
        std::vector<float> sampleBuffer;

        NES_PulseChannel pulse1 = NES_PulseChannel(true);
        NES_PulseChannel pulse2 = NES_PulseChannel(false);
        NES_TriangleChannel triangle = NES_TriangleChannel();
        NES_NoiseChannel noise = NES_NoiseChannel();
        //NES_DMCChannel dmc = NES_DMCChannel(this);

        void clockFrameCounter();

        void quarterFrame();
        void halfFrame();

        void generateSample();

        void cycleReset();

        uint8_t read4015();
        void write4015(uint8_t data);
        void write4017(uint8_t data);

        float mixSamples(uint8_t pulse1, uint8_t pulse2, uint8_t triangle, uint8_t noise, uint8_t dmc);
};