#include "./Mappers/M000.hpp"
#include "./Mappers/M001.hpp"
#include "./Mappers/M002.hpp"
#include "./Mappers/M003.hpp"
#include "./Mappers/M004.hpp"
#include "./Gamepak.hpp"

/*
    TODO: MODIFY TO SUPPORT NES 2.0 HEADER FORMAT
*/
using namespace NES_NS;
Gamepak::Gamepak(const string& path) {
    // open rom file
    ifstream file(path, ifstream::binary);

    // verify file was found and opened
    if (!file.is_open()) return;

    // read header metadata
    Header header = {};
    file.read(reinterpret_cast<char*>(&header), sizeof(Header));

    readHeader(header);
    
    initMapper(mapperID);

    if (hasTrainer) file.seekg(512, ios::cur);

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

bool Gamepak::readHeader(const Header& h) {
    if (memcmp(h.name, "NES\x1A", 4) != 0) return false;

    prgBanks = h.byte4;
    chrBanks = h.byte5;

    mapperID = (h.byte7 & 0xF0) | (h.byte6 >> 4);

    mirror = ((h.byte6 & 0x08) ? MIRROR::FOUR_SCREEN : ((h.byte6 & 0x01) ? MIRROR::VERTICAL : MIRROR::HORIZONTAL));

    hasTrainer = (h.byte6 & 0x04);

    return true;
}

MIRROR Gamepak::getMirror() const {
    MIRROR m = mapper->getMirrorMode();
    if (m == MIRROR::HARDWARE)
        return mirror;
    return m;
}

u8 Gamepak::read(u16 addr, bool readonly) {
    return mapper->cpuRead(addr, readonly);
}

void Gamepak::write(u16 addr, u8 data) {
    return mapper->cpuWrite(addr, data);
}

u8 Gamepak::ppuRead(u16 addr, bool readonly) const {
    return mapper->ppuRead(addr, readonly);
}

void Gamepak::ppuWrite(u16 addr, u8 data) const {
    return mapper->ppuWrite(addr, data);
}

void Gamepak::initMapper(u8 id) {
    switch (id) {
        case 0: mapper = make_shared<M000>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        case 1: mapper = make_shared<M001>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        case 2: mapper = make_shared<M002>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        case 3: mapper = make_shared<M003>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        case 4: mapper = make_shared<M004>(prgBanks, prgMemory, chrBanks, chrMemory); break;
        default:
            // TODO: render this as a message box and return to GAME_LIST view
            string num = to_string(id);
            string msg = "THIS MAPPER (M" + num.insert(0, 3 - num.size(), '0') + ") IS CURRENTLY NOT IMPLEMENTED";
            printf(msg.c_str());
            exit(EXIT_FAILURE);
    }
}