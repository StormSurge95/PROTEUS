#pragma once

#include "./Mapper.h"

namespace NES_NS {
    class M002 : public Mapper {
        public:
            M002(u8 pBnk, std::vector<u8>& pMem, u8 cBnk, std::vector<u8>& cMem) :
                Mapper(pBnk, pMem, cBnk, cMem) {};

            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr >= 0x8000 && addr <= 0xBFFF) addr = (PRGBankSelect * 0x4000) + (addr & 0x3FFF);
                else if (addr >= 0xC000) addr = ((PRGBanks - 1) * 0x4000) + (addr & 0x3FFF);

                return PRGMemory->at(addr);
            }

            void cpuWrite(u16 addr, u8 data) override {
                if (addr >= 0x8000) PRGBankSelect = data & (PRGBanks - 1);
            }

            u8 ppuRead(u16 addr, bool readonly = false) override {
                if (addr < 0x2000) {
                    return CHRMemory->at(addr);
                }
                return 0x00;
            }

            void ppuWrite(u16 addr, u8 data) override {
                if (addr < 0x2000)
                    CHRMemory->at(addr) = data;
            }

        private:
            u8 PRGBankSelect = 0;
    };
}