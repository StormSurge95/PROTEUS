#pragma once

#include "./Mapper.h"

namespace NES_NS {
    class M004 : public Mapper {
        public:
            M004(u8 pBnk, std::vector<u8>& pMem, u8 cBnk, std::vector<u8>& cMem) :
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
            void cpuWrite(u16 addr, u8 data) override {
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

            u8 ppuRead(u16 addr, bool readonly = false) override {
                return 0x00;
            }

            void ppuWrite(u16 addr, u8 data) override {

            }

            void clock(u64 counter = 0x00) override {

            }

            MIRROR getMirrorMode() const override {
                return MIRROR::HARDWARE;
            }

        private:
            
    };
}