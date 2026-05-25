#pragma once

#include "./NesMapper.hpp"
namespace NS_NES {
    /**
     * @class M000 M000.h "./Mappers/M000.h"
     * @brief NROM Mapper
     * @details
     * - CPU $6000-$7FFF: Unbanked PRG-RAM, mirrored as necessary to fill entire 8KiB window, write protectable with an external switch. (Family BASIC only)
     * 
     * - CPU $8000-$BFFF: First 16 KiB of PRG-ROM
     * 
     * - CPU $C000-$FFFF: Last 16 KiB of PRG-ROM (NROM-256) or mirror of $8000-$BFFF (NROM-128).
     * 
     * - PPU $0000-$1FFF: 8 KiB of CHR-ROM
     * 
     * All banks are fixed.
     */
    class M000 : public Mapper {
        public:
            /**
             * @brief Explicit Contructor
             * @param pBnk Number of PRG-ROM banks
             * @param pMem Reference to PRG-ROM memory in gamepak
             * @param cBnk Number of CHR-ROM banks
             * @param cMem Reference to CHR-ROM memory in gamepak
             */
            M000(u16 pBnk, vector<u8>* pMem, u16 cBnk, vector<u8>* cMem, vector<u8>* pRam = nullptr) :
                Mapper(pBnk, pMem, cBnk, cMem, pRam) {}

            /**
             * @brief Data read request from CPU for PRG memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects
             * @return The data that was read; or zero if invalid address.
             */
            u8 cpuRead(u16 addr, bool readonly = false) override {
                // TODO: implement PRG-RAM
                if (addr >= 0x8000) {
                    // mask address based on number of PRG-ROM banks
                    addr = addr & (prgBanks > 1 ? 0x7FFF : 0x3FFF);
                    // return requested data
                    return prgRom->at(addr);
                }

                // address is not mapped by mapper; invalid operation
                return 0x00;
            }

            /**
             * @brief Data read request from PPU for CHR memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects
             * @return The data that was read; or zero if invalid address.
             */
            u8 ppuRead(u16 addr, bool readonly = false) override {
                if (addr <= 0x1FFF)
                    return chrMem->at(addr);
                return 0x00;
            }

            /**
             * @brief Data write request from PPU for CHR memory.
             * @note The only CHR memory that is able to be written to is CHR-RAM;
             * if the game does not possess CHR-RAM, this function does nothing.
             * @param addr Address to be written to.
             * @param data Data to be written
             */
            void ppuWrite(u16 addr, u8 data) override {
                if (addr <= 0x1FFF && chrBanks == 0)
                    chrMem->at(addr) = data;
            }
    };
}