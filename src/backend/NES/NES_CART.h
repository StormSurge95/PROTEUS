#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../core/IDevice.h"
#include "./Mappers/Mapper.h"
#include "./NES_MIRRORS.h"

struct INESHeader {
    char name[4] = { 0 };
    uint8_t prgChunks = 0;
    uint8_t chrChunks = 0;
    uint8_t flags6 = 0;
    uint8_t flags7 = 0;
    uint8_t prgRamSize = 0;
    uint8_t tvSystem1 = 0;
    uint8_t tvSystem2 = 0;
    char padding[5] = { 0 };
};

class NES_CART : IDevice<uint8_t, uint16_t> {
    friend class NES_DBG;
    public:
        std::shared_ptr<Mapper> mapper = nullptr;

        NES_CART(const std::string& path);
        ~NES_CART() = default;

        inline void clock() const { mapper->clock(); }
        inline bool isValid() const { return valid; }

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        uint8_t ppuRead(uint16_t addr, bool readonly = false) const;
        void ppuWrite(uint16_t addr, uint8_t data) const;

        MIRROR getMirror() const;

        std::vector<uint8_t> PRG() { return prgMemory; }
        std::vector<uint8_t> CHR() { return chrMemory; }

    private:
        bool valid = false;

        uint8_t mapperID = 0;
        uint8_t prgBanks = 0;
        uint8_t chrBanks = 0;
        MIRROR mirror = HORIZONTAL;

        std::vector<uint8_t> prgMemory = { 0 };
        std::vector<uint8_t> chrMemory = { 0 };

        void initMapper(uint8_t mapperID);
};