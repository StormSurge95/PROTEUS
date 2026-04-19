#pragma once

#include <cstdint>

#include "./NES_LengthCounter.h"

static const uint8_t NES_DUTY_TABLE[4][8] = {
    {0,0,0,0,0,0,0,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,1,1,1,1},
    {1,1,1,1,1,1,0,0}
};

class NES_PulseChannel {
    private:
        bool isPulse1 = false;
        bool enabled = false;

        uint16_t timer = 0x0000;
        uint16_t period = 0x0000;

        uint8_t dutyMode = 0x00;
        uint8_t dutyStep = 0x00;

        NES_LengthCounter lengthCounter;

        bool envelopeStart = false;
        bool constantVolume = false;
        uint8_t envelopeDivider = 0x00;
        uint8_t envelopePeriod = 0x00;
        uint8_t decayLevel = 0x0F;

        bool sweepEnabled = false;
        uint8_t sweepDivider = 0x00;
        uint8_t sweepPeriod = 0x00;
        bool sweepNegate = false;
        uint8_t sweepShift = 0x00;
        bool sweepReload = false;

        inline uint8_t volume() const { return constantVolume ? envelopePeriod : decayLevel; }

    public:
        NES_PulseChannel(bool p1): isPulse1(p1) {}
        ~NES_PulseChannel() = default;

        void clockTimer();
        void clockLength();
        void clockEnvelope();
        void clockSweep();

        void write(uint16_t addr, uint8_t data);

        uint8_t output() const;

        inline uint8_t status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
        inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
};