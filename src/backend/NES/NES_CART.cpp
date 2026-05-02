#include "./Mappers/M000.h"
#include "./Mappers/M001.h"
#include "./Mappers/M002.h"
#include "./Mappers/M003.h"
#include "./Mappers/M004.h"
#include "NES_CART.h"

#include <fstream>
#include <string>

NES_CART::NES_CART(const std::string& path) {
    // open rom file
    std::ifstream file(path, std::ifstream::binary);

    // verify file was found and opened
    if (!file.is_open()) return;

    // read header metadata
    INESHeader header = {};
    file.read(reinterpret_cast<char*>(&header), sizeof(INESHeader));

    // verify that ROM is a NES ROM
    if (header.name[0] != 'N' || header.name[1] != 'E' ||
        header.name[2] != 'S' || header.name[3] != 0x1A) return;

    // get number of PRG/CHR banks
    prgBanks = header.prgChunks;
    chrBanks = header.chrChunks;

    // get mapper id
    mapperID = (header.flags7 & 0x0F) | (header.flags6 >> 4);
    initMapper(mapperID);

    mirror = ((header.flags6 & 0x08) ? FOUR_SCREEN : ((header.flags6 & 0x01) ? VERTICAL : HORIZONTAL));

    if (header.flags6 & 0x04) file.seekg(512, std::ios::cur);

    // read prg memory
    prgMemory.resize((size_t)prgBanks * 16384);
    file.read(reinterpret_cast<char*>(prgMemory.data()), prgMemory.size());

    if (chrBanks == 0) {
        // initialize chr memory as RAM
        chrMemory.assign(8192, 0);
    } else {
        chrMemory.resize((size_t)chrBanks * 8192);
        file.read(reinterpret_cast<char*>(chrMemory.data()), chrMemory.size());
    }

    valid = true;
}

MIRROR NES_CART::getMirror() const {
    MIRROR m = mapper->getMirrorMode();
    if (m == MIRROR::HARDWARE)
        return mirror;
    return m;
}

uint8_t NES_CART::read(uint16_t addr, bool readonly) {
    return mapper->cpuRead(addr, readonly);
}

void NES_CART::write(uint16_t addr, uint8_t data) {
    return mapper->cpuWrite(addr, data);
}

uint8_t NES_CART::ppuRead(uint16_t addr, bool readonly) const {
    return mapper->ppuRead(addr, readonly);
}

void NES_CART::ppuWrite(uint16_t addr, uint8_t data) const {
    return mapper->ppuWrite(addr, data);
}

void NES_CART::initMapper(uint8_t id) {
    switch (id) {
        case 0: mapper = std::make_shared<M000>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        case 1: mapper = std::make_shared<M001>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        case 2: mapper = std::make_shared<M002>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        case 3: mapper = std::make_shared<M003>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        case 4: mapper = std::make_shared<M004>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        default:
            // TODO: render this as a message box and return to GAME_LIST view
            std::string num = std::to_string(id);
            std::string msg = "THIS MAPPER (M" + num.insert(0, 3 - num.size(), '0') + ") IS CURRENTLY NOT IMPLEMENTED";
            printf(msg.c_str());
            exit(EXIT_FAILURE);
    }
}