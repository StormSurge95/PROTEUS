#include "./NES_APU.h"
#include "./NES_BUS.h"
#include "./NES_CART.h"
#include "./NES_CONT.h"
#include "./NES_PPU.h"

#include <format>
#include <memory>

NES_BUS::NES_BUS() {
    ram.fill(0x00);
}

std::string NES_BUS::getPPUstatus() {
    return std::format("PPU:{:3},{:3}", ppu->cycle == 0 ? ppu->scanline - 1 : ppu->scanline, ppu->cycle == 0 ? 340 : ppu->cycle - 1);
}

uint8_t NES_BUS::read(uint16_t addr, bool readonly) {
    if (addr >= 0x0000 && addr <= 0x07FF)
        return ram[addr & 0x07FF];
    if (addr >= 0x2000 && addr <= 0x3FFF)
        return ppu->read(addr, readonly);
    if (addr == 0x4014)
        return ppu->readOAMByte();
    if (addr >= 0x4000 && addr <= 0x4015)
        return apu->read(addr, readonly);
    if (addr == 0x4016)
        return player1->onRead();
    if (addr == 0x4017)
        return player2->onRead();
    if (addr >= 0x4020 && addr <= 0xFFFF)
        return cart->read(addr, readonly);

    return 0x00;
}

void NES_BUS::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0x1FFF)
        ram[addr & 0x07FF] = data;
    else if (addr >= 0x2000 && addr <= 0x3FFF)
        ppu->write(addr, data);
    else if (addr == 0x4014) {
        dmaPage = data;
        dmaAddr = 0x00;
        dmaActive = true;
        dmaDummy = true;
    } else if (addr == 0x4016)
        player1->onWrite(data);
    else if (addr >= 0x4000 && addr <= 0x4017)
        apu->write(addr, data);
    else if (addr >= 0x4020 && addr <= 0xFFFF)
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

void NES_BUS::clockDMA(uint64_t counter) {
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
                dmaActive = false;
                dmaDummy = true;
            }
        }
    }
}