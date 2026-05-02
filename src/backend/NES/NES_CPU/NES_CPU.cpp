#include "../../../core/Helpers.h"
#include "NES_CPU.h"

#include <fstream>
#include <vector>

NES_CPU::NES_CPU(bool d) {
    debug = d;
    lookup = {
        //0x00                                  0x01                                    0x02                                    0x03                                    0x04                                    0x05                                    0x06                                    0x07                                    0x08                                    0x09                                    0x0A                                    0x0B                                    0x0C                                    0x0D                                    0x0E                                    0x0F
        {"BRK",2,nullptr,        &NES_CPU::BRK},{"ORA",2,&NES_CPU::IZX_R,&NES_CPU::ORA},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"SLO",2,&NES_CPU::IZX_M,&NES_CPU::SLO},{"NOP",2,&NES_CPU::ZP0_R,&NES_CPU::NOP},{"ORA",2,&NES_CPU::ZP0_R,&NES_CPU::ORA},{"ASL",2,&NES_CPU::ZP0_M,&NES_CPU::ASL},{"SLO",2,&NES_CPU::ZP0_M,&NES_CPU::SLO},{"PHP",1,nullptr,        &NES_CPU::PHP},{"ORA",2,&NES_CPU::IMM_A,&NES_CPU::ORA},{"ASL",1,&NES_CPU::ACC_A,&NES_CPU::ASL},{"ANC",2,&NES_CPU::IMM_A,&NES_CPU::ANC},{"NOP",3,&NES_CPU::ABS_R,&NES_CPU::NOP},{"ORA",3,&NES_CPU::ABS_R,&NES_CPU::ORA},{"ASL",3,&NES_CPU::ABS_M,&NES_CPU::ASL},{"SLO",3,&NES_CPU::ABS_M,&NES_CPU::SLO},
        //0x10                                  0x11                                    0x12                                    0x13                                    0x14                                    0x15                                    0x16                                    0x17                                    0x18                                    0x19                                    0x1A                                    0x1B                                    0x1C                                    0x1D                                    0x1E                                    0x1F
        {"BPL",2,&NES_CPU::REL_B,&NES_CPU::BPL},{"ORA",2,&NES_CPU::IZY_R,&NES_CPU::ORA},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"SLO",2,&NES_CPU::IZY_M,&NES_CPU::SLO},{"NOP",2,&NES_CPU::ZPX_R,&NES_CPU::NOP},{"ORA",2,&NES_CPU::ZPX_R,&NES_CPU::ORA},{"ASL",2,&NES_CPU::ZPX_M,&NES_CPU::ASL},{"SLO",2,&NES_CPU::ZPX_M,&NES_CPU::SLO},{"CLC",1,&NES_CPU::IMP_A,&NES_CPU::CLC},{"ORA",3,&NES_CPU::ABY_R,&NES_CPU::ORA},{"NOP",1,&NES_CPU::IMP_A,&NES_CPU::NOP},{"SLO",3,&NES_CPU::ABY_M,&NES_CPU::SLO},{"NOP",3,&NES_CPU::ABX_R,&NES_CPU::NOP},{"ORA",3,&NES_CPU::ABX_R,&NES_CPU::ORA},{"ASL",3,&NES_CPU::ABX_M,&NES_CPU::ASL},{"SLO",3,&NES_CPU::ABX_M,&NES_CPU::SLO},
        //0x20                                  0x21                                    0x22                                    0x23                                    0x24                                    0x25                                    0x26                                    0x27                                    0x28                                    0x19                                    0x2A                                    0x2B                                    0x2C                                    0x2D                                    0x2E                                    0x2F
        {"JSR",3,nullptr,        &NES_CPU::JSR},{"AND",2,&NES_CPU::IZX_R,&NES_CPU::AND},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"RLA",2,&NES_CPU::IZX_M,&NES_CPU::RLA},{"BIT",2,&NES_CPU::ZP0_R,&NES_CPU::BIT},{"AND",2,&NES_CPU::ZP0_R,&NES_CPU::AND},{"ROL",2,&NES_CPU::ZP0_M,&NES_CPU::ROL},{"RLA",2,&NES_CPU::ZP0_M,&NES_CPU::RLA},{"PLP",1,nullptr,        &NES_CPU::PLP},{"AND",2,&NES_CPU::IMM_A,&NES_CPU::AND},{"ROL",1,&NES_CPU::ACC_A,&NES_CPU::ROL},{"ANC",2,&NES_CPU::IMM_A,&NES_CPU::ANC},{"BIT",3,&NES_CPU::ABS_R,&NES_CPU::BIT},{"AND",3,&NES_CPU::ABS_R,&NES_CPU::AND},{"ROL",3,&NES_CPU::ABS_M,&NES_CPU::ROL},{"RLA",3,&NES_CPU::ABS_M,&NES_CPU::RLA},
        //0x30                                  0x31                                    0x32                                    0x33                                    0x34                                    0x35                                    0x36                                    0x37                                    0x38                                    0x39                                    0x3A                                    0x3B                                    0x3C                                    0x3D                                    0x3E                                    0x3F
        {"BMI",2,&NES_CPU::REL_B,&NES_CPU::BMI},{"AND",2,&NES_CPU::IZY_R,&NES_CPU::AND},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"RLA",2,&NES_CPU::IZY_M,&NES_CPU::RLA},{"NOP",2,&NES_CPU::ZPX_R,&NES_CPU::NOP},{"AND",2,&NES_CPU::ZPX_R,&NES_CPU::AND},{"ROL",2,&NES_CPU::ZPX_M,&NES_CPU::ROL},{"RLA",2,&NES_CPU::ZPX_M,&NES_CPU::RLA},{"SEC",1,&NES_CPU::IMP_A,&NES_CPU::SEC},{"AND",3,&NES_CPU::ABY_R,&NES_CPU::AND},{"NOP",1,&NES_CPU::IMP_A,&NES_CPU::NOP},{"RLA",3,&NES_CPU::ABY_M,&NES_CPU::RLA},{"NOP",3,&NES_CPU::ABX_R,&NES_CPU::NOP},{"AND",3,&NES_CPU::ABX_R,&NES_CPU::AND},{"ROL",3,&NES_CPU::ABX_M,&NES_CPU::ROL},{"RLA",3,&NES_CPU::ABX_M,&NES_CPU::RLA},
        //0x40                                  0x41                                    0x42                                    0x43                                    0x44                                    0x45                                    0x46                                    0x47                                    0x48                                    0x49                                    0x4A                                    0x4B                                    0x4C                                    0x4D                                    0x4E                                    0x4F
        {"RTI",1,nullptr,        &NES_CPU::RTI},{"EOR",2,&NES_CPU::IZX_R,&NES_CPU::EOR},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"SRE",2,&NES_CPU::IZX_M,&NES_CPU::SRE},{"NOP",2,&NES_CPU::ZP0_R,&NES_CPU::NOP},{"EOR",2,&NES_CPU::ZP0_R,&NES_CPU::EOR},{"LSR",2,&NES_CPU::ZP0_M,&NES_CPU::LSR},{"SRE",2,&NES_CPU::ZP0_M,&NES_CPU::SRE},{"PHA",1,nullptr,        &NES_CPU::PHA},{"EOR",2,&NES_CPU::IMM_A,&NES_CPU::EOR},{"LSR",1,&NES_CPU::ACC_A,&NES_CPU::LSR},{"ASR",2,&NES_CPU::IMM_A,&NES_CPU::ASR},{"JMP",3,&NES_CPU::ABS_J,&NES_CPU::JMP},{"EOR",3,&NES_CPU::ABS_R,&NES_CPU::EOR},{"LSR",3,&NES_CPU::ABS_M,&NES_CPU::LSR},{"SRE",3,&NES_CPU::ABS_M,&NES_CPU::SRE},
        //0x50                                  0x51                                    0x52                                    0x53                                    0x54                                    0x55                                    0x56                                    0x57                                    0x58                                    0x59                                    0x5A                                    0x5B                                    0x5C                                    0x5D                                    0x5E                                    0x5F
        {"BVC",2,&NES_CPU::REL_B,&NES_CPU::BVC},{"EOR",2,&NES_CPU::IZY_R,&NES_CPU::EOR},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"SRE",2,&NES_CPU::IZY_M,&NES_CPU::SRE},{"NOP",2,&NES_CPU::ZPX_R,&NES_CPU::NOP},{"EOR",2,&NES_CPU::ZPX_R,&NES_CPU::EOR},{"LSR",2,&NES_CPU::ZPX_M,&NES_CPU::LSR},{"SRE",2,&NES_CPU::ZPX_M,&NES_CPU::SRE},{"CLI",1,&NES_CPU::IMP_A,&NES_CPU::CLI},{"EOR",3,&NES_CPU::ABY_R,&NES_CPU::EOR},{"NOP",1,&NES_CPU::IMP_A,&NES_CPU::NOP},{"SRE",3,&NES_CPU::ABY_M,&NES_CPU::SRE},{"NOP",3,&NES_CPU::ABX_R,&NES_CPU::NOP},{"EOR",3,&NES_CPU::ABX_R,&NES_CPU::EOR},{"LSR",3,&NES_CPU::ABX_M,&NES_CPU::LSR},{"SRE",3,&NES_CPU::ABX_M,&NES_CPU::SRE},
        //0x60                                  0x61                                    0x62                                    0x63                                    0x64                                    0x65                                    0x66                                    0x67                                    0x68                                    0x69                                    0x6A                                    0x6B                                    0x6C                                    0x6D                                    0x6E                                    0x6F
        {"RTS",1,nullptr,        &NES_CPU::RTS},{"ADC",2,&NES_CPU::IZX_R,&NES_CPU::ADC},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"RRA",2,&NES_CPU::IZX_M,&NES_CPU::RRA},{"NOP",2,&NES_CPU::ZP0_R,&NES_CPU::NOP},{"ADC",2,&NES_CPU::ZP0_R,&NES_CPU::ADC},{"ROR",2,&NES_CPU::ZP0_M,&NES_CPU::ROR},{"RRA",2,&NES_CPU::ZP0_M,&NES_CPU::RRA},{"PLA",1,nullptr,        &NES_CPU::PLA},{"ADC",2,&NES_CPU::IMM_A,&NES_CPU::ADC},{"ROR",1,&NES_CPU::ACC_A,&NES_CPU::ROR},{"ARR",2,&NES_CPU::IMM_A,&NES_CPU::ARR},{"JMP",3,&NES_CPU::IND_J,&NES_CPU::JMP},{"ADC",3,&NES_CPU::ABS_R,&NES_CPU::ADC},{"ROR",3,&NES_CPU::ABS_M,&NES_CPU::ROR},{"RRA",3,&NES_CPU::ABS_M,&NES_CPU::RRA},
        //0x70                                  0x71                                    0x72                                    0x73                                    0x74                                    0x75                                    0x76                                    0x77                                    0x78                                    0x79                                    0x7A                                    0x7B                                    0x7C                                    0x7D                                    0x7E                                    0x7F
        {"BVS",2,&NES_CPU::REL_B,&NES_CPU::BVS},{"ADC",2,&NES_CPU::IZY_R,&NES_CPU::ADC},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"RRA",2,&NES_CPU::IZY_M,&NES_CPU::RRA},{"NOP",2,&NES_CPU::ZPX_R,&NES_CPU::NOP},{"ADC",2,&NES_CPU::ZPX_R,&NES_CPU::ADC},{"ROR",2,&NES_CPU::ZPX_M,&NES_CPU::ROR},{"RRA",2,&NES_CPU::ZPX_M,&NES_CPU::RRA},{"SEI",1,&NES_CPU::IMP_A,&NES_CPU::SEI},{"ADC",3,&NES_CPU::ABY_R,&NES_CPU::ADC},{"NOP",1,&NES_CPU::IMP_A,&NES_CPU::NOP},{"RRA",3,&NES_CPU::ABY_M,&NES_CPU::RRA},{"NOP",3,&NES_CPU::ABX_R,&NES_CPU::NOP},{"ADC",3,&NES_CPU::ABX_R,&NES_CPU::ADC},{"ROR",3,&NES_CPU::ABX_M,&NES_CPU::ROR},{"RRA",3,&NES_CPU::ABX_M,&NES_CPU::RRA},
        //0x80                                  0x81                                    0x82                                    0x83                                    0x84                                    0x85                                    0x86                                    0x87                                    0x88                                    0x89                                    0x8A                                    0x8B                                    0x8C                                    0x8D                                    0x8E                                    0x8F
        {"NOP",2,&NES_CPU::IMM_A,&NES_CPU::NOP},{"STA",2,&NES_CPU::IZX_W,&NES_CPU::STA},{"NOP",2,&NES_CPU::IMM_A,&NES_CPU::NOP},{"SAX",2,&NES_CPU::IZX_W,&NES_CPU::SAX},{"STY",2,&NES_CPU::ZP0_W,&NES_CPU::STY},{"STA",2,&NES_CPU::ZP0_W,&NES_CPU::STA},{"STX",2,&NES_CPU::ZP0_W,&NES_CPU::STX},{"SAX",2,&NES_CPU::ZP0_W,&NES_CPU::SAX},{"DEY",1,&NES_CPU::IMP_A,&NES_CPU::DEY},{"NOP",2,&NES_CPU::IMM_A,&NES_CPU::NOP},{"TXA",1,&NES_CPU::IMP_A,&NES_CPU::TXA},{"ANE",2,&NES_CPU::IMM_A,&NES_CPU::ANE},{"STY",3,&NES_CPU::ABS_W,&NES_CPU::STY},{"STA",3,&NES_CPU::ABS_W,&NES_CPU::STA},{"STX",3,&NES_CPU::ABS_W,&NES_CPU::STX},{"SAX",3,&NES_CPU::ABS_W,&NES_CPU::SAX},
        //0x90                                  0x91                                    0x92                                    0x93                                    0x94                                    0x95                                    0x96                                    0x97                                    0x98                                    0x99                                    0x9A                                    0x9B                                    0x9C                                    0x9D                                    0x9E                                    0x9F
        {"BCC",2,&NES_CPU::REL_B,&NES_CPU::BCC},{"STA",2,&NES_CPU::IZY_W,&NES_CPU::STA},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"SHA",2,&NES_CPU::IZY_W,&NES_CPU::SHA},{"STY",2,&NES_CPU::ZPX_W,&NES_CPU::STY},{"STA",2,&NES_CPU::ZPX_W,&NES_CPU::STA},{"STX",2,&NES_CPU::ZPY_W,&NES_CPU::STX},{"SAX",2,&NES_CPU::ZPY_W,&NES_CPU::SAX},{"TYA",1,&NES_CPU::IMP_A,&NES_CPU::TYA},{"STA",3,&NES_CPU::ABY_W,&NES_CPU::STA},{"TXS",1,&NES_CPU::IMP_A,&NES_CPU::TXS},{"SHS",3,&NES_CPU::ABY_W,&NES_CPU::SHS},{"SHY",3,&NES_CPU::ABX_W,&NES_CPU::SHY},{"STA",3,&NES_CPU::ABX_W,&NES_CPU::STA},{"SHX",3,&NES_CPU::ABY_W,&NES_CPU::SHX},{"SHA",3,&NES_CPU::ABY_W,&NES_CPU::SHA},
        //0xA0                                  0xA1                                    0xA2                                    0xA3                                    0xA4                                    0xA5                                    0xA6                                    0xA7                                    0xA8                                    0xA9                                    0xAA                                    0xAB                                    0xAC                                    0xAD                                    0xAE                                    0xAF
        {"LDY",2,&NES_CPU::IMM_A,&NES_CPU::LDY},{"LDA",2,&NES_CPU::IZX_R,&NES_CPU::LDA},{"LDX",2,&NES_CPU::IMM_A,&NES_CPU::LDX},{"LAX",2,&NES_CPU::IZX_R,&NES_CPU::LAX},{"LDY",2,&NES_CPU::ZP0_R,&NES_CPU::LDY},{"LDA",2,&NES_CPU::ZP0_R,&NES_CPU::LDA},{"LDX",2,&NES_CPU::ZP0_R,&NES_CPU::LDX},{"LAX",2,&NES_CPU::ZP0_R,&NES_CPU::LAX},{"TAY",1,&NES_CPU::IMP_A,&NES_CPU::TAY},{"LDA",2,&NES_CPU::IMM_A,&NES_CPU::LDA},{"TAX",1,&NES_CPU::IMP_A,&NES_CPU::TAX},{"LXA",2,&NES_CPU::IMM_A,&NES_CPU::LXA},{"LDY",3,&NES_CPU::ABS_R,&NES_CPU::LDY},{"LDA",3,&NES_CPU::ABS_R,&NES_CPU::LDA},{"LDX",3,&NES_CPU::ABS_R,&NES_CPU::LDX},{"LAX",3,&NES_CPU::ABS_R,&NES_CPU::LAX},
        //0xB0                                  0xB1                                    0xB2                                    0xB3                                    0xB4                                    0xB5                                    0xB6                                    0xB7                                    0xB8                                    0xB9                                    0xBA                                    0xBB                                    0xBC                                    0xBD                                    0xBE                                    0xBF
        {"BCS",2,&NES_CPU::REL_B,&NES_CPU::BCS},{"LDA",2,&NES_CPU::IZY_R,&NES_CPU::LDA},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"LAX",2,&NES_CPU::IZY_R,&NES_CPU::LAX},{"LDY",2,&NES_CPU::ZPX_R,&NES_CPU::LDY},{"LDA",2,&NES_CPU::ZPX_R,&NES_CPU::LDA},{"LDX",2,&NES_CPU::ZPY_R,&NES_CPU::LDX},{"LAX",2,&NES_CPU::ZPY_R,&NES_CPU::LAX},{"CLV",1,&NES_CPU::IMP_A,&NES_CPU::CLV},{"LDA",3,&NES_CPU::ABY_R,&NES_CPU::LDA},{"TSX",1,&NES_CPU::IMP_A,&NES_CPU::TSX},{"LAS",3,&NES_CPU::ABY_R,&NES_CPU::LAS},{"LDY",3,&NES_CPU::ABX_R,&NES_CPU::LDY},{"LDA",3,&NES_CPU::ABX_R,&NES_CPU::LDA},{"LDX",3,&NES_CPU::ABY_R,&NES_CPU::LDX},{"LAX",3,&NES_CPU::ABY_R,&NES_CPU::LAX},
        //0xC0                                  0xC1                                    0xC2                                    0xC3                                    0xC4                                    0xC5                                    0xC6                                    0xC7                                    0xC8                                    0xC9                                    0xCA                                    0xCB                                    0xCC                                    0xCD                                    0xCE                                    0xCF
        {"CPY",2,&NES_CPU::IMM_A,&NES_CPU::CPY},{"CMP",2,&NES_CPU::IZX_R,&NES_CPU::CMP},{"NOP",2,&NES_CPU::IMM_A,&NES_CPU::NOP},{"DCP",2,&NES_CPU::IZX_M,&NES_CPU::DCP},{"CPY",2,&NES_CPU::ZP0_R,&NES_CPU::CPY},{"CMP",2,&NES_CPU::ZP0_R,&NES_CPU::CMP},{"DEC",2,&NES_CPU::ZP0_M,&NES_CPU::DEC},{"DCP",2,&NES_CPU::ZP0_M,&NES_CPU::DCP},{"INY",1,&NES_CPU::IMP_A,&NES_CPU::INY},{"CMP",2,&NES_CPU::IMM_A,&NES_CPU::CMP},{"DEX",1,&NES_CPU::IMP_A,&NES_CPU::DEX},{"AXS",2,&NES_CPU::IMM_A,&NES_CPU::AXS},{"CPY",3,&NES_CPU::ABS_R,&NES_CPU::CPY},{"CMP",3,&NES_CPU::ABS_R,&NES_CPU::CMP},{"DEC",3,&NES_CPU::ABS_M,&NES_CPU::DEC},{"DCP",3,&NES_CPU::ABS_M,&NES_CPU::DCP},
        //0xD0                                  0xD1                                    0xD2                                    0xD3                                    0xD4                                    0xD5                                    0xD6                                    0xD7                                    0xD8                                    0xD9                                    0xDA                                    0xDB                                    0xDC                                    0xDD                                    0xDE                                    0xDF
        {"BNE",2,&NES_CPU::REL_B,&NES_CPU::BNE},{"CMP",2,&NES_CPU::IZY_R,&NES_CPU::CMP},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"DCP",2,&NES_CPU::IZY_M,&NES_CPU::DCP},{"NOP",2,&NES_CPU::ZPX_R,&NES_CPU::NOP},{"CMP",2,&NES_CPU::ZPX_R,&NES_CPU::CMP},{"DEC",2,&NES_CPU::ZPX_M,&NES_CPU::DEC},{"DCP",2,&NES_CPU::ZPX_M,&NES_CPU::DCP},{"CLD",1,&NES_CPU::IMP_A,&NES_CPU::CLD},{"CMP",3,&NES_CPU::ABY_R,&NES_CPU::CMP},{"NOP",1,&NES_CPU::IMP_A,&NES_CPU::NOP},{"DCP",3,&NES_CPU::ABY_M,&NES_CPU::DCP},{"NOP",3,&NES_CPU::ABX_R,&NES_CPU::NOP},{"CMP",3,&NES_CPU::ABX_R,&NES_CPU::CMP},{"DEC",3,&NES_CPU::ABX_M,&NES_CPU::DEC},{"DCP",3,&NES_CPU::ABX_M,&NES_CPU::DCP},
        //0xE0                                  0xE1                                    0xE2                                    0xE3                                    0xE4                                    0xE5                                    0xE6                                    0xE7                                    0xE8                                    0xE9                                    0xEA                                    0xEB                                    0xEC                                    0xED                                    0xEE                                    0xEF
        {"CPX",2,&NES_CPU::IMM_A,&NES_CPU::CPX},{"SBC",2,&NES_CPU::IZX_R,&NES_CPU::SBC},{"NOP",2,&NES_CPU::IMM_A,&NES_CPU::NOP},{"ISC",2,&NES_CPU::IZX_M,&NES_CPU::ISC},{"CPX",2,&NES_CPU::ZP0_R,&NES_CPU::CPX},{"SBC",2,&NES_CPU::ZP0_R,&NES_CPU::SBC},{"INC",2,&NES_CPU::ZP0_M,&NES_CPU::INC},{"ISC",2,&NES_CPU::ZP0_M,&NES_CPU::ISC},{"INX",1,&NES_CPU::IMP_A,&NES_CPU::INX},{"SBC",2,&NES_CPU::IMM_A,&NES_CPU::SBC},{"NOP",1,&NES_CPU::IMP_A,&NES_CPU::NOP},{"SBC",2,&NES_CPU::IMM_A,&NES_CPU::SBC},{"CPX",3,&NES_CPU::ABS_R,&NES_CPU::CPX},{"SBC",3,&NES_CPU::ABS_R,&NES_CPU::SBC},{"INC",3,&NES_CPU::ABS_M,&NES_CPU::INC},{"ISC",3,&NES_CPU::ABS_M,&NES_CPU::ISC},
        //0xF0                                  0xF1                                    0xF2                                    0xF3                                    0xF4                                    0xF5                                    0xF6                                    0xF7                                    0xF8                                    0xF9                                    0xFA                                    0xFB                                    0xFC                                    0xFD                                    0xFE                                    0xFF
        {"BEQ",2,&NES_CPU::REL_B,&NES_CPU::BEQ},{"SBC",2,&NES_CPU::IZY_R,&NES_CPU::SBC},{"JAM",1,&NES_CPU::IMP_A,&NES_CPU::JAM},{"ISC",2,&NES_CPU::IZY_M,&NES_CPU::ISC},{"NOP",2,&NES_CPU::ZPX_R,&NES_CPU::NOP},{"SBC",2,&NES_CPU::ZPX_R,&NES_CPU::SBC},{"INC",2,&NES_CPU::ZPX_M,&NES_CPU::INC},{"ISC",2,&NES_CPU::ZPX_M,&NES_CPU::ISC},{"SED",1,&NES_CPU::IMP_A,&NES_CPU::SED},{"SBC",3,&NES_CPU::ABY_R,&NES_CPU::SBC},{"NOP",1,&NES_CPU::IMP_A,&NES_CPU::NOP},{"ISC",3,&NES_CPU::ABY_M,&NES_CPU::ISC},{"NOP",3,&NES_CPU::ABX_R,&NES_CPU::NOP},{"SBC",3,&NES_CPU::ABX_R,&NES_CPU::SBC},{"INC",3,&NES_CPU::ABX_M,&NES_CPU::INC},{"ISC",3,&NES_CPU::ABX_M,&NES_CPU::ISC}
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
    pendingNMI = false;
    pendingIRQ = false;
    pendingRST = true;
    cycles = 0;
}

void NES_CPU::clock() {
    cycles++;
    if (cycles == 1) {
        //if (updateStatus) {
        //    status = dStatus;
        //    updateStatus = false;
        //}
        if (pendingRST) {
            currInst = &RST_INST;
            pendingRST = false;
        } else if (pendingNMI) {
            currInst = &NMI_INST;
            pendingNMI = false;
        } else if (pendingIRQ) {
            currInst = &IRQ_INST;
            pendingIRQ = false;
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
    if (cycles == 0) pollInterrupts();
    totalCycles++;
}

void NES_CPU::pollInterrupts() {
    if (nmiTrigger) {
        pendingNMI = true;
        nmiTrigger = false;
    }
    if (irqTrigger && getFlag(I) == 0) {
        if (!delayInterrupt) {
            pendingIRQ = true;
            irqTrigger = false;
        }
    }
    delayInterrupt = false;
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

std::string NES_CPU::formatInst() const  {
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