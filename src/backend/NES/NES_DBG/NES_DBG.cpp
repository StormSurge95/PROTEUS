#include <sstream>
#include <string>

#include "NES_DBG.h"
#include "../../../core/Helpers.h"

NES_DBG::NES_DBG(std::shared_ptr<NES_CORE> station) {
    bus = station->bus;
    cpu = station->cpu;
    ppu = station->ppu;
}

void NES_DBG::StepInstruction() {}

void NES_DBG::StepCycle() {}

void NES_DBG::Clear() {
    enabled = false;
    bus.reset();
    cpu.reset();
    ppu.reset();
    bus = nullptr;
    cpu = nullptr;
    ppu = nullptr;
}

std::string NES_DBG::GetStateCPU() {
    std::stringstream ss;
    // print current status flags
    ss << " N V - B D I Z C\n";
    ss << " " << hex(cpu->getFlag(NES_CPU::FLAGS::N), 1) << " " <<
        hex(cpu->getFlag(NES_CPU::FLAGS::V), 1) << " " <<
        hex(cpu->getFlag(NES_CPU::FLAGS::U), 1) << " " <<
        hex(cpu->getFlag(NES_CPU::FLAGS::B), 1) << " " <<
        hex(cpu->getFlag(NES_CPU::FLAGS::D), 1) << " " <<
        hex(cpu->getFlag(NES_CPU::FLAGS::I), 1) << " " <<
        hex(cpu->getFlag(NES_CPU::FLAGS::Z), 1) << " " <<
        hex(cpu->getFlag(NES_CPU::FLAGS::C), 1) << std::endl;

    // print current register values
    ss << " PC: " << hex(cpu->pc.value(), 4) << std::endl;
    ss << "  A: " << hex(cpu->a, 2) << std::endl;
    ss << "  X: " << hex(cpu->x, 2) << std::endl;
    ss << "  Y: " << hex(cpu->y, 2) << std::endl;
    ss << " SP: " << hex(cpu->sp, 2) << std::endl;

    // collect and print disassembled instructions
    ss << GetDisassembly();

    return ss.str();
}

std::string NES_DBG::GetStateRAM() {
    std::stringstream ss;

    for (uint16_t i = 0x0000; i <= 0x07FF; i++) {
        ss << " ";
        if (i == (i & 0x07F0)) ss << hex(i & 0xFF, 2) << ": ";
        ss << hex(bus->read(i, true));
        if ((i & 0x0F) == 15 && i != 0x07FF) ss << "\n";
    }

    return ss.str();
}

void NES_DBG::ScanInstructions(uint16_t maxOffset = 0x3F, bool allowPage = false) {
    instAddrs.clear();

    uint8_t pI = 0;
    uint16_t first = cpu->pc.value();
    uint16_t start = first;

    while (instAddrs.size() < 12) {
        start--;
        uint8_t offset = first - start;
        if (offset > maxOffset || (!allowPage && (start & 0xFF00) != (first & 0xFF00))) break;
        uint8_t opcode = bus->read(start, true);
        NES_CPU::INST i = cpu->lookup[opcode];
        if (i.name == "JAM") continue;
        if (i.bytes == offset) {
            instAddrs.push_back(start);
        }
    }

    first = cpu->pc.value();
    start = first;
    instAddrs.push_back(start);

    while (instAddrs.size() < 25) {
        start += cpu->lookup[cpu->read(start, true)].bytes;
        instAddrs.push_back(start);
    }
}

std::string NES_DBG::GetDisassembly() {
    ScanInstructions();

    std::stringstream ss; // stream to hold our disassembled data

    for (const uint16_t& addr : instAddrs) {
        ss << " $" << hex(addr, 4) << ": " << cpu->disassembleInst(addr) << std::endl;
    }

    return ss.str();
}

std::vector<uint32_t> NES_DBG::GetPaletteColors() {
    std::vector<uint32_t> colors;
    for (int i = 0; i < 16; ++i) {
        uint8_t paletteIndex = ppu->ppuRead(0x3F00 + i, true);
        colors.push_back(ppu->masterPalette[paletteIndex]);
    }
    return colors;
}

std::vector<uint32_t> NES_DBG::GetPatternTable(int index) {
    std::vector<uint32_t> pixels(16384, 0xFF000000);

    // TODO: Get Pattern Table pixel data from PPU
    uint16_t baseAddr = index * 0x1000;

    for (uint16_t tileIndex = 0; tileIndex < 256; tileIndex++) {
        uint8_t tileX = (tileIndex % 16);
        uint8_t tileY = (tileIndex / 16);

        for (uint8_t row = 0; row < 8; row++) {
            uint8_t p0 = ppu->ppuRead(baseAddr + tileIndex * 16 + row);
            uint8_t p1 = ppu->ppuRead(baseAddr + tileIndex * 16 + row + 8);

            for (uint8_t col = 0; col < 8; col++) {
                uint8_t b0 = (p0 >> (7 - col)) & 0x01;
                uint8_t b1 = (p1 >> (7 - col)) & 0x01;

                uint8_t colorIndex = (b1 << 1) | b0;

                uint32_t color = debugPalette[colorIndex];

                uint16_t pixelX = tileX * 8 + col;
                uint16_t pixelY = tileY * 8 + row;

                pixels[(size_t)(pixelY * 128 + pixelX)] = color;
            }
        }
    }

    return pixels;
}