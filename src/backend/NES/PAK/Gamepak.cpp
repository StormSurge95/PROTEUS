#include "./Mappers/M000.hpp"
#include "./Mappers/M001.hpp"
#include "./Mappers/M002.hpp"
#include "./Mappers/M003.hpp"
#include "./Mappers/M004.hpp"
#include "./Gamepak.hpp"

/* TODO:
 * Figure out how to implement the functionalities
 * of the various forms of RAM that may or may not
 * be available in any given rom.
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

    valid = readHeader(header);

    if (valid) {
        initMapper(mapperID);

        if (hasTrainer) file.seekg(512, ios::cur);

        // read prg memory
        prgMemory.resize((size_t)memory.prg.romSize);
        file.read(reinterpret_cast<char*>(prgMemory.data()), prgMemory.size());

        if (memory.chr.romBanks == 0) {
            // initialize chr memory as RAM
            chrMemory.assign(8192, 0);
        } else {
            chrMemory.resize((size_t)memory.chr.romSize);
            file.read(reinterpret_cast<char*>(chrMemory.data()), chrMemory.size());
        }
    }
}

bool Gamepak::readHeader(const Header& h) {
    // first, validate the header itself
    if (memcmp(h.name, "NES\x1A", 4) != 0) return false;

    // next, check header format
    switch (h.byte7 & 0x0C) {
        case 0x08:
            return readHeaderNES2(h);
        case 0x00:
            return readHeaderINES(h);
        case 0x04:
        default:
            return readHeaderANES(h);
    }
}

bool Gamepak::readHeaderANES(const Header& h) {
    memory.prg.romBanks = h.byte4;
    memory.chr.romBanks = h.byte5;

    mapperID = (h.byte6 >> 4);

    mirror = ((h.byte6 & 0x08) ? MIRROR::FOUR_SCREEN : ((h.byte6 & 0x01) ? MIRROR::VERTICAL : MIRROR::HORIZONTAL));

    hasTrainer = (h.byte6 & 0x04);

    return true;
}

bool Gamepak::readHeaderINES(const Header& h) {
    // byte 4
    memory.prg.romBanks = h.byte4;
    memory.prg.romSize = ((u32)h.byte4 << 14);

    // byte 5
    memory.chr.romBanks = h.byte5;
    memory.chr.romSize = ((u32)h.byte5 << 13);

    // byte 6
    mirror = ((h.byte6 & 0x08) ? MIRROR::FOUR_SCREEN : ((h.byte6 & 0x01) ? MIRROR::VERTICAL : MIRROR::HORIZONTAL));
    hasTrainer = (h.byte6 & 0x04) > 0;
    hasBattery = (h.byte6 & 0x02) > 0;

    mapperID = (h.byte7 & 0xF0) | (h.byte6 >> 4);

    return true;
}

bool Gamepak::readHeaderNES2(const Header& h) {
    // NES 2.0 is backwards compatible with iNES;
    // so for the first 7 bytes, we can use the
    // existing function to save code lines
    readHeaderINES(h);

    // byte 7 - console type
    cType = (ConsoleType)(h.byte7 & 0x03);

    // byte 8 - mapper MSB & submapper
    mapperID |= (((u16)h.byte8 & 0x0F) << 8);
    subMapperID = ((h.byte8 & 0xF0) >> 4);

    // byte 9 - PRG-ROM MSB
    u16 prgMSB = (h.byte9 & 0x0F);
    // if MSB is $F, an exponent-multiplier notation is used
    if (prgMSB == 0x0F) {
        u8 m = memory.prg.romBanks & 0x03;
        u8 e = (memory.prg.romBanks & 0xFC) >> 2;

        memory.prg.romBanks = (u16)(pow(2, e) * (m * 2 + 1));
    } else memory.prg.romBanks |= (prgMSB << 8);
    memory.prg.romSize = ((u32)memory.prg.romBanks << 14);
    // byte 9 - CHR-ROM MSB
    u16 chrMSB = (h.byte9 & 0xF0) >> 4;
    if (chrMSB == 0x0F) {
        u8 m = memory.chr.romBanks & 0x03;
        u8 e = (memory.chr.romBanks & 0xFC) >> 2;

        memory.chr.romBanks = (u16)(pow(2, e) * (m * 2 + 1));
    } else memory.chr.romBanks |= (chrMSB << 8);
    memory.chr.romSize = ((u32)memory.chr.romBanks << 13);

    // byte 10 - PRG-RAM info
    u8 vshift = h.byteA & 0x0F;
    if (vshift == 0) memory.prg.vramSize = 0;
    else memory.prg.vramSize = (64 << vshift);
    u8 nvshift = ((h.byteA & 0xF0) >> 4);
    if (nvshift == 0) memory.prg.nvramSize = 0;
    else memory.prg.nvramSize = (64 << nvshift);

    // byte 11 - CHR-RAM info
    vshift = h.byteB & 0x0F;
    if (vshift == 0) memory.chr.vramSize = 0;
    else memory.chr.vramSize = (64 << vshift);
    nvshift = ((h.byteB & 0xF0) >> 4);
    if (nvshift == 0) memory.chr.nvramSize = 0;
    else memory.chr.nvramSize = (64 << nvshift);

    // byte 12 - region
    region = (ConsoleRegion)(h.byteC & 0x03);

    // byte 13 - vs-system/extended-console info

    if (cType == ConsoleType::VS_SYSTEM) {
        // vs system
        vsPPU = VsPPU(h.byteD & 0x0F);
        vsHardware = VsHardware((h.byteD & 0xF0) >> 4);
    } else if (cType != ConsoleType::NES_FAMICOM && cType != ConsoleType::PLAYCHOICE_10) {
        // extended console
        cType = ConsoleType(h.byteD & 0x0F);
    }

    // byte 14 - misc rom count
    miscRoms = (h.byteE & 0x03);

    // byte 15 - expansion device
    expDev = ExpansionDevice(max(h.byteF & 0x7F, 0x4F));

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

void Gamepak::initMapper(u16 id) {
    switch (id) {
        case 0: mapper = make_shared<M000>(memory.prg.romBanks, prgMemory, memory.chr.romBanks, chrMemory); break;
        case 1: mapper = make_shared<M001>(memory.prg.romBanks, prgMemory, memory.chr.romBanks, chrMemory); break;
        case 2: mapper = make_shared<M002>(memory.prg.romBanks, prgMemory, memory.chr.romBanks, chrMemory); break;
        case 3: mapper = make_shared<M003>(memory.prg.romBanks, prgMemory, memory.chr.romBanks, chrMemory); break;
        case 4: mapper = make_shared<M004>(memory.prg.romBanks, prgMemory, memory.chr.romBanks, chrMemory); break;
        default:
            // TODO: render this as a message box and return to GAME_LIST view
            string num = to_string(id);
            string msg = "THIS MAPPER (M" + num.insert(0, 3 - num.size(), '0') + ") IS CURRENTLY NOT IMPLEMENTED";
            printf(msg.c_str());
            exit(EXIT_FAILURE);
    }
}