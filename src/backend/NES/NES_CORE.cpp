#include "NES_CORE.h"

#include <memory>

NES_CORE::NES_CORE(bool debug) {
    bus = std::make_shared<NES_BUS>();
    cpu = std::make_shared<NES_CPU>(debug);
    ppu = std::make_shared<NES_PPU>();
    player1 = std::make_shared<NES_CONT>(1);
    player2 = std::make_shared<NES_CONT>(2);
    player1->other = player2;
    player2->other = player1;

    cpu->connectBUS(bus);
    bus->connectCPU(cpu);

    bus->connectPPU(ppu);
    ppu->connectCPU(cpu);

    bus->connectCONT(player1, 1);
    bus->connectCONT(player2, 2);
}

bool NES_CORE::loadCart(const std::string& path) {
    cart = std::make_shared<NES_CART>(path);

    if (cart->isValid()) {
        bus->connectCART(cart);
        ppu->connectCART(cart);

        cpu->reset();
        ppu->reset();

        return true;
    }
    return false;
}

void NES_CORE::reset() {
    cpu->reset();
    ppu->reset();
}

void NES_CORE::clock() {
    do {
        // clock ppu - 3ppu:1cpu
        ppu->clock();

        if (masterClock % 3 == 0) {
            // clock cpu
            if (bus->dmaActive) bus->clockDMA(masterClock);
            else {
                cart->mapper->clock(masterClock);
                cpu->clock();
            }
        }

        if (ppu->nmiRequested) {
            cpu->nmiTrigger = true;
            ppu->nmiRequested = false;
        }

        masterClock++;
    } while (!ppu->frameComplete);

    ppu->frameComplete = false;
}

void NES_CORE::update(uint8_t player, bool* buttons) {
    if (player == 0) {
        for (int x = 0; x < 8; x++) {
            player1->update((NES_BUTTONS)x, buttons[x]);
        }
    } else if (player == 1) {
        for (int x = 0; x < 8; x++) {
            player2->update((NES_BUTTONS)x, buttons[x]);
        }
    }
}