#pragma once

#include "./Mapper.h"
namespace NES_NS {
    class M000 : public Mapper {
        public:
            M000(u8 pBnk, std::vector<u8>& pMem, u8 cBnk, std::vector<u8>& cMem) :
                Mapper(pBnk, pMem, cBnk, cMem) {}

            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr >= 0x8000) {
                    addr = addr & (PRGBanks > 1 ? 0x7FFF : 0x3FFF);
                    return PRGMemory->at(addr);
                }

                return 0x00;
            }

            u8 ppuRead(u16 addr, bool readonly = false) override {
                if (addr <= 0x1FFF)
                    return CHRMemory->at(addr);
                return 0x00;
            }

            void ppuWrite(u16 addr, u8 data) override {
                if (!hasCHR_RAM) return; // Only CHR-RAM is writable

                if (addr <= 0x1FFF)
                    CHRMemory->at(addr) = data;
            }
    };
}