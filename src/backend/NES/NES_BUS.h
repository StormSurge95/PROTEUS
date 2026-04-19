#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <string>

#include "../../core/IDataBus.h"

class NES_CPU;
class NES_APU;
class NES_PPU;
class NES_CONT;
class NES_CART;

class NES_BUS : IDataBus<uint8_t, uint16_t> {
    public:
        bool oamActive = false;
        bool dmcActive = false;

        NES_BUS(bool sstMode = false);
        ~NES_BUS() = default;

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        uint8_t readSST(uint16_t addr);
        void writeSST(uint16_t addr, uint8_t data);

        void connectCPU(std::shared_ptr<NES_CPU> c) { cpu = c; }
        void connectPPU(std::shared_ptr<NES_PPU> p) { ppu = p; }
        void connectAPU(std::shared_ptr<NES_APU> a) { apu = a; }
        void connectCART(std::shared_ptr<NES_CART> c) { cart = c; }
        void connectCONT(std::shared_ptr<NES_CONT> c, uint8_t player);
        void clockOAM(uint64_t counter);
        void clockDMC(uint64_t counter);

        std::string getPPUstatus();

    private:
        std::vector<uint8_t> ram;
        std::shared_ptr<NES_CPU> cpu = nullptr;
        std::shared_ptr<NES_PPU> ppu = nullptr;
        std::shared_ptr<NES_APU> apu = nullptr;
        std::shared_ptr<NES_CART> cart = nullptr;
        std::shared_ptr<NES_CONT> player1 = nullptr;
        std::shared_ptr<NES_CONT> player2 = nullptr;

        uint8_t openBus = 0x00;

        bool dmaDummy = true;
        bool sstMode = false;

        uint8_t dmaPage = 0x00;
        uint8_t dmaAddr = 0x00;
        uint8_t dmaData = 0x00;
};