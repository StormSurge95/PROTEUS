#pragma once

#include "./Mapper.h"

namespace NES_NS {
    /**
     * @brief MMC3 Mapper
     * @details
     * Nintendo MMC3 is a mapper ASIC used in Nintendo's TxROM Game Pak boards.
     * 
     * BANKS:
     * - CPU $6000-$7FFF: 8KB PRG-RAM bank (optional)
     * - CPU $8000-$9FFF OR $C000-$DFFF: 8KB switchable PRG-ROM bank
     * - CPU $A000-$BFFF: 8KB switchable PRG-ROM bank
     * - CPU $C000-$DFFF OR $8000-$9FFF: 8KB switchable PRG-ROM bank
     * - CPU $E000-$FFFF: 8KB PRG-ROM bank, fixed to the last bank
     * - PPU $0000-$07FF OR $1000-$17FF: 2KB switchable CHR bank
     * - PPU $0800-$0FFF OR $1800-$1FFF: 2KB switchable CHR bank
     * - PPU $1000-$13FF OR $0000-$03FF: 1KB switchable CHR bank
     * - PPU $1400-$17FF OR $0400-$07FF: 1KB switchable CHR bank
     * - PPU $1800-$1BFF OR $0800-$0BFF: 1KB switchable CHR bank
     * - PPU $1C00-$1FFF OR $0C00-$0FFF: 1KB switchable CHR bank
     * 
     * MMC3 has 4 pairs of registers at $8000-$9FFF, $A000-$BFFF, $C000-$DFFF, and $E000-$FFFF
     * Within each pair of registers:
     *      EVEN addresses ($8000, $8002, etc.) select the LOW register
     *      ODD addresses ($8001, $8003, etc.) select the HIGH register
     * These pairs can be broken down into two independent functional units:
     *      memory mapping ($8000-$9FFF & $A000-$BFFF)
     *      scanline counting ($C000-$DFFF & $E000-$FFFF)
     * 
     * REGISTERS:
     * - Bank Select ($8000-$9FFE, even)            \
     * - Bank Data ($8001-$9FFF, odd)                \
     *                                                |-->memory mapping
     * - Nametable Arrangement ($A000-$BFFE, even)   /
     * - PRG-RAM Protect ($A001-$BFFF, odd)         /
     * 
     * - IRQ Latch ($C000-$DFFE, even)      \
     * - IRQ Reload ($C001-$DFFF, odd)       \
     *                                        |-->scanline counting
     * - IRQ Disable ($E000-$FFFE, even)     /
     * - IRQ Enable ($E001-$FFFF, odd)      /
     */
    class M004 : public Mapper {
        public:
            M004(u8 pBnk, vector<u8>& pMem, u8 cBnk, vector<u8>& cMem) :
                Mapper(pBnk, pMem, cBnk, cMem) {}

            /**
             * @brief Data read request from CPU for PRG memory.
             * @todo TODO: Ensure that all accesses respect maximum PRGBanks.
             * @todo TODO: Incorporate PRG-RAM
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or zero if invalid address.
             */
            u8 cpuRead(u16 addr, bool readonly = false) override {
                u16 mapped = addr & 0x1FFF;
                if (addr >= 0xE000 && addr <= 0xFFFF) {
                    // fixed to last bank
                    mapped += (((u16)PRGBanks - 1) << 15);
                    return PRGMemory->at(mapped);
                }
                if (bankData.pMode) { // PRG-ROM bank mode 1
                    if (addr >= 0x8000 && addr <= 0x9FFF) { // fixed to second-last bank
                        mapped += (((u16)PRGBanks - 2) << 15);
                        return PRGMemory->at(mapped);
                    }
                    if (addr >= 0xA000 && addr <= 0xBFFF) { // R7
                        mapped += ((u16)bankData.R7 << 15);
                        return PRGMemory->at(mapped);
                    }
                    if (addr >= 0xC000 && addr <= 0xDFFF) { // R6
                        mapped += ((u16)bankData.R6 << 15);
                        return PRGMemory->at(mapped);
                    }
                } else { // PRG-ROM bank mode 0
                    if (addr >= 0x8000 && addr <= 0x9FFF) { // R6
                        mapped += ((u16)bankData.R6 << 15);
                        return PRGMemory->at(mapped);
                    }
                    if (addr >= 0xA000 && addr <= 0xBFFF) { // R7
                        mapped += ((u16)bankData.R7 << 15);
                        return PRGMemory->at(mapped);
                    }
                    if (addr >= 0xC000 && addr <= 0xDFFF) { // second-last
                        mapped += (((u16)PRGBanks - 2) << 15);
                        return PRGMemory->at(mapped);
                    }
                }

                return 0x00;
            }

            /**
             * @brief Data write request from CPU for PRG memory.
             * @todo TODO: Incorporate PRG-RAM
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void cpuWrite(u16 addr, u8 data) override {
                bool even = (addr & 0x01) == 0;
                if (addr >= 0x8000 && addr <= 0x9FFF) { // Bank Select & Bank Data
                    if (even) { // Even: Bank Select
                        bankSelect = data;
                    } else { // Odd: Bank Data
                        switch (bankSelect & 0x07) {
                            case 0: bankData.R0 = data; break;
                            case 1: bankData.R1 = data; break;
                            case 2: bankData.R2 = data; break;
                            case 3: bankData.R3 = data; break;
                            case 4: bankData.R4 = data; break;
                            case 5: bankData.R5 = data; break;
                            case 6: bankData.R6 = data; break;
                            case 7: bankData.R7 = data; break;
                        }
                        bankData.pMode = ((bankSelect >> 6) & 0x01);
                        bankData.cMode = ((bankSelect >> 7) & 0x01);
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
                        irqLatch.irqReload = data;
                    else { // Odd: Reload
                        // writing any value to this register clears the MMC3 IRQ counter immediately
                        // then reloads it at the NEXT rising edge of the PPU address
                        // presumably at PPU cycle 260 of the current scanline
                        irqLatch.irqCounter = 0x00;
                    }
                }
                if (addr >= 0xE000 && addr <= 0xFFFF) { // IRQ Enable & IRQ Disable
                    if (even) // Even: disable
                        irqEnabled = false;
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
                if (addr <= 0x1FFF) {
                    u16 mapped = addr;
                    if (bankData.cMode) { // CHR A12 Inversion 1
                        if (addr <= 0x0FFF) { // four 1KB banks
                            mapped &= 0x03FF;
                            if (addr <= 0x03FF) // R2
                                mapped += ((u16)bankData.R2 << 15);
                            else if (addr <= 0x07FF) // R3
                                mapped += ((u16)bankData.R3 << 15);
                            else if (addr <= 0x0BFF) // R4
                                mapped += ((u16)bankData.R4 << 15);
                            else if (addr <= 0x0FFF) // R5
                                mapped += ((u16)bankData.R5 << 15);
                        } else { // two 2KB banks
                            mapped &= 0x07FF;
                            if (addr <= 0x17FF) // R0
                                mapped += ((u16)bankData.R0 << 15);
                            else if (addr <= 0x1FFF) // R1
                                mapped += ((u16)bankData.R1 << 15);
                        }
                    } else { // CHR A12 Inversion 0
                        if (addr <= 0x0FFF) { // two 2KB banks
                            mapped &= 0x07FF;
                            if (addr <= 0x07FF) // R0
                                mapped += ((u16)bankData.R0 << 15);
                            else // R1
                                mapped += ((u16)bankData.R1 << 15);
                        } else { // four 1KB banks
                            mapped &= 0x03FF;
                            if (addr <= 0x13FF) // R2
                                mapped += ((u16)bankData.R2 << 15);
                            else if (addr <= 0x17FF) // R3
                                mapped += ((u16)bankData.R3 << 15);
                            else if (addr <= 0x1BFF) // R4
                                mapped += ((u16)bankData.R4 << 15);
                            else // R5
                                mapped += ((u16)bankData.R5 << 15);
                        }
                    }
                    return CHRMemory->at(mapped);
                }
                return 0x00;
            }

            /**
             * @brief Data write request from PPU for CHR memory.
             * @note If there is no CHR-RAM; this function does nothing.
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void ppuWrite(u16 addr, u8 data) override {
                if (CHRBanks != 0) return;
                // TODO: implement writing to CHR-RAM
            }

            /**
             * @brief Keeps track of current ppu cycle/scanline/frame and is used to record/trigger IRQ
             * @param counter 
             */
            void ppuclock(u64 counter = 0x00) override {
                // TODO
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
                bool pMode = false;
                bool cMode = false;
            } bankData;

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

            struct {
                u8 irqCounter = 0;
                u8 irqReload = 0;
            } irqLatch;

            bool irqEnabled = false;

            bool MMC6 = false;
    };
}