#pragma once

#include "./Mapper.h"

class M002 : public Mapper {
    public:
        M002(uint8_t pBnk, std::vector<uint8_t>& pMem, uint8_t cBnk, std::vector<uint8_t>& cMem) :
            Mapper(pBnk, pMem, cBnk, cMem) {};

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            if (addr >= 0x8000 && addr <= 0xBFFF) addr = (PRGBankSelect * 0x4000) + (addr & 0x3FFF);
            else if (addr >= 0xC000) addr = ((PRGBanks - 1) * 0x4000) + (addr & 0x3FFF);

            return PRGMemory->at(addr);
        }

        void cpuWrite(uint16_t addr, uint8_t data) override {
            if (addr >= 0x8000) PRGBankSelect = data & (PRGBanks - 1);
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            if (addr < 0x2000) {
                return CHRMemory->at(addr);
            }
            return 0x00;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {
            if (addr < 0x2000)
                CHRMemory->at(addr) = data;
        }

    private:
        uint8_t PRGBankSelect = 0;
};