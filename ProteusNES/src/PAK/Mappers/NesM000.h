#pragma once

#include "./NesMapper.h"
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
            M000(u16 pBnk, vector<u8>* pMem, u16 cBnk, vector<u8>* cMem, vector<u8>* pRam = nullptr, u8 id2 = 0) :
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
             * @param readonly Flag to block side-effects
             * @return The data that was read; or zero if invalid address.
             */
            bool cpuRead(u16 addr, u8& data, bool readonly = false) override {
                if (addr >= 0x8000) {
                    // mask address based on number of PRG-ROM banks
                    u16 mapped = addr & (prgBanks > 1 ? 0x7FFF : 0x3FFF);
                    // return requested data
                    data = prgRom->at(mapped);
                    if (eventSink) eventSink->OnMapperRegisterRead(string("PRG-ROM: ") + hex(mapped, 4), addr, data);
                    return true;
                } else if (addr >= 0x6000 && prgRam != nullptr) {
                    u16 mapped = addr & 0x1FFF;
                    data = prgRam->at(mapped);
                    if (eventSink) eventSink->OnMapperRegisterRead(string("PRG-RAM: ") + hex(mapped, 4), addr, data);
                    return true;
                }

                // address is not mapped by mapper; invalid operation
                return false;
            }

            /**
             * @brief Data write request from CPU for PRG memory.
             * @param addr Address to be written to.
             * @param data Data to be written.
             * @note If rom does not contain any PRG-RAM, this function does nothing.
             */
            void cpuWrite(u16 addr, u8 data) override {
                if (addr >= 0x6000 && addr < 0x8000 && prgRam != nullptr) {
                    u16 mapped = addr & 0x1FFF;
                    if (eventSink) eventSink->OnMapperRegisterWrite(string("PRG-RAM: ") + hex(mapped, 4), addr, data);
                    prgRam->at(mapped) = data;
                }
            }

            /**
             * @brief Data read request from PPU for CHR memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects
             * @return The data that was read; or zero if invalid address.
             */
            bool ppuRead(u16 addr, u8& data, bool readonly = false) override {
                if (addr <= 0x1FFF) {
                    data = chrMem->at(addr);
                    if (eventSink) eventSink->OnMapperRegisterRead(chrBanks == 0 ? "CHR-RAM" : "CHR-ROM", addr, data);
                    return true;
                }
                return false;
            }

            /**
             * @brief Data write request from PPU for CHR memory.
             * @note The only CHR memory that is able to be written to is CHR-RAM;
             * if the game does not possess CHR-RAM, this function does nothing.
             * @param addr Address to be written to.
             * @param data Data to be written
             */
            void ppuWrite(u16 addr, u8 data) override {
                if (addr <= 0x1FFF && chrBanks == 0) {
                    if (eventSink) eventSink->OnMapperRegisterWrite("CHR-RAM", addr, data);
                    chrMem->at(addr) = data;
                }
            }

            vector<array<string, 2>> getDebugData() override { return {{ "Mapper ID", "0 (NROM)" }}; };
    };
}