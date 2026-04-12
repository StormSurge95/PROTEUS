#pragma once

#include "./Mapper.h"

class M003 : public Mapper {
    public:
    M003(uint8_t pBnk, std::vector<uint8_t>& pMem, uint8_t cBnk, std::vector<uint8_t>& cMem) :
        Mapper(pBnk, pMem, cBnk, cMem) {
    };

    uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
        if (addr >= 0x8000 && addr <= 0xFFFF)
            return PRGMemory->at(addr & (PRGBanks == 1 ? 0x3FFF : 0x7FFF));

        return 0x00;
    }

    void cpuWrite(uint16_t addr, uint8_t data) override {
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            if (CHRBanks == 0) {
                bankSelect = 0;
                return;
            }

            uint8_t romData = cpuRead(addr, true);
            uint8_t latched = data & romData;

            if ((CHRBanks & (CHRBanks - 1)) == 0)
                bankSelect = latched & (CHRBanks - 1);
            else
                bankSelect = latched % CHRBanks;
        }
    }

    uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
        if (addr < 0x2000) {
            if (CHRBanks == 0)
                return CHRMemory->at(addr);

            return CHRMemory->at((size_t)bankSelect * 0x2000 + (size_t)addr);
        }

        return 0x00;
    }

    void ppuWrite(uint16_t addr, uint8_t data) override {
        if (addr < 0x2000 && CHRBanks == 0)
            CHRMemory->at(addr) = data;
    }

    private:
    uint8_t bankSelect = 0;
};