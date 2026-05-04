#pragma once

#include "./Mapper.h"

namespace NES_NS {
    class M000 : public Mapper {
        public:
            M000(u8 pBnk, vector<u8>& pMem, u8 cBnk, vector<u8>& cMem, u32 rSize, vector<u8>& rMem) :
                Mapper(pBnk, pMem, cBnk, cMem, rSize, rMem) {}

            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr >= 0x6000 && addr <= 0x7FFF) {
                    return SAVMemory->at(addr & 0x1FFF);
                }
                if (addr >= 0x8000) {
                    addr = addr & (PRGBanks > 1 ? 0x7FFF : 0x3FFF);
                    return PRGMemory->at(addr);
                }
            }

            void cpuWrite(u16 addr, u8 data) override {
                addr &= 0x1FFF;
                if (addr >= 0x6000 && addr <= 0x7FFF && RAMSize > addr) {
                    SAVMemory->at(addr) = data;
                }
            }

            u8 ppuRead(u16 addr, bool readonly = false) override {
                return CHRMemory->at(addr);
            }

            void ppuWrite(u16 addr, u8 data) override {
                if (!hasCHR_RAM) return; // Only CHR-RAM is writable

                if (addr <= 0x1FFF)
                    CHRMemory->at(addr) = data;
            }
    };
}