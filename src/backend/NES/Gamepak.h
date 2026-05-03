#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class Gamepak : IDevice<u8, u16> {
        friend class Debugger;
        public:
            sptr<Mapper> mapper = nullptr;

            Gamepak(const string&);
            ~Gamepak() = default;

            inline bool isValid() const { return valid; }

            u8 read(u16 addr, bool readonly = false) override;
            void write(u16 addr, u8 data) override;

            u8 ppuRead(u16 addr, bool readonly = false) const;
            void ppuWrite(u16 addr, u8 data) const;

            MIRROR getMirror() const;

            vector<u8> PRG() { return prgMemory; }
            vector<u8> CHR() { return chrMemory; }

        private:
            bool valid = false;

            u8 mapperID = 0;
            u8 prgBanks = 0;
            u8 chrBanks = 0;
            MIRROR mirror = MIRROR::HORIZONTAL;

            vector<u8> prgMemory = { 0 };
            vector<u8> chrMemory = { 0 };

            void initMapper(u8 mapperID);
    };
}