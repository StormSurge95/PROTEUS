#include "./NesCPU.h"
#include "../PPU/NesPPU.h"
#include "../APU/NesAPU.h"
#include "../PAK/NesGamepak.h"
#include "../PAK/Mappers/NesMapper.h"
#include "../NesController.h"

using namespace NS_NES;

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

void CPU::initSST(SingleStateTest::State state) {
    pc = state.pc;
    a = state.a;
    x = state.x;
    y = state.y;
    sp = state.s;
    status = state.p;
    for (auto& e : state.ram) {
        ram[e[0]] = (e[1] & 0xFF);
    }
}

bool CPU::checkSST(SingleStateTest::State state, string& result) {
    bool pass = true;
    result = "PASS\n";
    stringstream ss;
    ss << "                 EXPECTED | ACTUAL\n";
    if (pc != state.pc) {
        pass = false;
    }
    ss << "         PC:       " << hex(state.pc, 4) << " | " << hex(pc.value(), 4) << endl;
    if (a != state.a) {
        pass = false;
    }
    ss << "          A:         " << hex(state.a) << " | " << hex(a) << endl;
    if (x != state.x) {
        pass = false;
    }
    ss << "          X:         " << hex(state.x) << " | " << hex(x) << endl;
    if (y != state.y) {
        pass = false;
    }
    ss << "          Y:         " << hex(state.y) << " | " << hex(y) << endl;
    if (sp != state.s) {
        pass = false;
    }
    ss << "         SP:         " << hex(state.s) << " | " << hex(sp) << endl;
    if (status != state.p) {
        pass = false;
    }
    ss << "          P:     ";
    ss << ((state.p & 0x80) > 0 ? "N" : "n");
    ss << ((state.p & 0x40) > 0 ? "V" : "v");
    ss << ((state.p & 0x20) > 0 ? "U" : "u");
    ss << ((state.p & 0x10) > 0 ? "B" : "b");
    ss << ((state.p & 0x08) > 0 ? "D" : "d");
    ss << ((state.p & 0x04) > 0 ? "I" : "i");
    ss << ((state.p & 0x02) > 0 ? "Z" : "z");
    ss << ((state.p & 0x01) > 0 ? "C" : "c");
    ss << " | ";
    ss << ((status & 0x80) > 0 ? "N" : "n");
    ss << ((status & 0x40) > 0 ? "V" : "v");
    ss << ((status & 0x20) > 0 ? "U" : "u");
    ss << ((status & 0x10) > 0 ? "B" : "b");
    ss << ((status & 0x08) > 0 ? "D" : "d");
    ss << ((status & 0x04) > 0 ? "I" : "i");
    ss << ((status & 0x02) > 0 ? "Z" : "z");
    ss << ((status & 0x01) > 0 ? "C" : "c") << endl;
    for (auto& e : state.ram) {
        u16 addr = e[0];
        u8 ex = (u8)e[1];
        u8 ac = ram[addr];
        if (ac != ex) {
            result = string("INSTRUCTION ") + lookup[opcode].name + " FAIL! RAM[0x" + hex(addr, 4) + "] was wrong; expected: " + hex(ex) + ", actual: " + hex(ac) + "\n";
            pass = false;
        }
        ss << "RAM[" << hex(addr, 4) << "]:         " << hex(ex) << " | " << hex(ac) << endl;
    }
    if (!pass) result = ss.str();
    return pass;
}

u8 CPU::read(u16 addr, bool readonly) {
    #ifdef TEST_SST
    return ram[addr];
    #else
    // update last read address for use during dummy dma reads.
    lastReadAddr = addrBus = addr;
    // create helper variable to prevent updating open bus when readonly is set
    u8 ret = cpuBus;
    // all reads directly update the open bus in some way, sometimes only partially.
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        // read from WRAM
        ret = ram[addr & 0x07FF];
        if (readonly) return ret;
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // read from PPU registers
        ret = ppu.lock()->read(addr, readonly);
        if (readonly) return ret;
    } else if (addr == 0x4015) {
        return (cpuBus & 0x20) | (apu.lock()->read(addr, readonly) & 0xDF);
    } else if (addr == 0x4016) {
        // read Player 1 Controller
        ret = (cpuBus & 0xE0) | (player1.lock()->onRead() & 0x1F);
        if (readonly) return ret;
        else if (eventSink) eventSink->OnControllerRead(1, 0x4016, ret);
    } else if (addr == 0x4017) {
        // read Player 2 Controller
        ret = (cpuBus & 0xE0) | (player2.lock()->onRead() & 0x1F);
        if (readonly) return ret;
        else if (eventSink) eventSink->OnControllerRead(2, 0x4017, ret);
    } else if (addr >= 0x6000 && addr <= 0xFFFF) {
        // read cartridge memory (including SRAM, if present)
        if (!cart.lock()->mapper->cpuRead(addr, ret, readonly)) ret = cpuBus;
        if (readonly) return ret;
    }

    // getting here means readonly is clear; so update cpuBus and return it.
    cpuBus = ret;
    return cpuBus;
    #endif
}

void CPU::write(u16 addr, u8 data) {
    #ifdef TEST_SST
    ram[addr] = data;
    #else
    addrBus = addr;
    // all writes fully update open bus
    cpuBus = data;
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        // write to WRAM
        ram[addr & 0x07FF] = data;
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // write to PPU registers
        ppu.lock()->write(addr, data);
    } else if (addr == 0x4014) {
        // write to PPU OAM; triggering OAMDMA
        oamPage = data;
        oamAddr = 0x00;
        oamActive = true;
        oamPhase = OAM_PHASE::HALT;
    } else if (addr == 0x4016) {
        // write to player1 controller
        conWriteVal = data;
        conWriteDel = isGetCycle() ? 1 : 2;
        if (eventSink) eventSink->OnControllerWrite(1, 0x4016, data);
    } else if (addr >= 0x4000 && addr <= 0x4017) {
        apu.lock()->write(addr, data);
    } else if (addr >= 0x5FFF && addr <= 0xFFFF) {
        // write to Cartridge memory (including SRAM, if present)
        cart.lock()->mapper->cpuWrite(addr, data);
    }

    #endif
}

void CPU::connectCONT(sptr<Controller>& c, u8 player) {
    // if player 1, simply connect
    if (player == 1)
        player1 = c;
    // otherwise, connect controller and then interconnect both controllers
    else {
        player2 = c;
        // technically shouldn't be necessary because this is donw within the NES constructor...
        // ...but oh well I guess
        player1.lock()->other = player2;
        player2.lock()->other = player1;
    }
}

void CPU::clockConWrite() {
    if (conWriteDel != 0 && --conWriteDel == 0) {
        player1.lock()->onWrite(conWriteVal);
    }
}

void CPU::halt() {
    if (halted) return;

    dmaHaltAddr = cycles == 0 ? pc.value() : lastReadAddr;

    if (currInst && currInst->address == &CPU::ABS_R && cycles == 3) {
        const u16 effAddr = absAddr.value();
        const u16 decAddr = effAddr >= 0x2000 && effAddr <= 0x3FFF ? 0x2000 | (effAddr & 0x0007) : effAddr;

        switch (decAddr) {
            case 0x2002: case 0x2007:
            case 0x4015: case 0x4016: case 0x4017:
                dmaHaltAddr = effAddr;
                break;
            default:
                if ((effAddr & 0xFFE0) == 0x4000)
                    dmaHaltAddr = effAddr;
                break;
        }
    }

    halted = true;

    switch (opcode) {
        case 0x93:
            if (cycles == 4) magic = true;
            break;
        case 0x9B: case 0x9C:
        case 0x9E: case 0x9F:
        case 0xBB:
            if (cycles == 3) magic = true;
            break;
        default:
            magic = false;
            break;
    }
}

bool CPU::serviceDMA() {
    if (dmcAbort) {
        dmcAbort = false;
        dmcActive = false;
        dmcPhase = DMC_PHASE::IDLE;

        // OAM may still own the CPU after DMC aborts.
        halted = oamActive;
    }

    const bool getCycle = isGetCycle();

    /**
     * "Load DMCDMA" attempts to halt on get.
     * "Reload DMCDMA" attempts to halt on put.
     * 
     * After a failed halt caused by a CPU write, it retries every
     * cycle without again requiring the original scheduled phase.
     */
    const bool schedDmcHalt = dmcLoad ? getCycle : !getCycle;

    const bool dmcCanStart = dmcPending && (dmcHaltRetry || schedDmcHalt);

    /**
     * A pending DMC request that has not reached its halt phase does
     * not consume the CPU cycle. Active OAM must still continue.
     */
    if (!halted && !oamActive && !dmcActive && !dmcCanStart) return false;

    /**
     * Capture the CPU only once. DMA cannot halt during a CPU write.
     * This also permits both writes of an RMW instruction to finish.
     */
    if (!halted) {
        if (nextCycleWrites()) {
            if (dmcCanStart) dmcHaltRetry = true;
            return false;
        }

        halt();
    }

    /**
     * If OAM already halted the CPU, DMC can begin without consulting
     * `nextCycleWrites()`: the CPU is no longer executing instruction
     * microcycles.
     */
    if (dmcCanStart) {
        dmcPending = dmcHaltRetry = dmcLoad = false;
        dmcActive = true;
        dmcPhase = DMC_PHASE::HALT;
    }

    /**
     * Preserve DMC phase as it existed during this bus cycle.
     * This is needed to decide whether repeated joypad reads
     * should be suppressed.
     */
    const bool dmcWasActive = dmcActive;
    const DMC_PHASE prevDmcPhase = dmcPhase;

    /**
     * DMC is evaluated first because a ready DMC read owns a get
     * cycle. DMC halt/dummy/alignment return false, allowing OAM
     * to continue on those cycles.
     */
    const bool dmcUsedBus = clockDMC();
    const bool oamUsedBus = clockOAM(!dmcUsedBus);
    const bool busUsed = dmcUsedBus || oamUsedBus;

    if (!busUsed) {
        const u16 stalledReg = dmaHaltAddr >= 0x2000 && dmaHaltAddr <= 0x3FFF ? 0x2000 | (dmaHaltAddr & 0x0007) : dmaHaltAddr;

        const bool stalledJoy = stalledReg == 0x4016 || stalledReg == 0x4017;

        /**
         * The DMC halt cycle performs the first read, while subsequent
         * DMC dummy/alignment cycles do not produce separate joypad clocks.
         * 
         * OAM-only idle/alignment cycles retain their existing read.
         */
        const bool suppressJoyRead = dmcWasActive && prevDmcPhase != DMC_PHASE::HALT && stalledJoy;

        if (!suppressJoyRead) read(stalledReg);
    }

    /**
     * Only the arbiter releases the CPU. Completing one DMA cannot
     * resume execution while the other remains active.
     */
    halted = oamActive || dmcActive;

    return true;
}

bool CPU::clockOAM(bool isBusAvail) {
    if (!oamActive) return false;

    switch (oamPhase) {
        case OAM_PHASE::HALT:
            /**
             * The caller (`serviceDMA`) performs the repeated CPU read(s) if
             * neither DMA engine otherwise uses the bus.
             * 
             * Leaving HALT always advances OAM to the GET phase.
             * If the following cycles is a put cycle, the GET phase simply
             * waits, producing the required OAM alignment cycle.
             */
            oamPhase = OAM_PHASE::GET;
            return false;
        case OAM_PHASE::GET: {
            if (!isBusAvail || !isGetCycle()) return false;

            const u16 srcAddr = (static_cast<u16>(oamPage) << 8) | oamAddr;

            const bool srcInInternalWindow = (srcAddr & 0xFFE0) == 0x4000;

            /**
             * $4000-$401F has no ordinary memory-mapped value in the
             * current bus implementation. If the stalled 6502 address
             * does not activate the internal registers, this is open bus.
             * 
             * Do not call read(srcAddr) here because that would decode $4015-$4017 from the DMA address itself.
             */
            if (srcInInternalWindow) {
                addrBus = srcAddr;
                oamData = cpuBus;
            } else {
                oamData = read(srcAddr);
            }

            /**
             * A15-A5 come from the stalled 6502 address. A4-A0 come from
             * the DMA address currently selected by the address multiplexer.
             */
            if ((dmaHaltAddr & 0xFFE0) == 0x4000) {
                const u16 internalAddr = 0x4000 | (srcAddr & 0x001F);

                switch (internalAddr) {
                    case 0x4015: case 0x4016: case 0x4017:
                        oamData = read(internalAddr);
                        break;
                    default: break;
                }
            }

            oamPhase = OAM_PHASE::PUT;
            return true;
        }
        case OAM_PHASE::PUT:
            if (!isBusAvail || isGetCycle()) return false;

            // $2004 is the OAMDATA register; so we hardcode that
            // here because there's nowhere else that OAMDMA would
            // be writing OAM data to.
            addrBus = 0x2004;
            cpuBus = oamData;
            
            ppu.lock()->writeOAMByte(
                static_cast<u8>(
                    ppu.lock()->getOAMADDR() + oamAddr
                ),
                oamData
            );

            oamAddr++;

            if (oamAddr == 0x00) {
                // OAMDMA is complete
                oamActive = false;
                oamPhase = OAM_PHASE::IDLE;
            } else {
                // there are more OAM bytes to copy
                oamPhase = OAM_PHASE::GET;
            }

            return true;
        case OAM_PHASE::IDLE: default:
            oamActive = false;
            return false;
    }
}

bool CPU::clockDMC() {
    if (!dmcActive) return false;

    switch (dmcPhase) {
        case DMC_PHASE::HALT:
            // No DMC bus access. OAM may use this same cycle.
            dmcPhase = DMC_PHASE::DUMMY;
            return false;
        case DMC_PHASE::DUMMY:
            /**
             * If the dummy occurs on a get, the following put must
             * be consumed as alignment. If it occurs on a put, the
             * following get can perform the DMC read immediately.
             */
            dmcPhase = isGetCycle() ? DMC_PHASE::ALIGN : DMC_PHASE::READ;
            return false;
        case DMC_PHASE::ALIGN:
            // No DMC bus access. OAM may write on this put cycle.
            dmcPhase = DMC_PHASE::READ;
            return false;
        case DMC_PHASE::READ:
            if (!isGetCycle()) return false;

            dmcData = read(dmcAddr);

            if ((dmaHaltAddr & 0xFFE0) == 0x4000) {
                const u16 internalAddr = 0x4000 | (dmcAddr & 0x001F);

                if (internalAddr >= 0x4015 && internalAddr <= 0x4017) read(internalAddr);
            }

            apu.lock()->dmcOnByteFetched(dmcData);

            dmcActive = false;
            dmcPhase = DMC_PHASE::IDLE;
            return true;
        case DMC_PHASE::IDLE: default:
            dmcActive = false;
            return false;
    }
}

void CPU::powerup(u32 s) {
    initPRNG(s);

    // initialize WRAM with random values
    for (u8& byte : ram) byte = nextByte();

    conWriteVal = 0x00;
    conWriteDel = 0x00;

    // clear instruction/decode scratch state
    magic = paged = branch = false;
    absAddr = relAddr = indAddr = fetched = opcode = offset = 0;
    currInst = nullptr;
    prevInstAddrs.clear();

    // clear cpu bus/open bus helpers
    lastReadAddr = addrBus = cpuBus = 0;

    // clear DMA state
    halted = oamActive = dmcPending = dmcActive =
    dmcLoad = dmcHaltRetry = dmcAbort = false;
    oamPhase = OAM_PHASE::IDLE;
    dmcPhase = DMC_PHASE::IDLE;
    dmcAddr = dmcData = oamPage = oamAddr = oamData = 0;

    // clear interrupt and poll bookkeeping
    interruptSource = pendingInterruptSource = INTERRUPT::NONE;
    resetPending = irqLine_APU = irqLine_DMC = irqLine_Mapper = false;
    nmiPending = nmiLineSampled = false;
    interruptFlagViaPoll = false;
    IFVP = {};

    deferredStatusRead = false;
    deferredStatusOperate = nullptr;

    // reset execution counter
    totalCycles = cycles = 0;

    // initialize visible data registers
    a = nextByte();
    x = nextByte();
    y = nextByte();

    // initialize stack pointer so that reset sequence can set it properly
    sp = 0x00;

    // `U` flag should always be set; `I` flag will be set by the reset sequence
    status = (u8)FLAGS::U;

    // PC is not yet valid; reset sequence will handle setting it properly
    pc = 0x0000;

    // request actual reset sequence
    requestReset();
}

void CPU::reset() {
    // request the real reset sequence
    resetPending = true;

    // abort current instruction context so next CPU step starts from interrupt polling
    cycles = 0;
    interruptSource = pendingInterruptSource = INTERRUPT::NONE;
    currInst = nullptr;
    opcode = fetched = 0;
    absAddr = relAddr = indAddr = 0;
    offset = 0;
    paged = branch = magic = false;

    // cancle cpu-side dma/halt state so reset is not delayed by interops
    halted = false;
    oamActive = false;
    oamPage = oamAddr = oamData = 0;
    dmcPending = dmcActive = dmcLoad =
    dmcHaltRetry = dmcAbort = false;
    dmcAddr = 0;
    dmcData = 0;
    oamPhase = OAM_PHASE::IDLE;
    dmcPhase = DMC_PHASE::IDLE;

    // clear cpu-owned pending edge/latch state
    nmiPending = nmiLineSampled = false;
    interruptFlagViaPoll = false;
    IFVP = {};

    deferredStatusRead = false;
    deferredStatusOperate = nullptr;
}

void CPU::powerdown() {
    // stop active execution state
    halted = false;
    cycles = 0;
    interruptSource = pendingInterruptSource = INTERRUPT::NONE;

    // cancel pending reset/interrupt activity
    resetPending = nmiPending = nmiLineSampled = irqLine_APU =
    irqLine_DMC = irqLine_Mapper = interruptFlagViaPoll = false;
    IFVP = {};

    conWriteVal = 0x00;
    conWriteDel = 0x00;
    
    // cancel DMA state completely
    oamActive = dmcPending = dmcActive =
    dmcLoad = dmcHaltRetry = dmcAbort = false;
    oamPage = oamAddr = oamData = dmcAddr = dmcData = 0;
    oamPhase = OAM_PHASE::IDLE;
    dmcPhase = DMC_PHASE::IDLE;

    // clear transient decode/bus helper state
    currInst = nullptr;
    magic = paged = branch = false;
    fetched = opcode = absAddr = relAddr =
    indAddr = offset = cpuBus = addrBus =
    lastReadAddr = 0;
    prevInstAddrs.clear();

    // invalidate CPU state
    pc = a = x = y = sp = status = 0;

    // clear lifecycle state
    totalCycles = 0;

    deferredStatusRead = false;
    deferredStatusOperate = nullptr;
}

void CPU::clock() {
    onGetCycle = (totalCycles & 0x01) == 0;

    clockConWrite();

    if (!serviceDMA()) {
        const INTERRUPT candidate = pollInterrupts();
        const u8 exeCycle = cycles + 1;
        const bool wasIseq = interruptSource != INTERRUPT::NONE;
        const bool wasBranch = cycles != 0 && currInst != nullptr && currInst->address == &CPU::REL_B;

        clockInstruction();

        if (!wasIseq) {
            if (wasBranch) {
                // branch-specific hardware polling points
                if (exeCycle == 2 || exeCycle == 4) {
                    latchInterrupt(pendingInterruptSource, candidate);
                }
            } else if (cycles == 0) {
                latchInterrupt(pendingInterruptSource, candidate);
            }
        }
    }
    // sampleNmiLine();
    // increment total cycles
    totalCycles++;
}

void CPU::clockInstruction() {
    if (halted) return;
    /// We initialize `cycles` to `0`, but only start operations when it is `1`; so our logic requires pre-incrementing.
    cycles++;
    if (cycles == 1) {
        // reset helper variables and process delayed I-flag
        newInstruction();

        // carry mid-instruction poll results forward
        interruptSource = pendingInterruptSource;
        pendingInterruptSource = INTERRUPT::NONE;

        // emit acknowledgement events only after interrupt is actually selected for this instruction
        if (eventSink) {
            if (interruptSource == INTERRUPT::NMI) {
                eventSink->OnInterrupt(INTERRUPT_EVENT::NMI_ACK);
            } else if (interruptSource == INTERRUPT::IRQ) {
                eventSink->OnInterrupt(INTERRUPT_EVENT::IRQ_ACK);
            }
        }

        // save current pc for use in event emit(s)
        u16 instPC = pc.value();

        // On cycle `1`, we either trigger an interrupt/reset, or read the next opcode to prepare for the next instruction.
        if (interruptSource != INTERRUPT::NONE) {
            // interrupts force BRK into opcode slot
            opcode = 0x00;
        } else {
            // otherwise, read next opcode and set next instruction as necessary
            prevInstAddrs.push_back(pc);
            if (prevInstAddrs.size() > 13) prevInstAddrs.pop_front();
            opcode = read(pc++);
            if (opcode == 0x00) interruptSource = INTERRUPT::BRK;
        }

        // emit instruction event
        if (eventSink) eventSink->OnInstructionExecute(instPC, opcode, a, x, y, sp, status, totalCycles);

        // set current instruction based on opcode value
        currInst = &lookup[opcode];
    } else {
        if (currInst->address != nullptr) // if this instruction requires addressing mode logic, then perform that function
            (this->*currInst->address)();
        else // otherwise, simply perform the operation function, as it will handle the cycle logic itself
            (this->*currInst->operate)();
    }
}

INTERRUPT CPU::pollInterrupts() {
    if (resetPending)
        return INTERRUPT::RST;
    else if (nmiPending)
        return INTERRUPT::NMI; // acknowledge NMI
    else if (hasPendingIrq() && !interruptFlagViaPoll)
        return INTERRUPT::IRQ; // acknowlege IRQ

    return INTERRUPT::NONE;
}

const CPU_STATE CPU::GetState() const {
    return {
        pc.value(), a, x, y, sp, status, totalCycles, hasPendingIrq(), nmiPending
    };
}

void CPU::requestDmcDma(u16 addr, bool load) {
    if (dmcPending || dmcActive) return;

    dmcPending = true;
    dmcHaltRetry = false;
    dmcLoad = load;
    dmcAddr = addr;
}

void CPU::stopDmcDma() {
    if (dmcPending) {
        // The DMA has been scheduled but has not halted the CPU.
        // Cancelling here also prevents a failed halt from retrying.
        dmcPending = false;
        dmcHaltRetry = false;
        dmcLoad = false;
        return;
    }

    if (dmcActive && dmcPhase == DMC_PHASE::DUMMY) {
        // The halt cycle already occurred. The arbiter will terminate
        // the DMA before its dummy/alignment/read cycles.
        dmcAbort = true;
    }
}

void CPU::sampleNmiLine(bool line) {
    if (line && !nmiLineSampled) {
        nmiPending = true;
    }

    nmiLineSampled = line;
}

void CPU::newInstruction() {
    if (IFVP.pending) {
        interruptFlagViaPoll = IFVP.value;
        IFVP.pending = false;
    }

    absAddr = relAddr = indAddr = offset = fetched = 0;
    paged = branch = false;
}

void CPU::syncIFVP() {
    IFVP.pending = false;
    interruptFlagViaPoll = getFlag(FLAGS::I) != 0;
}

bool CPU::nextCycleWrites() const {
    if (cycles == 0 || currInst == nullptr) return false;

    const u8 next = cycles + 1;
    const auto mode = currInst->address;

    if (mode == &CPU::ABS_W) return next == 4;
    if (mode == &CPU::ABS_M) return (next == 5 || next == 6);

    if (mode == &CPU::ABX_W || mode == &CPU::ABY_W) return next == 5;

    if (mode == &CPU::ABX_M || mode == &CPU::ABY_M) return (next == 6 || next == 7);

    if (mode == &CPU::ZP0_W) return next == 3;
    if (mode == &CPU::ZP0_M) return (next == 4 || next == 5);

    if (mode == &CPU::ZPX_W || mode == &CPU::ZPY_W) return next == 4;

    if (mode == &CPU::ZPX_M || mode == &CPU::ZPY_M) return (next == 5 || next == 6);

    if (mode == &CPU::IZX_W || mode == &CPU::IZY_W) return next == 6;

    if (mode == &CPU::IZX_M || mode == &CPU::IZY_M) return (next == 7 || next == 8);

    const auto oper = currInst->operate;

    if (oper == &CPU::JSR) return (next == 4 || next == 5);

    if (oper == &CPU::BRK) {
        return interruptSource != INTERRUPT::RST && next >= 3 && next <= 5;
    }

    return (oper == &CPU::PHA || oper == &CPU::PHP) && next == 3;
}

bool CPU::beginDeferredRead(u16 addr) {
    #ifndef TEST_SST
    if (addr >= 0x2000 && addr <= 0x3FFF && (addr & 0x0007) == 0x0002) {
        lastReadAddr = addrBus = addr;

        deferredStatusRead = true;
        deferredStatusOperate = currInst->operate;

        ppu.lock()->beginStatusRead();

        cycles = 0;

        return true;
    }
    #endif

    return false;

}

void CPU::completeDeferredRead() {
    if (!deferredStatusRead) return;

    cpuBus = fetched = ppu.lock()->finishStatusRead();
    
    auto operate = deferredStatusOperate;

    deferredStatusRead = false;
    deferredStatusOperate = nullptr;

    (this->*operate)();
}