#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class BUS : IDevice<u8, u16> {
        friend class Debugger;
        public:
            bool oamActive = false;
            bool dmcActive = false;
            bool dmaDummy = true;

            BUS();
            ~BUS() = default;

            u8 read(u16 addr, bool readonly = false) override;
            void write(u16 addr, u8 data) override;

            void connectCPU(sptr<CPU> c) { cpu = c; }
            void connectPPU(sptr<PPU> p) { ppu = p; }
            void connectAPU(sptr<APU> a) { apu = a; }
            void connectCART(sptr<Gamepak> c) { cart = c; }
            void connectCONT(sptr<Controller>, u8);
            void clockOAM(u64);
            void clockDMC(u64);

            //string getPPUstatus();

        private:
            array<u8, 2048> ram;
            sptr<CPU> cpu = nullptr;
            sptr<PPU> ppu = nullptr;
            sptr<APU> apu = nullptr;
            sptr<Gamepak> cart = nullptr;
            sptr<Controller> player1 = nullptr;
            sptr<Controller> player2 = nullptr;

            u8 openBus = 0x00;

            u8 dmaPage = 0x00;
            u8 dmaAddr = 0x00;
            u8 dmaData = 0x00;

            u16 lastReadAddr = 0x0000;
    };
}