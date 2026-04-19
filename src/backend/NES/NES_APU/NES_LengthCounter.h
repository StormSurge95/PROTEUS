#pragma once

#include <cstdint>

static const uint8_t NES_LENGTH_TABLE[32] = {
    10, 254, 20,  2, 40,  4, 80,  6,
    160, 8, 60, 10, 14, 12, 26, 14,
    12, 16, 24, 18, 48, 20, 96, 22,
    192,24, 72, 26, 16, 28, 32, 30
};

struct NES_LengthCounter {
    bool halt = false;
    uint8_t counter = 0x00;
};