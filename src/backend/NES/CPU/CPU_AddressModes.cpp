#include "CPU.h"

using namespace NES_NS;

void CPU::ACC_A() {
    fetched = a;
    (this->*currInst->operate)();
    a = fetched;
    cycles = 0;
}
void CPU::IMM_A() {
    fetched = read(pc++);
    (this->*currInst->operate)();
    cycles = 0;
}
void CPU::IMP_A() {
    (this->*currInst->operate)();
    cycles = 0;
}
void CPU::IND_J() {
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
            schedulePoll();
            break;
        case 5:
            absAddr.hi = read(indAddr.value());
            (this->*currInst->operate)();
            cycles = 0;
            break;
    }
}
void CPU::REL_B() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            (this->*currInst->operate)();
            if (!branch) {
                cycles = 0;
            } else spage(offset);
            break;
        case 3:
            if (paged) schedulePoll();
            [[fallthrough]];
        case 4:
            read(pc);
            if (paged) {
                if (((offset >> 7) & 0x01) == 0) { // page forward
                    pc.hi++;
                } else { // page backward
                    pc.hi--;
                }
                paged = false;
                break;
            }
            cycles = 0;
            break;
    }
}

void CPU::ABS_W() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            schedulePoll();
            break;
        case 4:
            (this->*currInst->operate)();
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void CPU::ABS_R() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            schedulePoll();
            break;
        case 4:
            fetched = read(absAddr.value());
            (this->*currInst->operate)();
            cycles = 0;
            break;
    }
}
void CPU::ABS_M() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            break;
        case 4:
            fetched = read(absAddr.value());
            break;
        case 5:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            schedulePoll();
            break;
        case 6:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void CPU::ABS_J() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            schedulePoll();
            break;
        case 3:
            absAddr.hi = read(pc++);
            (this->*currInst->operate)();
            cycles = 0;
            break;
    }
}

void CPU::ABX_W() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            page(x);
            break;
        case 4:
            // magic happens here
            read(absAddr.value());
            if (paged) absAddr.hi++;
            schedulePoll();
            break;
        case 5:
            (this->*currInst->operate)();
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void CPU::ABX_R() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            page(x);
            if (!paged) schedulePoll();
            break;
        case 4:
        case 5:
            if (paged) {
                read(absAddr.value());
                absAddr.hi++;
                schedulePoll();
                paged = false;
            } else {
                fetched = read(absAddr.value());
                (this->*currInst->operate)();
                cycles = 0;
            }
            break;
    }
}
void CPU::ABX_M() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            page(x);
            break;
        case 4:
            read(absAddr.value());
            if (paged) absAddr.hi++;
            break;
        case 5:
            fetched = read(absAddr.value());
            break;
        case 6:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            schedulePoll();
            break;
        case 7:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}

void CPU::ABY_W() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            page(y);
            break;
        case 4:
            // magic happens here
            read(absAddr.value());
            if (paged) absAddr.hi++;
            schedulePoll();
            break;
        case 5:
            (this->*currInst->operate)();
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void CPU::ABY_R() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            page(y);
            if (!paged) schedulePoll();
            break;
        case 4:
        case 5:
            if (paged) {
                read(absAddr.value());
                absAddr.hi++;
                paged = false;
                schedulePoll();
            } else {
                fetched = read(absAddr.value());
                (this->*currInst->operate)();
                cycles = 0;
            }
            break;
    }
}
void CPU::ABY_M() {
    switch (cycles) {
        case 2:
            absAddr.lo = read(pc++);
            break;
        case 3:
            absAddr.hi = read(pc++);
            page(y);
            break;
        case 4:
            read(absAddr.value());
            if (paged) absAddr.hi++;
            break;
        case 5:
            fetched = read(absAddr.value());
            break;
        case 6:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            schedulePoll();
            break;
        case 7:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}

void CPU::ZP0_W() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            schedulePoll();
            break;
        case 3:
            (this->*currInst->operate)();
            write(offset, fetched);
            cycles = 0;
            break;
    }
}
void CPU::ZP0_R() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            schedulePoll();
            break;
        case 3:
            fetched = read(offset);
            (this->*currInst->operate)();
            cycles = 0;
            break;
    }
}
void CPU::ZP0_M() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            break;
        case 3:
            fetched = read(offset);
            break;
        case 4:
            write(offset, fetched);
            (this->*currInst->operate)();
            schedulePoll();
            break;
        case 5:
            write(offset, fetched);
            cycles = 0;
            break;
    }
}

void CPU::ZPX_W() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            break;
        case 3:
            read(offset);
            offset += x;
            schedulePoll();
            break;
        case 4:
            (this->*currInst->operate)();
            write(offset, fetched);
            cycles = 0;
            break;
    }
}
void CPU::ZPX_R() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            break;
        case 3:
            read(offset);
            offset += x;
            schedulePoll();
            break;
        case 4:
            fetched = read(offset);
            (this->*currInst->operate)();
            cycles = 0;
            break;
    }
}
void CPU::ZPX_M() {
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
            (this->*currInst->operate)();
            schedulePoll();
            break;
        case 6:
            write(offset, fetched);
            cycles = 0;
            break;
    }
}

void CPU::ZPY_W() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            break;
        case 3:
            read(offset);
            offset += y;
            schedulePoll();
            break;
        case 4:
            (this->*currInst->operate)();
            write(offset, fetched);
            cycles = 0;
            break;
    }
}
void CPU::ZPY_R() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            break;
        case 3:
            read(offset);
            offset += y;
            schedulePoll();
            break;
        case 4:
            fetched = read(offset);
            (this->*currInst->operate)();
            cycles = 0;
            break;
    }
}
void CPU::ZPY_M() {
    switch (cycles) {
        case 2:
            offset = read(pc++);
            break;
        case 3:
            read(offset);
            offset += y;
            break;
        case 4:
            fetched = read(offset);
            break;
        case 5:
            write(offset, fetched);
            (this->*currInst->operate)();
            schedulePoll();
            break;
        case 6:
            write(offset, fetched);
            cycles = 0;
            break;
    }
}

void CPU::IZX_W() {
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
            schedulePoll();
            break;
        case 6:
            (this->*currInst->operate)();
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void CPU::IZX_R() {
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
            schedulePoll();
            break;
        case 6:
            fetched = read(absAddr.value());
            (this->*currInst->operate)();
            cycles = 0;
            break;
    }
}
void CPU::IZX_M() {
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
            fetched = read(absAddr.value());
            break;
        case 7:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            schedulePoll();
            break;
        case 8:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}

void CPU::IZY_W() {
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
            // magic happens here
            read(absAddr.value());
            if (paged) absAddr.hi++;
            schedulePoll();
            break;
        case 6:
            (this->*currInst->operate)();
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}
void CPU::IZY_R() {
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
            if (!paged) schedulePoll();
            break;
        case 5:
        case 6:
            fetched = read(absAddr.value());
            if (paged) {
                absAddr.hi++;
                paged = false;
                schedulePoll();
                break;
            }
            (this->*currInst->operate)();
            cycles = 0;
            break;
    }
}
void CPU::IZY_M() {
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
            read(absAddr.value());
            if (paged) absAddr.hi++;
            break;
        case 6:
            fetched = read(absAddr.value());
            break;
        case 7:
            write(absAddr.value(), fetched);
            (this->*currInst->operate)();
            schedulePoll();
            break;
        case 8:
            write(absAddr.value(), fetched);
            cycles = 0;
            break;
    }
}