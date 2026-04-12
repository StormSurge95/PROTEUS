#pragma once

#include <cstdint>
#include <vector>

#include "../NES_MIRRORS.h"

class NES_CART;

class Mapper {
    public:
        Mapper() = default;
        Mapper(uint8_t pBnk, std::vector<uint8_t>& pMem, uint8_t cBnk, std::vector<uint8_t>& cMem, bool hasRam = false) :
            PRGBanks(pBnk), PRGMemory(&pMem), CHRBanks(cBnk), CHRMemory(&cMem), hasPRG_RAM(hasRam) {
            hasCHR_RAM = CHRBanks == 0;
        }
        virtual ~Mapper() = default;

        virtual uint8_t cpuRead(uint16_t addr, bool readonly = false) { return 0x00; }
        virtual void cpuWrite(uint16_t addr, uint8_t data) {}

        virtual uint8_t ppuRead(uint16_t addr, bool readonly = false) { return 0x00; }
        virtual void ppuWrite(uint16_t addr, uint8_t data) {}

        virtual void clock(uint64_t counter = 0x00) {};
        virtual MIRROR getMirrorMode() const { return MIRROR::HARDWARE; }
    protected:
        uint8_t PRGBanks = 0;
        uint8_t CHRBanks = 0;
        bool hasPRG_RAM = false;
        bool hasCHR_RAM = false;
        std::vector<uint8_t>* PRGMemory = nullptr;
        std::vector<uint8_t>* CHRMemory = nullptr;
};