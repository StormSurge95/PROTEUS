#include "CPU.h"

using namespace NES_NS;

// ACCESS
void CPU::LDA() {
    a = fetched;
    setZN(a);
}
void CPU::STA() {
    fetched = a;
}
void CPU::LDX() {
    x = fetched;
    setZN(x);
}
void CPU::STX() {
    fetched = x;
}
void CPU::LDY() {
    y = fetched;
    setZN(y);
}
void CPU::STY() {
    fetched = y;
}

// TRANSFER
void CPU::TAX() {
    x = a;
    setZN(x);
    cycles = 0;
}
void CPU::TXA() {
    a = x;
    setZN(a);
    cycles = 0;
}
void CPU::TAY() {
    y = a;
    setZN(y);
    cycles = 0;
}
void CPU::TYA() {
    a = y;
    setZN(a);
    cycles = 0;
}

// ARITHMETIC
void CPU::ADC() {
    u16 temp = a + fetched + getFlag(FLAGS::C);
    u8 res = temp & 0xFF;
    setZN(res);
    setFlag(FLAGS::C, temp > 0xFF);
    setFlag(FLAGS::V, (temp ^ a) & (temp ^ fetched) & 0x80);
    a = res;
}
void CPU::SBC() {
    s16 temp = a + (~fetched) + getFlag(FLAGS::C);
    u8 res = temp & 0xFF;
    setZN(res);
    setFlag(FLAGS::C, !(temp < 0x00));
    setFlag(FLAGS::V, (res ^ a) & (res ^ ~fetched) & 0x80);
    a = res;
}
void CPU::INC() {
    fetched++;
    setZN(fetched);
}
void CPU::DEC() {
    fetched--;
    setZN(fetched);
}
void CPU::INX() {
    x++;
    setZN(x);
    cycles = 0;
}
void CPU::DEX() {
    x--;
    setZN(x);
    cycles = 0;
}
void CPU::INY() {
    y++;
    setZN(y);
    cycles = 0;
}
void CPU::DEY() {
    y--;
    setZN(y);
    cycles = 0;
}

// SHIFT
void CPU::ASL() {
    setFlag(FLAGS::C, ((fetched >> 7) & 0x01) > 0);
    fetched <<= 1;
    setZN(fetched);
}
void CPU::LSR() {
    setFlag(FLAGS::C, (fetched & 0x01) > 0);
    fetched >>= 1;
    setZN(fetched);
}
void CPU::ROL() {
    u8 temp = (fetched << 1) | getFlag(FLAGS::C);
    setFlag(FLAGS::C, ((fetched >> 7) & 0x01) > 0);
    fetched = temp;
    setZN(fetched);
}
void CPU::ROR() {
    u8 temp = (fetched >> 1) | (getFlag(FLAGS::C) << 7);
    setFlag(FLAGS::C, (fetched & 0x01) > 0);
    fetched = temp;
    setZN(fetched);
}

// BITWISE
void CPU::AND() {
    a &= fetched;
    setZN(a);
}
void CPU::ORA() {
    a |= fetched;
    setZN(a);
}
void CPU::EOR() {
    a ^= fetched;
    setZN(a);
}
void CPU::BIT() {
    setFlag(FLAGS::V, ((fetched >> 6) & 0x01) > 0);
    setFlag(FLAGS::N, ((fetched >> 7) & 0x01) > 0);
    fetched &= a;
    setFlag(FLAGS::Z, fetched == 0);
}

// COMPARE
void CPU::CMP() {
    setFlag(FLAGS::C, a >= fetched);
    setFlag(FLAGS::Z, a == fetched);
    u8 temp = a - fetched;
    setFlag(FLAGS::N, ((temp >> 7) & 0x01) > 0);
}
void CPU::CPX() {
    setFlag(FLAGS::C, x >= fetched);
    setFlag(FLAGS::Z, x == fetched);
    u8 temp = x - fetched;
    setFlag(FLAGS::N, ((temp >> 7) & 0x01) > 0);
}
void CPU::CPY() {
    setFlag(FLAGS::C, y >= fetched);
    setFlag(FLAGS::Z, y == fetched);
    u8 temp = y - fetched;
    setFlag(FLAGS::N, ((temp >> 7) & 0x01) > 0);
}

// BRANCH
void CPU::BCC() {
    branch = getFlag(FLAGS::C) == 0;
}
void CPU::BCS() {
    branch = getFlag(FLAGS::C) == 1;
}
void CPU::BEQ() {
    branch = getFlag(FLAGS::Z) == 1;
}
void CPU::BNE() {
    branch = getFlag(FLAGS::Z) == 0;
}
void CPU::BPL() {
    branch = getFlag(FLAGS::N) == 0;
}
void CPU::BMI() {
    branch = getFlag(FLAGS::N) == 1;
}
void CPU::BVC() {
    branch = getFlag(FLAGS::V) == 0;
}
void CPU::BVS() {
    branch = getFlag(FLAGS::V) == 1;
}

// JUMP
void CPU::JMP() {
    pc.hi = absAddr.hi;
    pc.lo = absAddr.lo;
}
void CPU::JSR() {
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
void CPU::RTS() {
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
void CPU::BRK() {
    switch (cycles) {
        case 2:
            read(pc++);
            //if (nmiTrigger) { currInst = &NMI_INST; } //else if (irqTrigger) { currInst = &IRQ_INST; }
            break;
        case 3:
            write(0x0100 + sp, pc.hi);
            sp--;
            //if (nmiTrigger) { currInst = &NMI_INST; } //else if (irqTrigger) { currInst = &IRQ_INST; }
            break;
        case 4:
            write(0x0100 + sp, pc.lo);
            sp--;
            //if (nmiTrigger) { currInst = &NMI_INST; } //else if (irqTrigger) { currInst = &IRQ_INST; }
            break;
        case 5:
            setFlag(FLAGS::B, true);
            setFlag(FLAGS::U, true);
            write(0x0100 + sp, status);
            sp--;
            setFlag(FLAGS::B, false);
            break;
        case 6:
            setFlag(FLAGS::I, true);
            pc.lo = read(0xFFFE);
            break;
        case 7:
            pc.hi = read(0xFFFF);
            cycles = 0;
            break;
    }
}
void CPU::RTI() {
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
                u8 p = read(0x0100 + sp);
                setFlag(FLAGS::C, (p & 0x01) > 0);
                setFlag(FLAGS::Z, ((p >> 1) & 0x01) > 0);
                setFlag(FLAGS::I, ((p >> 2) & 0x01) > 0);
                setFlag(FLAGS::D, ((p >> 3) & 0x01) > 0);
                setFlag(FLAGS::V, ((p >> 6) & 0x01) > 0);
                setFlag(FLAGS::N, ((p >> 7) & 0x01) > 0);
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

// STACK
void CPU::PHA() {
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
void CPU::PLA() {
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
void CPU::PHP() {
    switch (cycles) {
        case 2:
            read(pc);
            break;
        case 3:
            setFlag(FLAGS::U, true);
            setFlag(FLAGS::B, true);
            write(0x0100 + sp, status);
            setFlag(FLAGS::B, false);
            sp--;
            cycles = 0;
            break;
    }

}
void CPU::PLP() {
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
void CPU::TXS() {
    sp = x;
    cycles = 0;
}
void CPU::TSX() {
    x = sp;
    setZN(x);
    cycles = 0;
}

// FLAGS
void CPU::CLC() {
    setFlag(FLAGS::C, false);
    cycles = 0;
}
void CPU::SEC() {
    setFlag(FLAGS::C, true);
    cycles = 0;
}
void CPU::CLI() {
    setFlag(FLAGS::I, false);
    cycles = 0;
}
void CPU::SEI() {
    setFlag(FLAGS::I, true);
    cycles = 0;
}
void CPU::CLD() {
    setFlag(FLAGS::D, false);
    cycles = 0;
}
void CPU::SED() {
    setFlag(FLAGS::D, true);
    cycles = 0;
}
void CPU::CLV() {
    setFlag(FLAGS::V, false);
    cycles = 0;
}

// OTHER
void CPU::NOP() {
    cycles = 0;
}

// UNOFFICIAL
void CPU::JAM() {
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
void CPU::SLO() {
    ASL();
    a |= fetched;
    setZN(a);
}
void CPU::RLA() {
    ROL();
    a &= fetched;
    setZN(a);
}
void CPU::SRE() {
    LSR();
    a ^= fetched;
    setZN(a);
}
void CPU::RRA() {
    ROR();
    ADC();
}
void CPU::SAX() {
    fetched = a & x;
}
void CPU::LAX() {
    a = x = fetched;
    setZN(a);
}
void CPU::DCP() {
    fetched--;
    CMP();
}
void CPU::ISC() {
    fetched++;
    SBC();
}
void CPU::SHA() {
    // {adr}:=A&X&H
    u8 h = absAddr.hi;
    if (!paged) h++;
    fetched = a & x & h;
    if (paged) absAddr.hi = fetched;
}
void CPU::SHS() {
    // S:=A&X
    sp = a & x;
    u8 h = absAddr.hi;
    if (!paged) h++;
    // {adr}:=S&H
    fetched = a & x & h;
    if (paged) absAddr.hi = fetched;
}
void CPU::SHY() {
    u8 h = absAddr.hi;
    if (!paged) h++;
    fetched = y & h;
    if (paged) absAddr.hi = fetched;
}
void CPU::SHX() {
    u8 h = absAddr.hi;
    if (!paged) h++;
    fetched = x & h;
    if (paged) absAddr.hi = fetched;
}
void CPU::LAS() {
    a = x = sp = fetched & sp;
    setZN(a);
}
void CPU::ANC() {
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
    setFlag(FLAGS::C, (a & 0x80) > 0);
    setFlag(FLAGS::N, getFlag(FLAGS::C));
    setFlag(FLAGS::Z, a == 0);
    cycles = 0;
}
void CPU::ASR() {
    // 0x4B - IMM
    a = a & fetched;
    setFlag(FLAGS::C, (a & 0x01) > 0);
    a >>= 1;
    setZN(a);
    cycles = 0;
}
void CPU::ARR() {
    // 0x6B - IMM
    a = a & fetched;
    u8 hB = getFlag(FLAGS::C) << 7;
    setFlag(FLAGS::C, (a & 0x80) > 0);
    a = (a >> 1) | hB;
    setFlag(FLAGS::V, getFlag(FLAGS::C) ^ ((a >> 5) & 0x01));
    setZN(a);
    cycles = 0;
}
void CPU::ANE() {
    // 0x8B - IMM
    a = (a | 0xEE) & x & fetched;
    setZN(a);
    cycles = 0;
}
void CPU::LXA() {
    // 0xAB - IMM
    a = x = (a | 0xFF) & fetched;
    setZN(a);
    cycles = 0;
}
void CPU::AXS() {
    // 0xCB - IMM
    u8 temp = a & x;
    setFlag(FLAGS::C, temp >= fetched);
    x = temp - fetched;
    setFlag(FLAGS::N, (x & 0x80) > 0);
    setFlag(FLAGS::Z, x == 0);
    cycles = 0;
}