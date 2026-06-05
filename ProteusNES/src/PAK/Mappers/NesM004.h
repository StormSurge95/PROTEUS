#pragma once

#include "./NesMapper.h"

namespace NS_NES {
    /**
     * @class M004 M004.h "./Mappers/M004.h"
     * @brief MMC3 Mapper
     * @details
     * Nintendo MMC3 is a mapper ASIC used in Nintendo's TxROM Game Pak boards.
     * 
     * - BANKS:
     *   - CPU $6000-$7FFF: 8KB PRG-RAM bank (optional)
     *   - CPU $8000-$9FFF OR $C000-$DFFF: 8KB switchable PRG-ROM bank
     *   - CPU $A000-$BFFF: 8KB switchable PRG-ROM bank
     *   - CPU $C000-$DFFF OR $8000-$9FFF: 8KB switchable PRG-ROM bank
     *   - CPU $E000-$FFFF: 8KB PRG-ROM bank, fixed to the last bank
     *   - PPU $0000-$07FF OR $1000-$17FF: 2KB switchable CHR bank
     *   - PPU $0800-$0FFF OR $1800-$1FFF: 2KB switchable CHR bank
     *   - PPU $1000-$13FF OR $0000-$03FF: 1KB switchable CHR bank
     *   - PPU $1400-$17FF OR $0400-$07FF: 1KB switchable CHR bank
     *   - PPU $1800-$1BFF OR $0800-$0BFF: 1KB switchable CHR bank
     *   - PPU $1C00-$1FFF OR $0C00-$0FFF: 1KB switchable CHR bank
     * 
     * MMC3 has 4 pairs of registers at $8000-$9FFF, $A000-$BFFF, $C000-$DFFF, and $E000-$FFFF
     * Within each pair of registers:
     *      EVEN addresses ($8000, $8002, etc.) select the LOW register
     *      ODD addresses ($8001, $8003, etc.) select the HIGH register
     * These pairs can be broken down into two independent functional units:
     *      memory mapping ($8000-$9FFF & $A000-$BFFF)
     *      scanline counting ($C000-$DFFF & $E000-$FFFF)
     * 
     * - REGISTERS:
     *   - Bank Select ($8000-$9FFE, even)            \
     *   - Bank Data ($8001-$9FFF, odd)                \
     *                                                  |-->memory mapping
     *   - Nametable Arrangement ($A000-$BFFE, even)   /
     *   - PRG-RAM Protect ($A001-$BFFF, odd)         /
     * 
     *   - IRQ Latch ($C000-$DFFE, even)      \
     *   - IRQ Reload ($C001-$DFFF, odd)       \
     *                                          |-->scanline counting
     *   - IRQ Disable ($E000-$FFFE, even)     /
     *   - IRQ Enable ($E001-$FFFF, odd)      /
     */
    class M004 : public Mapper {
        public:
            M004(u16 pBnk, vector<u8>* pMem, u16 cBnk, vector<u8>* cMem, vector<u8>* pRam = nullptr, u8 id2 = 0) :
                Mapper(pBnk << 1, pMem, cBnk << 3, cMem, pRam, id2) {
            }

            void powerup() override {
                MMC6 = subMapperID == 1;

                bankSelect = 0;
                pMode = false;
                cMode = false;

                bankData = { 0, 0, 0, 0, 0, 0, 0, 0 };

                nametableArrangement = MIRROR::HORIZONTAL;

                ramProtect = { false, false, false, false, false, false };

                irqCounter = 0;
                irqReload = 0;
                irqEnabled = false;
                irqRequested = false;
                reloadPending = false;

                prevA12High = false;
                lowFetchStreak = 0;
            }

            void reset() override {
                powerup();
            }

            void powerdown() override {
                powerup();
            }

            /**
             * @brief Data read request from CPU for PRG memory.
             * @todo TODO: Ensure that all accesses respect maximum prgBanks.
             * @todo TODO: Incorporate PRG-RAM
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or zero if invalid address.
             */
            u8 cpuRead(u16 addr, bool readonly = false) override {
                u32 mapped = addr & 0x1FFF;
                u8 bank;
                if (addr >= 0xE000 && addr <= 0xFFFF) {
                    // fixed to last bank
                    bank = prgBanks - 1;
                } else if (addr >= 0x6000 && addr <= 0x7FFF) { // TODO: make this respect RAM protect
                    if (prgRam != nullptr) return prgRam->at(mapped);
                    else return 0x00; // TODO: make this return open bus
                } else if (pMode) { // PRG-ROM bank mode 1
                    if (addr >= 0x8000 && addr <= 0x9FFF) { // fixed to second-last bank
                        bank = prgBanks - 2;
                    } else if (addr >= 0xA000 && addr <= 0xBFFF) { // R7
                        bank = bankData.R7;
                    } else if (addr >= 0xC000 && addr <= 0xDFFF) { // R6
                        bank = bankData.R6;
                    }
                } else { // PRG-ROM bank mode 0
                    if (addr >= 0x8000 && addr <= 0x9FFF) { // R6
                        bank = bankData.R6;
                    } else if (addr >= 0xA000 && addr <= 0xBFFF) { // R7
                        bank = bankData.R7;
                    } else if (addr >= 0xC000 && addr <= 0xDFFF) { // second-last
                        bank = prgBanks - 2;
                    }
                }
                mapped += u16(bank) << 13;
                return prgRom->at(mapped);
            }

            /**
             * @brief Data write request from CPU for PRG memory.
             * @todo TODO: Incorporate PRG-RAM
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void cpuWrite(u16 addr, u8 data) override {
                bool even = (addr & 0x01) == 0;
                if (addr >= 0x6000 && addr <= 0x7FFF && prgRam != nullptr) { // TODO: make this respect RAM protect
                    prgRam->at(addr & 0x1FFF) = data;
                    return;
                }
                if (addr >= 0x8000 && addr <= 0x9FFF) { // Bank Select & Bank Data
                    if (even) { // Even: Bank Select
                        bankSelect = data;
                        pMode = ((bankSelect >> 6) & 0x01);
                        cMode = ((bankSelect >> 7) & 0x01);
                    } else { // Odd: Bank Data
                        u8 cdiv = chrMem->size() / 0x0400;
                        switch (bankSelect & 0x07) {
                            case 0: bankData.R0 = (data % chrBanks) & ~1; break;
                            case 1: bankData.R1 = (data % chrBanks) & ~1; break;
                            case 2: bankData.R2 = data % chrBanks; break;
                            case 3: bankData.R3 = data % chrBanks; break;
                            case 4: bankData.R4 = data % chrBanks; break;
                            case 5: bankData.R5 = data % chrBanks; break;
                            case 6: bankData.R6 = data % prgBanks; break;
                            case 7: bankData.R7 = data % prgBanks; break;
                        }
                    }
                }
                if (addr >= 0xA000 && addr <= 0xBFFF) { // Nametable Arrangment & PRG-RAM protect
                    if (even) { // Even: Nametable Arrangement
                        // TODO: this does nothing if hardware mirror is four screen
                        nametableArrangement = (data & 0x01) == 0 ? MIRROR::HORIZONTAL : MIRROR::VERTICAL;
                    } else { // Odd: PRG-RAM protect
                        if (MMC6) {
                            ramProtect.write_70_71 = ((data >> 4) & 0x01);
                            ramProtect.read_70_71 = ((data >> 5) & 0x01);
                            ramProtect.write_72_73 = ((data >> 6) & 0x01);
                            ramProtect.read_72_73 = ((data >> 7) & 0x01);
                        } else {
                            ramProtect.allowWrites = ((data >> 6) & 0x01);
                            ramProtect.chipEnable = ((data >> 7) & 0x01);
                        }
                    }
                }
                if (addr >= 0xC000 && addr <= 0xDFFF) { // IRQ Latch & IRQ Reload
                    if (even) // Even: latch
                        irqReload = data;
                    else { // Odd: Reload
                        // writing any value to this register clears the MMC3 IRQ counter immediately
                        // then reloads it at the NEXT rising edge of the PPU address
                        // presumably at PPU cycle 260 of the current scanline
                        irqCounter = 0x00;
                        reloadPending = true;
                    }
                }
                if (addr >= 0xE000 && addr <= 0xFFFF) { // IRQ Enable & IRQ Disable
                    if (even) // Even: disable
                        irqEnabled = irqRequested = false;
                    else // Odd: Enable
                        irqEnabled = true;
                }
            }

            /**
             * @brief Data read request from PPU for CHR memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or zero if invalid address.
             */
            u8 ppuRead(u16 addr, bool readonly = false) override {
                addr &= 0x1FFF;
                u32 mapped = addr;
                u8 bank;
                if (cMode) { // CHR A12 Inversion 1
                    if (addr <= 0x0FFF) { // four 1KB banks
                        mapped &= 0x03FF;
                        if (addr <= 0x03FF) bank = bankData.R2;
                        else if (addr <= 0x07FF) bank = bankData.R3;
                        else if (addr <= 0x0BFF) bank = bankData.R4;
                        else if (addr <= 0x0FFF) bank = bankData.R5;
                    } else { // two 2KB banks
                        mapped &= 0x07FF;
                        if (addr <= 0x17FF) bank = bankData.R0;
                        else if (addr <= 0x1FFF) bank = bankData.R1;
                    }
                } else { // CHR A12 Inversion 0
                    if (addr <= 0x0FFF) { // two 2KB banks
                        mapped &= 0x07FF;
                        if (addr <= 0x07FF) bank = bankData.R0;
                        else bank = bankData.R1;
                    } else { // four 1KB banks
                        mapped &= 0x03FF;
                        if (addr <= 0x13FF) bank = bankData.R2;
                        else if (addr <= 0x17FF) bank = bankData.R3;
                        else if (addr <= 0x1BFF) bank = bankData.R4;
                        else bank = bankData.R5;
                    }
                }
                mapped += u16(bank) << 10;
                return chrMem->at(mapped);
            }

            /**
             * @brief Data write request from PPU for CHR memory.
             * @note If there is no CHR-RAM; this function does nothing.
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void ppuWrite(u16 addr, u8 data) override {
                if (chrBanks != 0 || addr >= 0x2000) return;
                u32 mapped = addr;
                u8 bank;
                if (cMode) { // CHR A12 Inversion 1
                    if (addr <= 0x0FFF) { // four 1KB banks
                        mapped &= 0x03FF;
                        if (addr <= 0x03FF) bank = bankData.R2;
                        else if (addr <= 0x07FF) bank = bankData.R3;
                        else if (addr <= 0x0BFF) bank = bankData.R4;
                        else if (addr <= 0x0FFF) bank = bankData.R5;
                    } else { // two 2KB banks
                        mapped &= 0x07FF;
                        if (addr <= 0x17FF) bank = bankData.R0;
                        else if (addr <= 0x1FFF) bank = bankData.R1;
                    }
                } else { // CHR A12 Inversion 0
                    if (addr <= 0x0FFF) { // two 2KB banks
                        mapped &= 0x07FF;
                        if (addr <= 0x07FF) bank = bankData.R0;
                        else bank = bankData.R1;
                    } else { // four 1KB banks
                        mapped &= 0x03FF;
                        if (addr <= 0x13FF) bank = bankData.R2;
                        else if (addr <= 0x17FF) bank = bankData.R3;
                        else if (addr <= 0x1BFF) bank = bankData.R4;
                        else bank = bankData.R5;
                    }
                }
                mapped += u16(bank) << 10;
                chrMem->at(mapped) = data;
            }

            /**
             * @brief Keeps track of current ppu cycle/scanline/frame and is used to record/trigger IRQ
             * @param counter 
             */
            void ppuclock(u64 counter = 0x0000) override {}

            /**
             * @brief Keeps track of the PPU A12 line (bit 12 of address bus)
             *      and is used to clock the IRQ counter
             * @param addr The address in question to be used for
             *          observation of the A12 line.
             */
            void observeAddressPPU(u16 addr, bool isWrite) override {
                if (isWrite) return;

                bool currA12 = (addr & 0x1000) != 0;
                if (!currA12) {
                    if (lowFetchStreak < 0xFF)
                        lowFetchStreak++;
                    prevA12High = false;
                    return;
                }

                if (!prevA12High && lowFetchStreak >= LOW_FETCH_THRESH) {
                    clockIRQ();
                }

                prevA12High = true;
                lowFetchStreak = 0;
            }

            /**
             * @brief PPU nametable mirroring is controlled via the mapper
             * @note UNLESS the cartridge is hard-wired for four-screen mirroring
             * @todo TODO: find some way to check cartridge mirror mode
             * @return The current mirroring mode based on the nametableArrangement register
             */
            MIRROR getMirrorMode() const override {
                return nametableArrangement;
            }

            bool irqRequestActive() const override { return irqRequested; }

            vector<array<string, 2>> getDebugData() override { return {}; }

        private:
            struct BANK_DATA {
                u8 R0 = 0;
                u8 R1 = 0;
                u8 R2 = 0;
                u8 R3 = 0;
                u8 R4 = 0;
                u8 R5 = 0;
                u8 R6 = 0;
                u8 R7 = 0;
            } bankData;

            bool pMode = false;
            bool cMode = false;

            /**
             * @details
             * 7  bit  0
             * ---- ----
             * CPMx xRRR
             * |||   |||
             * |||   +++-- Specify which bank register to update on next write to BankData register
             * |||              000: R0: Select 2KB CHR bank at PPU $0000-$07FF (or $1000-$17FF)
             * |||              001: R1: Select 2KB CHR bank at PPU $0800-$0FFF (or $1800-$1FFF)
             * |||              010: R2: Select 1KB CHR bank at PPU $1000-$13FF (or $0000-$03FF)
             * |||              011: R3: Select 1KB CHR bank at PPU $1400-$17FF (or $0400-$07FF)
             * |||              100: R4: Select 1KB CHR bank at PPU $1800-$1BFF (or $0800-$0BFF)
             * |||              101: R5: Select 1KB CHR bank at PPU $1C00-$1FFF (or $0C00-$0FFF)
             * |||              110: R6: Select 8KB PRG ROM bank at $8000-$9FFF (or $C000-$DFFF)
             * |||              111: R7: Select 8KB PRG ROM bank at $A000-$BFFF
             * |||
             * ||+-------- PRG-RAM enable (MMC6 ONLY)
             * ||
             * |+--------- PRG-ROM Bank Mode
             * |                0: $8000-$9FFF swappable; $C000-$DFFF fixed to second-last bank
             * |                1: $C000-$DFFF swappable; $8000-$9FFF fixed to second-last bank
             * |
             * +---------- CHR A12 Inversion
             *                  0: Two 2KB banks at $0000-$0FFF; four 1KB banks at $1000-$1FFF
             *                  1: Two 2KB banks at $1000-$1FFF; four 1KB banks at $0000-$0FFF
             */
            u8 bankSelect = 0;

            /**
             * @brief This bit has no effect on cartridges with hardwired 4-screen VRAM (identified through bit 3 of byte 6 of the header).
             * @details
             * 7  bit  0
             * ---- ----
             * xxxx xxxN
             *         |
             *         +-- Nametable arrangement (0: horizontal [A10]; 1: vertical [A11])
             */
            MIRROR nametableArrangement = MIRROR::HORIZONTAL;

            /**
             * @brief Disabling PRG-RAM through bit 7 causes reads from the PRG-RAM region to return open bus.
             * @details
             * MMC3:
             * 7  bit  0
             * ---- ----
             * RMxx xxxx
             * ||
             * ||
             * |+--------- Write protection (0: allow writes; 1: deny writes)
             * +---------- PRG-RAM chip enable (0: disable; 1: enable)
             * 
             * MMC6:
             * 7  bit  0
             * ---- ----
             * HhLl xxxx
             * ||||
             * |||+------- Enable writing to RAM at $7000-$71FF
             * ||+-------- Enable reading RAM at $7000-$71FF
             * |+--------- Enable writing to RAM at $7200-$73FF
             * +---------- Enable reading RAM at $7200-$73FF
             */
            struct RAM_PROTECT {
                bool allowWrites = false;
                bool chipEnable = false;
                bool write_70_71 = false;
                bool read_70_71 = false;
                bool write_72_73 = false;
                bool read_72_73 = false;
            } ramProtect;

            u8 irqCounter = 0;
            u8 irqReload = 0;

            bool irqEnabled = false;
            bool irqRequested = false;
            bool reloadPending = false;

            bool prevA12High = false;
            u8 lowFetchStreak = 0;
            static constexpr u8 LOW_FETCH_THRESH = 1;

            bool MMC6 = false;

            /**
             * @brief Clocks the IRQ counter value and triggers
             *      an IRQ within the CPU when counter reaches zero.
             */
            void clockIRQ() {
                if (irqCounter == 0 || reloadPending) {
                    irqCounter = irqReload;
                    reloadPending = false;
                } else {
                    irqCounter--;
                }
                if (irqCounter == 0 && irqEnabled) {
                    irqRequested = true;
                }
            }
    };
}