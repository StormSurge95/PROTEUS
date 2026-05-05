#pragma once

#include "./Mapper.h"

namespace NES_NS {
    class M001 : public Mapper {
        public:
            M001(u8 pBnk, std::vector<u8>& pMem, u8 cBnk, std::vector<u8>& cMem) :
                Mapper(pBnk, pMem, cBnk, cMem), PRGRam(0x2000, 0x00) {}

            void reset() {
                shiftReg = 0x10;
                control = 0x0C;

                CHRBank0 = 0;
                CHRBank1 = 0;
                PRGBank = 0;
            }

            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr >= 0x8000) {
                    u32 mappedAddr = mapPRG(addr);
                    return PRGMemory->at(mappedAddr % PRGMemory->size());
                } else if (addr >= 0x6000 && !PRGRamDisabled)
                    return PRGRam[addr & 0x1FFF];

                return 0x00;
            }

            void cpuWrite(u16 addr, u8 data) override {
                if (addr < 0x8000) {
                    if (PRGRamDisabled) return;
                    PRGRam[addr & 0x1FFF] = data;
                    return;
                }

                if (data & 0x80) {
                    shiftReg = 0x10;
                    control |= 0x0C;
                    return;
                }

                if (cpuCycle - lastWriteCycle < 2) {
                    lastWriteCycle = cpuCycle;
                    return;
                }

                lastWriteCycle = cpuCycle;

                bool complete = shiftReg & 0x01;

                shiftReg >>= 1;
                shiftReg |= (data & 0x01) << 4;

                if (complete) {
                    u8 val = shiftReg & 0x1F;
                    writeRegister(addr, val);
                    shiftReg = 0x10;
                }
            }

            u8 ppuRead(u16 addr, bool readonly = false) override {
                if (addr < 0x2000) {
                    u32 mappedAddr = mapCHR(addr);
                    return CHRMemory->at(mappedAddr);
                }
                return 0x00;
            }

            void ppuWrite(u16 addr, u8 data) override {
                if (addr < 0x2000 && CHRBanks == 0) {
                    u32 mappedAddr = mapCHR(addr);
                    CHRMemory->at(mappedAddr) = data;
                }
            }

            void clock(u64 counter = 0x00) override {
                cpuCycle = counter;
            }

            MIRROR getMirrorMode() const override {
                switch (control & 0x03) {
                    default:
                    case 3: return MIRROR::HORIZONTAL;
                    case 0: return MIRROR::ONE_SCREEN_LO;
                    case 1: return MIRROR::ONE_SCREEN_HI;
                    case 2: return MIRROR::VERTICAL;
                }
            }

        private:
            u8 shiftReg = 0x10;
            u8 control = 0x1C;

            u8 CHRBank0 = 0;
            u8 CHRBank1 = 0;
            u8 PRGBank = 0;
            bool PRGRamDisabled = false;
            std::vector<u8> PRGRam;

            u64 cpuCycle = 0;
            u64 lastWriteCycle = 0xFFFFFFFFFFFFFFFF;

            void writeRegister(u16 addr, u8 val) {
                switch ((addr >> 13) & 0x03) {
                    case 0: control = val; break;
                    case 1: CHRBank0 = val; break;
                    case 2: CHRBank1 = val; break;
                    case 3:
                        PRGRamDisabled = !!(val & 0x10);
                        PRGBank = val % PRGBanks;
                        break;
                }
            }

            u32 mapPRG(u16 addr) {
                u8 mode = (control >> 2) & 0x03;
                u32 b16c = static_cast<u32>(PRGBanks);
                if (b16c < 1) b16c = 1;
                u32 b32c = b16c / 2;
                if (b32c < 1) b32c = 1;

                u32 bank = 0x00;
                u32 offset = addr & 0x3FFF;

                switch (mode) {
                    case 0:
                    case 1:
                        bank = ((PRGBank & 0x0E) >> 1) % b32c;
                        return (bank * 0x8000) + (addr & 0x7FFF);
                    case 2:
                        if (addr < 0xC000) return (addr & 0x3FFF);
                        else {
                            bank = (PRGBank & 0x0F) % b16c;
                            return (bank * 0x4000) + offset;
                        }
                    default:
                        if (addr < 0xC000) {
                            bank = (PRGBank & 0x0F) % b16c;
                            return (bank * 0x4000) + offset;
                        } else
                            return ((PRGBanks - 1) * 0x4000) + offset;
                }
            }

            u32 mapCHR(u16 addr) {
                bool mode = !!((control >> 4) & 0x01);
                
                u32 count = (u32)CHRMemory->size() / 0x1000;

                u32 b0 = CHRBank0 % count;
                u32 b1 = CHRBank1 % count;

                if (mode) {
                    if (addr < 0x1000)
                        return (((hasCHR_RAM ? (u32)CHRBank0 : b0) << 12) + addr) % CHRMemory->size();
                    else
                        return (((hasCHR_RAM ? (u32)CHRBank1 : b1) << 12) + (addr & 0x0FFF)) % CHRMemory->size();
                } else {
                    u32 bank = (u32)((hasCHR_RAM ? CHRBank0 : b0) & 0x1E) << 12;
                    return (bank + addr) % CHRMemory->size();
                }
            }
    };
}