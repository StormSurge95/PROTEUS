#pragma once

#include <cstdint>

class NES_APU;
class NES_BUS;

class NES_DMCChannel {
    private:
        bool enabled = false;
        bool interrupt = false;
        bool active = false;
        bool loop = false;

        NES_APU* apu;

        uint16_t timer = 0x0000;
        uint16_t period = 0x0000;

        uint16_t sampleAddress = 0x0000;
        uint16_t sampleLength = 0x0000;
        uint8_t sampleBuffer = 0x00;
        uint8_t shifter = 0x00;
        uint16_t currentAddr = 0x0000;
        uint16_t bytesRemaining = 0x0000;
        uint8_t bitsRemaining = 0x00;

        inline static const uint16_t RATE_TABLE[16] = {
            428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
        };

        uint8_t outputLevel = 0x00;

        void fetchByte();
    public:
        NES_DMCChannel(NES_APU* apu);
        ~NES_DMCChannel() = default;

        void clockTimer();
        void clockShifter();

        void write(uint16_t addr, uint8_t data);

        uint8_t output() const;

        inline uint8_t status() const { return ((+interrupt << 7) | (+active << 4)); }
        inline void enable(bool set) { enabled = set; }
};