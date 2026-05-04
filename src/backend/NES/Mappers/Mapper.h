#pragma once

#include "../NES_PCH.h"

namespace NES_NS {
    class Mapper {
        public:
            Mapper() = default;
            Mapper(u8 pBnk, vector<u8>& pMem, u8 cBnk, vector<u8>& cMem, u32 rSize, vector<u8>& rMem) :
                PRGBanks(pBnk), PRGMemory(&pMem), CHRBanks(cBnk), CHRMemory(&cMem), RAMSize(rSize), SAVMemory(&rMem) {
                hasCHR_RAM = CHRBanks == 0;
            }
            virtual ~Mapper() = default;

            virtual u8 cpuRead(u16 addr, bool readonly = false) { return 0x00; }
            virtual void cpuWrite(u16 addr, u8 data) {}

            virtual u8 ppuRead(u16 addr, bool readonly = false) { return 0x00; }
            virtual void ppuWrite(u16 addr, u8 data) {}

            virtual void clock(u64 counter = 0x00) {};
            virtual MIRROR getMirrorMode() const { return MIRROR::HARDWARE; }
        protected:
            u8 PRGBanks = 0;
            u8 CHRBanks = 0;
            u32 RAMSize = 0;
            bool hasCHR_RAM = false;
            vector<u8>* PRGMemory = nullptr;
            vector<u8>* CHRMemory = nullptr;
            vector<u8>* SAVMemory = nullptr;
    };
}