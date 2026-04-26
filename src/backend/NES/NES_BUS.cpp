#include "./NES_APU/NES_APU.h"
#include "./NES_BUS.h"
#include "./NES_CART.h"
#include "./NES_CONT.h"
#include "./NES_PPU/NES_PPU.h"

#include <format>
#include <memory>

NES_BUS::NES_BUS() {
    ram.fill(0xFF);
}

std::string NES_BUS::getPPUstatus() {
    return std::format("PPU:{:3},{:3}", ppu->cycle == 0 ? ppu->scanline - 1 : ppu->scanline, ppu->cycle == 0 ? 340 : ppu->cycle - 1);
}

uint8_t NES_BUS::read(uint16_t addr, bool readonly) {
    if (addr >= 0x0000 && addr <= 0x1FFF)
        openBus = ram[addr & 0x07FF];
    else if (addr >= 0x2000 && addr <= 0x3FFF)
        openBus = ppu->read(addr, readonly);
    else if (addr == 0x4014)
        openBus = ppu->readOAMByte();
    else if (addr >= 0x4000 && addr <= 0x4014)
        openBus = apu->read(addr, readonly);
    else if (addr == 0x4015)
        return (openBus & 0x20) | (apu->read(addr, readonly) & 0xDF);
    else if (addr == 0x4016)
        openBus = (openBus & 0xE0) | (player1->onRead() & 0x1F);
    else if (addr == 0x4017)
        openBus = (openBus & 0xE0) | (player2->onRead() & 0x1F);
    else if (addr >= 0x8000 && addr <= 0xFFFF)
        openBus = cart->read(addr, readonly);

    return openBus;
}

void NES_BUS::write(uint16_t addr, uint8_t data) {
    openBus = data;
    if (addr >= 0x0000 && addr <= 0x1FFF)
        ram[addr & 0x07FF] = data;
    else if (addr >= 0x2000 && addr <= 0x3FFF)
        ppu->write(addr, data);
    else if (addr == 0x4014) {
        oamActive = true;
        dmaPage = data;
        dmaAddr = 0x00;
        dmaDummy = true;
    } else if (addr == 0x4016)
        player1->onWrite(data);
    else if (addr >= 0x4000 && addr <= 0x4017) {
        if (addr == 0x4015) {
            dmcActive = true;
            dmaDummy = true;
        }
        apu->write(addr, data);
    } else if (addr >= 0x8000 && addr <= 0xFFFF)
        cart->write(addr, data);
}

void NES_BUS::connectCONT(std::shared_ptr<NES_CONT> c, uint8_t player) {
    if (player == 1)
        player1 = c;
    else {
        player2 = c;
        player1->other = player2;
        player2->other = player1;
    }
}

void NES_BUS::clockOAM(uint64_t counter) {
    bool odd = !!(counter & 0x01);
    if (dmaDummy) {
        if (odd)
            dmaDummy = false;
    } else {
        if (!odd)
            dmaData = read(((uint16_t)dmaPage << 8) | dmaAddr);
        else {
            uint8_t i = (ppu->read(0x2003, true) + dmaAddr) & 0xFF;
            ppu->writeOAMByte(i, dmaData);

            dmaAddr++;
            if (dmaAddr == 0x00) {
                oamActive = false;
                dmaDummy = true;
            }
        }
    }
}

void NES_BUS::clockDMC(uint64_t counter) {
    bool odd = !!(counter & 0x01);
    if (dmaDummy) {
        if (odd)
            dmaDummy = false;
    } else {
        if (!odd) {}
    }
}