#pragma once

#include <cstdint>

#include "./NES_LengthCounter.h"

class NES_NoiseChannel {
    private:
        bool enabled = false;
        bool mode = false;

        uint16_t timer = 0x0000;
        uint16_t period = 0x0000;

        uint16_t shiftRegister = 0x01;

        NES_LengthCounter lengthCounter;

        bool envelopeStart = false;
        bool constantVolume = false;
        uint8_t envelopeDivider = 0x00;
        uint8_t envelopePeriod = 0x00;
        uint8_t decayLevel = 0x0F;

        inline static const uint16_t PERIOD_TABLE[16] = {
            4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
        };

        void feedback();

        inline uint8_t volume() const { return constantVolume ? envelopePeriod : decayLevel; }
    public:
        NES_NoiseChannel() = default;
        ~NES_NoiseChannel() = default;

        void clockTimer();
        void clockLength();
        void clockEnvelope();

        void write(uint16_t addr, uint8_t data);

        uint8_t output() const;

        inline uint8_t status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
        inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
};