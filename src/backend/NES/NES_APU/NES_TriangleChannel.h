#pragma once

#include <cstdint>

#include "./NES_LengthCounter.h"

class NES_TriangleChannel {
    private:
        bool enabled = false;

        uint16_t timer = 0x0000;
        uint16_t period = 0x0000;

        NES_LengthCounter lengthCounter;

        uint16_t linearCounter = 0x0000;
        uint16_t linearPeriod = 0x0000;
        bool linearReload = false;

        inline static const uint8_t SEQUENCE[32] = {
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        };
        uint8_t step = 0x00;
    public:
        NES_TriangleChannel() = default;
        ~NES_TriangleChannel() = default;

        void clockTimer();
        void clockLength();
        void clockLinear();

        void write(uint16_t addr, uint8_t data);

        uint8_t output() const;

        inline uint8_t status() const { return lengthCounter.counter > 0 ? 0x01 : 0x00; }
        inline void enable(bool set) { enabled = set; if (!enabled) lengthCounter.counter = 0x00; }
};