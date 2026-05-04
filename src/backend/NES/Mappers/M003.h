#pragma once

#include "./Mapper.h"

namespace NES_NS {
    class M003 : public Mapper {
        public:
            M003(u8 pBnk, std::vector<u8>& pMem, u8 cBnk, std::vector<u8>& cMem, u32 rSize, vector<u8>& rMem) :
                Mapper(pBnk, pMem, cBnk, cMem, rSize, rMem) {
            };

            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr >= 0x6000 && addr <= 0x7FFF)
                    return SAVMemory->at(addr & 0x1FFF);
                else
                    return PRGMemory->at(addr & (PRGBanks == 1 ? 0x3FFF : 0x7FFF));
            }

            void cpuWrite(u16 addr, u8 data) override {
                if (addr >= 0x6000 && addr <= 0x7FFF)
                    SAVMemory->at(addr & 0x1FFF) = data;
                else {
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