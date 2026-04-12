#pragma once

#include "./Mapper.h"

class M004 : public Mapper {
    public:
        M004(uint8_t pBnk, std::vector<uint8_t>& pMem, uint8_t cBnk, std::vector<uint8_t>& cMem) :
            Mapper(pBnk, pMem, cBnk, cMem) {}

        enum BANK {
            R0, R1, R2, R3, R4, R5, R6, R7
        };
        struct BANK_SELECT {
            BANK nextWrite = R0;
            bool PRGBankMode = false;
            bool CHRBankMode = false;
        } bankSelect = {};

        // TODO: FINISH MAPPER
        void cpuWrite(uint16_t addr, uint8_t data) override {
            if (addr >= 0x8000 && addr <= 0x9FFF) {
                if ((addr & 0x01) == 0x00) {
                    bankSelect.nextWrite = (BANK)(data & 0x07);
                    bankSelect.PRGBankMode = !!((data >> 6) & 0x01);
                    bankSelect.CHRBankMode = !!((data >> 7) & 0x01);
                } else {
                    switch (bankSelect.nextWrite) {
                        case R0:
                            if (bankSelect.CHRBankMode)
                                CHRMemory->at(addr & 0x17FF) = data;
                            else
                                CHRMemory->at(addr & 0x07FF) = data;
                            return;
                        case R1:
                        case R2:
                        case R3:
                        case R4:
                        case R5:
                        case R6:
                        case R7:
                        default:
                            return;
                    }
                }
            } else if (addr >= 0xA000 && addr <= 0xBFFF) {
                if ((addr & 0x01) == 0x00) {

                } else {

                }
            } else if (addr >= 0xC000 && addr <= 0xDFFF) {
                if ((addr & 0x01) == 0x00) {

                } else {

                }
            } else if (addr >= 0xE000 && addr <= 0xFFFF) {
                if ((addr & 0x01) == 0x00) {

                } else {

                }
            }
        }

        uint8_t ppuRead(uint16_t addr, bool readonly = false) override {
            return 0x00;
        }

        void ppuWrite(uint16_t addr, uint8_t data) override {

        }

        void clock(uint64_t counter = 0x00) override {

        }

        MIRROR getMirrorMode() const override {
            return HARDWARE;
        }

    private:
            
};