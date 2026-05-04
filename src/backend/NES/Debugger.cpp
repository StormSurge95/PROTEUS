#include "./NES.h"
#include "./BUS.h"
#include "./Debugger.h"

using namespace NES_NS;

Debugger::Debugger(sptr<NES> station) {
    bus = station->bus;
    cpu = station->cpu;
    ppu = station->ppu;
}

void Debugger::StepInstruction() {}

void Debugger::StepCycle() {}

void Debugger::Clear() {
    enabled = false;
    bus.reset();
    cpu.reset();
    ppu.reset();
    bus = nullptr;
    cpu = nullptr;
    ppu = nullptr;
}

string Debugger::GetStateCPU() {
    stringstream ss;
    // print current status flags
    ss << " N V - B D I Z C\n";
    ss << " " << GetFlags(cpu->status) << endl;

    // print current register values
    ss << " PC: " << hex(cpu->pc.value(), 4) << endl;
    ss << "  A: " << hex(cpu->a, 2) << endl;
    ss << "  X: " << hex(cpu->x, 2) << endl;
    ss << "  Y: " << hex(cpu->y, 2) << endl;
    ss << " SP: " << hex(cpu->sp, 2) << endl;

    // collect and print disassembled instructions
    ss << GetDisassembly();

    return ss.str();
}

string Debugger::GetStateRAM() {
    stringstream ss;

    for (u16 i = 0x0000; i <= 0x07FF; i++) {
        ss << " ";
        if (i == (i & 0x07F0)) ss << hex(i & 0xFF, 2) << ": ";
        ss << hex(bus->read(i, true));
        if ((i & 0x0F) == 15 && i != 0x07FF) ss << "\n";
    }

    return ss.str();
}

void Debugger::ScanInstructions(u16 maxOffset = 0x3F, bool allowPage = false) {
    instAddrs.clear();

    u8 pI = 0;
    u16 first = cpu->pc.value();
    u16 start = first;

    while (instAddrs.size() < 12) {
        start--;
        u8 offset = first - start;
        if (offset > maxOffset || (!allowPage && (start & 0xFF00) != (first & 0xFF00))) break;
        u8 opcode = bus->read(start, true);
        INST i = cpu->lookup[opcode];
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

string Debugger::GetDisassembly() {
    ScanInstructions();

    stringstream ss; // stream to hold our disassembled data

    for (const u16& addr : instAddrs) {
        ss << " $" << hex(addr, 4) << ": " << cpu->disassembleInst(addr) << endl;
    }

    return ss.str();
}

vector<u32> Debugger::GetPaletteColors() {
    vector<u32> colors;
    for (int i = 0; i < 16; ++i) {
        u8 paletteIndex = ppu->ppuRead(0x3F00 + i, true);
        colors.push_back(ppu->masterPalette[paletteIndex]);
    }
    return colors;
}

vector<u32> Debugger::GetPatternTable(int index) {
    vector<u32> pixels(16384, 0xFF000000);

    // TODO: Get Pattern Table pixel data from PPU
    u16 baseAddr = index * 0x1000;

    for (u16 tileIndex = 0; tileIndex < 256; tileIndex++) {
        u8 tileX = (tileIndex % 16);
        u8 tileY = (tileIndex / 16);

        for (u8 row = 0; row < 8; row++) {
            u8 p0 = ppu->ppuRead(baseAddr + tileIndex * 16 + row);
            u8 p1 = ppu->ppuRead(baseAddr + tileIndex * 16 + row + 8);

            for (u8 col = 0; col < 8; col++) {
                u8 b0 = (p0 >> (7 - col)) & 0x01;
                u8 b1 = (p1 >> (7 - col)) & 0x01;

                u8 colorIndex = (b1 << 1) | b0;

                u32 color = debugPalette[colorIndex];

                u16 pixelX = tileX * 8 + col;
                u16 pixelY = tileY * 8 + row;

                pixels[(size_t)(pixelY * 128 + pixelX)] = color;
            }
        }
    }

    return pixels;
}

string Debugger::GetFlags(int status) {
    stringstream ss;
    ss << ((status & 0x80) > 0 ? "N" : "n");
    ss << ((status & 0x40) > 0 ? "V" : "v");
    ss << ((status & 0x20) > 0 ? "U" : "u");
    ss << ((status & 0x10) > 0 ? "B" : "b");
    ss << ((status & 0x08) > 0 ? "D" : "d");
    ss << ((status & 0x04) > 0 ? "I" : "i");
    ss << ((status & 0x02) > 0 ? "Z" : "z");
    ss << ((status & 0x01) > 0 ? "C" : "c");
    return ss.str();
}