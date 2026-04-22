#include "../../../core/Helpers.h"
#include "NES_CPU.h"

#include <fstream>
#include <vector>

NES_CPU::NES_CPU(bool d) {
    debug = d;
    lookup = {
        //0x00                                  0x01                                    0x02                                    0x03                                    0x04                                    0x05                                    0x06                                    0x07                                    0x08                                    0x09                                    0x0A                                    0x0B                                    0x0C                                    0x0D                                    0x0E                                    0x0F
        {"BRK",J,nullptr,      &NES_CPU::BRK,2},{"ORA",R,&NES_CPU::IZX,&NES_CPU::ORA,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"SLO",M,&NES_CPU::IZX,&NES_CPU::SLO,2},{"NOP",R,&NES_CPU::ZP0,&NES_CPU::NOP,2},{"ORA",R,&NES_CPU::ZP0,&NES_CPU::ORA,2},{"ASL",M,&NES_CPU::ZP0,&NES_CPU::ASL,2},{"SLO",M,&NES_CPU::ZP0,&NES_CPU::SLO,2},{"PHP",X,nullptr,      &NES_CPU::PHP,1},{"ORA",X,&NES_CPU::IMM,&NES_CPU::ORA,2},{"ASL",X,&NES_CPU::ACC,&NES_CPU::ASL,1},{"ANC",X,&NES_CPU::IMM,&NES_CPU::ANC,2},{"NOP",R,&NES_CPU::ABS,&NES_CPU::NOP,3},{"ORA",R,&NES_CPU::ABS,&NES_CPU::ORA,3},{"ASL",M,&NES_CPU::ABS,&NES_CPU::ASL,3},{"SLO",M,&NES_CPU::ABS,&NES_CPU::SLO,3},
        //0x10                                  0x11                                    0x12                                    0x13                                    0x14                                    0x15                                    0x16                                    0x17                                    0x18                                    0x19                                    0x1A                                    0x1B                                    0x1C                                    0x1D                                    0x1E                                    0x1F
        {"BPL",J,&NES_CPU::REL,&NES_CPU::BPL,2},{"ORA",R,&NES_CPU::IZY,&NES_CPU::ORA,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"SLO",M,&NES_CPU::IZY,&NES_CPU::SLO,2},{"NOP",R,&NES_CPU::ZPX,&NES_CPU::NOP,2},{"ORA",R,&NES_CPU::ZPX,&NES_CPU::ORA,2},{"ASL",M,&NES_CPU::ZPX,&NES_CPU::ASL,2},{"SLO",M,&NES_CPU::ZPX,&NES_CPU::SLO,2},{"CLC",X,&NES_CPU::IMP,&NES_CPU::CLC,1},{"ORA",R,&NES_CPU::ABY,&NES_CPU::ORA,3},{"NOP",X,&NES_CPU::IMP,&NES_CPU::NOP,1},{"SLO",M,&NES_CPU::ABY,&NES_CPU::SLO,3},{"NOP",R,&NES_CPU::ABX,&NES_CPU::NOP,3},{"ORA",R,&NES_CPU::ABX,&NES_CPU::ORA,3},{"ASL",M,&NES_CPU::ABX,&NES_CPU::ASL,3},{"SLO",M,&NES_CPU::ABX,&NES_CPU::SLO,3},
        //0x20                                  0x21                                    0x22                                    0x23                                    0x24                                    0x25                                    0x26                                    0x27                                    0x28                                    0x19                                    0x2A                                    0x2B                                    0x2C                                    0x2D                                    0x2E                                    0x2F
        {"JSR",J,nullptr,      &NES_CPU::JSR,3},{"AND",R,&NES_CPU::IZX,&NES_CPU::AND,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"RLA",M,&NES_CPU::IZX,&NES_CPU::RLA,2},{"BIT",R,&NES_CPU::ZP0,&NES_CPU::BIT,2},{"AND",R,&NES_CPU::ZP0,&NES_CPU::AND,2},{"ROL",M,&NES_CPU::ZP0,&NES_CPU::ROL,2},{"RLA",M,&NES_CPU::ZP0,&NES_CPU::RLA,2},{"PLP",X,nullptr,      &NES_CPU::PLP,1},{"AND",X,&NES_CPU::IMM,&NES_CPU::AND,2},{"ROL",X,&NES_CPU::ACC,&NES_CPU::ROL,1},{"ANC",X,&NES_CPU::IMM,&NES_CPU::ANC,2},{"BIT",R,&NES_CPU::ABS,&NES_CPU::BIT,3},{"AND",R,&NES_CPU::ABS,&NES_CPU::AND,3},{"ROL",M,&NES_CPU::ABS,&NES_CPU::ROL,3},{"RLA",M,&NES_CPU::ABS,&NES_CPU::RLA,3},
        //0x30                                  0x31                                    0x32                                    0x33                                    0x34                                    0x35                                    0x36                                    0x37                                    0x38                                    0x39                                    0x3A                                    0x3B                                    0x3C                                    0x3D                                    0x3E                                    0x3F
        {"BMI",J,&NES_CPU::REL,&NES_CPU::BMI,2},{"AND",R,&NES_CPU::IZY,&NES_CPU::AND,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"RLA",M,&NES_CPU::IZY,&NES_CPU::RLA,2},{"NOP",R,&NES_CPU::ZPX,&NES_CPU::NOP,2},{"AND",R,&NES_CPU::ZPX,&NES_CPU::AND,2},{"ROL",M,&NES_CPU::ZPX,&NES_CPU::ROL,2},{"RLA",M,&NES_CPU::ZPX,&NES_CPU::RLA,2},{"SEC",X,&NES_CPU::IMP,&NES_CPU::SEC,1},{"AND",R,&NES_CPU::ABY,&NES_CPU::AND,3},{"NOP",X,&NES_CPU::IMP,&NES_CPU::NOP,1},{"RLA",M,&NES_CPU::ABY,&NES_CPU::RLA,3},{"NOP",R,&NES_CPU::ABX,&NES_CPU::NOP,3},{"AND",R,&NES_CPU::ABX,&NES_CPU::AND,3},{"ROL",M,&NES_CPU::ABX,&NES_CPU::ROL,3},{"RLA",M,&NES_CPU::ABX,&NES_CPU::RLA,3},
        //0x40                                  0x41                                    0x42                                    0x43                                    0x44                                    0x45                                    0x46                                    0x47                                    0x48                                    0x49                                    0x4A                                    0x4B                                    0x4C                                    0x4D                                    0x4E                                    0x4F
        {"RTI",J,nullptr,      &NES_CPU::RTI,1},{"EOR",R,&NES_CPU::IZX,&NES_CPU::EOR,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"SRE",M,&NES_CPU::IZX,&NES_CPU::SRE,2},{"NOP",R,&NES_CPU::ZP0,&NES_CPU::NOP,2},{"EOR",R,&NES_CPU::ZP0,&NES_CPU::EOR,2},{"LSR",M,&NES_CPU::ZP0,&NES_CPU::LSR,2},{"SRE",M,&NES_CPU::ZP0,&NES_CPU::SRE,2},{"PHA",X,nullptr,      &NES_CPU::PHA,1},{"EOR",X,&NES_CPU::IMM,&NES_CPU::EOR,2},{"LSR",X,&NES_CPU::ACC,&NES_CPU::LSR,1},{"ASR",X,&NES_CPU::IMM,&NES_CPU::ASR,2},{"JMP",J,&NES_CPU::ABS,&NES_CPU::JMP,3},{"EOR",R,&NES_CPU::ABS,&NES_CPU::EOR,3},{"LSR",M,&NES_CPU::ABS,&NES_CPU::LSR,3},{"SRE",M,&NES_CPU::ABS,&NES_CPU::SRE,3},
        //0x50                                  0x51                                    0x52                                    0x53                                    0x54                                    0x55                                    0x56                                    0x57                                    0x58                                    0x59                                    0x5A                                    0x5B                                    0x5C                                    0x5D                                    0x5E                                    0x5F
        {"BVC",J,&NES_CPU::REL,&NES_CPU::BVC,2},{"EOR",R,&NES_CPU::IZY,&NES_CPU::EOR,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"SRE",M,&NES_CPU::IZY,&NES_CPU::SRE,2},{"NOP",R,&NES_CPU::ZPX,&NES_CPU::NOP,2},{"EOR",R,&NES_CPU::ZPX,&NES_CPU::EOR,2},{"LSR",M,&NES_CPU::ZPX,&NES_CPU::LSR,2},{"SRE",M,&NES_CPU::ZPX,&NES_CPU::SRE,2},{"CLI",X,&NES_CPU::IMP,&NES_CPU::CLI,1},{"EOR",R,&NES_CPU::ABY,&NES_CPU::EOR,3},{"NOP",X,&NES_CPU::IMP,&NES_CPU::NOP,1},{"SRE",M,&NES_CPU::ABY,&NES_CPU::SRE,3},{"NOP",R,&NES_CPU::ABX,&NES_CPU::NOP,3},{"EOR",R,&NES_CPU::ABX,&NES_CPU::EOR,3},{"LSR",M,&NES_CPU::ABX,&NES_CPU::LSR,3},{"SRE",M,&NES_CPU::ABX,&NES_CPU::SRE,3},
        //0x60                                  0x61                                    0x62                                    0x63                                    0x64                                    0x65                                    0x66                                    0x67                                    0x68                                    0x69                                    0x6A                                    0x6B                                    0x6C                                    0x6D                                    0x6E                                    0x6F
        {"RTS",J,nullptr,      &NES_CPU::RTS,1},{"ADC",R,&NES_CPU::IZX,&NES_CPU::ADC,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"RRA",M,&NES_CPU::IZX,&NES_CPU::RRA,2},{"NOP",R,&NES_CPU::ZP0,&NES_CPU::NOP,2},{"ADC",R,&NES_CPU::ZP0,&NES_CPU::ADC,2},{"ROR",M,&NES_CPU::ZP0,&NES_CPU::ROR,2},{"RRA",M,&NES_CPU::ZP0,&NES_CPU::RRA,2},{"PLA",X,nullptr,      &NES_CPU::PLA,1},{"ADC",X,&NES_CPU::IMM,&NES_CPU::ADC,2},{"ROR",X,&NES_CPU::ACC,&NES_CPU::ROR,1},{"ARR",X,&NES_CPU::IMM,&NES_CPU::ARR,2},{"JMP",J,&NES_CPU::IND,&NES_CPU::JMP,3},{"ADC",R,&NES_CPU::ABS,&NES_CPU::ADC,3},{"ROR",M,&NES_CPU::ABS,&NES_CPU::ROR,3},{"RRA",M,&NES_CPU::ABS,&NES_CPU::RRA,3},
        //0x70                                  0x71                                    0x72                                    0x73                                    0x74                                    0x75                                    0x76                                    0x77                                    0x78                                    0x79                                    0x7A                                    0x7B                                    0x7C                                    0x7D                                    0x7E                                    0x7F
        {"BVS",J,&NES_CPU::REL,&NES_CPU::BVS,2},{"ADC",R,&NES_CPU::IZY,&NES_CPU::ADC,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"RRA",M,&NES_CPU::IZY,&NES_CPU::RRA,2},{"NOP",R,&NES_CPU::ZPX,&NES_CPU::NOP,2},{"ADC",R,&NES_CPU::ZPX,&NES_CPU::ADC,2},{"ROR",M,&NES_CPU::ZPX,&NES_CPU::ROR,2},{"RRA",M,&NES_CPU::ZPX,&NES_CPU::RRA,2},{"SEI",X,&NES_CPU::IMP,&NES_CPU::SEI,1},{"ADC",R,&NES_CPU::ABY,&NES_CPU::ADC,3},{"NOP",X,&NES_CPU::IMP,&NES_CPU::NOP,1},{"RRA",M,&NES_CPU::ABY,&NES_CPU::RRA,3},{"NOP",R,&NES_CPU::ABX,&NES_CPU::NOP,3},{"ADC",R,&NES_CPU::ABX,&NES_CPU::ADC,3},{"ROR",M,&NES_CPU::ABX,&NES_CPU::ROR,3},{"RRA",M,&NES_CPU::ABX,&NES_CPU::RRA,3},
        //0x80                                  0x81                                    0x82                                    0x83                                    0x84                                    0x85                                    0x86                                    0x87                                    0x88                                    0x89                                    0x8A                                    0x8B                                    0x8C                                    0x8D                                    0x8E                                    0x8F
        {"NOP",X,&NES_CPU::IMM,&NES_CPU::NOP,2},{"STA",W,&NES_CPU::IZX,&NES_CPU::STA,2},{"NOP",X,&NES_CPU::IMM,&NES_CPU::NOP,2},{"SAX",W,&NES_CPU::IZX,&NES_CPU::SAX,2},{"STY",W,&NES_CPU::ZP0,&NES_CPU::STY,2},{"STA",W,&NES_CPU::ZP0,&NES_CPU::STA,2},{"STX",W,&NES_CPU::ZP0,&NES_CPU::STX,2},{"SAX",W,&NES_CPU::ZP0,&NES_CPU::SAX,2},{"DEY",X,&NES_CPU::IMP,&NES_CPU::DEY,1},{"NOP",X,&NES_CPU::IMM,&NES_CPU::NOP,2},{"TXA",X,&NES_CPU::IMP,&NES_CPU::TXA,1},{"ANE",X,&NES_CPU::IMM,&NES_CPU::ANE,2},{"STY",W,&NES_CPU::ABS,&NES_CPU::STY,3},{"STA",W,&NES_CPU::ABS,&NES_CPU::STA,3},{"STX",W,&NES_CPU::ABS,&NES_CPU::STX,3},{"SAX",W,&NES_CPU::ABS,&NES_CPU::SAX,3},
        //0x90                                  0x91                                    0x92                                    0x93                                    0x94                                    0x95                                    0x96                                    0x97                                    0x98                                    0x99                                    0x9A                                    0x9B                                    0x9C                                    0x9D                                    0x9E                                    0x9F
        {"BCC",J,&NES_CPU::REL,&NES_CPU::BCC,2},{"STA",W,&NES_CPU::IZY,&NES_CPU::STA,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"SHA",W,&NES_CPU::IZY,&NES_CPU::SHA,2},{"STY",W,&NES_CPU::ZPX,&NES_CPU::STY,2},{"STA",W,&NES_CPU::ZPX,&NES_CPU::STA,2},{"STX",W,&NES_CPU::ZPY,&NES_CPU::STX,2},{"SAX",W,&NES_CPU::ZPY,&NES_CPU::SAX,2},{"TYA",X,&NES_CPU::IMP,&NES_CPU::TYA,1},{"STA",W,&NES_CPU::ABY,&NES_CPU::STA,3},{"TXS",X,&NES_CPU::IMP,&NES_CPU::TXS,1},{"SHS",W,&NES_CPU::ABY,&NES_CPU::SHS,3},{"SHY",W,&NES_CPU::ABX,&NES_CPU::SHY,3},{"STA",W,&NES_CPU::ABX,&NES_CPU::STA,3},{"SHX",W,&NES_CPU::ABY,&NES_CPU::SHX,3},{"SHA",W,&NES_CPU::ABY,&NES_CPU::SHA,3},
        //0xA0                                  0xA1                                    0xA2                                    0xA3                                    0xA4                                    0xA5                                    0xA6                                    0xA7                                    0xA8                                    0xA9                                    0xAA                                    0xAB                                    0xAC                                    0xAD                                    0xAE                                    0xAF
        {"LDY",X,&NES_CPU::IMM,&NES_CPU::LDY,2},{"LDA",R,&NES_CPU::IZX,&NES_CPU::LDA,2},{"LDX",X,&NES_CPU::IMM,&NES_CPU::LDX,2},{"LAX",R,&NES_CPU::IZX,&NES_CPU::LAX,2},{"LDY",R,&NES_CPU::ZP0,&NES_CPU::LDY,2},{"LDA",R,&NES_CPU::ZP0,&NES_CPU::LDA,2},{"LDX",R,&NES_CPU::ZP0,&NES_CPU::LDX,2},{"LAX",R,&NES_CPU::ZP0,&NES_CPU::LAX,2},{"TAY",X,&NES_CPU::IMP,&NES_CPU::TAY,1},{"LDA",X,&NES_CPU::IMM,&NES_CPU::LDA,2},{"TAX",X,&NES_CPU::IMP,&NES_CPU::TAX,1},{"LXA",X,&NES_CPU::IMM,&NES_CPU::LXA,2},{"LDY",R,&NES_CPU::ABS,&NES_CPU::LDY,3},{"LDA",R,&NES_CPU::ABS,&NES_CPU::LDA,3},{"LDX",R,&NES_CPU::ABS,&NES_CPU::LDX,3},{"LAX",R,&NES_CPU::ABS,&NES_CPU::LAX,3},
        //0xB0                                  0xB1                                    0xB2                                    0xB3                                    0xB4                                    0xB5                                    0xB6                                    0xB7                                    0xB8                                    0xB9                                    0xBA                                    0xBB                                    0xBC                                    0xBD                                    0xBE                                    0xBF
        {"BCS",J,&NES_CPU::REL,&NES_CPU::BCS,2},{"LDA",R,&NES_CPU::IZY,&NES_CPU::LDA,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"LAX",R,&NES_CPU::IZY,&NES_CPU::LAX,2},{"LDY",R,&NES_CPU::ZPX,&NES_CPU::LDY,2},{"LDA",R,&NES_CPU::ZPX,&NES_CPU::LDA,2},{"LDX",R,&NES_CPU::ZPY,&NES_CPU::LDX,2},{"LAX",R,&NES_CPU::ZPY,&NES_CPU::LAX,2},{"CLV",X,&NES_CPU::IMP,&NES_CPU::CLV,1},{"LDA",R,&NES_CPU::ABY,&NES_CPU::LDA,3},{"TSX",X,&NES_CPU::IMP,&NES_CPU::TSX,1},{"LAS",R,&NES_CPU::ABY,&NES_CPU::LAS,3},{"LDY",R,&NES_CPU::ABX,&NES_CPU::LDY,3},{"LDA",R,&NES_CPU::ABX,&NES_CPU::LDA,3},{"LDX",R,&NES_CPU::ABY,&NES_CPU::LDX,3},{"LAX",R,&NES_CPU::ABY,&NES_CPU::LAX,3},
        //0xC0                                  0xC1                                    0xC2                                    0xC3                                    0xC4                                    0xC5                                    0xC6                                    0xC7                                    0xC8                                    0xC9                                    0xCA                                    0xCB                                    0xCC                                    0xCD                                    0xCE                                    0xCF
        {"CPY",X,&NES_CPU::IMM,&NES_CPU::CPY,2},{"CMP",R,&NES_CPU::IZX,&NES_CPU::CMP,2},{"NOP",X,&NES_CPU::IMM,&NES_CPU::NOP,2},{"DCP",M,&NES_CPU::IZX,&NES_CPU::DCP,2},{"CPY",R,&NES_CPU::ZP0,&NES_CPU::CPY,2},{"CMP",R,&NES_CPU::ZP0,&NES_CPU::CMP,2},{"DEC",M,&NES_CPU::ZP0,&NES_CPU::DEC,2},{"DCP",M,&NES_CPU::ZP0,&NES_CPU::DCP,2},{"INY",X,&NES_CPU::IMP,&NES_CPU::INY,1},{"CMP",X,&NES_CPU::IMM,&NES_CPU::CMP,2},{"DEX",X,&NES_CPU::IMP,&NES_CPU::DEX,1},{"AXS",X,&NES_CPU::IMM,&NES_CPU::AXS,2},{"CPY",R,&NES_CPU::ABS,&NES_CPU::CPY,3},{"CMP",R,&NES_CPU::ABS,&NES_CPU::CMP,3},{"DEC",M,&NES_CPU::ABS,&NES_CPU::DEC,3},{"DCP",M,&NES_CPU::ABS,&NES_CPU::DCP,3},
        //0xD0                                  0xD1                                    0xD2                                    0xD3                                    0xD4                                    0xD5                                    0xD6                                    0xD7                                    0xD8                                    0xD9                                    0xDA                                    0xDB                                    0xDC                                    0xDD                                    0xDE                                    0xDF
        {"BNE",J,&NES_CPU::REL,&NES_CPU::BNE,2},{"CMP",R,&NES_CPU::IZY,&NES_CPU::CMP,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"DCP",M,&NES_CPU::IZY,&NES_CPU::DCP,2},{"NOP",R,&NES_CPU::ZPX,&NES_CPU::NOP,2},{"CMP",R,&NES_CPU::ZPX,&NES_CPU::CMP,2},{"DEC",M,&NES_CPU::ZPX,&NES_CPU::DEC,2},{"DCP",M,&NES_CPU::ZPX,&NES_CPU::DCP,2},{"CLD",X,&NES_CPU::IMP,&NES_CPU::CLD,1},{"CMP",R,&NES_CPU::ABY,&NES_CPU::CMP,3},{"NOP",X,&NES_CPU::IMP,&NES_CPU::NOP,1},{"DCP",M,&NES_CPU::ABY,&NES_CPU::DCP,3},{"NOP",R,&NES_CPU::ABX,&NES_CPU::NOP,3},{"CMP",R,&NES_CPU::ABX,&NES_CPU::CMP,3},{"DEC",M,&NES_CPU::ABX,&NES_CPU::DEC,3},{"DCP",M,&NES_CPU::ABX,&NES_CPU::DCP,3},
        //0xE0                                  0xE1                                    0xE2                                    0xE3                                    0xE4                                    0xE5                                    0xE6                                    0xE7                                    0xE8                                    0xE9                                    0xEA                                    0xEB                                    0xEC                                    0xED                                    0xEE                                    0xEF
        {"CPX",X,&NES_CPU::IMM,&NES_CPU::CPX,2},{"SBC",R,&NES_CPU::IZX,&NES_CPU::SBC,2},{"NOP",X,&NES_CPU::IMM,&NES_CPU::NOP,2},{"ISC",M,&NES_CPU::IZX,&NES_CPU::ISC,2},{"CPX",R,&NES_CPU::ZP0,&NES_CPU::CPX,2},{"SBC",R,&NES_CPU::ZP0,&NES_CPU::SBC,2},{"INC",M,&NES_CPU::ZP0,&NES_CPU::INC,2},{"ISC",M,&NES_CPU::ZP0,&NES_CPU::ISC,2},{"INX",X,&NES_CPU::IMP,&NES_CPU::INX,1},{"SBC",X,&NES_CPU::IMM,&NES_CPU::SBC,2},{"NOP",X,&NES_CPU::IMP,&NES_CPU::NOP,1},{"SBC",X,&NES_CPU::IMM,&NES_CPU::SBC,2},{"CPX",R,&NES_CPU::ABS,&NES_CPU::CPX,3},{"SBC",R,&NES_CPU::ABS,&NES_CPU::SBC,3},{"INC",M,&NES_CPU::ABS,&NES_CPU::INC,3},{"ISC",M,&NES_CPU::ABS,&NES_CPU::ISC,3},
        //0xF0                                  0xF1                                    0xF2                                    0xF3                                    0xF4                                    0xF5                                    0xF6                                    0xF7                                    0xF8                                    0xF9                                    0xFA                                    0xFB                                    0xFC                                    0xFD                                    0xFE                                    0xFF
        {"BEQ",J,&NES_CPU::REL,&NES_CPU::BEQ,2},{"SBC",R,&NES_CPU::IZY,&NES_CPU::SBC,2},{"JAM",X,&NES_CPU::IMP,&NES_CPU::JAM,1},{"ISC",M,&NES_CPU::IZY,&NES_CPU::ISC,2},{"NOP",R,&NES_CPU::ZPX,&NES_CPU::NOP,2},{"SBC",R,&NES_CPU::ZPX,&NES_CPU::SBC,2},{"INC",M,&NES_CPU::ZPX,&NES_CPU::INC,2},{"ISC",M,&NES_CPU::ZPX,&NES_CPU::ISC,2},{"SED",X,&NES_CPU::IMP,&NES_CPU::SED,1},{"SBC",R,&NES_CPU::ABY,&NES_CPU::SBC,3},{"NOP",X,&NES_CPU::IMP,&NES_CPU::NOP,1},{"ISC",M,&NES_CPU::ABY,&NES_CPU::ISC,3},{"NOP",R,&NES_CPU::ABX,&NES_CPU::NOP,3},{"SBC",R,&NES_CPU::ABX,&NES_CPU::SBC,3},{"INC",M,&NES_CPU::ABX,&NES_CPU::INC,3},{"ISC",M,&NES_CPU::ABX,&NES_CPU::ISC,3}
    };
    if (debug) {
        traceStream = new std::ofstream("C:/devenv/PROTEUS/trace.log");
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
    cycles++;
    if (cycles == 1) {
        if (updateStatus) {
            status = dStatus;
            updateStatus = false;
        }
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
                printf(t.c_str());
                if (traceStream != nullptr && traceStream->is_open())
                    *traceStream << t.c_str();
            }
            opcode = read(pc++);
            currInst = &lookup[opcode];
        }
    } else {
        if (currInst->address != nullptr)
            (this->*currInst->address)();
        else
            (this->*currInst->operate)();
    }
    totalCycles++;
}

void NES_CPU::pollInterrupts() {
    if (nmiTrigger) {
        doNMI = true;
        nmiTrigger = false;
        return;
    }

    if (!doIRQ && irqTrigger) {
        if (!delayIRQ) {
            doIRQ = true;
            irqTrigger = false;
        } else {
            delayIRQ = false;
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
    ss << "P:" << hex(status, 2) << " ";
    //ss << (getFlag(N) ? "N" : "n");
    //ss << (getFlag(V) ? "V" : "v");
    //ss << (getFlag(U) ? "U" : "u");
    //ss << (getFlag(B) ? "B" : "b");
    //ss << (getFlag(D) ? "D" : "d");
    //ss << (getFlag(I) ? "I" : "i");
    //ss << (getFlag(Z) ? "Z" : "z");
    //ss << (getFlag(C) ? "C" : "c") << " ";
    ss << "SP:" << hex(sp, 2) << " ";
    //ss << traceStack() << " }";
    ss << bus->getPPUstatus() << " ";
    ss << "CYC:" << totalCycles;

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

std::string NES_CPU::traceInterrupt(INTERRUPT i) {
    uint16_t PC = pc;

    const INST& inst = (i == RST_INTERRUPT ? NES_CPU::RST_INST : (i == NMI_INTERRUPT ? NES_CPU::NMI_INST : NES_CPU::IRQ_INST));

    std::stringstream ss;

    ss << hex(PC, 4) << "  ";

    ss << "INTERRUPT ";

    ss << inst.name << " ";

    ADDR v;

    switch (i) {
        case RST_INTERRUPT:
            v.lo = read(0xFFFC, true);
            v.hi = read(0xFFFD, true);
            break;
        case NMI_INTERRUPT:
            v.lo = read(0xFFFA, true);
            v.hi = read(0xFFFB, true);
            break;
        case IRQ_INTERRUPT:
            v.lo = read(0xFFFE, true);
            v.hi = read(0xFFFF, true);
            break;
    }

    ss << "$" << hex(v.value(), 4);

    while (ss.str().size() < 48) ss << " ";

    ss << formatInst();

    ss << std::endl;

    return ss.str();
}

#pragma region Interrupts
void NES_CPU::IRQ() {
    switch (cycles) {
        case 2:
            read(pc.value());
            //if (nmiTrigger) { currInst = &NMI_INST; }
            break;
        case 3:
            write(0x0100 + sp, pc.hi);
            sp--;
            //if (nmiTrigger) { currInst = &NMI_INST; }
            break;
        case 4:
            write(0x0100 + sp, pc.lo);
            sp--;
            //if (nmiTrigger) { currInst = &NMI_INST; }
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
            pc.lo = read(0xFFFE);
            break;
        case 7:
            pc.hi = read(0xFFFF);
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
            cycles = 0;
            break;
    }
}
#pragma endregion

#pragma region Addressing Modes
void NES_CPU::ABS() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            if (currInst->type == J)
                pollInterrupts();
            break;
        case 3:
            absAddr.hi = read(pc++);
            if (currInst->type == J) {
                (this->*currInst->operate)();
                cycles = 0;
            } else if (currInst->type != M)
                pollInterrupts();
            break;
        case 4:
            if (currInst->type == W) {
                (this->*currInst->operate)();
                write(absAddr.value(), fetched);
                cycles = 0;
            } else {
                fetched = read(absAddr.value());
                if (currInst->type == R) {
                    (this->*currInst->operate)();
                    cycles = 0;
                }
            }
            break;
        case 5:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            pollInterrupts();
            break;
        case 6:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void NES_CPU::ABX() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            page(x);
            if (!paged && currInst->type == R)
                pollInterrupts();
            break;
        case 4:
            if (paged || currInst->type != R) {
                read(absAddr.value());
                if (paged) absAddr.hi++;
                if (currInst->type == W) pollInterrupts();
                break;
            }
            cycles++;
            [[fallthrough]];
        case 5:
            if (currInst->type == W) {
                (this->*currInst->operate)();
                write(absAddr.value(), fetched);
                cycles = 0;
            } else {
                fetched = read(absAddr.value());
                if (currInst->type == R) {
                    (this->*currInst->operate)();
                    cycles = 0;
                }
            }
            break;
        case 6:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            pollInterrupts();
            break;
        case 7:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void NES_CPU::ABY() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            page(y);
            if (!paged && currInst->type == R)
                pollInterrupts();
            break;
        case 4:
            if (paged || currInst->type != R) {
                read(absAddr.value());
                if (paged) absAddr.hi++;
                if (currInst->type == W) pollInterrupts();
                break;
            }
            cycles++;
            [[fallthrough]];
        case 5:
            if (currInst->type == W) {
                (this->*currInst->operate)();
                write(absAddr.value(), fetched);
                cycles = 0;
            } else {
                fetched = read(absAddr.value());
                if (currInst->type == R) {
                    (this->*currInst->operate)();
                    cycles = 0;
                }
            }
            break;
        case 6:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            pollInterrupts();
            break;
        case 7:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void NES_CPU::ZP0() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            if (currInst->type != M)
                pollInterrupts();
            break;
        case 3:
            if (currInst->type == W) {
                (this->*currInst->operate)();
                write(offset, fetched);
                cycles = 0;
            } else {
                fetched = read(offset);
                if (currInst->type == R) {
                    (this->*currInst->operate)();
                    cycles = 0;
                }
            }
            break;
        case 4:
            write(offset, fetched);
            (this->*currInst->operate)();
            pollInterrupts();
            break;
        case 5:
            write(offset, fetched);
            cycles = 0;
            break;
    }
}
void NES_CPU::ZPX() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            break;
        case 3:
            read(offset);
            offset += x;
            if (currInst->type != M)
                pollInterrupts();
            break;
        case 4:
            if (currInst->type == W) {
                (this->*currInst->operate)();
                write(offset, fetched);
                cycles = 0;
            } else {
                fetched = read(offset);
                if (currInst->type == R) {
                    (this->*currInst->operate)();
                    cycles = 0;
                }
            }
            break;
        case 5:
            write(offset, fetched);
            (this->*currInst->operate)();
            pollInterrupts();
            break;
        case 6:
            write(offset, fetched);
            cycles = 0;
            break;
    }
}
void NES_CPU::ZPY() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            break;
        case 3:
            read(offset);
            offset += y;
            if (currInst->type != M)
                pollInterrupts();
            break;
        case 4:
            if (currInst->type == W) {
                (this->*currInst->operate)();
                write(offset, fetched);
                cycles = 0;
            } else {
                fetched = read(offset);
                if (currInst->type == R) {
                    (this->*currInst->operate)();
                    cycles = 0;
                }
            }
            break;
        case 5:
            write(offset, fetched);
            (this->*currInst->operate)();
            pollInterrupts();
            break;
        case 6:
            write(offset, fetched);
            cycles = 0;
            break;
    }
}
void NES_CPU::ACC() {
    fetched = a;
    (this->*currInst->operate)();
    a = fetched;
    pollInterrupts();
    cycles = 0;
}
void NES_CPU::IMM() {
    fetched = read(pc++);
    (this->*currInst->operate)();
    pollInterrupts();
    cycles = 0;
}
void NES_CPU::IMP() {
    (this->*currInst->operate)();
    pollInterrupts();
    cycles = 0;
}
void NES_CPU::IND() {
    switch (cycles) {
        case 2:
            indAddr.lo = read(pc++);
            break;
        case 3:
            indAddr.hi = read(pc++);
            break;
        case 4:
            absAddr.lo = read(indAddr.value());
            indAddr.lo++;
            pollInterrupts();
            break;
        case 5:
            absAddr.hi = read(indAddr.value());
            (this->*currInst->operate)();
            cycles = 0;
            break;
    }
}
void NES_CPU::IZX() {
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
            if (currInst->type != M)
                pollInterrupts();
            break;
        case 6:
            if (currInst->type == W) {
                (this->*currInst->operate)();
                write(absAddr.value(), fetched);
                cycles = 0;
            } else {
                fetched = read(absAddr.value());
                if (currInst->type == R) {
                    (this->*currInst->operate)();
                    cycles = 0;
                }
            }
            break;
        case 7:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            pollInterrupts();
            break;
        case 8:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void NES_CPU::IZY() {
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
            if (!paged && currInst->type == R) pollInterrupts();
            break;
        case 5:
            fetched = read(absAddr.value());
            if (paged) {
                absAddr.hi++;
                if (currInst->type != M) pollInterrupts();
            } else if (currInst->type == R) {
                (this->*currInst->operate)();
                cycles = 0;
            }
            break;
        case 6:
            if (currInst->type == W) {
                (this->*currInst->operate)();
                write(absAddr.value(), fetched);
                cycles = 0;
            } else {
                fetched = read(absAddr.value());
                if (currInst->type == R) {
                    (this->*currInst->operate)();
                    cycles = 0;
                }
            }
            break;
        case 7:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            pollInterrupts();
            break;
        case 8:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void NES_CPU::REL() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            (this->*currInst->operate)();
            if (!branch) {
                pollInterrupts();
                cycles = 0;
            } else spage(offset);
            break;
        case 3:
            read(pc);
            if (paged) {
                if (((offset >> 7) & 0x01) == 0) { // page forward
                    pc.hi++;
                } else { // page backward
                    pc.hi--;
                }
                break;
            }
            cycles = 0;
            break;
        case 4:
            read(pc);
            pollInterrupts();
            cycles = 0;
            break;
    }
}
#pragma endregion

#pragma region Instructions
#pragma region ACCESS
void NES_CPU::LDA() {
    a = fetched;
    setZN(a);
}
void NES_CPU::STA() {
    fetched = a;
}
void NES_CPU::LDX() {
    x = fetched;
    setZN(x);
}
void NES_CPU::STX() {
    fetched = x;
}
void NES_CPU::LDY() {
    y = fetched;
    setZN(y);
}
void NES_CPU::STY() {
    fetched = y;
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
void NES_CPU::ADC() {
    uint16_t temp = a + fetched + getFlag(C);
    uint8_t res = temp & 0xFF;
    setZN(res);
    setFlag(C, temp > 0xFF);
    setFlag(V, (temp ^ a) & (temp ^ fetched) & 0x80);
    a = res;
}
void NES_CPU::SBC() {
    int16_t temp = a + (~fetched) + getFlag(C);
    uint8_t res = temp & 0xFF;
    setZN(res);
    setFlag(C, !(temp < 0x00));
    setFlag(V, (res ^ a) & (res ^ ~fetched) & 0x80);
    a = res;
}
void NES_CPU::INC() {
    fetched++;
    setZN(fetched);
}
void NES_CPU::DEC() {
    fetched--;
    setZN(fetched);
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
void NES_CPU::ASL() {
    setFlag(C, ((fetched >> 7) & 0x01) > 0);
    fetched <<= 1;
    setZN(fetched);
}
void NES_CPU::LSR() {
    setFlag(C, (fetched & 0x01) > 0);
    fetched >>= 1;
    setZN(fetched);
}
void NES_CPU::ROL() {
    uint8_t temp = (fetched << 1) | getFlag(C);
    setFlag(C, ((fetched >> 7) & 0x01) > 0);
    fetched = temp;
    setZN(fetched);
}
void NES_CPU::ROR() {
    uint8_t temp = (fetched >> 1) | (getFlag(C) << 7);
    setFlag(C, (fetched & 0x01) > 0);
    fetched = temp;
    setZN(fetched);
}
#pragma endregion
#pragma region BITWISE
void NES_CPU::AND() {
    a &= fetched;
    setZN(a);
}
void NES_CPU::ORA() {
    a |= fetched;
    setZN(a);
}
void NES_CPU::EOR() {
    a ^= fetched;
    setZN(a);
}
void NES_CPU::BIT() {
    setFlag(V, ((fetched >> 6) & 0x01) > 0);
    setFlag(N, ((fetched >> 7) & 0x01) > 0);
    fetched &= a;
    setFlag(Z, fetched == 0);
}
#pragma endregion
#pragma region COMPARE
void NES_CPU::CMP() {
    setFlag(C, a >= fetched);
    setFlag(Z, a == fetched);
    uint8_t temp = a - fetched;
    setFlag(N, ((temp >> 7) & 0x01) > 0);
}
void NES_CPU::CPX() {
    setFlag(C, x >= fetched);
    setFlag(Z, x == fetched);
    uint8_t temp = x - fetched;
    setFlag(N, ((temp >> 7) & 0x01) > 0);
}
void NES_CPU::CPY() {
    setFlag(C, y >= fetched);
    setFlag(Z, y == fetched);
    uint8_t temp = y - fetched;
    setFlag(N, ((temp >> 7) & 0x01) > 0);
}
#pragma endregion
#pragma region BRANCH
void NES_CPU::BCC() {
    branch = getFlag(C) == 0;
}
void NES_CPU::BCS() {
    branch = getFlag(C) == 1;
}
void NES_CPU::BEQ() {
    branch = getFlag(Z) == 1;
}
void NES_CPU::BNE() {
    branch = getFlag(Z) == 0;
}
void NES_CPU::BPL() {
    branch = getFlag(N) == 0;
}
void NES_CPU::BMI() {
    branch = getFlag(N) == 1;
}
void NES_CPU::BVC() {
    branch = getFlag(V) == 0;
}
void NES_CPU::BVS() {
    branch = getFlag(V) == 1;
}
#pragma endregion
#pragma region JUMP
void NES_CPU::JMP() {
    pc.hi = absAddr.hi;
    pc.lo = absAddr.lo;
}
void NES_CPU::JSR() {
    switch (cycles) {
        case 2:
            fetched = read(pc++);
            break;
        case 3:
            //???
            break;
        case 4:
            write(0x0100 | sp, pc.hi);
            sp--;
            break;
        case 5:
            write(0x0100 | sp, pc.lo);
            sp--;
            break;
        case 6:
            pc.hi = read(pc);
            pc.lo = fetched;
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
            if (nmiTrigger) { currInst = &NMI_INST; } //else if (irqTrigger) { currInst = &IRQ_INST; }
            break;
        case 3:
            write(0x0100 + sp, pc.hi);
            sp--;
            if (nmiTrigger) { currInst = &NMI_INST; } //else if (irqTrigger) { currInst = &IRQ_INST; }
            break;
        case 4:
            write(0x0100 + sp, pc.lo);
            sp--;
            if (nmiTrigger) { currInst = &NMI_INST; } //else if (irqTrigger) { currInst = &IRQ_INST; }
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
            setFlag(U, true);
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
            setFlags(read(0x0100 + sp));
            //delayNMI = true;
            //delayIRQ = true;
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
    cycles = 0;
}
void NES_CPU::SEI() {
    setFlag(I, true);
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
    cycles = 0;
}
#pragma endregion
#pragma region UNOFFICIAL
void NES_CPU::JAM() {
    switch (cycles) {
        case 2:
            read(pc);
            break;
        case 3:
            read(0xFFFF);
            break;
        case 4:
            read(0xFFFE);
            break;
        case 5:
            read(0xFFFE);
            break;
        case 6:
        default:
            read(0xFFFF);
            cycles--;
            break;
    }
}
void NES_CPU::SLO() {
    ASL();
    a |= fetched;
    setZN(a);
}
void NES_CPU::RLA() {
    ROL();
    a &= fetched;
    setZN(a);
}
void NES_CPU::SRE() {
    LSR();
    a ^= fetched;
    setZN(a);
}
void NES_CPU::RRA() {
    ROR();
    ADC();
}
void NES_CPU::SAX() {
    fetched = a & x;
}
void NES_CPU::LAX() {
    a = x = fetched;
    setZN(a);
}
void NES_CPU::DCP() {
    fetched--;
    CMP();
}
void NES_CPU::ISC() {
    fetched++;
    SBC();
}
void NES_CPU::SHA() {
    // {adr}:=A&X&H
    uint8_t h = absAddr.hi;
    if (!paged) h++;
    fetched = a & x & h;
    if (paged) absAddr.hi = fetched;
}
void NES_CPU::SHS() {
    // S:=A&X
    sp = a & x;
    uint8_t h = absAddr.hi;
    if (!paged) h++;
    // {adr}:=S&H
    fetched = a & x & h;
    if (paged) absAddr.hi = fetched;
}
void NES_CPU::SHY() {
    uint8_t h = absAddr.hi;
    if (!paged) h++;
    fetched = y & h;
    if (paged) absAddr.hi = fetched;
}
void NES_CPU::SHX() {
    uint8_t h = absAddr.hi;
    if (!paged) h++;
    fetched = x & h;
    if (paged) absAddr.hi = fetched;
}
void NES_CPU::LAS() {
    a = x = sp = fetched & sp;
    setZN(a);
}
void NES_CPU::ANC() {
    // 0x0B - IMM
    // 0x2B - IMM
    a &= fetched;
    //dStatus = status;
    //if ((a & 0x80) > 0) {
    //    dStatus |= (C | N);
    //} else {
    //    dStatus &= ~(C | N);
    //}
    //if (a == 0)
    //    dStatus |= Z;
    //else
    //    dStatus &= ~Z;
    //updateStatus = true;
    setFlag(C, (a & 0x80) > 0);
    setFlag(N, getFlag(C));
    setFlag(Z, a == 0);
    cycles = 0;
}
void NES_CPU::ASR() {
    // 0x4B - IMM
    a = a & fetched;
    setFlag(C, (a & 0x01) > 0);
    a >>= 1;
    setZN(a);
    cycles = 0;
}
void NES_CPU::ARR() {
    // 0x6B - IMM
    a = a & fetched;
    uint8_t hB = getFlag(C) << 7;
    setFlag(C, (a & 0x80) > 0);
    a = (a >> 1) | hB;
    setFlag(V, getFlag(C) ^ ((a >> 5) & 0x01));
    setZN(a);
    cycles = 0;
}
void NES_CPU::ANE() {
    // 0x8B - IMM
    a = (a | 0xEE) & x & fetched;
    setZN(a);
    cycles = 0;
}
void NES_CPU::LXA() {
    // 0xAB - IMM
    a = x = (a | 0xFF) & fetched;
    setZN(a);
    cycles = 0;
}
void NES_CPU::AXS() {
    // 0xCB - IMM
    uint8_t temp = a & x;
    setFlag(C, temp >= fetched);
    x = temp - fetched;
    setFlag(N, (x & 0x80) > 0);
    setFlag(Z, x == 0);
    cycles = 0;
}
#pragma endregion
#pragma endregion
