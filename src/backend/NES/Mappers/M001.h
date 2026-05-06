#pragma once

#include "./Mapper.h"

namespace NES_NS {
    /**
     * @brief MMC1 Mapper
     * @details
     * Nintendo MMC1 is a mapper ASIC used in Nintendo's SxROM, NES-EVENT and 2ME circuit boards.
     * iNES Mapper 1 and 166 denote the SxROM and 2ME circuit boards mounting the MMC1B (iNES Mapper 1) or MMC1A (iNES Mapper 155) revision of the ASIC.
     * Mapper 1 is used if the revision is not known or irrelevant.
     * 
     * Three submappers of mapper 1/155 are valid:
     * - Submapper 5: SEROM/SHROM/SH1ROM with 32 KiB of unbanked PRG-ROM
     * - Submapper 6: Famicom Network System's 2ME circuit board
     * - Submapper 7: Kaiser KS-7058 that mounts KS 203 MMC1 clone, but with hard-wired nametable arrangement
     * 
     * All other SxROM variants are denoted by their functional PRG-ROM/PRG-RAM/CHR-ROM/CHR-RAM sizes in the NES 2.0 header.
     * Without NES 2.0, the PRG-RAM size has to be assumed; 32 KiB are sufficient for compatibility with all known titles.
     * 
     * BANKS:
     * - CPU $6000-$7FFF: 8 KB PRG-RAM bank, optional
     * - CPU $8000-$BFFF: 16 KB PRG-ROM bank, either switchable or fixed to first bank
     * - CPU $C000-$FFFF: 16 KB PRG-ROM bank, either fixed to last bank or switchable
     * - PPU $0000-$0FFF: 4 KB switchable CHR bank
     * - PPU $1000-$1FFF: 4 KB switchable CHR bank
     * 
     * REGISTERS:
     * - Load Register ($8000-$FFFF)
     *      7  bit  0
     *      ---- ----
     *      Rxxx xxxD
     *      |       |
     *      |       +-- Data bit to be shifted into shift register, LSB first
     *      +---------- A write with bit set will reset shift register
     *                  and write Control with (Control OR $0C),
     *                  locking PRG-ROM at $C000-$FFFF to the last bank.       
     * - Control (internal, $8000-$9FFF)
     * - CHR bank 0 (internal, $A000-$BFFF)
     * - CHR bank 1 (internal, $C000-$DFFF)
     * - PRG bank (internal, $E000-$FFFF)
     */
    class M001 : public Mapper {
        public:
            /**
             * @brief Explicit Constructor
             * @param pBnk Number of PRG-ROM banks
             * @param pMem Reference to PRG memory on gamepak
             * @param cBnk Number of CHR-ROM banks
             * @param cMem Reference to CHR memory on gamepak
             */
            M001(u8 pBnk, vector<u8>& pMem, u8 cBnk, vector<u8>& cMem) :
                Mapper(pBnk, pMem, cBnk, cMem), PRGRam(0x2000, 0x00) {}

            /**
             * @brief Resets the mapper to a known state
             */
            void reset() {
                shiftReg = 0x10;
                control = 0x0C;

                CHRBank0 = 0;
                CHRBank1 = 0;
                PRGBank = 0;
            }

            /**
             * @brief Data read request from CPU for PRG memory.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects
             * @return The data that was read; or zero if invalid address.
             */
            u8 cpuRead(u16 addr, bool readonly = false) override {
                if (addr >= 0x8000) { // return PRG-ROM for addresses over $7FFF
                    u32 mappedAddr = mapPRG(addr); // properly map/mask the address
                    size_t size = PRGMemory->size();
                    if ((size & (size - 1)) == 0)
                        return PRGMemory->at(mappedAddr & (size - 1));
                    else
                        return PRGMemory->at(mappedAddr % size);
                } else if (addr >= 0x6000 && !PRGRamDisabled) // return PRG-RAM for addresses over $5FFF
                    return PRGRam[addr & 0x1FFF]; // properly mask the address
                // return 0 for invalid addresses
                return 0x00;
            }

            /**
             * @brief Data write request from CPU for PRG memory
             * @param addr Address to be written to.
             * @param data Data to be written.
             */
            void cpuWrite(u16 addr, u8 data) override {
                if (addr < 0x8000) {
                    // Only perform writes on PRG-RAM if the RAM is not disabled.
                    if (PRGRamDisabled) return;
                    PRGRam[addr & 0x1FFF] = data; // properly mask address
                    return;
                }

                // handle writes with bit R set
                if (data & 0x80) {
                    shiftReg = 0x10;
                    control |= 0x0C;
                    return;
                }

                // prevent consecutive writes
                if (cpuCycle - lastWriteCycle < 2) {
                    // update last attempted write cycle
                    lastWriteCycle = cpuCycle;
                    return;
                }

                // update last write cycle
                lastWriteCycle = cpuCycle;

                // shiftReg starts as 0b00010000
                // once the initial bit is in b0,
                // the next write will give us the
                // full 5-bit value that needs to
                // be written to the internal register
                bool complete = shiftReg & 0x01;

                // progress the shift register
                shiftReg >>= 1;
                // write the new bit
                shiftReg |= (data & 0x01) << 4;

                // handle completion of 5-bit value
                if (complete) {
                    // obtain the value to be written
                    u8 val = shiftReg & 0x1F;
                    // the register to be written is
                    // determined by the address written
                    // to when the last bit is placed into
                    // the shift register
                    writeRegister(addr, val);
                    // reset shift register
                    shiftReg = 0x10;
                }
            }

            /**
             * @brief Data read request from PPU for CHR memory
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data that was read; or zero if invalid address.
             */
            u8 ppuRead(u16 addr, bool readonly = false) override {
                if (addr < 0x2000) {
                    u32 mappedAddr = mapCHR(addr); // properly map/mask the address
                    return CHRMemory->at(mappedAddr);
                }
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
                if (addr < 0x2000 && CHRBanks == 0) {
                    u32 mappedAddr = mapCHR(addr); // properly map/mask address
                    CHRMemory->at(mappedAddr) = data;
                }
            }

            /**
             * @brief Keeps track of current cpu cycle used for preventing consecutive writes.
             * @param counter The current CPU cycle counter
             */
            void cpuclock(u64 counter = 0x00) override {
                cpuCycle = counter;
            }

            /**
             * @brief PPU nametable mirroring is controlled via the mapper
             * @return The current mirroring mode based on the Control register
             */
            MIRROR getMirrorMode() const override {
                switch (control & 0x03) {
                    default:
                    case 3: return MIRROR::HORIZONTAL;
                    case 0: return MIRROR::ONE_SCREEN_LO;
                    case 1: return MIRROR::ONE_SCREEN_HI;
                    case 2: return MIRROR::VERTICAL;
                }
            }

        private:
            /**
             * @brief Holds data to be written to various internal registers; loaded via Load Register
             */
            u8 shiftReg = 0x10;

            /**
             * @brief Control (internal, $8000-$9FFF)
             * @details
             *      4bit0
             *      -----
             *      CPPMM
             *      |||||
             *      |||++-- Nametable arrangement
             *      |||     - 00: one-screen, loawer bank
             *      |||     - 01: one-screen, upper bank
             *      |||     - 10: horizontal arrangment ("vertical mirroring", PPU A10)
             *      |||     - 11: vertical arrangment ("horizontal mirroring", PPU A11)
             *      |||
             *      |++---- PRG-ROM bank mode
             *      |       - 00/01: switch 32 KB at $8000, ignoring low bit of bank number
             *      |       - 10: fix first bank at $8000 and switch 16 KB bank at $C000
             *      |       - 11: fix last bank at $C000 and switch 16 KB bank at $8000
             *      |
             *      +------ CHR-ROM bank mode
             *              - 0: switch 8 KB at a time
             *              - 1: switch two separate 4 KB banks
             */
            u8 control = 0x1C;

            /**
             * @brief CHR bank 0 (internal, $A000-$BFFF)
             * @details
             *      4bit0
             *      -----
             *      CCCCC
             *      |||||
             *      +++++-- Select 4 KB or 8 KB CHR bank at PPU $0000 (low bit ignored in 8 KB mode)
             */
            u8 CHRBank0 = 0;

            /**
             * @brief CHR bank 1 (internal, $C000-$DFFF)
             * @details
             *      4bit0
             *      -----
             *      CCCCC
             *      |||||
             *      +++++-- Select 4 KB CHR bank at PPU $1000 (ignored in 8 KB mode)
             */
            u8 CHRBank1 = 0;

            /**
             * @brief PRG bank (internal, $E000-$FFFF)
             * @details
             *      4bit0
             *      -----
             *      RPPPP
             *      |||||
             *      |++++-- Select 16 KB PRG-ROM bank (low bit ignored in 32 KB mode)
             *      +------ PRG-RAM disable flag
             *              - 0: PRG-RAM enabled
             *              - 1: PRG-RAM disabled
             */
            u8 PRGBank = 0;
            bool PRGRamDisabled = false; /// flag for disabling PRG-RAM
            std::vector<u8> PRGRam; // TODO: move this to base Mapper class

            u64 cpuCycle = 0;
            u64 lastWriteCycle = 0xFFFFFFFFFFFFFFFF; // default to max value to allow first write.

            /**
             * @brief Writes the completed shift register value to the desired internal register
             * @param addr Address used to determine which register to write to.
             * @param val Value to write to the desired register.
             */
            void writeRegister(u16 addr, u8 val) {
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

            /**
             * @brief Properly map PRG addresses
             * @param addr Original address.
             * @return Properly mapped address.
             */
            u32 mapPRG(u16 addr) {
                // determine which PRG mode we are in
                u8 mode = (control >> 2) & 0x03;
                u32 b16c = static_cast<u32>(PRGBanks);
                if (b16c < 1) b16c = 1;
                u32 b32c = b16c / 2;
                if (b32c < 1) b32c = 1;

                u32 bank = 0x00;
                u32 offset = addr & 0x3FFF;

                switch (mode) {
                    case 0:
                    case 1: // switch 32 KB at $8000, ignoring low bit of bank number
                        bank = ((PRGBank & 0x0E) >> 1) % b32c;
                        return (bank * 0x8000) + (addr & 0x7FFF);
                    case 2: // fix first bank at $8000 and switch 16 KB bank at $C000
                        if (addr < 0xC000) return (addr & 0x3FFF);
                        else {
                            bank = (PRGBank & 0x0F) % b16c;
                            return (bank * 0x4000) + offset;
                        }
                    default:
                    case 3: // fix last bank at $C000 and switch 16 KB bank at $8000
                        if (addr < 0xC000) {
                            bank = (PRGBank & 0x0F) % b16c;
                            return (bank * 0x4000) + offset;
                        } else
                            return ((PRGBanks - 1) * 0x4000) + offset;
                }
            }

            /**
             * @brief Properly map CHR addresses
             * @param addr Original address.
             * @return Properly mapped address.
             */
            u32 mapCHR(u16 addr) {
                // determine CHR bank mode
                bool mode = !!((control >> 4) & 0x01);
                
                // determine what values to use for bank0 and bank1
                u32 count = (u32)CHRMemory->size() >> 12;
                u32 b0 = CHRBank0 % count;
                u32 b1 = CHRBank1 % count;
                if (hasCHR_RAM) {
                    b0 = CHRBank0;
                    b1 = CHRBank1;
                }

                u32 size = (u32)CHRMemory->size();
                bool bit = (size & (size - 1)) == 0;

                if (mode) { // switch two separate 4KB banks
                    if (addr < 0x1000) { // use first 4KB bank
                        addr += (b0 << 12);
                        if (bit) return addr & (size - 1);
                        else return addr % size;
                    } else { // use second 4KB bank
                        addr = ((b1 << 12) + (addr & 0x0FFF));
                        if (bit) return addr & (size - 1);
                        else return addr % size;
                    }
                } else { // switch 8KB at a time
                    addr += (u32)(b0 & 0x1E) << 12;
                    if (bit) return addr & (size - 1);
                    else return addr % size;
                }
            }
    };
}