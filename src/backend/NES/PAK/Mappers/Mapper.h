#pragma once

#include "../../shared/NES_PCH.h"

namespace NES_NS {
    /// @interface Mapper Mapper.h "./Mapper.h"
    class Mapper {
            // Allow Debugger class to access all private members of the Mapper class
            friend class Debugger;
        public:
            /// @brief default constructor
            Mapper() = default;
            /**
             * @brief Explicit constructor
             * @param pBnk Number of PRG-ROM banks
             * @param pMem Reference to PRG-ROM memory
             * @param cBnk Number of CHR-ROM banks
             * @param cMem Reference to CHR-MEM memory
             * @param hasRam Whether or not the ROM in question has PRG-RAM
             */
            Mapper(u8 pBnk, vector<u8>& pMem, u8 cBnk, vector<u8>& cMem, bool hasRam = false) :
                PRGBanks(pBnk), PRGMemory(&pMem), CHRBanks(cBnk), CHRMemory(&cMem), hasPRG_RAM(hasRam) {
                hasCHR_RAM = CHRBanks == 0;
            }
            /// @brief default destructor
            virtual ~Mapper() = default;

            /**
             * @brief Read memory operation originating from CPU.
             * @param addr The address to be read from.
             * @param readonly Flag to block unwanted side-effects.
             * @return The byte of data read from memory.
             */
            virtual u8 cpuRead(u16 addr, bool readonly = false) { return 0x00; }
            /**
             * @brief Write memory operation originating from CPU
             * @param addr The address to be written to.
             * @param data The byte of data to be written.
             */
            virtual void cpuWrite(u16 addr, u8 data) {}

            /**
             * @brief Read memory operation originating from PPU.
             * @param addr The address to be read from.
             * @param readonly Flag to block unwanted side-effects.
             * @return The byte of data read from memory.
             */
            virtual u8 ppuRead(u16 addr, bool readonly = false) { return 0x00; }
            /**
             * @brief Write memory operation originating from PPU.
             * @param addr The address to be written to.
             * @param data The byte of data to be written.
             */
            virtual void ppuWrite(u16 addr, u8 data) {}

            /**
             * @brief Clock function as called on CPU timing.
             * @param counter The master clock cycle of the CPU.
             * @details
             * In general, this function is purely meant for preventing and/or
             * causing certain operations based on the state of the CPU.
             */
            virtual void cpuclock(u64 counter = 0x00) {};
            /**
             * @brief Clock function as called on PPU timing.
             * @param counter The master clock cycle of the PPU.
             * @details
             * In general, this function is purely meant for preventing and/or
             * causing certain operations based on the state of the PPU.
             */
            virtual void ppuclock(u64 counter = 0x00) {};
            /**
             * @brief Getter function for the Mapper's intended mirroring mode.
             * @return If the mapper does not handle mirroring, then we simply return
             * `MIRROR::HARDWARE` to show that we can use whatever the ROM header
             * says; otherwise, we return whatever mirroring the mapper says to use.
             */
            virtual MIRROR getMirrorMode() const { return MIRROR::HARDWARE; }
        protected:
            /// @brief total number of PRG-ROM banks
            u8 PRGBanks = 0;
            /**
             * @brief total number of CHR-ROM banks
             * @note If 0, then we automatically set `hasCHR_RAM` to true
             */
            u8 CHRBanks = 0;
            /// @brief whether or not this rom has PRG-RAM
            bool hasPRG_RAM = false;
            /// @brief whether or not this rom has CHR-RAM
            bool hasCHR_RAM = false;
            /// @brief reference to this rom's PRG-ROM memory
            vector<u8>* PRGMemory = nullptr;
            /// @brief reference to this rom's CHR-MEM memory
            vector<u8>* CHRMemory = nullptr;
    };
}