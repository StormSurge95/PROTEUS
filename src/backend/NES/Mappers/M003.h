#pragma once

#include "./Mapper.h"

namespace NES_NS {
    class M003 : public Mapper {
        public:
            M003(u8 pBnk, std::vector<u8>& pMem, u8 cBnk, std::vector<u8>& cMem) :
                Mapper(pBnk, pMem, cBnk, cMem) {
            };

            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr >= 0x8000 && addr <= 0xFFFF)
                    return PRGMemory->at(addr & (PRGBanks == 1 ? 0x3FFF : 0x7FFF));

                return 0x00;
            }

            void cpuWrite(u16 addr, u8 data) override {
                if (addr >= 0x8000 && addr <= 0xFFFF) {
                    if (CHRBanks == 0) {
                        bankSelect = 0;
                        return;
                    }

                    u8 romData = cpuRead(addr, true);
                    u8 latched = data & romData;

                    if ((CHRBanks & (CHRBanks - 1)) == 0)
                        bankSelect = latched & (CHRBanks - 1);
                    else
                        bankSelect = latched % CHRBanks;
                }
            }

            u8 ppuRead(u16 addr, bool readonly = false) override {
                if (addr < 0x2000) {
                    if (CHRBanks == 0)
                        return CHRMemory->at(addr);

                    return CHRMemory->at((size_t)bankSelect * 0x2000 + (size_t)addr);
                }

                return 0x00;
            }

            void ppuWrite(u16 addr, u8 data) override {
                if (addr < 0x2000 && CHRBanks == 0)
                    CHRMemory->at(addr) = data;
            }

        private:
            u8 bankSelect = 0;
    };
}