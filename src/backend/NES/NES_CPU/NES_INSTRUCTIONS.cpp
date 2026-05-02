#include "NES_CPU.h"

// ACCESS
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

// TRANSFER
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

// ARITHMETIC
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

// SHIFT
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

// BITWISE
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

// COMPARE
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

// BRANCH
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

// JUMP
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

// STACK
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

// FLAGS
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

// OTHER
void NES_CPU::NOP() {
    cycles = 0;
}

// UNOFFICIAL
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