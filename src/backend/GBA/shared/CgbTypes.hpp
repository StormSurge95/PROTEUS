#pragma once

namespace NS_GBA {
    enum class CgbCartridgeType : u8 {
        ROM_ONLY,
        MBC1_MAPPER,
        MBC1_RAM,
        MBC1_RAM_BATTERY,
        MBC2_MAPPER = 0x05,
        MBC2_BATTERY,
        ROM_RAM = 0x08,     // no licensed games make use of this type; exact behavior is unknown
        ROM_RAM_BATTERY,    // ditto
        MMM01 = 0x0B,
        MMM01_RAM,
        MMM01_RAM_BATTERY,
        MBC3_TIMER_BATTERY = 0x0F,
        MBC3_TIMER_RAM_BATTERY, // MBC3 with 64 KiB of SRAM refers to MBC30, used only in Pocket Monsters: Crystal Version (the Japanese release of Pokemon Crystal Version).
        MBC3_MAPPER,
        MBC3_RAM,               // ditto
        MBC3_RAM_BATTERY,       // ditto
        MBC5_MAPPER = 0x19,
        MBC5_RAM,
        MBC5_RAM_BATTERY,
        MBC5_RUMBLE,
        MBC5_RUMBLE_RAM,
        MBC5_RUMBLE_RAM_BATTERY,
        MBC6_MAPPER = 0x20,
        MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
        POCKET_CAMERA = 0xFC,
        BANDAI_TAMA5,
        HuC3,
        HuC1_RAM_BATTERY
    };

    /**
     * @brief Nintendo CGB/CMG ROM Header
     * @note GAMEBOY HEADERS START AT OFFSET $0100
     */
    struct CgbHeader {
        u32 EntryPoint = 0x00000000;
        u8 Logo[48] = {};
        u8 Title[16] = {};
        u8 NewLiscenseeCode[2] = {};
        u8 SgbFlag = 0x00;
        CgbCartridgeType Type = CgbCartridgeType(0x00);
        u8 RomSize = 0x00;
        u8 RamSize = 0x00;
        u8 Destination = 0x00;
        u8 OldLiscenseeCode = 0x00;
        u8 Version = 0x00;
        u8 HeaderChecksum = 0x00;
        u16 GlobalChecksum = 0x0000;
    };
}