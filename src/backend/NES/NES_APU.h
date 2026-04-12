#pragma once

#include "../../core/IDevice.h"
#include "./NES_BUS.h"

#include <memory>
#include <vector>

enum CHANNEL_TYPE {
    PULSE1, PULSE2, TRIANGLE, NOISE, DMC
};

struct Divider {
    uint8_t reload = 0x00;
    uint8_t counter = 0x00;
};

class NES_APU_Channel {
    public:
        NES_APU_Channel() = default;
        virtual ~NES_APU_Channel() = default;

        virtual CHANNEL_TYPE which() const = 0;

        virtual void onWrite(uint16_t addr, uint8_t data) = 0;

        virtual void clock() = 0;
        virtual void onQuarterFrame() = 0;
        virtual void onHalfFrame() = 0;
};

class PulseChannel;

class SweepUnit {
    public:
        PulseChannel* channel = nullptr;

        Divider div = {};
        bool reload = false;

        bool enabled = false;
        bool negate = false;
        uint8_t shift = 0x00;

        uint16_t period = 0x0000;
        uint16_t target = 0x0000;

        SweepUnit(PulseChannel* c) { channel = c; }
        ~SweepUnit() = default;

        void onWrite(uint8_t data);
        void updateTarget();

        inline bool mute() const { return ((period < 8) || (target > 0x07FF)); }
        void onHalfFrame();
        inline void clock() { updateTarget(); }
};

class VolumeEnvelope {
    public:
        NES_APU_Channel* channel = nullptr;

        bool start = false;
        Divider div = {};
        uint8_t decayCounter = 0x00;
        bool loop = false;

        VolumeEnvelope(NES_APU_Channel* c) { channel = c; }
        ~VolumeEnvelope() = default;

        void onWrite(uint16_t addr, uint8_t data);

        void clock();
};

class LengthCounter {
    public:
        NES_APU_Channel* channel = nullptr;

        inline static const uint8_t LOADS[32] = {
            10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
            12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
        };

        bool enabled = false;
        bool halted = false;
        uint8_t counter = 0x00;
        bool halt = false;

        LengthCounter(NES_APU_Channel* c) { channel = c; }
        ~LengthCounter() = default;

        virtual void clock();
        virtual void onWrite(uint16_t addr, uint8_t data);
        inline void load(uint8_t index) { counter = LOADS[index]; }
};

class LinearCounter : public LengthCounter {
    public:
        uint8_t init = 0x00;
        bool reload = false;

        LinearCounter(NES_APU_Channel* c) : LengthCounter(c) {}
        ~LinearCounter() = default;

        void clock() override;
        void onWrite(uint16_t addr, uint8_t data) override;

};

class PulseChannel : public NES_APU_Channel {
    public:
        bool isPulse1 = false;

        uint8_t dutyCycle = 0x00;
        uint8_t dutyIndex = 0x00;
        bool constVol = false;

        SweepUnit* sweep = nullptr;
        VolumeEnvelope* envelope = nullptr;

        uint16_t timer = 0x0000;
        LengthCounter* counter;

        inline static const bool DUTIES[4][8] = {
            { 0, 0, 0, 0, 0, 0, 0, 1 },
            { 0, 0, 0, 0, 0, 0, 1, 1 },
            { 0, 0, 0, 0, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 0, 0 }
        };

        PulseChannel(bool isPulse1);
        ~PulseChannel();

        CHANNEL_TYPE which() const override { return isPulse1 ? PULSE1 : PULSE2; }

        void onWrite(uint16_t addr, uint8_t data) override;
        void clock() override;
        inline void onQuarterFrame() override { envelope->clock(); }
        void onHalfFrame() override;
        inline bool sequencer() const { return !!DUTIES[dutyCycle][dutyIndex]; }
        float sample() const;
};

class TriangleChannel : public NES_APU_Channel {
    public:
        TriangleChannel() {};
        ~TriangleChannel() {};
};

class NoiseChannel : public NES_APU_Channel {
    public:
        NoiseChannel() {};
        ~NoiseChannel() {};
};

class DMCChannel : public NES_APU_Channel {
    public:
        DMCChannel() {};
        ~DMCChannel() {};
};

class FrameCounter {
    public:
        NES_APU* apu;

        bool use5step = false;
        bool inhibitIRQ = false;
        uint16_t cycle = 0x00;

        inline static const uint16_t q1 = 7457;
        inline static const uint16_t q2 = 14913;
        inline static const uint16_t q3 = 22371;
        inline static const uint16_t q4_4 = 29830;
        inline static const uint16_t q4_5 = 37282;

        FrameCounter(NES_APU* a) { apu = a; }
        ~FrameCounter() = default;

        void onWrite(uint16_t addr, uint8_t data);

        void clock();
        void onQuarterFrame();
        void onHalfFrame();
};

class NES_APU : public IDevice<uint8_t, uint16_t> {
    public:
        std::shared_ptr<NES_BUS> bus;
        bool irqRequest = false;
        std::vector<float> audioBuffer;

        inline static const float CYCLES_PER_SAMPLE = 1789773.0f / 44100.0f;
        inline static const int MAX_BUFFER_LEN = 1000000;

        NES_APU();
        ~NES_APU();

        const float* getAudioBuffer(int& len) { len = (int)audioBuffer.size(); return audioBuffer.data(); }
        inline bool bufferIsFull() const { return audioBuffer.size() >= MAX_BUFFER_LEN; }

        struct CHANNELS {
            PulseChannel* pulse1 = nullptr;
            PulseChannel* pulse2 = nullptr;
            TriangleChannel* triangle = nullptr;
            NoiseChannel* noise = nullptr;
            DMCChannel* dmc = nullptr;

            CHANNELS();
            ~CHANNELS();
        }* channels;
        FrameCounter* counter = nullptr;

        std::vector<float> sampleBuffer = {};
        bool sampleReady = false;

        inline void connectBUS(std::shared_ptr<NES_BUS> b) { bus = b; }
        void reset();

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        void clock();

        void sample();
        float getSample();
};