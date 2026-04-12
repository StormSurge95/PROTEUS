#pragma once

#include <array>
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
        bool dmaActive = false;

        NES_BUS();
        ~NES_BUS() = default;

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        void connectCPU(std::shared_ptr<NES_CPU> c) { cpu = c; }
        void connectPPU(std::shared_ptr<NES_PPU> p) { ppu = p; }
        void connectAPU(std::shared_ptr<NES_APU> a) { apu = a; }
        void connectCART(std::shared_ptr<NES_CART> c) { cart = c; }
        void connectCONT(std::shared_ptr<NES_CONT> c, uint8_t player);
        void clockDMA(uint64_t counter);

        std::string getPPUstatus();

    private:
        std::array<uint8_t, 2048> ram;
        std::shared_ptr<NES_CPU> cpu = nullptr;
        std::shared_ptr<NES_PPU> ppu = nullptr;
        std::shared_ptr<NES_APU> apu = nullptr;
        std::shared_ptr<NES_CART> cart = nullptr;
        std::shared_ptr<NES_CONT> player1 = nullptr;
        std::shared_ptr<NES_CONT> player2 = nullptr;

        bool dmaDummy = true;

        uint8_t dmaPage = 0x00;
        uint8_t dmaAddr = 0x00;
        uint8_t dmaData = 0x00;
};