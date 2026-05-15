#include "./NES.h"
#include "./Debugger.h"

using namespace NES_NS;

Debugger::Debugger(sptr<NES> station) {
    nes = station;
}

void Debugger::StepInstruction() {
    if (nes == nullptr || nes->cpu == nullptr || nes->ppu == nullptr || nes->apu == nullptr) return;

    u64 startTotalCycles = nes->cpu->totalCycles;
    bool sawClock = false;

    do {
        u64 prevTotal = nes->cpu->totalCycles;

        StepCycle();

        if (nes->cpu->totalCycles != prevTotal)
            sawClock = true;

        if (nes->cpu->currInst->name == "JAM")
            break;
    } while (!sawClock || nes->cpu->cycles != 0);
}

void Debugger::StepCycle() {
    // StepCycle() should mean to step one MASTER cycle rather than cpu/ppu/apu

    // validate state
    if (nes == nullptr || nes->cpu == nullptr || nes->ppu == nullptr || nes->apu == nullptr) return;
    //printf("%s\n", hex(nes->cpu->pc.value()).c_str());
    nes->clockMaster();
}

void Debugger::Clear() {
    enabled = false;
    nes->reset();
    nes = nullptr;
}

string** Debugger::GetStateCPU(u8& numRegs) const {
    numRegs = 6;
    string** regs = new string*[numRegs];
    for (u8 i = 0; i < 6; i++)
        regs[i] = new string[2];

    regs[0][0] = "STATUS";
    regs[0][1] = GetFlags(nes->cpu->status);
    regs[1][0] = "PC";
    regs[1][1] = hex(nes->cpu->pc.value(), 4);
    regs[2][0] = "A";
    regs[2][1] = hex(nes->cpu->a, 2);
    regs[3][0] = "X"; 
    regs[3][1] = hex(nes->cpu->x, 2);
    regs[4][0] = "Y";
    regs[4][1] = hex(nes->cpu->y, 2);
    regs[5][0] = "SP";
    regs[5][1] = hex(nes->cpu->sp, 2);

    return regs;
}

string** Debugger::GetStatePPU(u8& numRegs) const {
    numRegs = 8;
    string** regs = new string*[numRegs];
    for (u8 i = 0; i < numRegs; i++)
        regs[i] = new string[2];

    // TODO: add a mask for putting "-" into the string for bits that are unused by the ppu
    regs[0][0] = "PPUCTRL ($2000)";
    regs[0][1] = bin(nes->ppu->PPUCTRL);
    regs[1][0] = "PPUMASK ($2001)";
    regs[1][1] = bin(nes->ppu->PPUMASK);
    regs[2][0] = "PPUSTATUS ($2002)";
    regs[2][1] = bin(nes->ppu->PPUSTATUS);
    regs[3][0] = "OAMADDR ($2003)";
    regs[3][1] = bin(nes->ppu->OAMADDR);
    regs[4][0] = "OAMDATA ($2004)";
    regs[4][1] = bin(nes->ppu->OAMDATA);
    regs[5][0] = "PPUSCROLL ($2005)";
    regs[5][1] = bin(nes->ppu->PPUSCROLL);
    regs[6][0] = "PPUADDR ($2006)";
    regs[6][1] = bin(nes->ppu->PPUADDR);
    regs[7][0] = "PPUDATA ($2007)";
    regs[7][1] = bin(nes->ppu->PPUDATA);

    return regs;
}

string* Debugger::GetStateRAM(u64& numLines) const {
    // for NES; there will be 112 lines of RAM; each having 16 bytes
    numLines = 112;

    // create our array of lines
    // use dynamic allocation so that we can return the pointer without it being destroyed
    string* lines = new string[numLines];

    // for each line...
    for (u8 l = 0; l < numLines; l++) {
        // calculate start address (0x0010, 0x0010, 0x0020, etc)
        u16 start = (u16)l << 4;
        // create streams to hold the data
        stringstream bytes;
        stringstream chars;
        // add each byte to our data streams
        for (u8 i = 0x00; i < 0x10; i++) {
            u16 addr = start + i;
            u8 byte = nes->cpu->read(addr, true);
            bytes << hex(byte, 2);
            chars << (char)byte;
            if (i < 0x0F) {
                bytes << " ";
                chars << " ";
            }
        }
        // create stream so we can turn everything into a string to put into our `lines` array
        stringstream ss;
        // put the start address first
        ss << hex(start, 4) << ": " << bytes.str() << "     " << chars.str();
        // put the created string into our `lines` array
        lines[l] = ss.str();
    }

    return lines;
}

string Debugger::DisassembleInstruction(u16 addr) const {
    u16 line_addr = addr;
    u8 oc = nes->cpu->read(addr++, true);
    u8 val = 0x00;
    s8 sval = 0;
    u8 addr8 = 0x00;
    ADDR addr16 = 0x0000;
    ADDR ind = 0x0000;

    stringstream ss;
    ss << nes->cpu->lookup[oc].name << " "; // print instruction name
    switch (oc) {
        case 0x0A: case 0x2A: case 0x4A: case 0x6A: // Accumulator
            ss << "A"; // print A for Accumulator
            break;
        case 0x09: case 0x0B: case 0x29: case 0x2B:
        case 0x49: case 0x4B: case 0x69: case 0x6B:
        case 0x80: case 0x82: case 0x89: case 0x8B:
        case 0xA0: case 0xA2: case 0xA9: case 0xAB:
        case 0xC0: case 0xC2: case 0xC9: case 0xCB:
        case 0xE0: case 0xE2: case 0xE9: case 0xEB: // Immediate
            val = nes->cpu->read(addr++, true);
            ss << "#$" << hex(val, 2); // print argument
            break;
        case 0x04: case 0x05: case 0x06: case 0x07:
        case 0x24: case 0x25: case 0x26: case 0x27:
        case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x64: case 0x65: case 0x66: case 0x67:
        case 0x84: case 0x85: case 0x86: case 0x87:
        case 0xA4: case 0xA5: case 0xA6: case 0xA7:
        case 0xC4: case 0xC5: case 0xC6: case 0xC7:
        case 0xE4: case 0xE5: case 0xE6: case 0xE7: // Zero Page
            addr8 = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr8, 2); // print zero page address
            ss << " = " << hex(nes->cpu->read(addr8, true), 2); // print value at that address
            break;
        case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x94: case 0x95: case 0xB4: case 0xB5:
        case 0xD4: case 0xD5: case 0xD6: case 0xD7:
        case 0xF4: case 0xF5: case 0xF6: case 0xF7: // Zero Page,X
            addr8 = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr8, 2) << ",X"; // print zero page address and index register
            addr8 += nes->cpu->x;
            ss << " @ " << hex(addr8, 2); // print actual zero page address
            ss << " = " << hex(nes->cpu->read(addr8, true), 2); // print value at that address
            break;
        case 0x96: case 0x97: case 0xB6: case 0xB7: // Zero Page,Y
            addr8 = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr8, 2) << ",Y"; // print zero page address and index register
            addr8 += nes->cpu->y;
            ss << " @ " << hex(addr8, 2); // print actual zero page address
            ss << " = " << hex(nes->cpu->read(addr8, true), 2); // print value at that address
            break;
        case 0x0C: case 0x0D: case 0x0E: case 0x0F:
        case 0x20: case 0x2C: case 0x2D: case 0x2E:
        case 0x2F: case 0x4C: case 0x4D: case 0x4E:
        case 0x4F: case 0x6D: case 0x6E: case 0x6F:
        case 0x8C: case 0x8D: case 0x8E: case 0x8F:
        case 0xAC: case 0xAD: case 0xAE: case 0xAF:
        case 0xCC: case 0xCD: case 0xCE: case 0xCF:
        case 0xEC: case 0xED: case 0xEE: case 0xEF: // Absolute
            addr16.lo = nes->cpu->read(addr++, true);
            addr16.hi = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr16.value(), 4); // print the absolute address
            if (oc != 0x4C && oc != 0x20) // if this instruction is not JMP or JSR:
                ss << " = " << hex(nes->cpu->read(addr16.value(), true), 2); // print the value at that address
            break;
        case 0x1C: case 0x1D: case 0x1E: case 0x1F:
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        case 0x7C: case 0x7D: case 0x7E: case 0x7F:
        case 0x9C: case 0x9D: case 0xBC: case 0xBD:
        case 0xDC: case 0xDD: case 0xDE: case 0xDF:
        case 0xFC: case 0xFD: case 0xFE: case 0xFF: // Absolute,X
            addr16.lo = nes->cpu->read(addr++, true);
            addr16.hi = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr16.value(), 4) << ",X"; // print the absolute address and the index register
            addr16.add(nes->cpu->x);
            ss << " @ " << hex(addr16.value(), 4); // print the actual absolute address
            ss << " = " << hex(nes->cpu->read(addr16.value(), true), 2); // print the value at that address
            break;
        case 0x19: case 0x1B: case 0x39: case 0x3B:
        case 0x59: case 0x5B: case 0x79: case 0x7B:
        case 0x99: case 0x9B: case 0x9E: case 0x9F:
        case 0xB9: case 0xBB: case 0xBE: case 0xBF:
        case 0xD9: case 0xDB: case 0xF9: case 0xFB: // Absolute,Y
            addr16.lo = nes->cpu->read(addr++, true);
            addr16.hi = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr16.value(), 4) << ",Y"; // print the absolute address and the index register
            addr16.add(nes->cpu->y);
            ss << " @ " << hex(addr16.value(), 4); // print the actual absolute address
            ss << " = " << hex(nes->cpu->read(addr16.value(), true), 2); // print the value at that address
            break;
        case 0x6C: // Indirect
            ind.lo = nes->cpu->read(addr++, true);
            ind.hi = nes->cpu->read(addr++, true);
            ss << "($" << hex(ind.value(), 4) << ")"; // print the indirect address
            addr16.lo = nes->cpu->read(ind, true);
            ind.lo++;
            addr16.hi = nes->cpu->read(ind, true);
            ss << " = " << hex(addr16.value(), 4); // print the absolute address
            break;
        case 0x01: case 0x03: case 0x21: case 0x23:
        case 0x41: case 0x43: case 0x61: case 0x63:
        case 0x81: case 0x83: case 0xA1: case 0xA3:
        case 0xC1: case 0xC3: case 0xE1: case 0xE3: // (Indirect,X)
            val = nes->cpu->read(addr++, true);
            ss << "($" << hex(val, 2) << ",X)"; // print the indirect zero page address and the index register
            val += nes->cpu->x;
            ss << " @ " << hex(val, 2); // print the actual zero page address
            addr16.lo = nes->cpu->read(val++, true);
            addr16.hi = nes->cpu->read(val++, true);
            ss << " = " << hex(addr16.value(), 4); // print the absolute address
            val = nes->cpu->read(addr16.value());
            ss << " = " << hex(val, 2); // print the value at that address
            break;
        case 0x11: case 0x13: case 0x31: case 0x33:
        case 0x51: case 0x53: case 0x71: case 0x73:
        case 0x91: case 0x93: case 0xB1: case 0xB3:
        case 0xD1: case 0xD3: case 0xF1: case 0xF3: // (Indirect),Y
            val = nes->cpu->read(addr++, true);
            ss << "($" << hex(val, 2) << "),Y"; // print the indirect zero page address and the index register
            addr16.lo = nes->cpu->read(val++, true);
            addr16.hi = nes->cpu->read(val++, true);
            ss << " = " << hex(addr16.value(), 4); // print the absolute address
            addr16.add(nes->cpu->y);
            ss << " @ " << hex(addr16.value(), 4); // print the actual absolute address
            val = nes->cpu->read(addr16.value(), true);
            ss << " = " << hex(val, 2); // print the value at that address
            break;
        case 0x10: case 0x30: case 0x50: case 0x70:
        case 0x90: case 0xB0: case 0xD0: case 0xF0: // Relative
            sval = nes->cpu->read(addr++, true);
            addr16 = addr;
            addr16.add_s(sval);
            ss << "$" << hex(addr16.value(), 4); // print the offset value
            break;
    }

    return ss.str();
}

void Debugger::ScanInstructions(array<u64, 25>& list) const {
    int index = 0;
    for (const u16& e : nes->cpu->prevInstAddrs)
        list[index++] = e;

    // at this point, `index` should be 12;
    // so we can simply use `11` to get the last entry
    // and not have to deal with VS giving us irrelevant
    // or impossible warnings and errors
    u16 start = (u16)list[11];
    while (index < 25) {
        start += nes->cpu->lookup[nes->cpu->read(start, true)].bytes;
        list[index++] = start;
    }
}

string* Debugger::GetDisassembly() const {
    array<u64, 25> addrs;
    ScanInstructions(addrs);

    string* lines = new string[(u8)addrs.size()];

    for (u8 i = 0; i < addrs.size(); i++) {
        u16 addr = (u16)addrs[i];
        string str = "$" + hex(addr, 4) + ": " + DisassembleInstruction(addr);
        lines[i].resize(str.length());
        lines[i] = str;
    }

    return lines;
}

vector<u32> Debugger::GetPaletteColors() {
    vector<u32> colors;
    for (int i = 0; i < 16; ++i) {
        u8 paletteIndex = nes->ppu->ppuRead(0x3F00 + i, true);
        colors.push_back(nes->ppu->masterPalette[paletteIndex]);
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
            u8 p0 = nes->ppu->ppuRead(baseAddr + tileIndex * 16 + row);
            u8 p1 = nes->ppu->ppuRead(baseAddr + tileIndex * 16 + row + 8);

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

string Debugger::GetFlags(int status) const {
    stringstream ss;
    ss << ((status & 0x80) > 0 ? "1 " : "0 ");
    ss << ((status & 0x40) > 0 ? "1 " : "0 ");
    ss << ((status & 0x20) > 0 ? "1 " : "0 ");
    ss << ((status & 0x10) > 0 ? "1 " : "0 ");
    ss << ((status & 0x08) > 0 ? "1 " : "0 ");
    ss << ((status & 0x04) > 0 ? "1 " : "0 ");
    ss << ((status & 0x02) > 0 ? "1 " : "0 ");
    ss << ((status & 0x01) > 0 ? "1 " : "0 ");
    return ss.str();
}

vector<u32> Debugger::GetNameTables(int id) {
    return vector<u32>();
}

string** Debugger::GetStateAPU(u8& numRegs) const {
    string* val = new string("hello");
    string** ret = new string*(val);
    return ret;
}

vector<u32> Debugger::GetPulse1() {
    return vector<u32>();
}

vector<u32> Debugger::GetPulse2() {
    return vector<u32>();
}

vector<u32> Debugger::GetTriangle() {
    return vector<u32>();
}

vector<u32> Debugger::GetNoise() {
    return vector<u32>();
}

vector<u32> Debugger::GetDMC() {
    return vector<u32>();
}

void Debugger::SetTracePath(string s) {
    path p = "C:\\Users\\Redux\\Desktop";
    p /= s;
    tracePath = p;
    string name = tracePath.filename().string();
    name.pop_back(); name.pop_back(); name.pop_back(); name.pop_back();
    std::chrono::zoned_time t{ std::chrono::current_zone(), std::chrono::floor<std::chrono::seconds>(system_clock::now()) };
    name += format("_{:%F_%T}", t);
    name += ".txt";
    replace(name.begin(), name.end(), ':', '-');
    tracePath.replace_filename(name);
    traceFile.open(tracePath);
    if (!traceFile.is_open()) {
        printf("FAILED TO OPEN TRACE FILE AT PATH: %s\n", tracePath.string().c_str());
    }
}