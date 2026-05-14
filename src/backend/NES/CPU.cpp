#include "./CPU.h"
#include "./PPU.h"
#include "./APU.h"
#include "./Gamepak.h"
#include "./Controller.h"

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
    ram.fill(0x00);
}

u8 CPU::read(u16 addr, bool readonly) {
    // update last read address for use during dummy dma reads.
    lastReadAddr = addr;
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
    } else if (addr == 0x4014) {
        // read PPU OAM data
        ret = ppu.lock()->readOAMByte();
        if (readonly) return ret;
    } else if (addr == 0x4015) {
        // read APU status
        ret = (cpuBus & 0x20) | (apu.lock()->read(addr, readonly) & 0xDF);
        if (readonly) return ret;
    } else if (addr == 0x4016) {
        // read Player 1 Controller
        ret = (cpuBus & 0xE0) | (player1.lock()->onRead() & 0x1F);
        if (readonly) return ret;
    } else if (addr == 0x4017) {
        // read Player 2 Controller
        ret = (cpuBus & 0xE0) | (player2.lock()->onRead() & 0x1F);
        if (readonly) return ret;
    } else if (addr >= 0x6000 && addr <= 0xFFFF) {
        // read cartridge memory (including SRAM, if present)
        ret = cart.lock()->read(addr, readonly);
        if (readonly) return ret;
    }
    // TODO: Handle PRG-RAM open bus stuff

    // getting here meand readonly is clear; so update cpuBus and return it.
    cpuBus = ret;
    return cpuBus;
}

void CPU::write(u16 addr, u8 data) {
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
        dmaPage = data;
        dmaAddr = 0x00;
        oamActive = true;
        dmaDummy = true;
    } else if (addr == 0x4016) {
        // write to player1 controller
        player1.lock()->onWrite(data);
    } else if (addr >= 0x4000 && addr <= 0x4017) {
        // write to APU registers
        if (addr == 0x4015 && ((data >> 4) & 0x01) > 0) {
            // enabling DMC channel immediately triggers DMCDMA
            dmcActive = true;
            dmaDummy = true;
        }
        apu.lock()->write(addr, data);
    } else if (addr >= 0x5FFF && addr <= 0xFFFF) {
        // write to Cartridge memory (including SRAM, if present)
        cart.lock()->write(addr, data);
    }
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

void CPU::clockOAM() {
    /**
     * via https://nesdev.org/wiki/DMA#OAM_DMA:
     * OAM DMA copies 256 bytes from a CPU page to PPU OAM via the OAMDATA ($2004) register.
     * It is triggered by writing the page number (the high byte of the address) to OAMDMA ($4014).
     * OAM DMA is scheduled to halt the CPU on the first cycle after the register write.
     * In the common case, it performs a halt cycle, an optional alignment cycle, and 256 get/put pairs.
     * The 256 get/put pairs copy forward from the start of the page. Because DMA can only read on get cycles,
     * an alignment cycle performing no useful work may be required before being able to read. All together,
     * OAM DMA on its own takes 513 or 514 cycles, depending on whether alignment is needed.
     * OAM DMA will copy from the page most recently written to $4014. This means that read-modify-write
     * instructions such as INC $4014, which are able to perform a second write before the CPU can be halted, will copy from the second page written, not the first.
     * OAM DMA has a lower priority than DMC DMA. If a DMC DMA get occurs during OAM DMA, OAM DMA is briefly paused.
     */
    bool put = (totalCycles & 0x01) > 0; // determine first cycle; odd = put, !odd = get
    if (dmaDummy) { // initial halt cycle
        if (put) // no alignment needed
            dmaDummy = false;
        else // alignment cycle (dummy read) needed
            read(lastReadAddr);
    } else {
        if (!put) // 'get' oam data from WRAM
            dmaData = read(((u16)dmaPage << 8) | dmaAddr);
        else { // 'put' oam data into PPU memory
            sptr<PPU> ppup = ppu.lock();
            u8 i = (ppup->getOAMADDR() + dmaAddr) & 0xFF;
            ppup->writeOAMByte(i, dmaData);

            // DMA should not update OAMADDR within the ppu
            // instead, increment helper variable
            dmaAddr++;
            if (dmaAddr == 0x00) {
                // helper variable is 8 bits so that overflow to 0 means
                // we have performed the put operation 256 times precisely
                oamActive = false;
                dmaDummy = true;
            }
        }
    }
    totalCycles++;
}

void CPU::clockDMC() {
    /**
     * via https://nesdev.org/wiki/DMA#DMC_DMA:
     * DMC DMA copies a single byte to the DMC unit's sample buffer. This occurs automatically after the DMC
     * enable bit, bit 4, of the sound channel enable register ($4015) is set to 1, which starts DPCM sample
     * playback using the current DMC settings in registers $4010-$4013. DMC DMA is scheduled when all of DPCM
     * playback is enabled, there are bytes left in the sample, and the sample buffer is empty. In the common
     * cases, DMC DMA performs a halt cycle, a dummy cycle, an optional alignment cycle, and a get.
     *
     * The exact timing depends on the type of DMC DMA. There are two types: load and reload. Load DMAs occur
     * after $4015 D4 is set, but only if the sample buffer is empty. They are scheduled to halt the CPU on
     * a get cycle during the 2nd APU cycle after the write (that is, the 3rd or 4th CPU cycle). Reload DMAs
     * occur in response to the sample buffer being emptied. Unlike load DMAs, they are scheduled to halt
     * the CPU on a put cycle.
     *
     * After the halt, DMC DMA always performs a dummy cycle where no work is done. If the next cycle is not
     * a get cycle, then a cycle will be spent on alignment. Then the DMA read is performed.
     *
     * DMC DMA normally takes 3 or 4 cycles, depending on whether alignment is needed. Because load and reload
     * DMAs schedule on different cycle types, load DMAs take 3 cycles and reload DMAs take 4 unless the halt
     * is delayed by an odd number of cycles. However, bugs can cause additional cycles.
     */
    bool put = (totalCycles & 0x01) > 0; // determine first cycle; odd = put, !odd = get
    // odd = put
    // !odd = get
    // load = halt on get
    // REload = halt on put
    if (dmaDummy) {
        if (put)
            dmaDummy = false;
        else
            read(lastReadAddr);
    } else {
        apu.lock()->dmcFetch(!put);
    }
}

void CPU::start() {
    // During power on, we simply have to read the first pc value from the reset vector of the cartridge
    pc.lo = read(0xFFFC);
    pc.hi = read(0xFFFD);
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
    //if (pollScheduled) pollInterrupts();
    /// We initialize `cycles` to `0`, but only start operations when it is `1`; so our logic requires pre-incrementing.
    cycles++;
    if (cycles == 1) {
        /// On cycle `1`, we either trigger an interrupt/reset, or read the next opcode to prepare for the next instruction.
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
            prevInstAddrs.push_back(pc);
            if (prevInstAddrs.size() > 13) prevInstAddrs.pop_front();
            opcode = read(pc++);
            currInst = &lookup[opcode];
            if (currInst->address == &CPU::IMM_A ||
                currInst->address == &CPU::ACC_A ||
                currInst->address == &CPU::IMP_A ||
                currInst->address == &CPU::REL_B)
                schedulePoll();
        }
    } else {
        if (currInst->address != nullptr) // if this instruction requires addressing mode logic, then perform that function
            (this->*currInst->address)();
        else // otherwise, simply perform the operation function, as it will handle the cycle logic itself
            (this->*currInst->operate)();
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
    pollScheduled = false;
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