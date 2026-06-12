#pragma once

#include "../../shared/NesPCH.h"
#include "../../shared/NesEventSink.h"

namespace NS_NES {
    /// @interface Mapper Mapper.h "./Mapper.h"
    class Mapper {
            // Allow Debugger class to access all private members of the Mapper class
            friend class NesDebugger;
        public:
            /**
             * @brief Explicit constructor
             * @param pBnk Number of PRG-ROM banks
             * @param pMem Reference to PRG-ROM memory
             * @param cBnk Number of CHR-ROM banks
             * @param cMem Reference to CHR-MEM memory
             * @param hasRam Whether or not the ROM in question has PRG-RAM
             */
            Mapper(u16 pBnk, vector<u8>* pMem, u16 cBnk, vector<u8>* cMem, vector<u8>* pRam = nullptr, u8 subMapper = 0) :
                prgBanks(pBnk), prgRom(pMem), chrBanks(cBnk), chrMem(cMem), prgRam(pRam), subMapperID(subMapper) {
                hasPrgRam = prgRam != nullptr;
                hasChrRam = chrBanks == 0;
            }
            /// @brief default destructor
            virtual ~Mapper() = default;
            
            void connectEventSink(NesEventSink* sink) { eventSink = sink; }

            virtual void powerup() = 0;
            virtual void reset() = 0;
            virtual void powerdown() = 0;

            /**
             * @brief Read memory operation originating from CPU.
             * @param addr The address to be read from.
             * @param readonly Flag to block unwanted side-effects.
             * @return The byte of data read from memory.
             */
            virtual bool cpuRead(u16 addr, u8& data, bool readonly = false) { return false; }
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
            virtual bool ppuRead(u16 addr, u8& data, bool readonly = false) { return false; }
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
            /**
             * @brief Getter function for the Mapper's IRQ flag, if it has one.
             * @return If the mapper does not produce IRQs, then we simply return
             *      false; otherwise, we return the current state of the IRQ flag.
             */
            virtual bool irqRequestActive() const { return false; }
            /**
             * @brief Observer function for keeping track of the PPU's A12 line.
             * @param addr The address to be used for observation of the A12 line.
             */
            virtual void observeAddressPPU(u16 addr) {}

            virtual vector<array<string, 2>> getDebugData() = 0;
        protected:
            NesEventSink* eventSink = nullptr;
            /// @brief submapper version to use for operations
            u8 subMapperID = 0;
            /// @brief total number of PRG-ROM banks
            u16 prgBanks = 0;
            /**
             * @brief total number of CHR-ROM banks
             * @note If 0, then we automatically set `hasCHR_RAM` to true
             */
            u16 chrBanks = 0;
            /// @brief whether or not this rom has PRG-RAM
            bool hasPrgRam = false;
            /// @brief whether or not this rom has CHR-RAM
            bool hasChrRam = false;
            /// @brief reference to this rom's PRG-ROM memory
            vector<u8>* prgRom = nullptr;
            /// @brief reference to this rom's CHR-MEM memory
            vector<u8>* chrMem = nullptr;
            /// @brief reference to this rom's PRG-RAM memory (if present)
            vector<u8>* prgRam = nullptr;
    };
}