#include "./NES_CONT.h"

uint8_t NES_CONT::onRead() {
    bool isStrobe = (player == 1 ? strobe : other->strobe);
    if (cursor >= 8) return 1;

    if (isStrobe)
        return +(buttons[0]);
    else {
        bool p = buttons[cursor];
        cursor++;
        return +p;
    }
}

void NES_CONT::onWrite(uint8_t data) {
    strobe = data & 0x01;

    if (strobe) {
        cursor = other->cursor = 0;
    }
}