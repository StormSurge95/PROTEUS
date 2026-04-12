#include "../../core/Helpers.h"
#include "NES_CPU.h"

#include <fstream>
#include <vector>

NES_CPU::NES_CPU(bool d) {
    debug = d;
    using a = NES_CPU;
    lookup = {
        /*0x00                   0x01                   0x02                   0x03                  0x04                   0x05                   0x06                   0x07                  0x08                   0x09                   0x0A                   0x0B                   0x0C                   0x0D                   0x0E                   0x0F*/
        { "BRK",&a::BRK,7,2 }, { "ORA",&a::ORA,6,2 }, { "KIL",&a::XXX,2,1 }, { "SLO",&a::XXX,8,1 },{ "NOP",&a::NOP,3,2 }, { "ORA",&a::ORA,3,2 }, { "ASL",&a::ASL,5,2 }, { "SLO",&a::XXX,5,1 },{ "PHP",&a::PHP,3,1 }, { "ORA",&a::ORA,2,2 }, { "ASL",&a::ASL,2,1 }, { "ANC",&a::XXX,2,1 }, { "NOP",&a::NOP,4,3 }, { "ORA",&a::ORA,4,3 }, { "ASL",&a::ASL,6,3 }, { "SLO",&a::XXX,6,1 },
        /*0x10                   0x11                   0x12                   0x13                  0x14                   0x15                   0x16                   0x17                  0x18                   0x19                   0x1A                   0x1B                   0x1C                   0x1D                   0x1E                   0x1F*/
        { "BPL",&a::BPL,2,2 }, { "ORA",&a::ORA,5,2 }, { "KIL",&a::XXX,2,1 }, { "SLO",&a::XXX,8,1 },{ "NOP",&a::NOP,4,2 }, { "ORA",&a::ORA,4,2 }, { "ASL",&a::ASL,6,2 }, { "SLO",&a::XXX,6,1 },{ "CLC",&a::CLC,2,1 }, { "ORA",&a::ORA,4,3 }, { "NOP",&a::NOP,2,1 }, { "SLO",&a::XXX,7,1 }, { "NOP",&a::NOP,4,3 }, { "ORA",&a::ORA,4,3 }, { "ASL",&a::ASL,7,3 }, { "SLO",&a::XXX,7,1 },
        /*0x20                   0x21                   0x22                   0x23                  0x24                   0x25                   0x26                   0x27                  0x28                   0x29                   0x2A                   0x2B                   0x2C                   0x2D                   0x2E                   0x2F*/
        { "JSR",&a::JSR,6,3 }, { "AND",&a::AND,6,2 }, { "KIL",&a::XXX,2,1 }, { "RLA",&a::XXX,8,1 },{ "BIT",&a::BIT,3,2 }, { "AND",&a::AND,3,2 }, { "ROL",&a::ROL,5,2 }, { "RLA",&a::XXX,5,1 },{ "PLP",&a::PLP,4,1 }, { "AND",&a::AND,2,2 }, { "ROL",&a::ROL,2,1 }, { "ANC",&a::XXX,2,1 }, { "BIT",&a::BIT,4,3 }, { "AND",&a::AND,4,3 }, { "ROL",&a::ROL,6,3 }, { "RLA",&a::XXX,6,1 },
        /*0x30                   0x31                   0x32                   0x33                  0x34                   0x35                   0x36                   0x37                  0x38                   0x39                   0x3A                   0x3B                   0x3C                   0x3D                   0x3E                   0x3F*/
        { "BMI",&a::BMI,2,2 }, { "AND",&a::AND,5,2 }, { "KIL",&a::XXX,2,1 }, { "RLA",&a::XXX,8,1 },{ "NOP",&a::NOP,4,2 }, { "AND",&a::AND,4,2 }, { "ROL",&a::ROL,6,2 }, { "RLA",&a::XXX,6,1 },{ "SEC",&a::SEC,2,1 }, { "AND",&a::AND,4,3 }, { "NOP",&a::NOP,2,1 }, { "RLA",&a::XXX,7,1 }, { "NOP",&a::NOP,4,3 }, { "AND",&a::AND,4,3 }, { "ROL",&a::ROL,7,3 }, { "RLA",&a::XXX,7,1 },
        /*0x40                   0x41                   0x42                   0x43                  0x44                   0x45                   0x46                   0x47                  0x48                   0x49                   0x4A                   0x4B                   0x4C                   0x4D                   0x4E                   0x4F*/
        { "RTI",&a::RTI,6,1 }, { "EOR",&a::EOR,6,2 }, { "KIL",&a::XXX,2,1 }, { "SRE",&a::XXX,8,1 },{ "NOP",&a::NOP,3,2 }, { "EOR",&a::EOR,3,2 }, { "LSR",&a::LSR,5,2 }, { "SRE",&a::XXX,5,1 },{ "PHA",&a::PHA,3,1 }, { "EOR",&a::EOR,2,2 }, { "LSR",&a::LSR,2,1 }, { "ALR",&a::XXX,2,1 }, { "JMP",&a::JMP,3,3 }, { "EOR",&a::EOR,4,3 }, { "LSR",&a::LSR,6,3 }, { "SRE",&a::XXX,6,1 },
        /*0x50                   0x51                   0x52                   0x53                  0x54                   0x55                   0x56                   0x57                  0x58                   0x59                   0x5A                   0x5B                   0x5C                   0x5D                   0x5E                   0x5F*/
        { "BVC",&a::BVC,2,2 }, { "EOR",&a::EOR,5,2 }, { "KIL",&a::XXX,2,1 }, { "SRE",&a::XXX,8,1 },{ "NOP",&a::NOP,4,2 }, { "EOR",&a::EOR,4,2 }, { "LSR",&a::LSR,6,2 }, { "SRE",&a::XXX,6,1 },{ "CLI",&a::CLI,2,1 }, { "EOR",&a::EOR,4,3 }, { "NOP",&a::NOP,2,1 }, { "SRE",&a::XXX,7,1 }, { "NOP",&a::NOP,4,3 }, { "EOR",&a::EOR,4,3 }, { "LSR",&a::LSR,7,3 }, { "SRE",&a::XXX,7,1 },
        /*0x60                   0x61                   0x62                   0x63                  0x64                   0x65                   0x66                   0x67                  0x68                   0x69                   0x6A                   0x6B                   0x6C                   0x6D                   0x6E                   0x6F*/
        { "RTS",&a::RTS,6,1 }, { "ADC",&a::ADC,6,2 }, { "KIL",&a::XXX,2,1 }, { "RRA",&a::XXX,8,1 },{ "NOP",&a::NOP,3,2 }, { "ADC",&a::ADC,3,2 }, { "ROR",&a::ROR,5,2 }, { "RRA",&a::XXX,5,1 },{ "PLA",&a::PLA,4,1 }, { "ADC",&a::ADC,2,2 }, { "ROR",&a::ROR,2,1 }, { "ARR",&a::XXX,2,1 }, { "JMP",&a::JMP,5,3 }, { "ADC",&a::ADC,4,3 }, { "ROR",&a::ROR,6,3 }, { "RRA",&a::XXX,6,1 },
        /*0x70                   0x71                   0x72                   0x73                  0x74                   0x75                   0x76                   0x77                  0x78                   0x79                   0x7A                   0x7B                   0x7C                   0x7D                   0x7E                   0x7F*/
        { "BVS",&a::BVS,2,2 }, { "ADC",&a::ADC,5,2 }, { "KIL",&a::XXX,2,1 }, { "RRA",&a::XXX,8,1 },{ "NOP",&a::NOP,4,2 }, { "ADC",&a::ADC,4,2 }, { "ROR",&a::ROR,6,2 }, { "RRA",&a::XXX,6,1 },{ "SEI",&a::SEI,2,1 }, { "ADC",&a::ADC,4,3 }, { "NOP",&a::NOP,2,1 }, { "RRA",&a::XXX,7,1 }, { "NOP",&a::NOP,4,3 }, { "ADC",&a::ADC,4,3 }, { "ROR",&a::ROR,7,3 }, { "RRA",&a::XXX,7,1 },
        /*0x80                   0x81                   0x82                   0x83                  0x84                   0x85                   0x86                   0x87                  0x88                   0x89                   0x8A                   0x8B                   0x8C                   0x8D                   0x8E                   0x8F*/
        { "NOP",&a::NOP,2,2 }, { "STA",&a::STA,6,2 }, { "NOP",&a::NOP,2,2 }, { "SAX",&a::XXX,6,1 },{ "STY",&a::STY,3,2 }, { "STA",&a::STA,3,2 }, { "STX",&a::STX,3,2 }, { "SAX",&a::XXX,3,1 },{ "DEY",&a::DEY,2,1 }, { "NOP",&a::NOP,2,2 }, { "TXA",&a::TXA,2,1 }, { "XAA",&a::XXX,2,1 }, { "STY",&a::STY,4,3 }, { "STA",&a::STA,4,3 }, { "STX",&a::STX,4,3 }, { "SAX",&a::XXX,4,1 },
        /*0x90                   0x91                   0x92                   0x93                  0x94                   0x95                   0x96                   0x97                  0x98                   0x99                   0x9A                   0x9B                   0x9C                   0x9D                   0x9E                   0x9F*/
        { "BCC",&a::BCC,2,2 }, { "STA",&a::STA,6,2 }, { "KIL",&a::XXX,2,1 }, { "AHX",&a::XXX,6,1 },{ "STY",&a::STY,4,2 }, { "STA",&a::STA,4,2 }, { "STX",&a::STX,4,2 }, { "SAX",&a::XXX,4,1 },{ "TYA",&a::TYA,2,1 }, { "STA",&a::STA,5,3 }, { "TXS",&a::TXS,2,1 }, { "TAS",&a::XXX,5,1 }, { "SHY",&a::XXX,5,3 }, { "STA",&a::STA,5,3 }, { "SHX",&a::XXX,5,1 }, { "AHX",&a::XXX,5,1 },
        /*0xA0                   0xA1                   0xA2                   0xA3                  0xA4                   0xA5                   0xA6                   0xA7                  0xA8                   0xA9                   0xAA                   0xAB                   0xAC                   0xAD                   0xAE                   0xAF*/
        { "LDY",&a::LDY,2,2 }, { "LDA",&a::LDA,6,2 }, { "LDX",&a::LDX,2,2 }, { "LAX",&a::XXX,6,1 },{ "LDY",&a::LDY,3,2 }, { "LDA",&a::LDA,3,2 }, { "LDX",&a::LDX,3,2 }, { "LAX",&a::XXX,3,1 },{ "TAY",&a::TAY,2,1 }, { "LDA",&a::LDA,2,2 }, { "TAX",&a::TAX,2,1 }, { "LAX",&a::XXX,2,1 }, { "LDY",&a::LDY,4,3 }, { "LDA",&a::LDA,4,3 }, { "LDX",&a::LDX,4,3 }, { "LAX",&a::XXX,4,1 },
        /*0xB0                   0xB1                   0xB2                   0xB3                  0xB4                   0xB5                   0xB6                   0xB7                  0xB8                   0xB9                   0xBA                   0xBB                   0xBC                   0xBD                   0xBE                   0xBF*/
        { "BCS",&a::BCS,2,2 }, { "LDA",&a::LDA,5,2 }, { "KIL",&a::XXX,2,1 }, { "LAX",&a::XXX,5,1 },{ "LDY",&a::LDY,4,2 }, { "LDA",&a::LDA,4,2 }, { "LDX",&a::LDX,4,2 }, { "LAX",&a::XXX,4,1 },{ "CLV",&a::CLV,2,1 }, { "LDA",&a::LDA,4,3 }, { "TSX",&a::TSX,2,1 }, { "LAS",&a::XXX,4,1 }, { "LDY",&a::LDY,4,3 }, { "LDA",&a::LDA,4,3 }, { "LDX",&a::LDX,4,3 }, { "LAX",&a::XXX,4,1 },
        /*0xC0                   0xC1                   0xC2                   0xC3                  0xC4                   0xC5                   0xC6                   0xC7                  0xC8                   0xC9                   0xCA                   0xCB                  0xCC                    0xCD                   0xCE                   0xCF*/
        { "CPY",&a::CPY,2,2 }, { "CMP",&a::CMP,6,2 }, { "NOP",&a::NOP,2,2 }, { "DCP",&a::XXX,8,1 },{ "CPY",&a::CPY,3,2 }, { "CMP",&a::CMP,3,2 }, { "DEC",&a::DEC,5,2 }, { "DCP",&a::XXX,5,1 },{ "INY",&a::INY,2,1 }, { "CMP",&a::CMP,2,2 }, { "DEX",&a::DEX,2,1 }, { "AXS",&a::XXX,2,1 }, { "CPY",&a::CPY,4,3 }, { "CMP",&a::CMP,4,3 }, { "DEC",&a::DEC,6,3 }, { "DCP",&a::XXX,6,1 },
        /*0xD0                   0xD1                   0xD2                   0xD3                  0xD4                   0xD5                   0xD6                   0xD7                  0xD8                   0xD9                   0xDA                   0xDB                  0xDC                    0xDD                   0xDE                   0xDF*/
        { "BNE",&a::BNE,2,2 }, { "CMP",&a::CMP,5,2 }, { "KIL",&a::XXX,2,1 }, { "DCP",&a::XXX,8,1 },{ "NOP",&a::NOP,4,2 }, { "CMP",&a::CMP,4,2 }, { "DEC",&a::DEC,6,2 }, { "DCP",&a::XXX,6,1 },{ "CLD",&a::CLD,2,1 }, { "CMP",&a::CMP,4,3 }, { "NOP",&a::NOP,2,1 }, { "DCP",&a::XXX,7,1 }, { "NOP",&a::NOP,4,3 }, { "CMP",&a::CMP,4,3 }, { "DEC",&a::DEC,7,3 }, { "DCP",&a::XXX,7,1 },
        /*0xE0                   0xE1                   0xE2                   0xE3                  0xE4                   0xE5                   0xE6                   0xE7                  0xE8                   0xE9                   0xEA                   0xEB                  0xEC                    0xED                   0xEE                   0xEF*/
        { "CPX",&a::CPX,2,2 }, { "SBC",&a::SBC,6,2 }, { "NOP",&a::NOP,2,2 }, { "ISC",&a::XXX,8,1 },{ "CPX",&a::CPX,3,2 }, { "SBC",&a::SBC,3,2 }, { "INC",&a::INC,5,2 }, { "ISC",&a::XXX,5,1 },{ "INX",&a::INX,2,1 }, { "SBC",&a::SBC,2,2 }, { "NOP",&a::NOP,2,1 }, { "SBC",&a::XXX,2,1 }, { "CPX",&a::CPX,4,3 }, { "SBC",&a::SBC,4,3 }, { "INC",&a::INC,6,3 }, { "ISC",&a::XXX,6,1 },
        /*0xF0                   0xF1                   0xF2                   0xF3                  0xF4                   0xF5                   0xF6                   0xF7                  0xF8                   0xF9                   0xFA                   0xFB                  0xFC                    0xFD                   0xFE                   0xFF*/
        { "BEQ",&a::BEQ,2,2 }, { "SBC",&a::SBC,5,2 }, { "KIL",&a::XXX,2,1 }, { "ISC",&a::XXX,8,1 },{ "NOP",&a::NOP,4,2 }, { "SBC",&a::SBC,4,2 }, { "INC",&a::INC,6,2 }, { "ISC",&a::XXX,6,1 },{ "SED",&a::SED,2,1 }, { "SBC",&a::SBC,4,3 }, { "NOP",&a::NOP,2,1 }, { "ISC",&a::XXX,7,1 }, { "NOP",&a::NOP,4,3 }, { "SBC",&a::SBC,4,3 }, { "INC",&a::INC,7,3 }, { "ISC",&a::XXX,7,1 }
    };
    if (debug) {
        traceStream = new std::ofstream("C:/devenv/EMU_GOD/trace.log");
    }
}

NES_CPU::~NES_CPU() {
    if (traceStream != nullptr && traceStream->is_open()) {
        traceStream->close();
        delete traceStream;
    }
}

uint8_t NES_CPU::read(uint16_t addr, bool readonly) {
    return bus->read(addr, readonly);
}

void NES_CPU::write(uint16_t addr, uint8_t data) {
    bus->write(addr, data);
}

void NES_CPU::reset() {
    fetched = 0x00;
    absAddr = 0x0000;
    relAddr = 0x0000;
    indAddr = 0x0000;
    offset = 0x00;
    soffset = 0x00;
    paged = false;
    doNMI = false;
    doIRQ = false;
    doRST = true;
    cycles = 0;
}

void NES_CPU::clock() {
    totalCycles++;
    cycles++;
    if (cycles == 1) {
        if (doRST) {
            currInst = &RST_INST;
            doRST = false;
        } else if (doNMI) {
            currInst = &NMI_INST;
            doNMI = false;
        } else if (doIRQ) {
            currInst = &IRQ_INST;
            doIRQ = false;
        } else {
            if (debug) {
                std::string t = trace();
                //printf(t.c_str());
                if (traceStream != nullptr && traceStream->is_open())
                    *traceStream << t.c_str();
            }
            opcode = read(pc++);
            currInst = &lookup[opcode];
        }
    } else {
        (this->*currInst->operate)();
    }
    if (cycles == currInst->cycles - 1)
        pollInterrupts();
}

void NES_CPU::pollInterrupts() {
    if (nmiTrigger) {
        doNMI = true;
        nmiTrigger = false;
    } else {
        doNMI = false;
        if (irqTrigger && !delayIRQ) {
            doIRQ = true;
            irqTrigger = false;
        } else {
            delayIRQ = false;
            doIRQ = false;
        }
    }
}

std::string NES_CPU::disassembleInst(uint16_t addr) {
    uint16_t line_addr = addr;
    uint8_t oc = read(addr++, true);
    uint8_t val = 0x00;
    int8_t sval = 0;
    uint8_t addr8 = 0x00;
    ADDR addr16 = 0x0000;
    ADDR ind = 0x0000;

    std::stringstream ss;
    ss << lookup[oc].name << " ";
    switch (oc) {
        case 0x0A: case 0x2A: case 0x4A: case 0x6A: // Accumulator
            ss << "A";
            break;
        case 0x09: case 0x0B: case 0x29: case 0x2B:
        case 0x49: case 0x4B: case 0x69: case 0x6B:
        case 0x80: case 0x82: case 0x89: case 0x8B:
        case 0xA0: case 0xA2: case 0xA9: case 0xAB:
        case 0xC0: case 0xC2: case 0xC9: case 0xCB:
        case 0xE0: case 0xE2: case 0xE9: case 0xEB: // Immediate
            val = read(addr++, true);
            ss << "#$" << hex(val, 2);
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
            ss << "$" << hex(addr8, 2);
            ss << " = " << hex(read(addr8, true), 2);
            break;
        case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x94: case 0x95: case 0xB4: case 0xB5:
        case 0xD4: case 0xD5: case 0xD6: case 0xD7:
        case 0xF4: case 0xF5: case 0xF6: case 0xF7: // Zero Page,X
            addr8 = read(addr++, true);
            ss << "$" << hex(addr8, 2) << ",X";
            addr8 += x;
            ss << " @ " << hex(addr8, 2);
            ss << " = " << hex(read(addr8, true), 2);
            break;
        case 0x96: case 0x97: case 0xB6: case 0xB7: // Zero Page,Y
            addr8 = read(addr++, true);
            ss << "$" << hex(addr8, 2) << ",Y";
            addr8 += y;
            ss << " @ " << hex(addr8, 2);
            ss << " = " << hex(read(addr8, true), 2);
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
            ss << "$" << hex(addr16.value(), 4);
            if (oc != 0x4C && oc != 0x20)
                ss << " = " << hex(read(addr16.value(), true), 2);
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
            ss << "$" << hex(addr16.value(), 4) << ",X";
            addr16.add(x);
            ss << " @ " << hex(addr16.value(), 4);
            ss << " = " << hex(read(addr16.value(), true), 2);
            break;
        case 0x19: case 0x1B: case 0x39: case 0x3B:
        case 0x59: case 0x5B: case 0x79: case 0x7B:
        case 0x99: case 0x9B: case 0x9E: case 0x9F:
        case 0xB9: case 0xBB: case 0xBE: case 0xBF:
        case 0xD9: case 0xDB: case 0xF9: case 0xFB: // Absolute,Y
            addr16.lo = read(addr++, true);
            addr16.hi = read(addr++, true);
            ss << "$" << hex(addr16.value(), 4) << ",Y";
            addr16.add(y);
            ss << " @ " << hex(addr16.value(), 4);
            ss << " = " << hex(read(addr16.value(), true), 2);
            break;
        case 0x6C: // Indirect
            ind.lo = read(addr++, true);
            ind.hi = read(addr++, true);
            ss << "($" << hex(ind.value(), 4) << ")";
            addr16.lo = read(ind, true);
            ind.lo++;
            addr16.hi = read(ind, true);
            ss << " = " << hex(addr16.value(), 4);
            break;
        case 0x01: case 0x03: case 0x21: case 0x23:
        case 0x41: case 0x43: case 0x61: case 0x63:
        case 0x81: case 0x83: case 0xA1: case 0xA3:
        case 0xC1: case 0xC3: case 0xE1: case 0xE3: // (Indirect,X)
            val = read(addr++, true);
            ss << "($" << hex(val, 2) << ",X)";
            val += x;
            ss << " @ " << hex(val, 2);
            addr16.lo = read(val++, true);
            addr16.hi = read(val++, true);
            ss << " = " << hex(addr16.value(), 4);
            val = read(addr16.value());
            ss << " = " << hex(val, 2);
            break;
        case 0x11: case 0x13: case 0x31: case 0x33:
        case 0x51: case 0x53: case 0x71: case 0x73:
        case 0x91: case 0x93: case 0xB1: case 0xB3:
        case 0xD1: case 0xD3: case 0xF1: case 0xF3: // (Indirect),Y
            val = read(addr++, true);
            ss << "($" << hex(val, 2) << "),Y";
            addr16.lo = read(val++, true);
            addr16.hi = read(val++, true);
            ss << " = " << hex(addr16.value(), 4);
            addr16.add(y);
            ss << " @ " << hex(addr16.value(), 4);
            val = read(addr16.value(), true);
            ss << " = " << hex(val, 2);
            break;
        case 0x10: case 0x30: case 0x50: case 0x70:
        case 0x90: case 0xB0: case 0xD0: case 0xF0: // Relative
            sval = read(addr++, true);
            addr16 = addr;
            addr16.add_s(sval);
            ss << "$" << hex(addr16.value(), 4);
            break;
    }

    return ss.str();
}

std::string NES_CPU::formatInst() {
    std::stringstream ss;

    ss << "A:" << hex(a, 2) << " ";
    ss << "X:" << hex(x, 2) << " ";
    ss << "Y:" << hex(y, 2) << " ";
    ss << "P:";// << hex(status, 2) << " ";
    ss << (getFlag(N) ? "N" : "n");
    ss << (getFlag(V) ? "V" : "v");
    ss << (getFlag(U) ? "U" : "u");
    ss << (getFlag(B) ? "B" : "b");
    ss << (getFlag(D) ? "D" : "d");
    ss << (getFlag(I) ? "I" : "i");
    ss << (getFlag(Z) ? "Z" : "z");
    ss << (getFlag(C) ? "C" : "c") << " ";
    ss << "SP:" << hex(sp, 2) << " {";
    ss << traceStack() << " }";
    //ss << bus->getPPUstatus() << " ";
    //ss << "CYC:" << totalCycles - 1;

    return ss.str();
}

std::string NES_CPU::traceStack() {
    uint8_t SP = sp;
    std::stringstream ss;

    ss << " " << hex(read(0x0100 + (SP + 1), true), 2) << ",";
    for (uint8_t i = 2; i <= 9; i++) {
        ss << " " << hex(read(0x0100 + (SP + i), true), 2) << ",";
    }
    ss << " " << hex(read(0x0100 + (SP + 10), true), 2);

    return ss.str();
}

std::string NES_CPU::trace() {
    uint16_t PC = pc;

    uint8_t oc = read(pc);

    const INST& inst = lookup[oc];

    std::stringstream ss;

    ss << hex(PC, 4) << "  ";

    uint8_t b1 = read(PC + 1, true);
    uint8_t b2 = read(PC + 2, true);

    ss << hex(oc, 2) << " ";

    if (inst.bytes >= 2)
        ss << hex(b1, 2) << " ";
    else
        ss << "   ";

    if (inst.bytes == 3)
        ss << hex(b2, 2);
    else
        ss << "  ";

    ss << " ";

    if (inst.name == "NOP" && oc != 0xEA)
        ss << "*";
    else
        ss << " ";

    ss << disassembleInst(PC);

    while (ss.str().size() < 48) ss << " ";

    ss << formatInst();

    ss << std::endl;

    return ss.str();
}

// first cycle of each addressing mode/instruction combo is handled
// within clock() when grabbing the actual opcode.
#pragma region Interrupts
void NES_CPU::IRQ() {
    switch (cycles) {
        case 2:
            read(pc++);
            break;
        case 3:
            sp--;
            write(0x0100 + sp, pc.hi);
            break;
        case 4:
            sp--;
            write(0x0100 + sp, pc.lo);
            break;
        case 5:
            setFlag(U, true);
            setFlag(B, false);
            sp--;
            write(0x0100 + sp, status);
            setFlag(B, false);
            break;
        case 6:
            setFlag(I, true);
            pc.lo = read(0xFFFE);
            break;
        case 7:
            pc.hi = read(0xFFFF);
            doIRQ = false;
            cycles = 0;
            break;
    }
}
void NES_CPU::NMI() {
    switch (cycles) {
        case 2:
            read(pc);
            break;
        case 3:
            write(0x0100 + sp, pc.hi);
            sp--;
            break;
        case 4:
            write(0x0100 + sp, pc.lo);
            sp--;
            break;
        case 5:
            setFlag(U, true);
            setFlag(B, false);
            write(0x0100 + sp, status);
            sp--;
            setFlag(B, false);
            break;
        case 6:
            setFlag(I, true);
            pc.lo = read(0xFFFA);
            break;
        case 7:
            pc.hi = read(0xFFFB);
            doNMI = false;
            cycles = 0;
            break;
    }
}
void NES_CPU::RST() {
    switch (cycles) {
        case 2:
            read(pc);
            break;
        case 3:
            sp--;
            // writes ignored during reset
            break;
        case 4:
            sp--;
            // writes ignored during reset
            break;
        case 5:
            sp--;
            // writes ignored during reset
            setFlag(I, true);
            setFlag(U, true);
            break;
        case 6:
            pc.lo = read(0xFFFC);
            break;
        case 7:
            pc.hi = read(0xFFFD);
            //pc = 0xC000;
            doRST = false;
            cycles = 0;
            break;
    }
}
#pragma endregion
#pragma region Instructions
#pragma region ACCESS
void NES_CPU::LDA() {
    switch (opcode) {
        case 0xA9: // IMM
            a = read(pc++);
            setZN(a);
            cycles = 0;
            break;
        case 0xA5: // ZP0
            switch (cycles) {
                case 2:
                    absAddr.hi = 0x00;
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    a = read(absAddr);
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xB5: // ZPX
            switch (cycles) {
                case 2:
                    absAddr.hi = 0x00;
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    read(absAddr);
                    absAddr.lo += x;
                    break;
                case 4:
                    a = read(absAddr);
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xAD: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    a = read(absAddr);
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xBD: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    a = read(absAddr);
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xB9: // ABY
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(y);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    a = read(absAddr);
                    setZN(a);
                    cycles = 0;
            }
            break;
        case 0xA1: // IZX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    absAddr.lo = read(offset++);
                    break;
                case 5:
                    absAddr.hi = read(offset);
                    break;
                case 6:
                    a = read(absAddr);
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xB1: // IZY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    absAddr.lo = read(offset++);
                    break;
                case 4:
                    absAddr.hi = read(offset);
                    page(y);
                    break;
                case 5:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 6:
                    a = read(absAddr);
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::STA() {
    switch (opcode) {
        case 0x85: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    write(offset, a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x95: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    write(offset, a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x8D: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    write(absAddr, a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x9D: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    read(absAddr);
                    if (paged) absAddr.hi++;
                    break;
                case 5:
                    write(absAddr, a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x99: // ABY
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(y);
                    break;
                case 4:
                    read(absAddr);
                    if (paged) absAddr.hi++;
                    break;
                case 5:
                    write(absAddr, a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x81: // IZX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    absAddr.lo = read(offset++);
                    break;
                case 5:
                    absAddr.hi = read(offset);
                    break;
                case 6:
                    write(absAddr, a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x91: // IZY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    absAddr.lo = read(offset++);
                    break;
                case 4:
                    absAddr.hi = read(offset);
                    page(y);
                    break;
                case 5:
                    read(absAddr);
                    if (paged) absAddr.hi++;
                    break;
                case 6:
                    write(absAddr, a);
                    cycles = 0;
                    break;
            }
    }
}
void NES_CPU::LDX() {
    switch (opcode) {
        case 0xA2: // IMM
            x = read(pc++);
            setZN(x);
            cycles = 0;
            break;
        case 0xA6:// ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    x = read(offset);
                    setZN(x);
                    cycles = 0;
                    break;
            }
            break;
        case 0xB6: // ZPY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += y;
                    break;
                case 4:
                    x = read(offset);
                    setZN(x);
                    cycles = 0;
                    break;
            }
            break;
        case 0xAE: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    x = read(absAddr);
                    setZN(x);
                    cycles = 0;
                    break;
            }
            break;
        case 0xBE: // ABY
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(y);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    x = read(absAddr);
                    setZN(x);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::STX() {
    switch (opcode) {
        case 0x86: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    write(offset, x);
                    cycles = 0;
                    break;
            }
            break;
        case 0x96: // ZPY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += y;
                    break;
                case 4:
                    write(offset, x);
                    cycles = 0;
                    break;
            }
            break;
        case 0x8E: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    write(absAddr, x);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::LDY() {
    switch (opcode) {
        case 0xA0: // IMM
            y = read(pc++);
            setZN(y);
            cycles = 0;
            break;
        case 0xA4: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    y = read(offset);
                    setZN(y);
                    cycles = 0;
                    break;
            }
            break;
        case 0xB4: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    y = read(offset);
                    setZN(y);
                    cycles = 0;
                    break;
            }
            break;
        case 0xAC: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    y = read(absAddr);
                    setZN(y);
                    cycles = 0;
                    break;
            }
            break;
        case 0xBC: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    y = read(absAddr);
                    setZN(y);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::STY() {
    switch (opcode) {
        case 0x84: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    write(offset, y);
                    cycles = 0;
                    break;
            }
            break;
        case 0x94: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    write(offset, y);
                    cycles = 0;
                    break;
            }
            break;
        case 0x8C: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    write(absAddr, y);
                    cycles = 0;
                    break;
            }
            break;
    }
}
#pragma endregion
#pragma region TRANSFER
void NES_CPU::TAX() {
    x = a;
    setZN(x);
    cycles = 0;
}
void NES_CPU::TXA() {
    a = x;
    setZN(a);
    cycles = 0;
}
void NES_CPU::TAY() {
    y = a;
    setZN(y);
    cycles = 0;
}
void NES_CPU::TYA() {
    a = y;
    setZN(a);
    cycles = 0;
}
#pragma endregion
#pragma region ARITHMETIC
void NES_CPU::ADChelper() {
    uint16_t temp = a + fetched + getFlag(C);
    uint8_t res = temp & 0xFF;
    setZN(res);
    setFlag(C, temp > 0xFF);
    setFlag(V, (temp ^ a) & (temp ^ fetched) & 0x80);
    a = res;
}
void NES_CPU::SBChelper() {
    int16_t temp = a + (~fetched) + getFlag(C);
    uint8_t res = temp & 0xFF;
    setZN(res);
    setFlag(C, !(temp < 0x00));
    setFlag(V, (res ^ a) & (res ^ ~fetched) & 0x80);
    a = res;
}

void NES_CPU::ADC() {
    switch (opcode) {
        case 0x69: // IMM
            fetched = read(pc++);
            ADChelper();
            cycles = 0;
            break;
        case 0x65: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    ADChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0x75: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    ADChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0x6D: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    ADChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0x7D: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    ADChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0x79: // ABY
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(y);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    ADChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0x61: // IZX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    absAddr.lo = read(offset++);
                    break;
                case 5:
                    absAddr.hi = read(offset);
                    break;
                case 6:
                    fetched = read(absAddr);
                    ADChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0x71: // IZY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    absAddr.lo = read(offset++);
                    break;
                case 4:
                    absAddr.hi = read(offset);
                    page(y);
                    break;
                case 5:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 6:
                    fetched = read(absAddr);
                    ADChelper();
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::SBC() {
    switch (opcode) {
        case 0xE9: // IMM
            fetched = read(pc++);
            SBChelper();
            cycles = 0;
            break;
        case 0xE5: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    SBChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0xF5: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    SBChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0xED: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    SBChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0xFD: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    SBChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0xF9: // ABY
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(y);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    SBChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0xE1: // IZX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    absAddr.lo = read(offset++);
                    break;
                case 5:
                    absAddr.hi = read(offset);
                    break;
                case 6:
                    fetched = read(absAddr);
                    SBChelper();
                    cycles = 0;
                    break;
            }
            break;
        case 0xF1: // IZY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    absAddr.lo = read(offset++);
                    break;
                case 4:
                    absAddr.hi = read(offset);
                    page(y);
                    break;
                case 5:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 6:
                    fetched = read(absAddr);
                    SBChelper();
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::INC() {
    switch (opcode) {
        case 0xE6: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    break;
                case 4:
                    write(offset, fetched);
                    fetched++;
                    break;
                case 5:
                    setZN(fetched);
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0xF6: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    break;
                case 5:
                    write(offset, fetched);
                    fetched++;
                    break;
                case 6:
                    setZN(fetched);
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0xEE: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    break;
                case 5:
                    write(absAddr, fetched);
                    fetched++;
                    break;
                case 6:
                    setZN(fetched);
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0xFE: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    read(absAddr);
                    if (paged) absAddr.hi++;
                    break;
                case 5:
                    fetched = read(absAddr);
                    break;
                case 6:
                    write(absAddr, fetched);
                    fetched++;
                    break;
                case 7:
                    setZN(fetched);
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::DEC() {
    switch (opcode) {
        case 0xC6: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    break;
                case 4:
                    write(offset, fetched);
                    fetched--;
                    break;
                case 5:
                    setZN(fetched);
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0xD6: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    break;
                case 5:
                    write(offset, fetched);
                    fetched--;
                    break;
                case 6:
                    setZN(fetched);
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0xCE: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    break;
                case 5:
                    write(absAddr, fetched);
                    fetched--;
                    break;
                case 6:
                    setZN(fetched);
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0xDE: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    read(absAddr);
                    if (paged) absAddr.hi++;
                    break;
                case 5:
                    fetched = read(absAddr);
                    break;
                case 6:
                    write(absAddr, fetched);
                    fetched--;
                    break;
                case 7:
                    setZN(fetched);
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::INX() {
    x++;
    setZN(x);
    cycles = 0;
}
void NES_CPU::DEX() {
    x--;
    setZN(x);
    cycles = 0;
}
void NES_CPU::INY() {
    y++;
    setZN(y);
    cycles = 0;
}
void NES_CPU::DEY() {
    y--;
    setZN(y);
    cycles = 0;
}
#pragma endregion
#pragma region SHIFT
void NES_CPU::shift(char dir, bool rotate) {
    uint8_t temp = fetched;
    if (dir == '<') { // shift left
        temp = (fetched << 1) | (rotate ? getFlag(C) : 0);
        setFlag(C, ((fetched >> 7) & 0x01) > 0);
    } else if (dir == '>') { // shift right
        temp = (fetched >> 1) | ((rotate ? getFlag(C) : 0) << 7);
        setFlag(C, (fetched & 0x01) > 0);
    }
    fetched = temp;
    setZN(fetched);
}

void NES_CPU::ASL() {
    switch (opcode) {
        case 0x0A: // ACC
            fetched = a;
            shift('<', false);
            a = fetched;
            cycles = 0;
            break;
        case 0x06: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    break;
                case 4:
                    write(offset, fetched);
                    shift('<', false);
                    break;
                case 5:
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x16: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    break;
                case 5:
                    write(offset, fetched);
                    shift('<', false);
                    break;
                case 6:
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x0E: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    break;
                case 5:
                    write(absAddr, fetched);
                    shift('<', false);
                    break;
                case 6:
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x1E: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    read(absAddr);
                    if (paged) absAddr.hi++;
                    break;
                case 5:
                    fetched = read(absAddr);
                    break;
                case 6:
                    write(absAddr, fetched);
                    shift('<', false);
                    break;
                case 7:
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::LSR() {
    switch (opcode) {
        case 0x4A: // ACC
            fetched = a;
            shift('>', false);
            a = fetched;
            cycles = 0;
            break;
        case 0x46: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    break;
                case 4:
                    write(offset, fetched);
                    shift('>', false);
                    break;
                case 5:
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x56: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    break;
                case 5:
                    write(offset, fetched);
                    shift('>', false);
                    break;
                case 6:
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x4E: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    break;
                case 5:
                    write(absAddr, fetched);
                    shift('>', false);
                    break;
                case 6:
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x5E: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    read(absAddr);
                    if (paged) absAddr.hi++;
                    break;
                case 5:
                    fetched = read(absAddr);
                    break;
                case 6:
                    write(absAddr, fetched);
                    shift('>', false);
                    break;
                case 7:
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::ROL() {
    switch (opcode) {
        case 0x2A: // ACC
            fetched = a;
            shift('<', true);
            a = fetched;
            cycles = 0;
            break;
        case 0x26: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    break;
                case 4:
                    write(offset, fetched);
                    shift('<', true);
                    break;
                case 5:
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x36: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    break;
                case 5:
                    write(offset, fetched);
                    shift('<', true);
                    break;
                case 6:
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x2E: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    break;
                case 5:
                    write(absAddr, fetched);
                    shift('<', true);
                    break;
                case 6:
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x3E: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    read(absAddr);
                    if (paged) absAddr.hi++;
                    break;
                case 5:
                    fetched = read(absAddr);
                    break;
                case 6:
                    write(absAddr, fetched);
                    shift('<', true);
                    break;
                case 7:
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::ROR() {
    switch (opcode) {
        case 0x6A: // ACC
            fetched = a;
            shift('>', true);
            a = fetched;
            cycles = 0;
            break;
        case 0x66: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    break;
                case 4:
                    write(offset, fetched);
                    shift('>', true);
                    break;
                case 5:
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x76: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    break;
                case 5:
                    write(offset, fetched);
                    shift('>', true);
                    break;
                case 6:
                    write(offset, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x6E: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    break;
                case 5:
                    write(absAddr, fetched);
                    shift('>', true);
                    break;
                case 6:
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
            break;
        case 0x7E: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    read(absAddr);
                    if (paged) absAddr.hi++;
                    break;
                case 5:
                    fetched = read(absAddr);
                    break;
                case 6:
                    write(absAddr, fetched);
                    shift('>', true);
                    break;
                case 7:
                    write(absAddr, fetched);
                    cycles = 0;
                    break;
            }
    }
}
#pragma endregion
#pragma region BITWISE
void NES_CPU::AND() {
    switch (opcode) {
        case 0x29: // IMM
            fetched = read(pc++);
            a = a & fetched;
            setZN(a);
            cycles = 0;
            break;
        case 0x25: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    a = a & fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x35: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    a = a & fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x2D: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    a = a & fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x3D: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    a = a & fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x39: // ABY
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(y);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    a = a & fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x21: // IZX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    absAddr.lo = read(offset++);
                    break;
                case 5:
                    absAddr.hi = read(offset);
                    break;
                case 6:
                    fetched = read(absAddr);
                    a = a & fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x31: // IZY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    absAddr.lo = read(offset++);
                    break;
                case 4:
                    absAddr.hi = read(offset);
                    page(y);
                    break;
                case 5:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 6:
                    fetched = read(absAddr);
                    a = a & fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::ORA() {
    switch (opcode) {
        case 0x09: // IMM
            fetched = read(pc++);
            a = a | fetched;
            setZN(a);
            cycles = 0;
            break;
        case 0x05: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    a = a | fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x15: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    a = a | fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x0D: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    a = a | fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x1D: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    a = a | fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x19: // ABY
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(y);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    a = a | fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x01: // IZX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    absAddr.lo = read(offset++);
                    break;
                case 5:
                    absAddr.hi = read(offset);
                    break;
                case 6:
                    fetched = read(absAddr);
                    a = a | fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x11: // IZY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    absAddr.lo = read(offset++);
                    break;
                case 4:
                    absAddr.hi = read(offset);
                    page(y);
                    break;
                case 5:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 6:
                    fetched = read(absAddr);
                    a = a | fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::EOR() {
    switch (opcode) {
        case 0x49: // IMM
            fetched = read(pc++);
            a = a ^ fetched;
            setZN(a);
            cycles = 0;
            break;
        case 0x45: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    a = a ^ fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x55: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    a = a ^ fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x4D: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    a = a ^ fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x5D: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    a = a ^ fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x59: // ABY
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(y);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    a = a ^ fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x41: // IZX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    absAddr.lo = read(offset++);
                    break;
                case 5:
                    absAddr.hi = read(offset);
                    break;
                case 6:
                    fetched = read(absAddr);
                    a = a ^ fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0x51: // IZY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    absAddr.lo = read(offset++);
                    break;
                case 4:
                    absAddr.hi = read(offset);
                    page(y);
                    break;
                case 5:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 6:
                    fetched = read(absAddr);
                    a = a ^ fetched;
                    setZN(a);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::BIT() {
    switch (opcode) {
        case 0x24: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    setFlag(V, ((fetched >> 6) & 0x01) > 0);
                    setFlag(N, ((fetched >> 7) & 0x01) > 0);
                    fetched = a & fetched;
                    setFlag(Z, fetched == 0);
                    cycles = 0;
                    break;
            }
            break;
        case 0x2C: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    setFlag(V, ((fetched >> 6) & 0x01) > 0);
                    setFlag(N, ((fetched >> 7) & 0x01) > 0);
                    fetched = a & fetched;
                    setFlag(Z, fetched == 0);
                    cycles = 0;
                    break;
            }
            break;
    }
}
#pragma endregion
#pragma region COMPARE
void NES_CPU::CMPop(uint8_t val) {
    setFlag(C, val >= fetched);
    setFlag(Z, val == fetched);
    fetched = val - fetched;
    setFlag(N, ((fetched >> 7) & 0x01) > 0);
}

void NES_CPU::CMP() {
    switch (opcode) {
        case 0xC9: // IMM
            fetched = read(pc++);
            CMPop(a);
            cycles = 0;
            break;
        case 0xC5: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    CMPop(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xD5: // ZPX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    fetched = read(offset);
                    CMPop(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xCD: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    CMPop(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xDD: // ABX
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    CMPop(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xD9: // ABY
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(y);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    fetched = read(absAddr);
                    CMPop(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xC1: // IZX
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    absAddr.lo = read(offset++);
                    break;
                case 5:
                    absAddr.hi = read(offset);
                    break;
                case 6:
                    fetched = read(absAddr);
                    CMPop(a);
                    cycles = 0;
                    break;
            }
            break;
        case 0xD1: // IZY
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    absAddr.lo = read(offset++);
                    break;
                case 4:
                    absAddr.hi = read(offset);
                    page(y);
                    break;
                case 5:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 6:
                    fetched = read(absAddr);
                    CMPop(a);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::CPX() {
    switch (opcode) {
        case 0xE0: // IMM
            fetched = read(pc++);
            CMPop(x);
            cycles = 0;
            break;
        case 0xE4: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    CMPop(x);
                    cycles = 0;
                    break;
            }
            break;
        case 0xEC: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    CMPop(x);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::CPY() {
    switch (opcode) {
        case 0xC0: // IMM
            fetched = read(pc++);
            CMPop(y);
            cycles = 0;
            break;
        case 0xC4: // ZP0
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    fetched = read(offset);
                    CMPop(y);
                    cycles = 0;
                    break;
            }
            break;
        case 0xCC: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    fetched = read(absAddr);
                    CMPop(y);
                    cycles = 0;
                    break;
            }
            break;
    }
}
#pragma endregion
#pragma region BRANCH
void NES_CPU::branch(bool take) {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            if (!take)
                cycles = 0;
            else
                spage(offset);
            break;
        case 3:
            read(pc);
            if (paged) {
                if (((offset >> 7) & 0x01) == 0) { // page foward
                    pc.hi++;
                } else { // page back
                    pc.hi--;
                }
                break;
            }
            cycles = 0;
            break;
        case 4:
            read(pc);
            cycles = 0;
            break;
    }
}

void NES_CPU::BCC() {
    branch(getFlag(C) == 0);
}
void NES_CPU::BCS() {
    branch(getFlag(C) == 1);
}
void NES_CPU::BEQ() {
    branch(getFlag(Z) == 1);
}
void NES_CPU::BNE() {
    branch(getFlag(Z) == 0);
}
void NES_CPU::BPL() {
    branch(getFlag(N) == 0);
}
void NES_CPU::BMI() {
    branch(getFlag(N) == 1);
}
void NES_CPU::BVC() {
    branch(getFlag(V) == 0);
}
void NES_CPU::BVS() {
    branch(getFlag(V) == 1);
}
#pragma endregion
#pragma region JUMP
void NES_CPU::JMP() {
    switch (opcode) {
        case 0x4C: // ABS
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    pc = absAddr;
                    cycles = 0;
                    break;
            }
            break;
        case 0x6C: // IND
            switch (cycles) {
                case 2:
                    indAddr.lo = read(pc++);
                    break;
                case 3:
                    indAddr.hi = read(pc++);
                    break;
                case 4:
                    absAddr.lo = read(indAddr);
                    break;
                case 5:
                    indAddr.lo++;
                    absAddr.hi = read(indAddr);
                    pc = absAddr;
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::JSR() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            // IDK?
            break;
        case 4:
            write(0x0100 + sp, pc.hi);
            sp--;
            break;
        case 5:
            write(0x0100 + sp, pc.lo);
            sp--;
            break;
        case 6:
            absAddr.hi = read(pc++);
            pc = absAddr;
            cycles = 0;
            break;
    }
}
void NES_CPU::RTS() {
    switch (cycles) {
        case 2:
            read(pc++);
            break;
        case 3:
            sp++;
            break;
        case 4:
            absAddr.lo = read(0x0100 + sp);
            sp++;
            break;
        case 5:
            absAddr.hi = read(0x0100 + sp);
            break;
        case 6:
            pc = absAddr;
            pc++;
            cycles = 0;
            break;
    }
}
void NES_CPU::BRK() {
    switch (cycles) {
        case 2:
            read(pc++);
            break;
        case 3:
            write(0x0100 + sp, pc.hi);
            sp--;
            break;
        case 4:
            write(0x0100 + sp, pc.lo);
            sp--;
            break;
        case 5:
            setFlag(B, true);
            setFlag(U, true);
            write(0x0100 + sp, status);
            sp--;
            setFlag(B, false);
            break;
        case 6:
            setFlag(I, true);
            pc.lo = read(0xFFFE);
            break;
        case 7:
            pc.hi = read(0xFFFF);
            cycles = 0;
            break;
    }
}
void NES_CPU::RTI() {
    switch (cycles) {
        case 2:
            read(pc);
            break;
        case 3:
            read(0x0100 + sp);
            sp++;
            break;
        case 4:
            {
                uint8_t p = read(0x0100 + sp);
                setFlag(C, (p & 0x01) > 0);
                setFlag(Z, ((p >> 1) & 0x01) > 0);
                setFlag(I, ((p >> 2) & 0x01) > 0);
                setFlag(D, ((p >> 3) & 0x01) > 0);
                setFlag(V, ((p >> 6) & 0x01) > 0);
                setFlag(N, ((p >> 7) & 0x01) > 0);
            }
            sp++;
            break;
        case 5:
            pc.lo = read(0x0100 + sp);
            sp++;
            break;
        case 6:
            pc.hi = read(0x0100 + sp);
            cycles = 0;
            break;
    }
}
#pragma endregion
#pragma region STACK
void NES_CPU::PHA() {
    switch (cycles) {
        case 2:
            read(pc);
            break;
        case 3:
            write(0x0100 + sp, a);
            sp--;
            cycles = 0;
            break;
    }
}
void NES_CPU::PLA() {
    switch (cycles) {
        case 2:
            read(pc);
            break;
        case 3:
            sp++;
            break;
        case 4:
            a = read(0x0100 + sp);
            setZN(a);
            cycles = 0;
            break;
    }

}
void NES_CPU::PHP() {
    switch (cycles) {
        case 2:
            read(pc);
            break;
        case 3:
            setFlag(B, true);
            write(0x0100 + sp, status);
            setFlag(B, false);
            sp--;
            cycles = 0;
            break;
    }

}
void NES_CPU::PLP() {
    switch (cycles) {
        case 2:
            read(pc);
            break;
        case 3:
            sp++;
            break;
        case 4:
            {
                uint8_t p = read(0x0100 + sp);
                setFlag(C, (p & 0x01) > 0);
                setFlag(Z, ((p >> 1) & 0x01) > 0);
                setFlag(I, ((p >> 2) & 0x01) > 0);
                setFlag(D, ((p >> 3) & 0x01) > 0);
                setFlag(V, ((p >> 6) & 0x01) > 0);
                setFlag(N, ((p >> 7) & 0x01) > 0);
            }
            delayNMI = true;
            delayIRQ = true;
            cycles = 0;
            break;
    }
}
void NES_CPU::TXS() {
    sp = x;
    cycles = 0;
}
void NES_CPU::TSX() {
    x = sp;
    setZN(x);
    cycles = 0;
}
#pragma endregion
#pragma region FLAGS
void NES_CPU::CLC() {
    setFlag(C, false);
    cycles = 0;
}
void NES_CPU::SEC() {
    setFlag(C, true);
    cycles = 0;
}
void NES_CPU::CLI() {
    setFlag(I, false);
    delayNMI = delayIRQ = true;
    cycles = 0;
}
void NES_CPU::SEI() {
    setFlag(I, true);
    delayNMI = delayIRQ = true;
    cycles = 0;
}
void NES_CPU::CLD() {
    setFlag(D, false);
    cycles = 0;
}
void NES_CPU::SED() {
    setFlag(D, true);
    cycles = 0;
}
void NES_CPU::CLV() {
    setFlag(V, false);
    cycles = 0;
}
#pragma endregion
#pragma region OTHER
void NES_CPU::NOP() {
    switch (opcode) {
        case 0x80: case 0x82: case 0x89: case 0xC2: case 0xE2: // IMM - 2
            read(pc++);
            [[fallthrough]];
        case 0x1A: case 0x3A: case 0x5A: case 0x7A:
        case 0xDA: case 0xEA: case 0xFA: // IMP - 2
            cycles = 0;
            break;
        case 0x04: case 0x44: case 0x64: // ZP0 - 3
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    cycles = 0;
                    break;
            }
            break;
        case 0x14: case 0x34: case 0x54:
        case 0x74: case 0xD4: case 0xF4: // ZPX - 4
            switch (cycles) {
                case 2:
                    offset = read(pc++);
                    break;
                case 3:
                    read(offset);
                    offset += x;
                    break;
                case 4:
                    read(offset);
                    cycles = 0;
                    break;
            }
            break;
        case 0x0C: // ABS - 4
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    break;
                case 4:
                    read(absAddr);
                    cycles = 0;
                    break;
            }
            break;
        case 0x1C: case 0x3C: case 0x5C:
        case 0x7C: case 0xDC: case 0xFC: // ABX - 4+
            switch (cycles) {
                case 2:
                    absAddr.lo = read(pc++);
                    break;
                case 3:
                    absAddr.hi = read(pc++);
                    page(x);
                    break;
                case 4:
                    if (paged) {
                        read(absAddr);
                        absAddr.hi++;
                        break;
                    }
                    [[fallthrough]];
                case 5:
                    read(absAddr);
                    cycles = 0;
                    break;
            }
            break;
    }
}
void NES_CPU::XXX() {
    offset = 0x00;
    printf("INVALID OPCODE (%X) ATTEMPTED\n", opcode);
    exit(EXIT_FAILURE);
}
#pragma endregion
#pragma endregion