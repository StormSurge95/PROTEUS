#pragma once

#include "./Mapper.h"

namespace NES_NS {
    /**
     * @class M003 M003.h "./Mappers/M003.h"
     * @brief CNROM Mapper
     * @details
     * CNROM is a discrete-logic circuit board providing up to four 8 KB banks of CHR-ROM.
     * iNES Mapper 3 denotes the common usage of CNROM mounting 2-4 banks of CHR-ROM
     * 
     * There are three valid submappers which determine the existence of bus conflicts:
     * - Submapper 0: Bus conflict behavior unknown
     * - Submapper 1: No bus conflicts
     * - Submapper 2: AND-type bus conflicts
     * 
     * BANKS:
     * - CPU $6000-$7FFF: 2 KiB of PRG-RAM, mirrored three times (Hayauchi Super Igo only)
     * - CPU $8000-$FFFF: 32 KB unbanked PRG-ROM
     * - PPU $0000-$1FFF: 8KB switchable window into 32 KiB CHR-ROM
     * 
     * REGISTERS:
     * - Bank Select ($8000-$FFFF)
     */
    class M003 : public Mapper {
        public:
            /**
             * @brief Explicit Constructor
             * @param pBnk Number of PRG-ROM banks
             * @param pMem Reference to PRG memory on gamepak
             * @param cBnk Number of CHR-ROM banks
             * @param cMem Reference to CHR memory on gamepak
             */
            M003(u8 pBnk, vector<u8>& pMem, u8 cBnk, vector<u8>& cMem) :
                Mapper(pBnk, pMem, cBnk, cMem) {
            };

            /**
             * @brief Data read request from CPU for PRG memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or zero if invalid address.
             */
            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr >= 0x8000 && addr <= 0xFFFF)
                    return PRGMemory->at(addr & (PRGBanks == 1 ? 0x3FFF : 0x7FFF));

                return 0x00;
            }

            /**
             * @brief Data write request from CPU for PRG memory.
             * @todo TODO: implement PRG-RAM
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void cpuWrite(u16 addr, u8 data) override {
                if (addr >= 0x8000 && addr <= 0xFFFF) {
                    if (CHRBanks == 0) {
                        // With no CHR banks, only one bank selection value is possible.
                        bankSelect = 0;
                        return;
                    }

                    // The original CNROM board is always subject to AND-type bus conflicts:
                    // the effective value is the value being written bitwise-AND'd with the
                    // PRG-ROM content at the address being written to.So far, the only.NES
                    // files requiring the absense of bus conflicts have been mapper hacks to
                    // mapper 3 from other mappers.
                    u8 romData = cpuRead(addr, true); // PRG-ROM content at address being written to
                    u8 latched = data & romData; // effective value

                    // determine whether or not CHRBanks is a power of 2
                    bool po2 = (CHRBanks & (CHRBanks - 1)) == 0;

                    if (po2) // power of 2 means we can use AND to lessen CPU load
                        bankSelect = latched & (CHRBanks - 1);
                    else // otherwise; we have to use modulus
                        bankSelect = latched % CHRBanks;
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
                    if (CHRBanks == 0)
                        return CHRMemory->at(addr);

                    addr += ((u16)bankSelect << 13);

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
                if (addr < 0x2000 && CHRBanks == 0)
                    CHRMemory->at(addr) = data;
            }

        private:
            // keeps track of which CHR bank to use
            u8 bankSelect = 0;
    };
}