#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class Gamepak : IDevice<u8, u16> {
        friend class Debugger;
        public:
            sptr<Mapper> mapper = nullptr;

            Gamepak(const string&);
            ~Gamepak();

            inline bool isValid() const { return valid; }

            u8 read(u16 addr, bool readonly = false) override;
            void write(u16 addr, u8 data) override;

            u8 ppuRead(u16 addr, bool readonly = false) const;
            void ppuWrite(u16 addr, u8 data) const;

            MIRROR getMirror() const;

            vector<u8> PRG() { return prgROM; }
            vector<u8> CHR() { return chrMEM; }

        private:
            fs::path filepath;
            fs::path savepath;

            ifstream file;

            bool valid = false;
            bool hasTrainer = false;
            bool hasSAV = false;
            bool hasRAM = false;

            u64 imageSize = 0;
            u32 prgSize = 0;
            u32 chrSize = 0;
            u32 savSize = 0;

            u8 mapperID = 0;
            u8 prgBanks = 0;
            u8 chrBanks = 0;
            MIRROR mirror = MIRROR::HORIZONTAL;

            vector<u8> prgROM = { 0 };
            vector<u8> savMEM = { 0 };
            vector<u8> chrMEM = { 0 };

            void initMapper(u8 mapperID);

            /**
             * @brief Processes Header structure from ROM and initializes Gamepak object.
             * @param header The header structure to use for validation.
             */
            void ReadHeader(Header&);

            /**
             * @brief Reads Header structure in iNES format to initialize Gamepak.
             * @param header The structure to use for initialization.
             */
            void ReadHeaderINES(Header&);

            /**
             * @brief Reads Header structure in archaic iNES format to initialize Gamepak.
             * @param header The structure to use for initialization.
             */
            void ReadHeaderANES(Header&);

            /**
             * @brief Reads Header structure in NES 2.0 format to initialize Gamepak.
             * @param header The structure to use for initialization.
             */
            void ReadHeaderNES2(Header&);

            /**
             * @brief Reads Header structure in iNES 0.7 format to initialize Gamepak
             * @param header The structure to use for initialization.
             */
            void ReadHeaderNES0(Header&);
    };
}