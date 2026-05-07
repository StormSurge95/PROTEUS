#include "./CPU.h"

using namespace NES_NS;

CPU::CPU() {
    // The only thing we do within the constructor is initialize the lookup table
    // TODO: Maybe there is some way to remove this massive wall of code and have something a little more streamlined?
    lookup = {
        //0x00                            0x01                            0x02                            0x03                            0x04                            0x05                            0x06                            0x07                            0x08                            0x09                            0x0A                            0x0B                            0x0C                            0x0D                            0x0E                            0x0F
        {"BRK",2,nullptr,    &CPU::BRK},{"ORA",2,&CPU::IZX_R,&CPU::ORA},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"SLO",2,&CPU::IZX_M,&CPU::SLO},{"NOP",2,&CPU::ZP0_R,&CPU::NOP},{"ORA",2,&CPU::ZP0_R,&CPU::ORA},{"ASL",2,&CPU::ZP0_M,&CPU::ASL},{"SLO",2,&CPU::ZP0_M,&CPU::SLO},{"PHP",1,nullptr,    &CPU::PHP},{"ORA",2,&CPU::IMM_A,&CPU::ORA},{"ASL",1,&CPU::ACC_A,&CPU::ASL},{"ANC",2,&CPU::IMM_A,&CPU::ANC},{"NOP",3,&CPU::ABS_R,&CPU::NOP},{"ORA",3,&CPU::ABS_R,&CPU::ORA},{"ASL",3,&CPU::ABS_M,&CPU::ASL},{"SLO",3,&CPU::ABS_M,&CPU::SLO},
        //0x10                            0x11                            0x12                            0x13                            0x14                            0x15                            0x16                            0x17                            0x18                            0x19                            0x1A                            0x1B                            0x1C                            0x1D                            0x1E                            0x1F
        {"BPL",2,&CPU::REL_B,&CPU::BPL},{"ORA",2,&CPU::IZY_R,&CPU::ORA},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"SLO",2,&CPU::IZY_M,&CPU::SLO},{"NOP",2,&CPU::ZPX_R,&CPU::NOP},{"ORA",2,&CPU::ZPX_R,&CPU::ORA},{"ASL",2,&CPU::ZPX_M,&CPU::ASL},{"SLO",2,&CPU::ZPX_M,&CPU::SLO},{"CLC",1,&CPU::IMP_A,&CPU::CLC},{"ORA",3,&CPU::ABY_R,&CPU::ORA},{"NOP",1,&CPU::IMP_A,&CPU::NOP},{"SLO",3,&CPU::ABY_M,&CPU::SLO},{"NOP",3,&CPU::ABX_R,&CPU::NOP},{"ORA",3,&CPU::ABX_R,&CPU::ORA},{"ASL",3,&CPU::ABX_M,&CPU::ASL},{"SLO",3,&CPU::ABX_M,&CPU::SLO},
        //0x20                            0x21                            0x22                            0x23                            0x24                            0x25                            0x26                            0x27                            0x28                            0x19                            0x2A                            0x2B                            0x2C                            0x2D                            0x2E                            0x2F
        {"JSR",3,nullptr,    &CPU::JSR},{"AND",2,&CPU::IZX_R,&CPU::AND},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"RLA",2,&CPU::IZX_M,&CPU::RLA},{"BIT",2,&CPU::ZP0_R,&CPU::BIT},{"AND",2,&CPU::ZP0_R,&CPU::AND},{"ROL",2,&CPU::ZP0_M,&CPU::ROL},{"RLA",2,&CPU::ZP0_M,&CPU::RLA},{"PLP",1,nullptr,    &CPU::PLP},{"AND",2,&CPU::IMM_A,&CPU::AND},{"ROL",1,&CPU::ACC_A,&CPU::ROL},{"ANC",2,&CPU::IMM_A,&CPU::ANC},{"BIT",3,&CPU::ABS_R,&CPU::BIT},{"AND",3,&CPU::ABS_R,&CPU::AND},{"ROL",3,&CPU::ABS_M,&CPU::ROL},{"RLA",3,&CPU::ABS_M,&CPU::RLA},
        //0x30                            0x31                            0x32                            0x33                            0x34                            0x35                            0x36                            0x37                            0x38                            0x39                            0x3A                            0x3B                            0x3C                            0x3D                            0x3E                            0x3F
        {"BMI",2,&CPU::REL_B,&CPU::BMI},{"AND",2,&CPU::IZY_R,&CPU::AND},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"RLA",2,&CPU::IZY_M,&CPU::RLA},{"NOP",2,&CPU::ZPX_R,&CPU::NOP},{"AND",2,&CPU::ZPX_R,&CPU::AND},{"ROL",2,&CPU::ZPX_M,&CPU::ROL},{"RLA",2,&CPU::ZPX_M,&CPU::RLA},{"SEC",1,&CPU::IMP_A,&CPU::SEC},{"AND",3,&CPU::ABY_R,&CPU::AND},{"NOP",1,&CPU::IMP_A,&CPU::NOP},{"RLA",3,&CPU::ABY_M,&CPU::RLA},{"NOP",3,&CPU::ABX_R,&CPU::NOP},{"AND",3,&CPU::ABX_R,&CPU::AND},{"ROL",3,&CPU::ABX_M,&CPU::ROL},{"RLA",3,&CPU::ABX_M,&CPU::RLA},
        //0x40                            0x41                            0x42                            0x43                            0x44                            0x45                            0x46                            0x47                            0x48                            0x49                            0x4A                            0x4B                            0x4C                            0x4D                            0x4E                            0x4F
        {"RTI",1,nullptr,    &CPU::RTI},{"EOR",2,&CPU::IZX_R,&CPU::EOR},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"SRE",2,&CPU::IZX_M,&CPU::SRE},{"NOP",2,&CPU::ZP0_R,&CPU::NOP},{"EOR",2,&CPU::ZP0_R,&CPU::EOR},{"LSR",2,&CPU::ZP0_M,&CPU::LSR},{"SRE",2,&CPU::ZP0_M,&CPU::SRE},{"PHA",1,nullptr,    &CPU::PHA},{"EOR",2,&CPU::IMM_A,&CPU::EOR},{"LSR",1,&CPU::ACC_A,&CPU::LSR},{"ASR",2,&CPU::IMM_A,&CPU::ASR},{"JMP",3,&CPU::ABS_J,&CPU::JMP},{"EOR",3,&CPU::ABS_R,&CPU::EOR},{"LSR",3,&CPU::ABS_M,&CPU::LSR},{"SRE",3,&CPU::ABS_M,&CPU::SRE},
        //0x50                            0x51                            0x52                            0x53                            0x54                            0x55                            0x56                            0x57                            0x58                            0x59                            0x5A                            0x5B                            0x5C                            0x5D                            0x5E                            0x5F
        {"BVC",2,&CPU::REL_B,&CPU::BVC},{"EOR",2,&CPU::IZY_R,&CPU::EOR},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"SRE",2,&CPU::IZY_M,&CPU::SRE},{"NOP",2,&CPU::ZPX_R,&CPU::NOP},{"EOR",2,&CPU::ZPX_R,&CPU::EOR},{"LSR",2,&CPU::ZPX_M,&CPU::LSR},{"SRE",2,&CPU::ZPX_M,&CPU::SRE},{"CLI",1,&CPU::IMP_A,&CPU::CLI},{"EOR",3,&CPU::ABY_R,&CPU::EOR},{"NOP",1,&CPU::IMP_A,&CPU::NOP},{"SRE",3,&CPU::ABY_M,&CPU::SRE},{"NOP",3,&CPU::ABX_R,&CPU::NOP},{"EOR",3,&CPU::ABX_R,&CPU::EOR},{"LSR",3,&CPU::ABX_M,&CPU::LSR},{"SRE",3,&CPU::ABX_M,&CPU::SRE},
        //0x60                            0x61                            0x62                            0x63                            0x64                            0x65                            0x66                            0x67                            0x68                            0x69                            0x6A                            0x6B                            0x6C                            0x6D                            0x6E                            0x6F
        {"RTS",1,nullptr,    &CPU::RTS},{"ADC",2,&CPU::IZX_R,&CPU::ADC},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"RRA",2,&CPU::IZX_M,&CPU::RRA},{"NOP",2,&CPU::ZP0_R,&CPU::NOP},{"ADC",2,&CPU::ZP0_R,&CPU::ADC},{"ROR",2,&CPU::ZP0_M,&CPU::ROR},{"RRA",2,&CPU::ZP0_M,&CPU::RRA},{"PLA",1,nullptr,    &CPU::PLA},{"ADC",2,&CPU::IMM_A,&CPU::ADC},{"ROR",1,&CPU::ACC_A,&CPU::ROR},{"ARR",2,&CPU::IMM_A,&CPU::ARR},{"JMP",3,&CPU::IND_J,&CPU::JMP},{"ADC",3,&CPU::ABS_R,&CPU::ADC},{"ROR",3,&CPU::ABS_M,&CPU::ROR},{"RRA",3,&CPU::ABS_M,&CPU::RRA},
        //0x70                            0x71                            0x72                            0x73                            0x74                            0x75                            0x76                            0x77                            0x78                            0x79                            0x7A                            0x7B                            0x7C                            0x7D                            0x7E                            0x7F
        {"BVS",2,&CPU::REL_B,&CPU::BVS},{"ADC",2,&CPU::IZY_R,&CPU::ADC},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"RRA",2,&CPU::IZY_M,&CPU::RRA},{"NOP",2,&CPU::ZPX_R,&CPU::NOP},{"ADC",2,&CPU::ZPX_R,&CPU::ADC},{"ROR",2,&CPU::ZPX_M,&CPU::ROR},{"RRA",2,&CPU::ZPX_M,&CPU::RRA},{"SEI",1,&CPU::IMP_A,&CPU::SEI},{"ADC",3,&CPU::ABY_R,&CPU::ADC},{"NOP",1,&CPU::IMP_A,&CPU::NOP},{"RRA",3,&CPU::ABY_M,&CPU::RRA},{"NOP",3,&CPU::ABX_R,&CPU::NOP},{"ADC",3,&CPU::ABX_R,&CPU::ADC},{"ROR",3,&CPU::ABX_M,&CPU::ROR},{"RRA",3,&CPU::ABX_M,&CPU::RRA},
        //0x80                            0x81                            0x82                            0x83                            0x84                            0x85                            0x86                            0x87                            0x88                            0x89                            0x8A                            0x8B                            0x8C                            0x8D                            0x8E                            0x8F
        {"NOP",2,&CPU::IMM_A,&CPU::NOP},{"STA",2,&CPU::IZX_W,&CPU::STA},{"NOP",2,&CPU::IMM_A,&CPU::NOP},{"SAX",2,&CPU::IZX_W,&CPU::SAX},{"STY",2,&CPU::ZP0_W,&CPU::STY},{"STA",2,&CPU::ZP0_W,&CPU::STA},{"STX",2,&CPU::ZP0_W,&CPU::STX},{"SAX",2,&CPU::ZP0_W,&CPU::SAX},{"DEY",1,&CPU::IMP_A,&CPU::DEY},{"NOP",2,&CPU::IMM_A,&CPU::NOP},{"TXA",1,&CPU::IMP_A,&CPU::TXA},{"ANE",2,&CPU::IMM_A,&CPU::ANE},{"STY",3,&CPU::ABS_W,&CPU::STY},{"STA",3,&CPU::ABS_W,&CPU::STA},{"STX",3,&CPU::ABS_W,&CPU::STX},{"SAX",3,&CPU::ABS_W,&CPU::SAX},
        //0x90                            0x91                            0x92                            0x93                            0x94                            0x95                            0x96                            0x97                            0x98                            0x99                            0x9A                            0x9B                            0x9C                            0x9D                            0x9E                            0x9F
        {"BCC",2,&CPU::REL_B,&CPU::BCC},{"STA",2,&CPU::IZY_W,&CPU::STA},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"SHA",2,&CPU::IZY_W,&CPU::SHA},{"STY",2,&CPU::ZPX_W,&CPU::STY},{"STA",2,&CPU::ZPX_W,&CPU::STA},{"STX",2,&CPU::ZPY_W,&CPU::STX},{"SAX",2,&CPU::ZPY_W,&CPU::SAX},{"TYA",1,&CPU::IMP_A,&CPU::TYA},{"STA",3,&CPU::ABY_W,&CPU::STA},{"TXS",1,&CPU::IMP_A,&CPU::TXS},{"SHS",3,&CPU::ABY_W,&CPU::SHS},{"SHY",3,&CPU::ABX_W,&CPU::SHY},{"STA",3,&CPU::ABX_W,&CPU::STA},{"SHX",3,&CPU::ABY_W,&CPU::SHX},{"SHA",3,&CPU::ABY_W,&CPU::SHA},
        //0xA0                            0xA1                            0xA2                            0xA3                            0xA4                            0xA5                            0xA6                            0xA7                            0xA8                            0xA9                            0xAA                            0xAB                            0xAC                            0xAD                            0xAE                            0xAF
        {"LDY",2,&CPU::IMM_A,&CPU::LDY},{"LDA",2,&CPU::IZX_R,&CPU::LDA},{"LDX",2,&CPU::IMM_A,&CPU::LDX},{"LAX",2,&CPU::IZX_R,&CPU::LAX},{"LDY",2,&CPU::ZP0_R,&CPU::LDY},{"LDA",2,&CPU::ZP0_R,&CPU::LDA},{"LDX",2,&CPU::ZP0_R,&CPU::LDX},{"LAX",2,&CPU::ZP0_R,&CPU::LAX},{"TAY",1,&CPU::IMP_A,&CPU::TAY},{"LDA",2,&CPU::IMM_A,&CPU::LDA},{"TAX",1,&CPU::IMP_A,&CPU::TAX},{"LXA",2,&CPU::IMM_A,&CPU::LXA},{"LDY",3,&CPU::ABS_R,&CPU::LDY},{"LDA",3,&CPU::ABS_R,&CPU::LDA},{"LDX",3,&CPU::ABS_R,&CPU::LDX},{"LAX",3,&CPU::ABS_R,&CPU::LAX},
        //0xB0                            0xB1                            0xB2                            0xB3                            0xB4                            0xB5                            0xB6                            0xB7                            0xB8                            0xB9                            0xBA                            0xBB                            0xBC                            0xBD                            0xBE                            0xBF
        {"BCS",2,&CPU::REL_B,&CPU::BCS},{"LDA",2,&CPU::IZY_R,&CPU::LDA},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"LAX",2,&CPU::IZY_R,&CPU::LAX},{"LDY",2,&CPU::ZPX_R,&CPU::LDY},{"LDA",2,&CPU::ZPX_R,&CPU::LDA},{"LDX",2,&CPU::ZPY_R,&CPU::LDX},{"LAX",2,&CPU::ZPY_R,&CPU::LAX},{"CLV",1,&CPU::IMP_A,&CPU::CLV},{"LDA",3,&CPU::ABY_R,&CPU::LDA},{"TSX",1,&CPU::IMP_A,&CPU::TSX},{"LAS",3,&CPU::ABY_R,&CPU::LAS},{"LDY",3,&CPU::ABX_R,&CPU::LDY},{"LDA",3,&CPU::ABX_R,&CPU::LDA},{"LDX",3,&CPU::ABY_R,&CPU::LDX},{"LAX",3,&CPU::ABY_R,&CPU::LAX},
        //0xC0                            0xC1                            0xC2                            0xC3                            0xC4                            0xC5                            0xC6                            0xC7                            0xC8                            0xC9                            0xCA                            0xCB                            0xCC                            0xCD                            0xCE                            0xCF
        {"CPY",2,&CPU::IMM_A,&CPU::CPY},{"CMP",2,&CPU::IZX_R,&CPU::CMP},{"NOP",2,&CPU::IMM_A,&CPU::NOP},{"DCP",2,&CPU::IZX_M,&CPU::DCP},{"CPY",2,&CPU::ZP0_R,&CPU::CPY},{"CMP",2,&CPU::ZP0_R,&CPU::CMP},{"DEC",2,&CPU::ZP0_M,&CPU::DEC},{"DCP",2,&CPU::ZP0_M,&CPU::DCP},{"INY",1,&CPU::IMP_A,&CPU::INY},{"CMP",2,&CPU::IMM_A,&CPU::CMP},{"DEX",1,&CPU::IMP_A,&CPU::DEX},{"AXS",2,&CPU::IMM_A,&CPU::AXS},{"CPY",3,&CPU::ABS_R,&CPU::CPY},{"CMP",3,&CPU::ABS_R,&CPU::CMP},{"DEC",3,&CPU::ABS_M,&CPU::DEC},{"DCP",3,&CPU::ABS_M,&CPU::DCP},
        //0xD0                            0xD1                            0xD2                            0xD3                            0xD4                            0xD5                            0xD6                            0xD7                            0xD8                            0xD9                            0xDA                            0xDB                            0xDC                            0xDD                            0xDE                            0xDF
        {"BNE",2,&CPU::REL_B,&CPU::BNE},{"CMP",2,&CPU::IZY_R,&CPU::CMP},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"DCP",2,&CPU::IZY_M,&CPU::DCP},{"NOP",2,&CPU::ZPX_R,&CPU::NOP},{"CMP",2,&CPU::ZPX_R,&CPU::CMP},{"DEC",2,&CPU::ZPX_M,&CPU::DEC},{"DCP",2,&CPU::ZPX_M,&CPU::DCP},{"CLD",1,&CPU::IMP_A,&CPU::CLD},{"CMP",3,&CPU::ABY_R,&CPU::CMP},{"NOP",1,&CPU::IMP_A,&CPU::NOP},{"DCP",3,&CPU::ABY_M,&CPU::DCP},{"NOP",3,&CPU::ABX_R,&CPU::NOP},{"CMP",3,&CPU::ABX_R,&CPU::CMP},{"DEC",3,&CPU::ABX_M,&CPU::DEC},{"DCP",3,&CPU::ABX_M,&CPU::DCP},
        //0xE0                            0xE1                            0xE2                            0xE3                            0xE4                            0xE5                            0xE6                            0xE7                            0xE8                            0xE9                            0xEA                            0xEB                            0xEC                            0xED                            0xEE                            0xEF
        {"CPX",2,&CPU::IMM_A,&CPU::CPX},{"SBC",2,&CPU::IZX_R,&CPU::SBC},{"NOP",2,&CPU::IMM_A,&CPU::NOP},{"ISC",2,&CPU::IZX_M,&CPU::ISC},{"CPX",2,&CPU::ZP0_R,&CPU::CPX},{"SBC",2,&CPU::ZP0_R,&CPU::SBC},{"INC",2,&CPU::ZP0_M,&CPU::INC},{"ISC",2,&CPU::ZP0_M,&CPU::ISC},{"INX",1,&CPU::IMP_A,&CPU::INX},{"SBC",2,&CPU::IMM_A,&CPU::SBC},{"NOP",1,&CPU::IMP_A,&CPU::NOP},{"SBC",2,&CPU::IMM_A,&CPU::SBC},{"CPX",3,&CPU::ABS_R,&CPU::CPX},{"SBC",3,&CPU::ABS_R,&CPU::SBC},{"INC",3,&CPU::ABS_M,&CPU::INC},{"ISC",3,&CPU::ABS_M,&CPU::ISC},
        //0xF0                            0xF1                            0xF2                            0xF3                            0xF4                            0xF5                            0xF6                            0xF7                            0xF8                            0xF9                            0xFA                            0xFB                            0xFC                            0xFD                            0xFE                            0xFF
        {"BEQ",2,&CPU::REL_B,&CPU::BEQ},{"SBC",2,&CPU::IZY_R,&CPU::SBC},{"JAM",1,&CPU::IMP_A,&CPU::JAM},{"ISC",2,&CPU::IZY_M,&CPU::ISC},{"NOP",2,&CPU::ZPX_R,&CPU::NOP},{"SBC",2,&CPU::ZPX_R,&CPU::SBC},{"INC",2,&CPU::ZPX_M,&CPU::INC},{"ISC",2,&CPU::ZPX_M,&CPU::ISC},{"SED",1,&CPU::IMP_A,&CPU::SED},{"SBC",3,&CPU::ABY_R,&CPU::SBC},{"NOP",1,&CPU::IMP_A,&CPU::NOP},{"ISC",3,&CPU::ABY_M,&CPU::ISC},{"NOP",3,&CPU::ABX_R,&CPU::NOP},{"SBC",3,&CPU::ABX_R,&CPU::SBC},{"INC",3,&CPU::ABX_M,&CPU::INC},{"ISC",3,&CPU::ABX_M,&CPU::ISC}
    };
}

u8 CPU::read(u16 addr, bool readonly) {
    return bus->read(addr, readonly);
}

void CPU::write(u16 addr, u8 data) {
    bus->write(addr, data);
}

void CPU::start() {
    // During power on, we simply have to read the first pc value from the reset vector of the cartridge
    pc.lo = bus->read(0xFFFC);
    pc.hi = bus->read(0xFFFD);
}

void CPU::reset() {
    // During reset, we set the state of the CPU to a known value by clearing everything
    // and then triggering our reset function
    fetched = 0x00;
    absAddr = 0x0000;
    relAddr = 0x0000;
    indAddr = 0x0000;
    offset = 0x00;
    paged = false;
    pendingNMI = false;
    pendingIRQ = false;
    pendingRST = true;
    cycles = 0;
}

void CPU::clock() {
    /// We initialize `cycles` to `0`, but only start operations when it is `1`; so our logic requires pre-incrementing.
    cycles++;
    if (cycles == 1) {
        /// On cycle `1`, we either trigger an interrupt/reset, or read the next opcode to prepare for the next instruction.
        /// @todo figure out why this didn't work properly before and fix it
        //if (updateStatus) {
        //    status = dStatus;
        //    updateStatus = false;
        //}
        if (pendingRST) {
            // if a reset is pending, set next 'instruction' to be a reset
            currInst = &RST_INST;
            pendingRST = false;
        } else if (pendingNMI) {
            // if a NMI is pending, set next 'instruction' to be a NMI
            currInst = &NMI_INST;
            pendingNMI = false;
        } else if (pendingIRQ) {
            // if an IRQ is pending, set next 'instruction' to be an IRQ
            currInst = &IRQ_INST;
            pendingIRQ = false;
        } else {
            // otherwise, read next opcode and set next instruction as necessary
            opcode = read(pc++);
            currInst = &lookup[opcode];
        }
    } else {
        if (currInst->address != nullptr) // if this instruction requires addressing mode logic, then perform that function
            (this->*currInst->address)();
        else // otherwise, simply perform the operation function, as it will handle the cycle logic itself
            (this->*currInst->operate)();
    }
    if (cycles == 0) {
        /// when `cycles == 0`, the most recent instruction is completed; so let's poll the interrupts
        pollInterrupts();
    }
    // increment total cycles
    totalCycles++;
}

void CPU::pollInterrupts() {
    if (nmiTrigger) {
        // acknowledge NMI
        pendingNMI = true;
        nmiTrigger = false;
    }
    if (irqTrigger && getFlag(FLAGS::I) == 0) {
        if (!delayInterrupt) {
            // acknolege IRQ
            pendingIRQ = true;
            irqTrigger = false;
        }
    }
    delayInterrupt = false;
}

string CPU::disassembleInst(u16 addr) {
    u16 line_addr = addr;
    u8 oc = read(addr++, true);
    u8 val = 0x00;
    s8 sval = 0;
    u8 addr8 = 0x00;
    ADDR addr16 = 0x0000;
    ADDR ind = 0x0000;

    stringstream ss;
    ss << lookup[oc].name << " "; // print instruction name
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
            val = read(addr++, true);
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
            addr8 = read(addr++, true);
            ss << "$" << hex(addr8, 2); // print zero page address
            ss << " = " << hex(read(addr8, true), 2); // print value at that address
            break;
        case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x94: case 0x95: case 0xB4: case 0xB5:
        case 0xD4: case 0xD5: case 0xD6: case 0xD7:
        case 0xF4: case 0xF5: case 0xF6: case 0xF7: // Zero Page,X
            addr8 = read(addr++, true);
            ss << "$" << hex(addr8, 2) << ",X"; // print zero page address and index register
            addr8 += x;
            ss << " @ " << hex(addr8, 2); // print actual zero page address
            ss << " = " << hex(read(addr8, true), 2); // print value at that address
            break;
        case 0x96: case 0x97: case 0xB6: case 0xB7: // Zero Page,Y
            addr8 = read(addr++, true);
            ss << "$" << hex(addr8, 2) << ",Y"; // print zero page address and index register
            addr8 += y;
            ss << " @ " << hex(addr8, 2); // print actual zero page address
            ss << " = " << hex(read(addr8, true), 2); // print value at that address
            break;
        case 0x0C: case 0x0D: case 0x0E: case 0x0F:
        case 0x20: case 0x2C: case 0x2D: case 0x2E:
        case 0x2F: case 0x4C: case 0x4D: case 0x4E:
        case 0x4F: case 0x6D: case 0x6E: case 0x6F:
        case 0x8C: case 0x8D: case 0x8E: case 0x8F:
        case 0xAC: case 0xAD: case 0xAE: case 0xAF:
        case 0xCC: case 0xCD: case 0xCE: case 0xCF:
        case 0xEC: case 0xED: case 0xEE: case 0xEF: // Absolute
            addr16.lo = read(addr++, true);
            addr16.hi = read(addr++, true);
            ss << "$" << hex(addr16.value(), 4); // print the absolute address
            if (oc != 0x4C && oc != 0x20) // if this instruction is not JMP or JSR:
                ss << " = " << hex(read(addr16.value(), true), 2); // print the value at that address
            break;
        case 0x1C: case 0x1D: case 0x1E: case 0x1F:
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        case 0x7C: case 0x7D: case 0x7E: case 0x7F:
        case 0x9C: case 0x9D: case 0xBC: case 0xBD:
        case 0xDC: case 0xDD: case 0xDE: case 0xDF:
        case 0xFC: case 0xFD: case 0xFE: case 0xFF: // Absolute,X
            addr16.lo = read(addr++, true);
            addr16.hi = read(addr++, true);
            ss << "$" << hex(addr16.value(), 4) << ",X"; // print the absolute address and the index register
            addr16.add(x);
            ss << " @ " << hex(addr16.value(), 4); // print the actual absolute address
            ss << " = " << hex(read(addr16.value(), true), 2); // print the value at that address
            break;
        case 0x19: case 0x1B: case 0x39: case 0x3B:
        case 0x59: case 0x5B: case 0x79: case 0x7B:
        case 0x99: case 0x9B: case 0x9E: case 0x9F:
        case 0xB9: case 0xBB: case 0xBE: case 0xBF:
        case 0xD9: case 0xDB: case 0xF9: case 0xFB: // Absolute,Y
            addr16.lo = read(addr++, true);
            addr16.hi = read(addr++, true);
            ss << "$" << hex(addr16.value(), 4) << ",Y"; // print the absolute address and the index register
            addr16.add(y);
            ss << " @ " << hex(addr16.value(), 4); // print the actual absolute address
            ss << " = " << hex(read(addr16.value(), true), 2); // print the value at that address
            break;
        case 0x6C: // Indirect
            ind.lo = read(addr++, true);
            ind.hi = read(addr++, true);
            ss << "($" << hex(ind.value(), 4) << ")"; // print the indirect address
            addr16.lo = read(ind, true);
            ind.lo++;
            addr16.hi = read(ind, true);
            ss << " = " << hex(addr16.value(), 4); // print the absolute address
            break;
        case 0x01: case 0x03: case 0x21: case 0x23:
        case 0x41: case 0x43: case 0x61: case 0x63:
        case 0x81: case 0x83: case 0xA1: case 0xA3:
        case 0xC1: case 0xC3: case 0xE1: case 0xE3: // (Indirect,X)
            val = read(addr++, true);
            ss << "($" << hex(val, 2) << ",X)"; // print the indirect zero page address and the index register
            val += x;
            ss << " @ " << hex(val, 2); // print the actual zero page address
            addr16.lo = read(val++, true);
            addr16.hi = read(val++, true);
            ss << " = " << hex(addr16.value(), 4); // print the absolute address
            val = read(addr16.value());
            ss << " = " << hex(val, 2); // print the value at that address
            break;
        case 0x11: case 0x13: case 0x31: case 0x33:
        case 0x51: case 0x53: case 0x71: case 0x73:
        case 0x91: case 0x93: case 0xB1: case 0xB3:
        case 0xD1: case 0xD3: case 0xF1: case 0xF3: // (Indirect),Y
            val = read(addr++, true);
            ss << "($" << hex(val, 2) << "),Y"; // print the indirect zero page address and the index register
            addr16.lo = read(val++, true);
            addr16.hi = read(val++, true);
            ss << " = " << hex(addr16.value(), 4); // print the absolute address
            addr16.add(y);
            ss << " @ " << hex(addr16.value(), 4); // print the actual absolute address
            val = read(addr16.value(), true);
            ss << " = " << hex(val, 2); // print the value at that address
            break;
        case 0x10: case 0x30: case 0x50: case 0x70:
        case 0x90: case 0xB0: case 0xD0: case 0xF0: // Relative
            sval = read(addr++, true);
            addr16 = addr;
            addr16.add_s(sval);
            ss << "$" << hex(addr16.value(), 4); // print the offset value
            break;
    }

    return ss.str();
}

void CPU::IRQ() {
    switch (cycles) {
        case 2: // read pc and discard
            read(pc.value());
            //if (nmiTrigger) { currInst = &NMI_INST; }
            break;
        case 3: // write pc.hi to stack
            write(0x0100 + sp, pc.hi);
            sp--;
            //if (nmiTrigger) { currInst = &NMI_INST; }
            break;
        case 4: // write pc.lo to stack
            write(0x0100 + sp, pc.lo);
            sp--;
            //if (nmiTrigger) { currInst = &NMI_INST; }
            break;
        case 5: // write status to stack with B flag NOT set
            setFlag(FLAGS::U, true);
            setFlag(FLAGS::B, false);
            write(0x0100 + sp, status);
            sp--;
            setFlag(FLAGS::B, false);
            break;
        case 6:
            // set I flag and read pc.lo from IRQ vector
            setFlag(FLAGS::I, true);
            pc.lo = read(0xFFFE);
            break;
        case 7:
            // read pc.hi from IRQ vector and reset cycles
            pc.hi = read(0xFFFF);
            cycles = 0;
            break;
    }
}
void CPU::NMI() {
    switch (cycles) {
        case 2: // read pc and discard
            read(pc.value());
            break;
        case 3: // write pc.hi to stack
            write(0x0100 + sp, pc.hi);
            sp--;
            break;
        case 4: // write pc.lo to stack
            write(0x0100 + sp, pc.lo);
            sp--;
            break;
        case 5: // write status to stack with B flag NOT set
            setFlag(FLAGS::U, true);
            setFlag(FLAGS::B, false);
            write(0x0100 + sp, status);
            sp--;
            setFlag(FLAGS::B, false);
            break;
        case 6: // set I flag and read pc.lo from NMI vector
            setFlag(FLAGS::I, true);
            pc.lo = read(0xFFFA);
            break;
        case 7: // read pc.hi from NMI vector and reset cycles
            pc.hi = read(0xFFFB);
            cycles = 0;
            break;
    }
}
void CPU::RST() {
    switch (cycles) {
        case 2: // read pc and discard
            read(pc);
            break;
        case 3: // decrement SP
            sp--;
            // writes ignored during reset
            break;
        case 4: // decrement SP
            sp--;
            // writes ignored during reset
            break;
        case 5: // decrement SP and set I/U flags
            sp--;
            // writes ignored during reset
            setFlag(FLAGS::I, true);
            setFlag(FLAGS::U, true);
            break;
        case 6: // read pc.lo from reset vector
            pc.lo = read(0xFFFC);
            break;
        case 7: // read pc.hi from reset vector and reset cycles
            pc.hi = read(0xFFFD);
            //pc = 0xC000; // manually set pc for running nestest without functional PPU
            cycles = 0;
            break;
    }
}