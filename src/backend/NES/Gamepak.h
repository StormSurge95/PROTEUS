#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    /**
     * @class Gamepak Gamepak.h "./Gamepak.h"
     * @implements IDevice<u8,u16>
     * @brief
     * Class to represent the NES game cartridge.
     * Implements IDevice interface with `u8` data size and `u16` address size
     * Contains and maintains all data within the opened ROM file.
     */
    class Gamepak : IDevice<u8, u16> {
        friend class Debugger;
        public:
            /// @brief reference to the mapper contained within this ROM
            sptr<Mapper> mapper = nullptr;

            /**
             * @brief Explicit constructor
             * @param [in] path The full path to the ROM file to be opened for attempted gameplay
             * @details Parses all data within the opened ROM file and constructs based on the header data.
             * @todo get PRG-RAM implemented and working properly
             * @todo refactor and separate parsing/processing code and fully implement other header formats
             */
            Gamepak(const string& path);
            /// @brief Default destructor
            ~Gamepak() = default;

            /// @brief Getter for ROM validity
            inline bool isValid() const { return valid; }

            /**
             * @brief Data read request originating from the CPU.
             * @param addr The address to be read from.
             * @param readonly Flag to block unwanted side-effects.
             * @return The data value read.
             */
            u8 read(u16 addr, bool readonly = false) override;
            /**
             * @brief Data write request originating from the CPU.
             * @param addr The address to be written to.
             * @param data The data to be written.
             */
            void write(u16 addr, u8 data) override;

            /**
             * @brief Data read request originating from the PPU.
             * @param addr The address to be read from.
             * @param readonly Flag to block unwanted side-effects.
             * @return The data value read.
             */
            u8 ppuRead(u16 addr, bool readonly = false) const;
            /**
             * @brief Data write request originating from the PPU.
             * @param addr The address to be written to.
             * @param data The data to be written.
             */
            void ppuWrite(u16 addr, u8 data) const;

            /// @brief Getter for the mirroring format of the ROM.
            MIRROR getMirror() const;

            /// @brief Getter for PRG-ROM memory
            vector<u8> PRG() { return prgMemory; }
            /// @brief Getter for CHR-MEM memory
            vector<u8> CHR() { return chrMemory; }

        private:
            /// @brief validity flag for ROM data
            bool valid = false;

            /// @brief ID of the mapper for this ROM
            u8 mapperID = 0;
            /// @brief Total number of PRG-ROM pages
            u8 prgBanks = 0;
            /// @brief Total number of CHR-ROM pages
            u8 chrBanks = 0;
            /// @brief Hardware (i.e. Cartridge; not Mapper) mirroring arrangement for this ROM
            MIRROR mirror = MIRROR::HORIZONTAL;

            /// @brief Vector containing all PRG-ROM memory data
            vector<u8> prgMemory = {};
            /// @brief Vector containing all CHR-MEM memory data
            vector<u8> chrMemory = {};

            /**
             * @brief Initializes and attaches the mapper specified by the ROM header file.
             * @param mapperID ID number for the mapper implemented by this ROM.
             */
            void initMapper(u8 mapperID);
    };
}