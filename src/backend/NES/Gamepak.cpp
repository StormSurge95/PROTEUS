#include "./Mappers/M000.h"
#include "./Mappers/M001.h"
#include "./Mappers/M002.h"
#include "./Mappers/M003.h"
#include "./Mappers/M004.h"
#include "./Gamepak.h"

/*
    TODO: MODIFY TO SUPPORT NES 2.0 HEADER FORMAT
*/
using namespace NES_NS;
Gamepak::Gamepak(const string& path) {
    filepath = path;

    printf("%s\n", path.c_str());

    // verify file exists
    if (!fs::exists(path)) {
        valid = false;
        return;
    }
    
    // open rom file
    file.open(path, ifstream::binary);

    // get size of file
    imageSize = fs::file_size(path);

    // read header metadata
    Header header = {};
    file.read(reinterpret_cast<char*>(&header), sizeof(Header));

    ReadHeader(header);

    initMapper(mapperID);

    valid = true;

    file.close();
}

Gamepak::~Gamepak() {
    printf("Destructing Gamepak object\n");
    if (hasSAV) {
        printf("Creating save file: %ls\n", savepath.c_str());
        std::ofstream sav(savepath, ios::binary | ios::trunc);
        for (const u8& b : savMEM)
            sav.put(b);
        sav.close();
    }
}

void Gamepak::ReadHeader(Header& header) {
    // General NES ROM validation
    if (memcmp(header.name, "NES\x1A", 4) != 0) {
        printf("Invalid ID: %s\n", header.name);
        valid = false;
        return;
    }

    u8 format = header.byte7 & 0x0C;

    switch (format) {
        case 0x08:
            printf("Supposed NES 2.0 header\n");
            ReadHeaderNES2(header);
            break;
        case 0x04:
            printf("Supposed archaic iNES header\n");
            ReadHeaderANES(header);
            break;
        case 0x00:
            printf("Supposed iNES header\n");
            ReadHeaderINES(header);
            break;
        default:
            printf("Supposed iNES 0.7 header\n");
            ReadHeaderNES0(header);
            break;
    }

    if (!valid) return;

    if (hasTrainer) file.seekg(512, ios::cur);

    // read PRG-ROM
    prgROM.resize(prgSize);
    file.read(reinterpret_cast<char*>(prgROM.data()), prgROM.size());

    // read CHR-ROM
    if (chrSize == 0) { // use chrMEM as CHR-RAM
        chrMEM.assign(8192, 0x00);
    } else { // use chrMEM as CHR-ROM
        chrMEM.resize(chrSize);
        file.read(reinterpret_cast<char*>(chrMEM.data()), chrMEM.size());
    }

    // If we have PRG-RAM functionality, look for save file to initialize values
    if (hasSAV) {
        printf("SAVE RAM PRESENT (%dB)\n", savSize);
        // why tf can I not just create a new path with a different extension using one call?
        savepath = filepath.stem();
        savepath += ".sav";
        printf("Save Path: %ls\n", savepath.c_str());
        // if the path exists, we have a save file
        if (fs::exists(savepath)) {
            printf("Save file exists; copying mem\n");
            savMEM.resize(savSize);
            // read it and store it's data
            ifstream save(savepath);
            save.read(reinterpret_cast<char*>(savMEM.data()), savMEM.size());
            save.close();
        } else { // no save file exists; initialize mem to 0
            printf("No save file found; init to zero\n");
            savMEM.assign(savSize, 0x00);
        }
    }
}

void Gamepak::ReadHeaderINES(Header& header) {
    // validate iNES format
    if (memcmp(&header.byteC, "\x0\x0\x0\x0", 4) != 0) {
        valid = false;
        return;
    }

    valid = true;

    // initialize PRG-ROM size
    prgBanks = header.byte4;
    prgSize = prgBanks * 16384;

    // initialize CHR-ROM size
    chrBanks = header.byte5;
    chrSize = chrBanks * 8192;

    // determine Trainer
    hasTrainer = (header.byte6 & 0x04) > 0;

    // determine RAM
    hasSAV = (header.byte6 & 0x02) > 0;
    savSize = 8192; // set savSize to 8192 for compatibility in the case of iNES

    // get Mapper ID
    mapperID = (header.byte7 & 0xF0) | (header.byte6 >> 4);

    // determine Mirroring arrangement
    if (header.byte6 & 0x08) mirror = MIRROR::FOUR_SCREEN;
    else if (header.byte6 & 0x01) mirror = MIRROR::VERTICAL;
    else mirror = MIRROR::HORIZONTAL;
}

void Gamepak::ReadHeaderANES(Header& header) {
    printf("Supposed archaic iNES header\n\n");
}

void Gamepak::ReadHeaderNES2(Header& header) {
    // determine Trainer
    hasTrainer = (header.byte6 & 0x04) > 0;

    // calculate size of PRG-ROM section
    u8 prgMSB = header.byte9 & 0x0F;
    if (prgMSB != 0x0F) { // standard bit value
        prgBanks = ((u16)prgMSB | header.byte4);
        prgSize = prgBanks * 16384;
    } else { // exponent-multiplier notation
        u8 mul = (header.byte4 & 0x03) * 2 + 1;
        u8 exp = (u8)pow(2, (header.byte4 >> 2));
        prgSize = mul * exp;
        prgBanks = (u8)ceil(prgSize / 16384.0);
    }

    // calculate size of CHR-ROM section
    u8 chrMSB = (header.byte9 & 0xF0) >> 4;
    if (chrMSB != 0x0F) { // standard bit value
        chrBanks = ((u16)chrMSB | header.byte5);
        chrSize = chrBanks * 8192;
    } else { // exponent-multiplier notation
        u8 mul = (header.byte5 & 0x03) * 2 + 1;
        u8 exp = (u8)pow(2, (header.byte5 >> 2));
        chrSize = mul * exp;
        chrBanks = (u8)ceil(chrSize / 8192.0);
    }

    // validate header
    u32 calcSize = prgSize + chrSize + 16 + (hasTrainer ? 512 : 0);
    if (calcSize > imageSize) {
        valid = false;
        return;
    }

    valid = true;

    // TODO: determine PRG-RAM
    printf("PRG-RAM size: %02x\n\n", header.byteA);

    // TODO: determine CHR-RAM
    // TODO: determine CHR-RAM
    printf("CHR-RAM size: %02x\n\n", header.byteB);
}

void Gamepak::ReadHeaderNES0(Header& header) {
    printf("Supposed iNES 0.7 header\n\n");
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
        case 0: mapper = make_shared<M000>(prgBanks, prgROM, chrBanks, chrMEM, savSize, savMEM); break;
        case 1: mapper = make_shared<M001>(prgBanks, prgROM, chrBanks, chrMEM, savSize, savMEM); break;
        case 2: mapper = make_shared<M002>(prgBanks, prgROM, chrBanks, chrMEM, savSize, savMEM); break;
        case 3: mapper = make_shared<M003>(prgBanks, prgROM, chrBanks, chrMEM, savSize, savMEM); break;
        case 4: mapper = make_shared<M004>(prgBanks, prgROM, chrBanks, chrMEM, savSize, savMEM); break;
        default:
            // TODO: render this as a message box and return to GAME_LIST view
            string num = to_string(id);
            string msg = "THIS MAPPER (M" + num.insert(0, 3 - num.size(), '0') + ") IS CURRENTLY NOT IMPLEMENTED";
            printf(msg.c_str());
            exit(EXIT_FAILURE);
    }
}