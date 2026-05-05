#include "./Controller.h"

using namespace NES_NS;

u8 Controller::onRead() {
    bool isStrobe = (player == 1 ? strobe : other.lock()->strobe);
    if (cursor >= 8) return 1;

    if (isStrobe)
        return +(buttons[0]);
    else {
        bool p = buttons[cursor];
        cursor++;
        return +p;
    }
}

void Controller::onWrite(u8 data) {
    strobe = data & 0x01;

    if (strobe) {
        cursor = other.lock()->cursor = 0;
    }
}