#include "./APU.h"
#include "./BUS.h"
#include "./Gamepak.h"
#include "./Controller.h"
#include "./PPU.h"

using namespace NES_NS;

BUS::BUS() {
    // initialize WRAM
    ram.fill(0x00);
}

u8 BUS::read(u16 addr, bool readonly) {
    // update last read address for use during dummy dma reads.
    lastReadAddr = addr;
    // create helper variable to prevent updating open bus when readonly is set
    u8 ret = 0x00;
    // All reads directly update the open bus in some way, sometimes only partially
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        // read from WRAM
        ret = ram[addr & 0x07FF];
        if (readonly) return ret;
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // read from PPU registers
        ret = ppu.lock()->read(addr, readonly);
        if (readonly) return ret;
    } else if (addr == 0x4014) {
        // read PPU OAM data
        ret = ppu.lock()->readOAMByte();
        if (readonly) return ret;
    } else if (addr == 0x4015) {
        // read APU status
        ret =  (openBus & 0x20) | (apu.lock()->read(addr, readonly) & 0xDF);
        if (readonly) return ret;
    } else if (addr == 0x4016) {
        // read Player 1 Controller
        ret = (openBus & 0xE0) | (player1.lock()->onRead() & 0x1F);
        if (readonly) return ret;
    } else if (addr == 0x4017) {
        // read player 2 Controller
        ret = (openBus & 0xE0) | (player2.lock()->onRead() & 0x1F);
        if (readonly) return ret;
    } else if (addr >= 0x6000 && addr <= 0xFFFF) {
        // read cartridge memory (including SRAM, if present)
        ret = cart.lock()->read(addr, readonly);
        if (readonly) return ret;
    }
    // TODO: Handle PRG-RAM open bus stuff

    // getting here means readonly is clear; so update openBus and return it.
    openBus = ret;
    return openBus;
}

void BUS::write(u16 addr, u8 data) {
    // all writes fully update open bus
    openBus = data;
    if (addr >= 0x0000 && addr <= 0x1FFF) // write to WRAM
        ram[addr & 0x07FF] = data;
    else if (addr >= 0x2000 && addr <= 0x3FFF) // write to PPU registers
        ppu.lock()->write(addr, data);
    else if (addr == 0x4014) { // write to PPU OAM; trigger OAMDMA
        dmaPage = data;
        dmaAddr = 0x00;
        oamActive = true;
        dmaDummy = true;
    } else if (addr == 0x4016) // write to Player 1 controller
        player1.lock()->onWrite(data);
    else if (addr >= 0x4000 && addr <= 0x4017) { // write to APU registers
        if (addr == 0x4015 && (data >> 4) > 0) {
            // enabled DMC triggers immediate DMCDMA
            dmcActive = true;
            dmaDummy = true;
        }
        apu.lock()->write(addr, data);
    } else if (addr >= 0x5FFF && addr <= 0xFFFF) // write to Cartridge memory (including SRAM, if present)
        cart.lock()->write(addr, data);
}

void BUS::connectCONT(sptr<Controller>& c, u8 player) {
    // if player 1, simply connect
    if (player == 1)
        player1 = c;
    // otherwise, connect controller and then interconnect both controllers
    else {
        player2 = c;
        // technically shouldn't be necessary because this is done within the NES constructor...
        // ...but oh well I guess
        player1.lock()->other = player2;
        player2.lock()->other = player1;
    }
}

void BUS::clockOAM(u64 counter) {
    /**
     * via https://nesdev.org/wiki/DMA#OAM_DMA:
     * OAM DMA copies 256 bytes from a CPU page to PPU OAM via the OAMDATA ($2004) register.
     * It is triggered by writing the page number (the high byte of the address) to OAMDMA ($4014).
     * OAM DMA is scheduled to halt the CPU on the first cycle after the register write.
     * In the common case, it performs a halt cycle, an optional alignment cycle, and 256 get/put pairs.
     * The 256 get/put pairs copy forward from the start of the page. Because DMA can only read on get cycles,
     * an alignment cycle performing no useful work may be required before being able to read. All together,
     * OAM DMA on its own takes 513 or 514 cycles, depending on whether alignment is needed.
     * OAM DMA will copy from the page most recently written to $4014. This means that read-modify-write
     * instructions such as INC $4014, which are able to perform a second write before the CPU can be halted, will copy from the second page written, not the first.
     * OAM DMA has a lower priority than DMC DMA. If a DMC DMA get occurs during OAM DMA, OAM DMA is briefly paused.
     */
    bool put = (counter & 0x01) > 0; // determine first cycle; odd = put, !odd = get
    if (dmaDummy) { // initial halt cycle
        if (put) // no alignment needed
            dmaDummy = false;
        else // alignment cycle (dummy read) needed
            read(lastReadAddr);
    } else {
        if (!put) // 'get' oam data from WRAM
            dmaData = read(((u16)dmaPage << 8) | dmaAddr);
        else { // 'put' oam data into PPU memory
            sptr<PPU> ppup = ppu.lock();
            u8 i = (ppup->getOAMADDR() + dmaAddr) & 0xFF;
            ppup->writeOAMByte(i, dmaData);

            // DMA should not update OAMADDR within the ppu
            // instead, increment helper variable
            dmaAddr++;
            if (dmaAddr == 0x00) {
                // helper variable is 8 bits so that overflow to 0 means
                // we have performed the put operation 256 times precisely
                oamActive = false;
                dmaDummy = true;
            }
        }
    }
}

void BUS::clockDMC(u64 counter) {
    /**
     * via https://nesdev.org/wiki/DMA#DMC_DMA:
     * DMC DMA copies a single byte to the DMC unit's sample buffer. This occurs automatically after the DMC
     * enable bit, bit 4, of the sound channel enable register ($4015) is set to 1, which starts DPCM sample
     * playback using the current DMC settings in registers $4010-$4013. DMC DMA is scheduled when all of DPCM
     * playback is enabled, there are bytes left in the sample, and the sample buffer is empty. In the common
     * cases, DMC DMA performs a halt cycle, a dummy cycle, an optional alignment cycle, and a get.
     * 
     * The exact timing depends on the type of DMC DMA. There are two types: load and reload. Load DMAs occur
     * after $4015 D4 is set, but only if the sample buffer is empty. They are scheduled to halt the CPU on
     * a get cycle during the 2nd APU cycle after the write (that is, the 3rd or 4th CPU cycle). Reload DMAs
     * occur in response to the sample buffer being emptied. Unlike load DMAs, they are scheduled to halt
     * the CPU on a put cycle.
     * 
     * After the halt, DMC DMA always performs a dummy cycle where no work is done. If the next cycle is not
     * a get cycle, then a cycle will be spent on alignment. Then the DMA read is performed.
     * 
     * DMC DMA normally takes 3 or 4 cycles, depending on whether alignment is needed. Because load and reload
     * DMAs schedule on different cycle types, load DMAs take 3 cycles and reload DMAs take 4 unless the halt
     * is delayed by an odd number of cycles. However, bugs can cause additional cycles.
     */
    bool put = (counter & 0x01) > 0; // determine first cycle; odd = put, !odd = get
    // odd = put
    // !odd = get
    // load = halt on get
    // REload = halt on put
    if (dmaDummy) {
        if (put)
            dmaDummy = false;
        else
            read(lastReadAddr);
    } else {
        apu.lock()->dmcFetch(!put);
    }
}