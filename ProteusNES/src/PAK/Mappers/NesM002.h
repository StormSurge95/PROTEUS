#pragma once

#include "./NesMapper.h"

namespace NS_NES {
    /**
     * @class M002 M002.h "./Mappers/M002.h"
     * @brief UxROM Mapper
     * @details
     * iNES Mapper 002 is the implementation of the most common usage of UxROM compatible boards
     * Example Games:
     * - Mega Man
     * - Castlevania
     * - Contra
     * - Duck Tales
     * - Metal Gear
     * 
     * BANKS:
     * - CPU $8000-$BFFF: 16 KB switchable PRG-ROM bank
     * - CPU $C000-$FFFF: 16 KB PRG-ROM bank, fixed to the last bank
     * 
     * REGISTERS:
     * - Bank select ($8000-$FFFF)
     */
    class M002 : public Mapper {
        public:
            /**
             * @brief Explicit Constructor
             * @param pBnk Number of PRG-ROM banks
             * @param pMem Reference to PRG memory on gamepak.
             * @param cBnk Number of CHR-ROM banks
             * @param cMem Reference to CHR memory on gamepak.
             */
            M002(u16 pBnk, vector<u8>* pMem, u16 cBnk, vector<u8>* cMem, vector<u8>* pRam = nullptr, u8 id2 = 0) :
                Mapper(pBnk, pMem, cBnk, cMem, pRam, id2) {
            }

            void powerup() override {

            }

            void reset() override {

            }

            void powerdown() override {
                
            }

            /**
             * @brief Data read request from CPU for PRG memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or zero if invalid address.
             */
            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr >= 0x8000) {
                    size_t mapped = (addr & 0x3FFF);
                    if (addr <= 0xBFFF) {
                        mapped += static_cast<size_t>(prgBank) * 0x4000; // 16KB bank
                    } else {
                        mapped += static_cast<size_t>(prgBanks - 1) * 0x4000;  // fixed last bank
                    }

                    u8 ret = prgRom->at(mapped);
                    if (eventSink) eventSink->OnMapperRegisterRead(format("PRG-ROM: {:04X}", mapped), addr, ret);
                    return ret;
                }
                return 0x00;
            }

            /**
             * @brief Data write request from CPU for PRG memory.
             * @todo TODO: implement PRG-RAM
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void cpuWrite(u16 addr, u8 data) override {
                if (addr >= 0x8000) {
                    if (prgBanks == 0) prgBank = 0;
                    else {
                        prgBank = data % ((prgBanks > 1) ? (prgBanks - 1) : 1);
                    }
                    if (eventSink) eventSink->OnMapperRegisterWrite("UxROM - PRG Bank", addr, data);
                } else if (addr >= 0x6000 && prgRam != nullptr) {
                    u16 mapped = addr & 0x1FFF;
                    if (eventSink) eventSink->OnMapperRegisterWrite(format("PRG-RAM: {:04X}", mapped), addr, data);
                    prgRam->at(mapped) = data;
                }
            }

            /**
             * @brief Data read request from PPU for CHR memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or zero if invalid address.
             */
            u8 ppuRead(u16 addr, bool readonly = false) override {
                if (addr < 0x2000) {
                    u8 ret = chrMem->at(addr);
                    if (eventSink) eventSink->OnMapperRegisterRead(chrBanks == 0 ? "CHR-RAM" : "CHR-ROM", addr, ret);
                    return ret;
                }
                return 0x00;
            }

            /**
             * @brief Data write request from PPU for CHR memory.
             * @note If the game does not possess CHR-RAM, this function does nothing.
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void ppuWrite(u16 addr, u8 data) override {
                if (addr < 0x2000 && chrBanks == 0) {
                    chrMem->at(addr) = data;
                    if (eventSink) eventSink->OnMapperRegisterWrite("CHR-RAM", addr, data);
                }
            }

            vector<array<string, 2>> getDebugData() override { return {}; }

        private:
            /**
             * @brief The current bank of PRG-ROM to use for reading
             */
            u8 prgBank = 0;
    };
}