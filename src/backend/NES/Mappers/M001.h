#pragma once

#include "./Mapper.h"

class M001 : public Mapper {
    public:
        M001(uint8_t pBnk, std::vector<uint8_t>& pMem, uint8_t cBnk, std::vector<uint8_t>& cMem) :
            Mapper(pBnk, pMem, cBnk, cMem), PRGRam(0x2000, 0x00) {}

        void reset() {
            shiftReg = 0x10;
            control = 0x0C;

            CHRBank0 = 0;
            CHRBank1 = 0;
            PRGBank = 0;
        }

        uint8_t cpuRead(uint16_t addr, bool readonly = false) override {
            if (addr >= 0x8000) {
                uint32_t mappedAddr = mapPRG(addr);
                return PRGMemory->at(mappedAddr % PRGMemory->size());
            } else if (addr >= 0x6000 && !PRGRamDisabled)
                return PRGRam[addr & 0x1FFF];

            return 0x00;
        }

        void cpuWrite(uint16_t addr, uint8_t data) override {
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
                uint8_t val = shiftReg & 0x1F;
                writeRegister(addr, val);
                shiftReg = 0x10;
            }
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            if (addr < 0x2000) {
                uint32_t mappedAddr = mapCHR(addr);
                return CHRMemory->at(mappedAddr);
            }
            return 0x00;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {
            if (addr < 0x2000 && CHRBanks == 0) {
                uint32_t mappedAddr = mapCHR(addr);
                CHRMemory->at(mappedAddr) = data;
            }
        }

        void clock(uint64_t counter = 0x00) override {
            cpuCycle = counter;
        }

        MIRROR getMirrorMode() const override {
            switch (control & 0x03) {
                case 0: return ONE_SCREEN_LO;
                case 1: return ONE_SCREEN_HI;
                case 2: return VERTICAL;
                default: return HORIZONTAL;
            }
        }

    private:
        uint8_t shiftReg = 0x10;
        uint8_t control = 0x1C;

        uint8_t CHRBank0 = 0;
        uint8_t CHRBank1 = 0;
        uint8_t PRGBank = 0;
        bool PRGRamDisabled = false;
        std::vector<uint8_t> PRGRam;

        uint64_t cpuCycle = 0;
        uint64_t lastWriteCycle = 0xFFFFFFFFFFFFFFFF;

        void writeRegister(uint16_t addr, uint8_t val) {
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

        uint32_t mapPRG(uint16_t addr) {
            uint8_t mode = (control >> 2) & 0x03;
            uint32_t b16c = static_cast<uint32_t>(PRGBanks);
            if (b16c < 1) b16c = 1;
            uint32_t b32c = b16c / 2;
            if (b32c < 1) b32c = 1;

            uint32_t bank = 0x00;
            uint32_t offset = addr & 0x3FFF;

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

        uint32_t mapCHR(uint16_t addr) {
            bool mode = !!((control >> 4) & 0x01);
                
            uint32_t count = (uint32_t)CHRMemory->size() / 0x1000;

            uint32_t b0 = CHRBank0 % count;
            uint32_t b1 = CHRBank1 % count;

            if (mode) {
                if (addr < 0x1000)
                    return (((hasCHR_RAM ? (uint32_t)CHRBank0 : b0) << 12) + addr) % CHRMemory->size();
                else
                    return (((hasCHR_RAM ? (uint32_t)CHRBank1 : b1) << 12) + (addr & 0x0FFF)) % CHRMemory->size();
            } else {
                uint32_t bank = (uint32_t)((hasCHR_RAM ? CHRBank0 : b0) & 0x1E) << 12;
                return (bank + addr) % CHRMemory->size();
            }
        }
};