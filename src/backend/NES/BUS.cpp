#include "./APU.h"
#include "./BUS.h"
#include "./Gamepak.h"
#include "./Controller.h"
#include "./PPU.h"

using namespace NES_NS;

BUS::BUS() {
    ram.fill(0x00);
}

u8 BUS::read(u16 addr, bool readonly) {
    lastReadAddr = addr;

    if (addr >= 0x0000 && addr <= 0x1FFF)
        openBus = ram[addr & 0x07FF];
    else if (addr >= 0x2000 && addr <= 0x3FFF)
        openBus = ppu.lock()->read(addr, readonly);
    else if (addr == 0x4014)
        openBus = ppu.lock()->readOAMByte();
    else if (addr == 0x4015)
        return (openBus & 0x20) | (apu.lock()->read(addr, readonly) & 0xDF);
    else if (addr == 0x4016)
        openBus = (openBus & 0xE0) | (player1.lock()->onRead() & 0x1F);
    else if (addr == 0x4017)
        openBus = (openBus & 0xE0) | (player2.lock()->onRead() & 0x1F);
    else if (addr >= 0x6000 && addr <= 0xFFFF)
        openBus = cart.lock()->read(addr, readonly);
    // TODO: Handle PRG-RAM open bus stuff

    return openBus;
}

void BUS::write(u16 addr, u8 data) {
    openBus = data;
    if (addr >= 0x0000 && addr <= 0x1FFF)
        ram[addr & 0x07FF] = data;
    else if (addr >= 0x2000 && addr <= 0x3FFF)
        ppu.lock()->write(addr, data);
    else if (addr == 0x4014) {
        dmaPage = data;
        dmaAddr = 0x00;
        oamActive = true;
        dmaDummy = true;
    } else if (addr == 0x4016)
        player1.lock()->onWrite(data);
    else if (addr >= 0x4000 && addr <= 0x4017) {
        if (addr == 0x4015) {
            dmcActive = true;
            dmaDummy = true;
        }
        apu.lock()->write(addr, data);
    } else if (addr >= 0x5FFF && addr <= 0xFFFF)
        cart.lock()->write(addr, data);
}

void BUS::connectCONT(sptr<Controller>& c, u8 player) {
    if (player == 1)
        player1 = c;
    else {
        player2 = c;
        player1.lock()->other = player2;
        player2.lock()->other = player1;
    }
}

void BUS::clockOAM(u64 counter) {
    bool odd = !!(counter & 0x01);
    // odd = put
    // !odd = get
    if (dmaDummy) {
        if (odd)
            dmaDummy = false;
        else
            read(lastReadAddr);
    } else {
        if (!odd)
            dmaData = read(((u16)dmaPage << 8) | dmaAddr);
        else {
            u8 i = (ppu.lock()->getOAMADDR() + dmaAddr) & 0xFF;
            ppu.lock()->writeOAMByte(i, dmaData);

            dmaAddr++;
            if (dmaAddr == 0x00) {
                oamActive = false;
                dmaDummy = true;
            }
        }
    }
}

void BUS::clockDMC(u64 counter) {
    bool odd = !!(counter & 0x01);
    // odd = put
    // !odd = get
    // !load = halt on put
    // load = halt on get
    if (dmaDummy) {
        if (odd)
            dmaDummy = false;
        else
            read(lastReadAddr);
    } else {
        apu.lock()->dmcFetch(!odd);
    }
}