#pragma once

#include "./Mapper.h"
class M000 : public Mapper {
    public:
        M000(uint8_t pBnk, std::vector<uint8_t>& pMem, uint8_t cBnk, std::vector<uint8_t>& cMem) :
            Mapper(pBnk, pMem, cBnk, cMem) {}

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            if (addr >= 0x8000) {
                addr = addr & (PRGBanks > 1 ? 0x7FFF : 0x3FFF);
                return PRGMemory->at(addr);
            }

            return 0x00;
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            if (addr <= 0x1FFF)
                return CHRMemory->at(addr);
            return 0x00;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {
            if (!hasCHR_RAM) return; // Only CHR-RAM is writable

            if (addr <= 0x1FFF)
                CHRMemory->at(addr) = data;
        }
};