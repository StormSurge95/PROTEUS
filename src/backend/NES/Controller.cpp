#include "./Controller.h"

using namespace NES_NS;

u8 Controller::onRead() {
    // strobe flag only matters on player 1 controller
    bool isStrobe = (player == 1 ? strobe : other.lock()->strobe);
    // on official nintendo brand controllers, all reads after first 8 return 1
    if (cursor >= 8) return 1;

    if (isStrobe)
        // while strobe flag is set, always return status of button 0 (A button).
        return +(buttons[0]);
    else {
        // otherwise, return status of button referred to by cursor.
        bool p = buttons[cursor];
        // then increment cursor
        cursor++;
        return +p;
    }
}

void Controller::onWrite(u8 data) {
    // update strobe flag
    strobe = data & 0x01;

    if (strobe) {
        // if strobe is set, reset cursor on both controllers
        cursor = other.lock()->cursor = 0;
    }
}