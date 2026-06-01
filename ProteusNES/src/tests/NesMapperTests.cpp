#include <iostream>
#include <string>
#include <vector>

#include "../NES.h"
#include "../shared/NesTypes.h"

using std::cout;
using std::stoi;
using std::runtime_error;
using std::filesystem::temp_directory_path;

using namespace NS_NES;

/// @brief Helper enum for testing a specific behavior
enum class Scenario {
    DEFAULT_MAPPING,
    PRG_ROM_MAPPING,
    PRG_RAM_MAPPING,
    CHR_ROM_MAPPING,
    CHR_RAM_MAPPING,
    MIRROR_BEHAVIOR,
    ADDR_WRAPPING,
    FOUR_SCREEN_MIRROR,
    VERTICAL_MIRROR,
    HORIZONTAL_MIRROR,
    INVALID
};

/**
 * @brief Helper function to parse the command line scenario argument.
 * @param s The provided command line argument to be parsed.
 * @return The Scenario equivalent of the argument, or `Scenario::INVALID` if no equivalent exists.
 */
static Scenario ParseScenario(const string& s) {
    if (s == "default") return Scenario::DEFAULT_MAPPING;
    if (s == "prom") return Scenario::PRG_ROM_MAPPING;
    if (s == "pram") return Scenario::PRG_RAM_MAPPING;
    if (s == "crom") return Scenario::CHR_ROM_MAPPING;
    if (s == "cram") return Scenario::CHR_RAM_MAPPING;
    if (s == "mirror") return Scenario::MIRROR_BEHAVIOR;
    if (s == "wrapping") return Scenario::ADDR_WRAPPING;
    return Scenario::INVALID;
}

/**
 * @brief Helper function to create a synthetic NES Gamepak ROM file for use in the tests.
 * @param tempPath The path to the location where the file should be created.
 * @param id The id of the NES mapper to be tested.
 * @param s The Scenario value for what mapper behavior is to be tested.
 */
static void CreateSyntheticPak(path tempPath, u8 id, Scenario s) {
    // set nes header based on mapper being tested
    Header h {
        .name = { 'N', 'E', 'S', 0x1A },
        .byte4 = 0, .byte5 = 0, .byte6 = 0, .byte7 = 0,
        .byte8 = 0, .byte9 = 0, .byteA = 0, .byteB = 0,
        .byteC = 0, .byteD = 0, .byteE = 0, .byteF = 0
    };

    switch (id) {
        case 0: h.byte4 = 1; h.byte5 = 1; break; // NROM baseline (16KB mirrored)
        case 1: h.byte4 = 4; h.byte5 = 2; break; // MMC1 needs bank-switch room
        case 2: h.byte4 = 4; h.byte5 = 1; break; // UxROM
        case 3: h.byte4 = 2; h.byte5 = 4; break; // CNROM needs CHR bank switching
        case 7: h.byte4 = 4; h.byte5 = 0; break; // AOROM commonly CHR-RAM
        case 4: case 9: case 10: // MMC3/MMC6 needs wider PRG/CHR space, and MMC2/MMC4 needs latch CHR behavior
            h.byte4 = 8; h.byte5 = 8; break;
    }

    switch (s) {
        case Scenario::PRG_RAM_MAPPING:
            h.byte6 |= (0x01 << 1); break;
        case Scenario::CHR_RAM_MAPPING:
            h.byte5 = 0; break;
        case Scenario::FOUR_SCREEN_MIRROR:
            h.byte6 |= (0x01 << 3); break;
        case Scenario::HORIZONTAL_MIRROR:
            h.byte6 &= (~0b00001001); break; // clear "alternative nametables" and "horizontal layout" flags to force horizontal mirroring
        case Scenario::VERTICAL_MIRROR:
            h.byte6 &= (~0b00001000); // clear "alternative nametable" flag
            h.byte6 |= 0x01; // set "horizontal layout" flag to force vertical mirroring
            break;
    }

    ofstream f(tempPath, ios::binary);
    f.write(reinterpret_cast<const char*>(&h), sizeof(Header));
    constexpr size_t pBankSize = 16 * 1024;
    for (size_t b = 0; b < h.byte4; b++) {
        u8 val = static_cast<u8>(0xA0 + (b & 0x3F));
        for (size_t i = 0; i < pBankSize; i++) {
            f.put(static_cast<char>(val));
        }
    }
    constexpr size_t cBankSize = 8 * 1024;
    for (size_t b = 0; b < h.byte5; b++) {
        u8 val = static_cast<u8>(0xC0 + (b & 0x3F));
        for (size_t i = 0; i < cBankSize; i++) {
            f.put(static_cast<char>(val));
        }
    }
    f.close();
}
    

static void ExpectCpuRead(u16 addr, u8 byte) {
    // todo
}

static void ExpectPpuRead(u16 addr, u8 byte) {
    // todo
}

static void ExpectMirror(MIRROR mode) {
    // todo
}

static void ExpectIRQ(bool triggered) {
    // todo
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Test requires exactly two arguments:\nNesMapperTests <mapper-id> <scenario>";
        return 2;
    }

    int id = stoi(argv[1]);
    if (!(id == 0 || id == 1 || id == 2 || id == 3 || id == 4 || id == 7 || id == 9 || id == 10)) {
        cout << "Invalid mapper-id argument! Valid ids are: 0, 1, 2, 3, 4, 7, 9, and 10";
        return 2;
    }

    Scenario scenario = ParseScenario(argv[2]);
    if (scenario == Scenario::INVALID) {
        cout << "Invalid scenario argument! Valid values are: 'default', 'prom', 'pram', 'crom', 'cram', 'mirror', and 'wrapping'.";
        return 2;
    }

    path tempPath = temp_directory_path();
    string filename = format("m{:0>4}.nes", id);
    tempPath /= filename;

    CreateSyntheticPak(tempPath, id, scenario);

    Gamepak pak(tempPath.string());

    return 0;
}