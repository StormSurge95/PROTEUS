#pragma once

#include "../../core/IDevice.h"

#include <array>
#include <memory>

enum NES_BUTTONS {
    BUTTON_A,
    BUTTON_B,
    BUTTON_SELECT,
    BUTTON_START,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT
};

class NES_CONT : public IODevice<uint8_t> {
    public:
        bool strobe = false;
        uint8_t cursor = 0x00;
        std::shared_ptr<NES_CONT> other = nullptr;

        NES_CONT(uint8_t p) { player = p; }
            
        uint8_t onRead() override;
        void onWrite(uint8_t data) override;

        inline void update(NES_BUTTONS btn, bool isPressed) { buttons[btn] = isPressed; }
    private:
        std::array<bool, 8> buttons = { false };
        uint8_t player = 0x00;
};