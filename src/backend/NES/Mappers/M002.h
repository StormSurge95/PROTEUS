#pragma once

#include "./Mapper.h"

namespace NES_NS {
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
            M002(u8 pBnk, vector<u8>& pMem, u8 cBnk, vector<u8>& cMem) :
                Mapper(pBnk, pMem, cBnk, cMem) {};

            /**
             * @brief Data read request from CPU for PRG memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or zero if invalid address.
             */
            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr < 0x8000) return 0x00;

                if (addr >= 0x8000 && addr <= 0xBFFF) { // map to switchable bank
                    addr &= 0x3FFF;
                    addr += ((u16)PRGBankSelect << 15);
                } else if (addr >= 0xC000) { // map to fixed last bank
                    addr &= 0x3FFF;
                    addr += ((u16)(PRGBanks - 1) << 15);
                }

                return PRGMemory->at(addr);
            }

            /**
             * @brief Data write request from CPU for PRG memory.
             * @todo TODO: implement PRG-RAM
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void cpuWrite(u16 addr, u8 data) override {
                if (addr >= 0x8000) PRGBankSelect = data & (PRGBanks - 1);
            }

            /**
             * @brief Data read request from PPU for CHR memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or zero if invalid address.
             */
            u8 ppuRead(u16 addr, bool readonly = false) override {
                if (addr < 0x2000) {
                    return CHRMemory->at(addr);
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
                if (addr < 0x2000)
                    CHRMemory->at(addr) = data;
            }

        private:
            /**
             * @brief The current bank of PRG-ROM to use for reading
             */
            u8 PRGBankSelect = 0;
    };
}