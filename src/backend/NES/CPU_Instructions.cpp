#include "CPU.h"

using namespace NES_NS;

// ACCESS
void CPU::LDA() { // load Accumulator with fetched value and update flags
    a = fetched;
    setZN(a);
}
void CPU::STA() { // store Accumulator into memory
    fetched = a;
}
void CPU::LDX() { // load X register with fetched value and update flags
    x = fetched;
    setZN(x);
}
void CPU::STX() { // store X register into memory
    fetched = x;
}
void CPU::LDY() { // load Y register with fetched value and update flags
    y = fetched;
    setZN(y);
}
void CPU::STY() { // store Y register into memory
    fetched = y;
}

// TRANSFER
void CPU::TAX() { // transfer Accumulator to X register and update flags
    x = a;
    setZN(x);
    cycles = 0;
}
void CPU::TXA() { // transfer X register to Accumulator and update flags
    a = x;
    setZN(a);
    cycles = 0;
}
void CPU::TAY() { // transfer Accumulator to Y register and update flags
    y = a;
    setZN(y);
    cycles = 0;
}
void CPU::TYA() { // transfer Y register to Accumulator and update flags
    a = y;
    setZN(a);
    cycles = 0;
}

// ARITHMETIC
void CPU::ADC() { // add fetched data to accumulator with carry and update flags
    u16 temp = a + fetched + getFlag(FLAGS::C);
    u8 res = temp & 0xFF;
    setZN(res);
    setFlag(FLAGS::C, temp > 0xFF);
    setFlag(FLAGS::V, (temp ^ a) & (temp ^ fetched) & 0x80);
    a = res;
}
void CPU::SBC() { // subtract fetched data from accumulator with carry and update flags
    s16 temp = a + (~fetched) + getFlag(FLAGS::C);
    u8 res = temp & 0xFF;
    setZN(res);
    setFlag(FLAGS::C, !(temp < 0x00));
    setFlag(FLAGS::V, (res ^ a) & (res ^ ~fetched) & 0x80);
    a = res;
}
void CPU::INC() { // increment memory
    fetched++;
    setZN(fetched);
}
void CPU::DEC() { // decrement memory
    fetched--;
    setZN(fetched);
}
void CPU::INX() { // increment X register
    x++;
    setZN(x);
    cycles = 0;
}
void CPU::DEX() { // decrement X register
    x--;
    setZN(x);
    cycles = 0;
}
void CPU::INY() { // increment Y register
    y++;
    setZN(y);
    cycles = 0;
}
void CPU::DEY() { // decrement Y register
    y--;
    setZN(y);
    cycles = 0;
}

// SHIFT
void CPU::ASL() { // shift fetched value left by 1 and update flags
    setFlag(FLAGS::C, ((fetched >> 7) & 0x01) > 0);
    fetched <<= 1;
    setZN(fetched);
}
void CPU::LSR() { // shift fetched value right by 1 and update flags
    setFlag(FLAGS::C, (fetched & 0x01) > 0);
    fetched >>= 1;
    setZN(fetched);
}
void CPU::ROL() { // rotate fetched value left with carry and update flags
    u8 temp = (fetched << 1) | getFlag(FLAGS::C);
    setFlag(FLAGS::C, ((fetched >> 7) & 0x01) > 0);
    fetched = temp;
    setZN(fetched);
}
void CPU::ROR() { // rotate fetched value right with carry and update flags
    u8 temp = (fetched >> 1) | (getFlag(FLAGS::C) << 7);
    setFlag(FLAGS::C, (fetched & 0x01) > 0);
    fetched = temp;
    setZN(fetched);
}

// BITWISE
void CPU::AND() { // bitwise AND with Accumulator
    a &= fetched;
    setZN(a);
}
void CPU::ORA() { // bitwise OR with Accumulator
    a |= fetched;
    setZN(a);
}
void CPU::EOR() { // bitwise EXCLUSIVE-OR with Accumulator
    a ^= fetched;
    setZN(a);
}
void CPU::BIT() { // bit test; implemented as a bitwise AND that only effects flags
    setFlag(FLAGS::V, ((fetched >> 6) & 0x01) > 0);
    setFlag(FLAGS::N, ((fetched >> 7) & 0x01) > 0);
    fetched &= a;
    setFlag(FLAGS::Z, fetched == 0);
}

// COMPARE
void CPU::CMP() { // logical compare memory to accumulator; implemented as a subtraction that only effects flags
    setFlag(FLAGS::C, a >= fetched);
    setFlag(FLAGS::Z, a == fetched);
    u8 temp = a - fetched;
    setFlag(FLAGS::N, ((temp >> 7) & 0x01) > 0);
}
void CPU::CPX() { // logical compare memory to X
    setFlag(FLAGS::C, x >= fetched);
    setFlag(FLAGS::Z, x == fetched);
    u8 temp = x - fetched;
    setFlag(FLAGS::N, ((temp >> 7) & 0x01) > 0);
}
void CPU::CPY() { // logical compare memory to Y
    setFlag(FLAGS::C, y >= fetched);
    setFlag(FLAGS::Z, y == fetched);
    u8 temp = y - fetched;
    setFlag(FLAGS::N, ((temp >> 7) & 0x01) > 0);
}

// BRANCH
void CPU::BCC() { // branch if C clear
    branch = getFlag(FLAGS::C) == 0;
}
void CPU::BCS() { // branch if C set
    branch = getFlag(FLAGS::C) == 1;
}
void CPU::BEQ() { // branch if Z set (equal)
    branch = getFlag(FLAGS::Z) == 1;
}
void CPU::BNE() { // branch if Z clear (not equal)
    branch = getFlag(FLAGS::Z) == 0;
}
void CPU::BPL() { // branch if N clear (positive)
    branch = getFlag(FLAGS::N) == 0;
}
void CPU::BMI() { // branch if N set (negative)
    branch = getFlag(FLAGS::N) == 1;
}
void CPU::BVC() { // branch if V clear
    branch = getFlag(FLAGS::V) == 0;
}
void CPU::BVS() { // branch if V set
    branch = getFlag(FLAGS::V) == 1;
}

// JUMP
void CPU::JMP() { // unconditional jump
    pc.hi = absAddr.hi;
    pc.lo = absAddr.lo;
}
void CPU::JSR() { // jump to a subroutine
    switch (cycles) {
        case 2: // first byte after opcode is pc.lo; but second byte is pc.hi, so we can't update pc just yet
            fetched = read(pc++);
            break;
        case 3: // ???
            // idk what happens here
            break;
        case 4: // write current pc.hi to stack
            write(0x0100 | sp, pc.hi);
            sp--;
            break;
        case 5: // write current pc.lo to stack
            write(0x0100 | sp, pc.lo);
            sp--;
            break;
        case 6: // now that current pc has been saved to the stack, we can update pc to the address of the subroutine
            pc.hi = read(pc);
            pc.lo = fetched;
            cycles = 0;
            break;
    }
}
void CPU::RTS() { // return from subroutine
    switch (cycles) {
        case 2: // read pc and discard
            read(pc++);
            break;
        case 3: // increment SP
            read(0x0100 + sp);
            sp++;
            break;
        case 4: // read pc.lo from stack
            absAddr.lo = read(0x0100 + sp);
            sp++;
            break;
        case 5: // read pc.hi from stack
            absAddr.hi = read(0x0100 + sp);
            break;
        case 6: // update pc to the read values and increment
            pc = absAddr;
            pc++;
            cycles = 0;
            break;
    }
}
void CPU::BRK() { // software interrupt (NMI and IRQ are hardware interrupts)
    switch (cycles) {
        case 2: // read pc and discard
            read(pc++);
            //if (nmiTrigger) { currInst = &NMI_INST; } //else if (irqTrigger) { currInst = &IRQ_INST; }
            break;
        case 3: // save pc.hi to stack
            write(0x0100 + sp, pc.hi);
            sp--;
            //if (nmiTrigger) { currInst = &NMI_INST; } //else if (irqTrigger) { currInst = &IRQ_INST; }
            break;
        case 4: // save pc.lo to stack
            write(0x0100 + sp, pc.lo);
            sp--;
            //if (nmiTrigger) { currInst = &NMI_INST; } //else if (irqTrigger) { currInst = &IRQ_INST; }
            break;
        case 5: // save status to stack with B flag SET
            setFlag(FLAGS::B, true);
            setFlag(FLAGS::U, true);
            write(0x0100 + sp, status);
            sp--;
            setFlag(FLAGS::B, false);
            break;
        case 6: // set I flag and read pc.lo from BRK vector
            setFlag(FLAGS::I, true);
            pc.lo = read(0xFFFE);
            break;
        case 7: // read pc.hi from BRK vector and reset cycles
            pc.hi = read(0xFFFF);
            cycles = 0;
            break;
    }
}
void CPU::RTI() { // return from interrupt
    switch (cycles) {
        case 2: // read pc and discard
            read(pc);
            break;
        case 3: // read from stack and discard
            read(0x0100 + sp);
            sp++;
            break;
        case 4: // read status from stack
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
        case 5: // read pc.lo from stack
            pc.lo = read(0x0100 + sp);
            sp++;
            break;
        case 6: // read pc.hi from stack
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
            status = (read(0x0100 + sp) & ~static_cast<u8>(FLAGS::B));
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