#pragma once

#include "NesPCH.h"

namespace NS_NES {
    class Mapper;

    /**
     * @class Gamepak Gamepak.h "./Gamepak.h"
     * @implements IDevice<u8,u16>
     * @brief
     * Class to represent the NES game cartridge.
     * Implements IDevice interface with `u8` data size and `u16` address size
     * Contains and maintains all data within the opened ROM file.
     */
    class Gamepak : IDevice<u8, u16> {
            // Allow Debugger class to access all private members of the Gamepak class
            friend class NesDebugger;
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
            ~Gamepak() { if (prgRamNonVolatile.size() > 0) SaveRAM(); }

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

            bool hasPrgRam() const { return prgRamNonVolatile.size() > 0 || prgRamVolatile.size() > 0; }

            /// @brief Getter for PRG-ROM memory
            vector<u8> PRG() { return prgMemory; }
            /// @brief Getter for CHR-MEM memory
            vector<u8> CHR() { return chrMemory; }

        private:
            path filePath;

            HeaderFormat hFormat = HeaderFormat::UNKNOWN;

            ConsoleRegion region = ConsoleRegion::NTSC;
            ConsoleType cType = ConsoleType::NES_FAMICOM;

            VsPPU vsPPU = VsPPU::PPU2C0X;
            VsHardware vsHardware = VsHardware::VS_UNISYSTEM;

            ExpansionDevice expDev = ExpansionDevice::UNSPECIFIED;

            /// @brief validity flag for ROM data
            bool valid = false;
            /// @brief flag to show whether this rom has the 512B trainer section
            bool hasTrainer = false;
            /// @brief flag to show whether this rom has battery-backed ram
            bool hasBattery = false;

            /// @brief ID of the mapper for this ROM
            u16 mapperID = 0;
            u8 subMapperID = 0;
            /// @brief PRG/CHR memory info structure
            PakInfo memory = {};
            /// @brief Hardware (i.e. Cartridge; not Mapper) mirroring arrangement for this ROM
            MIRROR mirror = MIRROR::HORIZONTAL;

            /// @brief Vector containing all PRG-ROM memory data
            vector<u8> prgMemory = {};
            vector<u8> prgRamVolatile = {};
            vector<u8> prgRamNonVolatile = {};
            /// @brief Vector containing all CHR-MEM memory data
            vector<u8> chrMemory = {};
            vector<u8> chrRamVolatile = {};
            vector<u8> chrRamNonVolatile = {};

            u8 miscRoms = 0;

            /**
             * @brief Helper function to process and validate the iNES header.
             * @details
             * If the provided header is valid, this function will also read the
             * various parts of the header and apply their values for later use.
             * @param h The header to be processed
             * @return True if header is valid; false otherwise.
             */
            bool readHeader(const Header& h);

            bool readHeaderINES(const Header& h);
            bool readHeaderNES2(const Header& h);
            bool readHeaderANES(const Header& h);

            /**
             * @brief Initializes and attaches the mapper specified by the ROM header file.
             * @param mapperID ID number for the mapper implemented by this ROM.
             */
            void initMapper(u16 mapperID);

            path GetSavePath();

            void SaveRAM();
            void LoadRAM();
    };
}