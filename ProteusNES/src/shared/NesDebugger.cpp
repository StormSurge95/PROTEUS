#include "../NES.h"
#include "./NesDebugger.h"
#include "../APU/NesAPU.h"
#include "./NesProfiles.h"

using namespace NS_NES;

NesDebugger::NesDebugger(NES* station) {
    nes = station;
    nes->connectEventSink(this);
}

void NesDebugger::Init() {
    evrPrevFrame.reserve(4096);
    evrLastFrame.reserve(4096);
    evrActiveFrame.reserve(4096);
    evEventsSnapshot.reserve(8192);

    const size_t pixelCount = size_t(GetDotsPerScanline()) * size_t(GetScanlinesPerFrame(nes->cart->region));
    evPixelsSnapshot.resize(pixelCount, 0x00000000);

    traceRecords.reserve(16384);
    traceTextBuffer.reserve(1 << 20);
    IDebugger::Init();
}

void NesDebugger::StepInstruction() {
    if (!enabled) return;
    if (nes == nullptr || nes->cpu == nullptr || nes->ppu == nullptr || nes->apu == nullptr) return;

    u64 startTotalCycles = nes->cpu->totalCycles;
    bool sawClock = false;

    do {
        u64 prevTotal = nes->cpu->totalCycles;

        StepCycle();

        if (nes->cpu->totalCycles != prevTotal)
            sawClock = true;

        if (nes->cpu->currInst->name == "JAM")
            break;
    } while (!sawClock || nes->cpu->cycles != 0);
}

void NesDebugger::StepCycle() {
    if (!enabled) return;
    // StepCycle() should mean to step one CPU cycle (12 master cycles for NTSC/15 for PAL)

    // validate state
    if (nes == nullptr || nes->cpu == nullptr || nes->ppu == nullptr || nes->apu == nullptr) return;
    u8 div = GetCpuClockDiv(nes->cart->region);
    for (u8 i = 0; i < div; i++)
        nes->clockMaster();
}

EventViewerDisplaySize NesDebugger::GetEventViewerDisplaySize() const {
    if (!enabled || !initialized) return { 0, 0 };
    u32 dots = GetDotsPerScanline();
    u32 lines = GetScanlinesPerFrame(nes->cart->region);
    return { dots, lines };
}

void NesDebugger::SetEventViewerConfig(const EventViewerConfig& cfg) {
    evConfig = cfg;
}

const EventViewerConfig& NesDebugger::GetEventViewerConfig() const {
    return evConfig;
}

const vector<u32>& NesDebugger::GetEventViewerPixels() const {
    return evPixelsSnapshot;
}

const vector<DebugEventRecord>& NesDebugger::GetEventViewerEvents() const {
    return evEventsSnapshot;
}

const DebugEventRecord& NesDebugger::GetEventAt(u16 scanline, u16 cycle) const {
    if (initialized) {
        for (const DebugEventRecord& event : evEventsSnapshot | std::views::reverse) {
            if (event.scanline == scanline && event.cycle == cycle) return event;
        }
    }

    return NullEvent;
}

void NesDebugger::TakeEventViewerSnapshot(bool forAutoRefresh) {
    if (!enabled || !initialized) return;
    std::fill(evPixelsSnapshot.begin(), evPixelsSnapshot.end(), 0x00000000);
    evEventsSnapshot.clear();

    if (evConfig.showPreviousFrame) {
        for (const DebugEventRecord& event : evrPrevFrame) {
            for (const auto& [flag, filter] : evConfig.eventFilters) {
                if ((event.flags & flag) != 0 && filter.show) {
                    DebugEventRecord ghost = event;
                    ghost.flags |= DebugEventFlags::PREVIOUS_FRAME;
                    ghost.color = Dim(ghost.color, 0.5f);
                    evEventsSnapshot.push_back(ghost);
                    break;
                }
            }
        }
    }

    for (const DebugEventRecord& event : evrLastFrame) {
        for (const auto& [flag, filter] : evConfig.eventFilters) {
            if ((event.flags & flag) != 0 && filter.show) {
                evEventsSnapshot.push_back(event);
                break;
            }
        }
    }

    for (const DebugEventRecord& evr : evEventsSnapshot) {
        u32 dots = GetDotsPerScanline();
        u32 lines = GetScanlinesPerFrame(nes->cart->region);
        if (evr.scanline < lines && evr.cycle < dots)
            evPixelsSnapshot[evr.scanline * dots + evr.cycle] = evr.color;
    }

    evSnapshotValid = true;
}

u32 NesDebugger::GetEventColor(EventType type) {
    if (!enabled || !initialized) return 0x00000000;
    return defaultEventColors[u8(type)];
}

void NesDebugger::OnPpuRegisterRead(u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::PPU_READ),
        .flags = DebugEvent_ReadVideo,
        .type = "PPU REG READ",
        .details = PPU_REGS[addr & 7]
    });
}

void NesDebugger::OnPpuRegisterWrite(u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::PPU_WRITE),
        .flags = DebugEvent_WriteVideo,
        .type = "PPU REG WRITE",
        .details = PPU_REGS[addr & 7]
    });
}

void NesDebugger::OnApuRegisterRead(u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::APU_READ),
        .flags = DebugEvent_ReadAudio,
        .type = "APU REG READ",
        .details = APU_REGS[addr % 24]
    });
}

void NesDebugger::OnApuRegisterWrite(u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::APU_WRITE),
        .flags = DebugEvent_WriteAudio,
        .type = "APU REG WRITE",
        .details = APU_REGS[addr % 24]
    });
}

void NesDebugger::OnMapperRegisterRead(string details, u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::MAPPER_READ),
        .flags = DebugEvent_ReadMapper,
        .type = "MAPPER REG READ",
        .details = details
    });
}

void NesDebugger::OnMapperRegisterWrite(string details, u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::MAPPER_WRITE),
        .flags = DebugEvent_WriteMapper,
        .type = "MAPPER REG WRITE",
        .details = details
    });
}

void NesDebugger::OnControllerRead(u8 player, u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::CONTROLLER_READ),
        .flags = DebugEvent_ReadInput,
        .type = "CONTROLLER READ",
        .details = "Player " + to_string(player)
    });
}

void NesDebugger::OnControllerWrite(u8 player, u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::CONTROLLER_WRITE),
        .flags = DebugEvent_WriteInput,
        .type = "CONTROLLER WRITE",
        .details = "Player " + to_string(player)
    });
}

void NesDebugger::OnDmcDmaRead(string details, u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::DMC_READ),
        .flags = DebugEvent_ReadDma,
        .type = "DMC DMA READ",
        .details = details
    });
}

void NesDebugger::OnOamDmaRead(u16 addr, u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = addr,
        .value = data,
        .color = GetEventColor(EventType::OAM_READ),
        .flags = DebugEvent_ReadDma,
        .type = "OAM DMA READ"
    });
}

void NesDebugger::OnOamDmaStart(u8 data) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = 0x4014,
        .value = data,
        .color = GetEventColor(EventType::OAM_START),
        .flags = DebugEvent_WriteDma,
        .type = "OAM DMA START"
    });
}

void NesDebugger::OnInterrupt(INTERRUPT_EVENT type) {
    if (!enabled || !initialized) return;
    const char* label;
    const char* details = INTERRUPT_DETAILS[u8(type)];
    u32 color;
    switch (type) {
        case INTERRUPT_EVENT::NMI_REQ:
        case INTERRUPT_EVENT::NMI_ACK:
            color = GetEventColor(EventType::INTERRUPT_NMI);
            label = "NMI";
            break;
        case INTERRUPT_EVENT::IRQ_REQ_DMC:
        case INTERRUPT_EVENT::IRQ_REQ_APU:
        case INTERRUPT_EVENT::IRQ_REQ_MAP:
        case INTERRUPT_EVENT::IRQ_ACK:
            color = GetEventColor(EventType::INTERRUPT_IRQ);
            label = "IRQ";
            break;
    }
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .address = nes->cpu->pc.value(),
        .color = color,
        .flags = DebugEvent_Interrupt,
        .type = label,
        .details = details
    });
}

void NesDebugger::OnSpriteZeroHit() {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .color = GetEventColor(EventType::ZERO_HIT),
        .flags = DebugEvent_SpriteZeroHit,
        .type = "SPRITE ZERO HIT"
    });
}

void NesDebugger::OnMarkedBreakpoint(string details) {
    if (!enabled || !initialized) return;
    PushEventRecord({
        .scanline = nes->ppu->scanline,
        .cycle = nes->ppu->cycle,
        .color = GetEventColor(EventType::BREAKPOINT),
        .flags = DebugEvent_Breakpoint,
        .type = "MARKED BREAKPOINT",
        .details = details
    });
}

void NesDebugger::OnFrameComplete() {
    if (!enabled || !initialized) return;
    evrPrevFrame.swap(evrLastFrame);
    evrLastFrame.swap(evrActiveFrame);
    evrActiveFrame.clear();
    traceFrame++;

    if (traceCfg.flushEveryFrame)
        FlushTrace();
}

vector<array<string, 3>> NesDebugger::GetStateCPU() const {
    if (!enabled) return {};
    vector<array<string, 3>> lines;

    u16 temp = nes->cpu->pc.value();
    lines.push_back({ "PC (Program Counter)", to_string(temp), hex(temp, 4) });
    temp = nes->cpu->opcode;
    lines.push_back({ "Current Instruction", nes->cpu->lookup[temp].name, hex(temp, 2) });
    temp = nes->cpu->cycles;
    lines.push_back({ "Instruction Cycle", to_string(temp), hex(temp, 2) });
    temp = nes->cpu->a;
    lines.push_back({ "A (accumulator)", to_string(temp), hex(temp, 2) });
    temp = nes->cpu->x;
    lines.push_back({ "X Register", to_string(temp), hex(temp, 2) });
    temp = nes->cpu->y;
    lines.push_back({ "Y Register", to_string(temp), hex(temp, 2) });
    temp = nes->cpu->sp;
    lines.push_back({ "Stack Pointer", hex(0x0100 + temp, 4), hex(temp, 2) });
    temp = nes->cpu->addrBus;
    lines.push_back({ "Address Bus", to_string(temp), hex(temp, 4) });
    temp = nes->cpu->cpuBus;
    lines.push_back({ "Data Bus", to_string(temp), hex(temp, 2) });
    lines.push_back({ "", "STATUS", "" });
    temp = nes->cpu->getFlag(FLAGS::C);
    lines.push_back({ "Carry", temp ? "Set" : "Clear", "" });
    temp = nes->cpu->getFlag(FLAGS::Z);
    lines.push_back({ "Zero", temp ? "Set" : "Clear", "" });
    temp = nes->cpu->getFlag(FLAGS::I);
    lines.push_back({ "Inhibit Interrupts", temp ? "Set" : "Clear", "" });
    temp = nes->cpu->getFlag(FLAGS::D);
    lines.push_back({ "Decimal", temp ? "Set" : "Clear", "" });
    temp = nes->cpu->getFlag(FLAGS::B);
    lines.push_back({ "Break", temp ? "Set" : "Clear", "" });
    temp = nes->cpu->getFlag(FLAGS::V);
    lines.push_back({ "Overflow", temp ? "Set" : "Clear", "" });
    temp = nes->cpu->getFlag(FLAGS::N);
    lines.push_back({ "Negative", temp ? "Set" : "Clear", "" });

    return lines;
}

vector<array<string, 4>> NesDebugger::GetStatePPU() const {
    if (!enabled) return {};
    vector<array<string, 4>> lines;

    u16 temp = 0x00;
    
    lines.push_back({ "", "CONTROL", "", "" });
    temp = nes->ppu->getControlData(CONTROL::NAMETABLE_BASE);
    lines.push_back({ "$2000.0-$2000-1", "Nametable Base Address", hex(nes->ppu->getNametableBase(), 4), hex(temp, 2) });
    temp = nes->ppu->getControlData(CONTROL::VRAM_INCREMENT);
    lines.push_back({ "$2000.2", "Increment Mode", temp == 0 ? "1 byte" : "32 bytes", hex(temp, 2) });
    temp = nes->ppu->getControlData(CONTROL::SPRITE_PATTERN_ADDR);
    lines.push_back({ "$2000.3", "Sprite Table Address", hex(nes->ppu->getSpritePatternTableAddr8x8(), 4), hex(temp, 2) });
    temp = nes->ppu->getControlData(CONTROL::BACKGROUND_PATTERN_ADDR);
    lines.push_back({ "$2000.4", "BG Table Address", hex(nes->ppu->getBackgroundPatternTableAddr(), 4), hex(temp, 2) });
    temp = nes->ppu->getControlData(CONTROL::SPRITE_SIZE);
    lines.push_back({ "$2000.5", "Sprite Size", temp == 0 ? "8x8" : "8x16", hex(temp, 2) });
    temp = nes->ppu->getControlData(CONTROL::MAIN_SECOND);
    lines.push_back({ "$2000.6", "Main/secondary PPU select", temp == 0 ? "Main" : "Secondary", hex(temp, 2) });
    temp = nes->ppu->getControlData(CONTROL::NMI_ENABLED);
    lines.push_back({ "$2000.7", "NMI enabld", temp ? "True" : "False", "" });
    lines.push_back({ "", "MASK", "", "" });
    temp = nes->ppu->getMaskData(MASK::GRAYSCALE);
    lines.push_back({ "$2001.0", "Grayscale", temp ? "True" : "False", "" });
    temp = nes->ppu->getMaskData(MASK::ENABLE_BACKGROUND_LEFT);
    lines.push_back({ "$2001.1", "BG - Show leftmost 8 pixels", temp ? "True" : "False", "" });
    temp = nes->ppu->getMaskData(MASK::ENABLE_SPRITES_LEFT);
    lines.push_back({ "$2001.2", "Sprites - Show leftmost 8 pixels", temp ? "True" : "False", "" });
    temp = nes->ppu->getMaskData(MASK::ENABLE_BACKGROUND);
    lines.push_back({ "$2001.3", "Background enabled", temp ? "True" : "False", "" });
    temp = nes->ppu->getMaskData(MASK::ENABLE_SPRITES);
    lines.push_back({ "$2001.4", "Sprites enabled", temp ? "True" : "False", "" });
    temp = nes->ppu->getMaskData(MASK::EMPHASIZE_RED);
    lines.push_back({ "$2001.5", "Emphasize Red", temp ? "True" : "False", "" });
    temp = nes->ppu->getMaskData(MASK::EMPHASIZE_GREEN);
    lines.push_back({ "$2001.6", "Emphasize Green", temp ? "True" : "False", "" });
    temp = nes->ppu->getMaskData(MASK::EMPHASIZE_BLUE);
    lines.push_back({ "$2001.7", "Emphasize Blue", temp ? "True" : "False", "" });
    lines.push_back({ "", "STATUS", "", "" });
    lines.push_back({ "$2002.5", "Sprite Overflow", nes->ppu->spritesOverflowed() ? "True" : "False", "" });
    lines.push_back({ "$2002.6", "Sprite 0 Hit", nes->ppu->spriteZeroHit() ? "True" : "False", "" });
    lines.push_back({ "$2002.7", "Vertical Blank", nes->ppu->inVBlank() ? "True" : "False", "" });
    lines.push_back({ "", "OTHER", "", "" });
    temp = nes->ppu->OAMADDR;
    lines.push_back({ "$2003", "OAM Address", to_string(temp), hex(temp, 2) });
    temp = nes->ppu->OAMDATA;
    lines.push_back({ "$2004", "OAM Data", to_string(temp), hex(temp, 2) });
    temp = nes->ppu->PPUSCROLL;
    lines.push_back({ "$2005", "PPU Scroll", to_string(temp), hex(temp, 2) });
    temp = nes->ppu->v;
    lines.push_back({ "$2006", "VRAM Address", to_string(temp), hex(temp, 4) });
    temp = nes->ppu->PPUDATA;
    lines.push_back({ "$2007", "VRAM Data", to_string(temp), hex(temp, 2) });
    temp = nes->ppu->scanline;
    lines.push_back({ "--", "Scanline (V)", to_string(temp), hex(temp, 4) });
    temp = nes->ppu->cycle;
    lines.push_back({ "--", "Cycle (H)", to_string(temp), hex(temp, 4) });

    return lines;
}

vector<string> NesDebugger::GetStateRAM() const {
    if (!enabled) return {};
    // for NES; there will be 112 lines of RAM; each having 16 bytes
    vector<string> lines;
    lines.reserve(112);

    // WRAM window
    for (u8 l = 0; l < 112; l++) {
        // calculate start address (0x0010, 0x0010, 0x0020, etc)
        u16 start = (u16)l << 4;
        // create streams to hold the data
        stringstream bytes;
        stringstream chars;
        // add each byte to our data streams
        for (u8 i = 0x00; i < 0x10; i++) {
            u16 addr = start + i;
            u8 byte = nes->cpu->read(addr, true);
            bytes << hex(byte, 2);
            chars << (char)byte;
            if (i < 0x0F) {
                bytes << " ";
                chars << " ";
            }
        }
        // create stream so we can turn everything into a string to put into our `lines` array
        stringstream ss;
        // put the start address first
        ss << hex(start, 4) << ": " << bytes.str() << "     " << chars.str();
        // put the created string into our `lines` array
        lines.push_back(ss.str());
    }

    // ROM window
    for (u8 l = 0; l < 16; l++) {
        u16 start = 0xFF00 + (l * 0x10);
        stringstream bytes;
        stringstream chars;
        for (u8 i = 0x00; i < 0x10; i++) {
            u16 addr = start + i;
            u8 byte = nes->cpu->read(addr, true);
            bytes << hex(byte, 2);
            chars << (char)byte;
            if (i < 0x0F) {
                bytes << ' ';
                chars << ' ';
            }
        }
        stringstream ss;
        ss << hex(start, 4) << ": " << bytes.str() << "     " << chars.str();
        lines.push_back(ss.str());
    }

    return lines;
}

string NesDebugger::DisassembleInstruction(u16 addr) const {
    if (!enabled) return "";
    u16 line_addr = addr;
    u8 oc = nes->cpu->read(addr++, true);
    u8 val = 0x00;
    s8 sval = 0;
    u8 addr8 = 0x00;
    ADDR addr16 = 0x0000;
    ADDR ind = 0x0000;

    stringstream ss;
    ss << nes->cpu->lookup[oc].name << " "; // print instruction name
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
            val = nes->cpu->read(addr++, true);
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
            addr8 = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr8, 2); // print zero page address
            ss << " = " << hex(nes->cpu->read(addr8, true), 2); // print value at that address
            break;
        case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x94: case 0x95: case 0xB4: case 0xB5:
        case 0xD4: case 0xD5: case 0xD6: case 0xD7:
        case 0xF4: case 0xF5: case 0xF6: case 0xF7: // Zero Page,X
            addr8 = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr8, 2) << ",X"; // print zero page address and index register
            addr8 += nes->cpu->x;
            ss << " @ " << hex(addr8, 2); // print actual zero page address
            ss << " = " << hex(nes->cpu->read(addr8, true), 2); // print value at that address
            break;
        case 0x96: case 0x97: case 0xB6: case 0xB7: // Zero Page,Y
            addr8 = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr8, 2) << ",Y"; // print zero page address and index register
            addr8 += nes->cpu->y;
            ss << " @ " << hex(addr8, 2); // print actual zero page address
            ss << " = " << hex(nes->cpu->read(addr8, true), 2); // print value at that address
            break;
        case 0x0C: case 0x0D: case 0x0E: case 0x0F:
        case 0x20: case 0x2C: case 0x2D: case 0x2E:
        case 0x2F: case 0x4C: case 0x4D: case 0x4E:
        case 0x4F: case 0x6D: case 0x6E: case 0x6F:
        case 0x8C: case 0x8D: case 0x8E: case 0x8F:
        case 0xAC: case 0xAD: case 0xAE: case 0xAF:
        case 0xCC: case 0xCD: case 0xCE: case 0xCF:
        case 0xEC: case 0xED: case 0xEE: case 0xEF: // Absolute
            addr16.lo = nes->cpu->read(addr++, true);
            addr16.hi = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr16.value(), 4); // print the absolute address
            if (oc != 0x4C && oc != 0x20) // if this instruction is not JMP or JSR:
                ss << " = " << hex(nes->cpu->read(addr16.value(), true), 2); // print the value at that address
            break;
        case 0x1C: case 0x1D: case 0x1E: case 0x1F:
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        case 0x7C: case 0x7D: case 0x7E: case 0x7F:
        case 0x9C: case 0x9D: case 0xBC: case 0xBD:
        case 0xDC: case 0xDD: case 0xDE: case 0xDF:
        case 0xFC: case 0xFD: case 0xFE: case 0xFF: // Absolute,X
            addr16.lo = nes->cpu->read(addr++, true);
            addr16.hi = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr16.value(), 4) << ",X"; // print the absolute address and the index register
            addr16.add(nes->cpu->x);
            ss << " @ " << hex(addr16.value(), 4); // print the actual absolute address
            ss << " = " << hex(nes->cpu->read(addr16.value(), true), 2); // print the value at that address
            break;
        case 0x19: case 0x1B: case 0x39: case 0x3B:
        case 0x59: case 0x5B: case 0x79: case 0x7B:
        case 0x99: case 0x9B: case 0x9E: case 0x9F:
        case 0xB9: case 0xBB: case 0xBE: case 0xBF:
        case 0xD9: case 0xDB: case 0xF9: case 0xFB: // Absolute,Y
            addr16.lo = nes->cpu->read(addr++, true);
            addr16.hi = nes->cpu->read(addr++, true);
            ss << "$" << hex(addr16.value(), 4) << ",Y"; // print the absolute address and the index register
            addr16.add(nes->cpu->y);
            ss << " @ " << hex(addr16.value(), 4); // print the actual absolute address
            ss << " = " << hex(nes->cpu->read(addr16.value(), true), 2); // print the value at that address
            break;
        case 0x6C: // Indirect
            ind.lo = nes->cpu->read(addr++, true);
            ind.hi = nes->cpu->read(addr++, true);
            ss << "($" << hex(ind.value(), 4) << ")"; // print the indirect address
            addr16.lo = nes->cpu->read(ind, true);
            ind.lo++;
            addr16.hi = nes->cpu->read(ind, true);
            ss << " = " << hex(addr16.value(), 4); // print the absolute address
            break;
        case 0x01: case 0x03: case 0x21: case 0x23:
        case 0x41: case 0x43: case 0x61: case 0x63:
        case 0x81: case 0x83: case 0xA1: case 0xA3:
        case 0xC1: case 0xC3: case 0xE1: case 0xE3: // (Indirect,X)
            val = nes->cpu->read(addr++, true);
            ss << "($" << hex(val, 2) << ",X)"; // print the indirect zero page address and the index register
            val += nes->cpu->x;
            ss << " @ " << hex(val, 2); // print the actual zero page address
            addr16.lo = nes->cpu->read(val++, true);
            addr16.hi = nes->cpu->read(val++, true);
            ss << " = " << hex(addr16.value(), 4); // print the absolute address
            val = nes->cpu->read(addr16.value(), true);
            ss << " = " << hex(val, 2); // print the value at that address
            break;
        case 0x11: case 0x13: case 0x31: case 0x33:
        case 0x51: case 0x53: case 0x71: case 0x73:
        case 0x91: case 0x93: case 0xB1: case 0xB3:
        case 0xD1: case 0xD3: case 0xF1: case 0xF3: // (Indirect),Y
            val = nes->cpu->read(addr++, true);
            ss << "($" << hex(val, 2) << "),Y"; // print the indirect zero page address and the index register
            addr16.lo = nes->cpu->read(val++, true);
            addr16.hi = nes->cpu->read(val++, true);
            ss << " = " << hex(addr16.value(), 4); // print the absolute address
            addr16.add(nes->cpu->y);
            ss << " @ " << hex(addr16.value(), 4); // print the actual absolute address
            val = nes->cpu->read(addr16.value(), true);
            ss << " = " << hex(val, 2); // print the value at that address
            break;
        case 0x10: case 0x30: case 0x50: case 0x70:
        case 0x90: case 0xB0: case 0xD0: case 0xF0: // Relative
            sval = nes->cpu->read(addr++, true);
            addr16 = addr;
            addr16.add_s(sval);
            ss << "$" << hex(addr16.value(), 4); // print the offset value
            break;
    }

    return ss.str();
}

void NesDebugger::ScanInstructions(array<u64, 25>& list) const {
    if (!enabled) return;
    int index = 0;
    for (const u16& e : nes->cpu->prevInstAddrs)
        list[index++] = e;

    // at this point, `index` should be 12;
    // so we can simply use `11` to get the last entry
    // and not have to deal with VS giving us irrelevant
    // or impossible warnings and errors
    u16 start = (u16)list[11];
    while (index < 25) {
        start += nes->cpu->lookup[nes->cpu->read(start, true)].bytes;
        list[index++] = start;
    }
}

vector<string> NesDebugger::GetDisassembly() const {
    if (!enabled) return {};
    array<u64, 25> addrs;
    ScanInstructions(addrs);

    vector<string> lines = {};

    for (u8 i = 0; i < addrs.size(); i++) {
        u16 addr = (u16)addrs[i];
        string str = "$" + hex(addr, 4) + ": " + DisassembleInstruction(addr);
        lines.push_back(str);
    }

    return lines;
}

vector<u8> NesDebugger::GetPaletteIndices() const {
    if (!enabled) return {};
    vector<u8> indices;
    for (u8 i = 0; i < 32; i++)
        indices.push_back(nes->ppu->ppuRead(0x3F00 + i, true) & 0x3F);
    return indices;
}

vector<u32> NesDebugger::GetPaletteColors(const vector<u8>& indices) const {
    if (!enabled) return {};
    vector<u32> colors;
    for (const u8& index : indices)
        colors.push_back(nes->ppu->masterPalette[index]);
    return colors;
}

const PaletteData NesDebugger::GetPaletteData() const {
    if (!enabled) return {};
    vector<u8> indices = GetPaletteIndices();
    vector<u32> colors = GetPaletteColors(indices);

    return { colors, indices, 4, 8, 4, 4 };
}

vector<u32> NesDebugger::GetPatternTable(int tableID, int paletteID) {
    vector<u32> pixels(16384, 0xFF000000);
    
    if (!enabled) return pixels;

    u16 baseAddr = tableID * 0x1000;

    // get the current color palette based on the provided index
    u16 paletteOffset = (paletteID < 4) ?
            paletteID * 4 : 0x10 + (paletteID - 4) * 4;

    for (u16 tileIndex = 0; tileIndex < 256; tileIndex++) {
        u8 tileX = (tileIndex % 16);
        u8 tileY = (tileIndex / 16);

        for (u8 row = 0; row < 8; row++) {
            u8 p0 = nes->ppu->ppuRead(baseAddr + tileIndex * 16 + row);
            u8 p1 = nes->ppu->ppuRead(baseAddr + tileIndex * 16 + row + 8);

            for (u8 col = 0; col < 8; col++) {
                u8 b0 = (p0 >> (7 - col)) & 0x01;
                u8 b1 = (p1 >> (7 - col)) & 0x01;

                u8 colorIndex = (b1 << 1) | b0;

                u16 paletteAddr = 0x3F00 + paletteOffset + colorIndex;
                u8 paletteEntry = nes->ppu->ppuRead(paletteAddr, true) & 0x3F;
                u32 color = nes->ppu->masterPalette[paletteEntry];

                u16 pixelX = tileX * 8 + col;
                u16 pixelY = tileY * 8 + row;

                pixels[(size_t)(pixelY * 128 + pixelX)] = color;
            }
        }
    }

    return pixels;
}

string NesDebugger::GetFlags(int status) const {
    if (!enabled) return "0 0 0 0 0 0 0 0";
    stringstream ss;
    ss << ((status & 0x80) > 0 ? "1 " : "0 ");
    ss << ((status & 0x40) > 0 ? "1 " : "0 ");
    ss << ((status & 0x20) > 0 ? "1 " : "0 ");
    ss << ((status & 0x10) > 0 ? "1 " : "0 ");
    ss << ((status & 0x08) > 0 ? "1 " : "0 ");
    ss << ((status & 0x04) > 0 ? "1 " : "0 ");
    ss << ((status & 0x02) > 0 ? "1 " : "0 ");
    ss << ((status & 0x01) > 0 ? "1 " : "0 ");
    return ss.str();
}

vector<u32> NesDebugger::GetNameTable(int id) {
    vector<u32> pixels(256 * 240, 0xFF000000);
    if (!enabled) return pixels;

    u16 ntBase = 0x2000 + (id * 0x0400);
    u16 bgPatternBase = nes->ppu->getBackgroundPatternTableAddr();

    for (size_t tileY = 0; tileY < 30; tileY++) {
        for (size_t tileX = 0; tileX < 32; tileX++) {
            u16 ntOffset = tileY * 32 + tileX;
            u8 tileIndex = nes->ppu->ppuRead(ntBase + ntOffset, true);

            u16 attrAddr =
                ntBase + 0x03C0 +
                ((tileY / 4) * 8) +
                (tileX / 4);

            u8 attrByte = nes->ppu->ppuRead(attrAddr, true);

            u8 shift =
                ((tileY & 0x02) ? 4 : 0) +
                ((tileX & 0x02) ? 2 : 0);

            u8 paletteIndex = (attrByte >> shift) & 0x03;
            u8 paletteBase = paletteIndex * 4;

            u16 tileAddr = bgPatternBase + (tileIndex * 16);

            for (u8 row = 0; row < 8; row++) {
                u8 p0 = nes->ppu->ppuRead(tileAddr + row, true);
                u8 p1 = nes->ppu->ppuRead(tileAddr + row + 8, true);

                for (u8 col = 0; col < 8; col++) {
                    u8 b0 = (p0 >> (7 - col)) & 1;
                    u8 b1 = (p1 >> (7 - col)) & 1;
                    u8 colorIndex = (b1 << 1) | b0;

                    u16 paletteAddr = 0x3F00;
                    if (colorIndex != 0)
                        paletteAddr += paletteBase + colorIndex;

                    u8 masterIndex = nes->ppu->ppuRead(paletteAddr, true) & 0x3F;
                    u32 color = nes->ppu->masterPalette[masterIndex];

                    u16 pixelX = tileX * 8 + col;
                    u16 pixelY = tileY * 8 + row;
                    pixels[pixelY * 256 + pixelX] = color;
                }
            }
        }
    }

    return pixels;
}

vector<u32> NesDebugger::GetSprites() const {
    const u8 sprW = 8;
    const u8 sprH = nes->ppu->getSpriteHeight();
    const u16 atlW = 8 * sprW;
    const u16 atlH = 8 * sprH;

    vector<u32> pixels(size_t(atlW) * atlH, 0x00000000);
    if (!enabled) return pixels;

    for (u8 spr = 0; spr < 64; spr++) {
        const array<u8, 4>& oam = nes->ppu->primaryOAM[spr];
        const u8 tile = oam[1];
        const u8 attr = oam[2];

        const u16 dstBaseX = (spr % 8) * 8;
        const u16 dstBaseY = (spr / 8) * sprH;

        const bool flipX = nes->ppu->flipX(attr);
        const bool flipY = nes->ppu->flipY(attr);
        const u8 pal = nes->ppu->getSpritePalette(attr);

        for (u8 row = 0; row < sprH; row++) {
            u8 sampleY = flipY ? (sprH - 1 - row) : row;

            u16 pattLo = 0;
            if (sprH == 8) {
                pattLo =
                    nes->ppu->getSpritePatternTableAddr8x8() +
                    (u16(tile) * 16) +
                    sampleY;
            } else {
                u16 table = (tile & 0x01) ? 0x1000 : 0x0000;
                u16 tileBase = tile & 0xFE;
                if (sampleY >= 8) {
                    tileBase += 1;
                    sampleY -= 8;
                }
                pattLo = table + (tileBase * 16) + sampleY;
            }

            u8 p0 = nes->ppu->ppuRead(pattLo, true);
            u8 p1 = nes->ppu->ppuRead(pattLo + 8, true);

            for (u8 col = 0; col < 8; col++) {
                const u8 bit = flipX ? col : (7 - col);
                const u8 colorIndex = (((p1 >> bit) & 1) << 1) | ((p0 >> bit) & 1);

                u32 color = 0x00000000;
                if (colorIndex != 0) {
                    const u16 palAddr = 0x3F10 + (pal * 4) + colorIndex;
                    const u8 masterIndex = nes->ppu->ppuRead(palAddr, true) & 0x3F;
                    color = nes->ppu->masterPalette[masterIndex];
                }

                pixels[(dstBaseY + row) * atlW + (dstBaseX + col)] = color;
            }
        }
    }

    return pixels;
}

vector<array<string, 4>> NesDebugger::GetStateAPU() const {
    if (!enabled) return {};
    vector<array<string, 4>> lines;

    // PULSE 1
    lines.push_back({ "", "Square 1", "", "" });
    u32 temp = nes->apu->pulse1->envelope.decay;
    lines.push_back({ "$4000.0-3", "Envelope Volume", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse1->envelope.constVol;
    lines.push_back({ "$4000.4", "Envelope - Constant Volume", temp ? "True" : "False", "" });
    temp = nes->apu->pulse1->lengthCounter.halt;
    lines.push_back({ "$4000.5", "Length Counter - Halted", temp ? "True" : "False", "" });
    temp = nes->apu->pulse1->dutyMode;
    lines.push_back({ "$4000.6-7", "Duty", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse1->sweep.shift;
    lines.push_back({ "$4001.0-2", "Sweep - Shift", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse1->sweep.negate;
    lines.push_back({ "$4001.3", "Sweep - Negate", temp ? "True" : "False", "" });
    temp = nes->apu->pulse1->sweep.period;
    lines.push_back({ "$4001.4-6", "Sweep - Period", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse1->sweep.enabled;
    lines.push_back({ "$4001.7", "Sweep - Enabled", temp ? "True" : "False", "" });
    temp = nes->apu->pulse1->period;
    lines.push_back({ "$4002/$4003.0-2", "Period", to_string(temp), hex(temp, 4) });
    temp = nes->apu->pulse1->lengthCounter.reloadVal;
    lines.push_back({ "$4003.3-7", "Length Counter - Reload Value", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse1->enabled;
    lines.push_back({ "--", "Enabled", temp ? "True" : "False", "" });
    temp = nes->apu->pulse1->timer;
    lines.push_back({ "--", "Timer", to_string(temp), hex(temp, 4) });
    temp = GetCpuClockRate(nes->cart->region) / (16 * (temp + 1));
    lines.push_back({ "--", "Frequency", to_string(temp) + " Hz", "" });
    temp = nes->apu->pulse1->dutyStep;
    lines.push_back({ "--", "Duty Position", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse1->lengthCounter.counter;
    lines.push_back({ "--", "Length Counter - Counter", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse1->envelope.period;
    lines.push_back({ "--", "Envelope - Period", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse1->envelope.divider;
    lines.push_back({ "--", "Envelope - Divider", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse1->output();
    lines.push_back({ "--", "Output", to_string(temp), hex(temp, 2) });

    // PULSE 2
    lines.push_back({ "", "Square 2", "", "" });
    temp = nes->apu->pulse2->envelope.decay;
    lines.push_back({ "$4004.0-3", "Envelope Volume", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse2->envelope.constVol;
    lines.push_back({ "$4004.4", "Envelope - Constant Volume", temp ? "True" : "False", "" });
    temp = nes->apu->pulse2->lengthCounter.halt;
    lines.push_back({ "$4004.5", "Length Counter - Halted", temp ? "True" : "False", "" });
    temp = nes->apu->pulse2->dutyMode;
    lines.push_back({ "$4004.6-7", "Duty", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse2->sweep.shift;
    lines.push_back({ "$4005.0-2", "Sweep - Shift", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse2->sweep.negate;
    lines.push_back({ "$4005.3", "Sweep - Negate", temp ? "True" : "False", "" });
    temp = nes->apu->pulse2->sweep.period;
    lines.push_back({ "$4005.4-6", "Sweep - Period", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse2->sweep.enabled;
    lines.push_back({ "$4005.7", "Sweep - Enabled", temp ? "True" : "False", "" });
    temp = nes->apu->pulse2->period;
    lines.push_back({ "$4006/$4007.0-2", "Period", to_string(temp), hex(temp, 4) });
    temp = nes->apu->pulse2->lengthCounter.reloadVal;
    lines.push_back({ "$4007.3-7", "Length Counter - Reload Value", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse2->enabled;
    lines.push_back({ "--", "Enabled", temp ? "True" : "False", "" });
    temp = nes->apu->pulse2->timer;
    lines.push_back({ "--", "Timer", to_string(temp), hex(temp, 4) });
    temp = GetCpuClockRate(nes->cart->region) / (16 * (temp + 1));
    lines.push_back({ "--", "Frequency", to_string(temp) + " Hz", "" });
    temp = nes->apu->pulse2->dutyStep;
    lines.push_back({ "--", "Duty Position", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse2->lengthCounter.counter;
    lines.push_back({ "--", "Length Counter - Counter", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse2->envelope.period;
    lines.push_back({ "--", "Envelope - Period", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse2->envelope.divider;
    lines.push_back({ "--", "Envelope - Divider", to_string(temp), hex(temp, 2) });
    temp = nes->apu->pulse2->output();
    lines.push_back({ "--", "Output", to_string(temp), hex(temp, 2) });

    // TRIANGLE
    lines.push_back({ "", "Triangle", "", "" });
    temp = nes->apu->triangle->linearPeriod;
    lines.push_back({ "$4008.0-6", "Linear Counter - Reload Value", to_string(temp), hex(temp, 2) });
    temp = nes->apu->triangle->lengthCounter.halt;
    lines.push_back({ "$4008.7", "Length Counter - Halted", temp ? "True" : "False", "" });
    temp = nes->apu->triangle->period;
    lines.push_back({ "$400A/$400B.0-2", "Period", to_string(temp), hex(temp, 4) });
    temp = nes->apu->triangle->lengthCounter.reloadVal;
    lines.push_back({ "$400B.3-7", "Length Counter - Reload Value", to_string(temp), hex(temp, 2) });
    temp = nes->apu->triangle->enabled;
    lines.push_back({ "--", "Enabled", temp ? "True" : "False", "" });
    temp = nes->apu->triangle->timer;
    lines.push_back({ "--", "Timer", to_string(temp), hex(temp, 4) });
    temp = GetCpuClockRate(nes->cart->region) / (32 * (temp + 1));
    lines.push_back({ "--", "Frequency", to_string(temp) + " Hz", "" });
    temp = nes->apu->triangle->step;
    lines.push_back({ "--", "Sequence Position", to_string(temp), hex(temp, 2) });
    temp = nes->apu->triangle->lengthCounter.counter;
    lines.push_back({ "--", "Length Counter - Counter", to_string(temp), hex(temp, 2) });
    temp = nes->apu->triangle->linearCounter;
    lines.push_back({ "--", "Linear Counter - Counter", to_string(temp), hex(temp, 2) });
    temp = nes->apu->triangle->linearReload;
    lines.push_back({ "--", "Linear Counter - Reload Flag", temp ? "True" : "False", "" });
    temp = nes->apu->triangle->output();
    lines.push_back({ "--", "Output", to_string(temp), hex(temp, 2) });

    // NOISE
    lines.push_back({ "", "Noise", "", "" });
    temp = nes->apu->noise->envelope.decay;
    lines.push_back({ "$400C.0-3", "Envelope Volume", to_string(temp), hex(temp, 2) });
    temp = nes->apu->noise->envelope.constVol;
    lines.push_back({ "$400C.4", "Envelope - Constant Volume", temp ? "True" : "False", "" });
    temp = nes->apu->noise->lengthCounter.halt;
    lines.push_back({ "$400C.5", "Length Counter - Halted", temp ? "True" : "False", "" });
    temp = nes->apu->noise->period;
    lines.push_back({ "$400E.0-3", "Period", to_string(temp), hex(temp, 2) });
    temp = nes->apu->noise->mode;
    lines.push_back({ "$400E.7", "Mode Flag", temp ? "True" : "False", "" });
    temp = nes->apu->noise->lengthCounter.reloadVal;
    lines.push_back({ "$400F.3-7", "Length Counter - Reload Value", to_string(temp), hex(temp, 2) });
    temp = nes->apu->noise->enabled;
    lines.push_back({ "--", "Enabled", temp ? "True" : "False", "" });
    temp = nes->apu->noise->timer;
    lines.push_back({ "--", "Timer", to_string(temp), hex(temp, 4) });
    temp = GetCpuClockRate(nes->cart->region) / nes->apu->noise->period;
    lines.push_back({ "--", "Frequency", to_string(temp) + " Hz", "" });
    temp = nes->apu->noise->shiftRegister;
    lines.push_back({ "--", "Shift Register", to_string(temp), hex(temp, 4) });
    temp = nes->apu->noise->envelope.period;
    lines.push_back({ "--", "Envelope - Period", to_string(temp), hex(temp, 2) });
    temp = nes->apu->noise->envelope.divider;
    lines.push_back({ "--", "Envelope - Divider", to_string(temp), hex(temp, 2) });
    temp = nes->apu->noise->lengthCounter.counter;
    lines.push_back({ "--", "Length Counter - Counter", to_string(temp), hex(temp, 2) });
    temp = nes->apu->noise->output();
    lines.push_back({ "--", "Output", to_string(temp), hex(temp, 2) });

    // DMC
    //lines.push_back({ "", "DMC", "", "" });
    //temp = nes->apu->dmc->period;
    //lines.push_back({ "$4010.0-3", "Period", to_string(temp), hex(temp, 4) });
    //temp = nes->apu->dmc->loop;
    //lines.push_back({ "$4010.6", "Loop Flag", temp ? "True" : "False", "" });
    //temp = nes->apu->dmc->irqEnabled;
    //lines.push_back({ "$4010.7", "IRQ Enabled", temp ? "True" : "False", "" });
    //temp = nes->apu->dmc->outputLevel;
    //lines.push_back({ "$4011", "Output Level", to_string(temp), hex(temp, 2) });
    //temp = nes->apu->dmc->sampleAddr;
    //lines.push_back({ "$4012", "Sample Address", to_string(temp), hex(temp, 4) });
    //temp = nes->apu->dmc->sampleLength;
    //lines.push_back({ "$4013", "Sample Length", to_string(temp), hex(temp, 4) });
    //temp = nes->apu->dmc->currAddr;
    //lines.push_back({ "--", "Next Sample Address", to_string(temp), hex(temp, 4) });
    //temp = nes->apu->dmc->bytesRemaining;
    //lines.push_back({ "--", "Sample Bytes Remaining", to_string(temp), hex(temp, 4) });
    //temp = nes->apu->dmc->timer;
    //lines.push_back({ "--", "Timer", to_string(temp), hex(temp, 4) });
    //temp = GetCpuClockRate(nes->cart->region) / nes->apu->dmc->period;
    //lines.push_back({ "--", "Frequency", to_string(temp) + " Hz", ""});

    // FRAME COUNTER
    lines.push_back({ "", "Frame Counter", "", "" });
    temp = nes->apu->inhibitIRQ;
    lines.push_back({ "$4017.6", "IRQ Enabled", temp ? "False" : "True", "" });
    temp = nes->apu->use5step;
    lines.push_back({ "$4017.7", "5-step Mode", temp ? "True" : "False", "" });
    temp = nes->apu->cycle;
    if (temp <= 7457)
        temp = 0;
    else if (temp <= 14913)
        temp = 1;
    else if (temp <= 22371)
        temp = 2;
    else
        temp = 3;
    lines.push_back({ "--", "Sequence Position", to_string(temp), hex(temp, 2) });

    return lines;
}

// TODO debug Pulse1 channel
vector<u32> NesDebugger::GetPulse1() {
    if (!enabled) return {};
    return vector<u32>();
}

// TODO debug Pulse2 channel
vector<u32> NesDebugger::GetPulse2() {
    if (!enabled) return {};
    return vector<u32>();
}

// TODO debug Triangle channel
vector<u32> NesDebugger::GetTriangle() {
    if (!enabled) return {};
    return vector<u32>();
}

// TODO debug Noise channel
vector<u32> NesDebugger::GetNoise() {
    if (!enabled) return {};
    return vector<u32>();
}

// TODO debug DMC channel
vector<u32> NesDebugger::GetDMC() {
    if (!enabled) return {};
    return vector<u32>();
}

vector<array<string, 2>> NesDebugger::GetPakHeader() const {
    if (!enabled || !initialized) return {};
    vector<array<string, 2>> values;

    switch (nes->cart->hFormat) {
        case HeaderFormat::ANES:
            values.push_back({ "Header Format", "Archaic iNES or iNES 0.7" });
            break;
        case HeaderFormat::INES:
            values.push_back({ "Header Format", "iNES 1.0" });
            break;
        case HeaderFormat::NES2:
            values.push_back({ "Header Format", "iNES 2.0" });
            break;
        default:
            values.push_back({ "Header Format", "Unknown Format" });
            break;
    }
    switch (nes->cart->cType) {
        case ConsoleType::NES_FAMICOM:
            values.push_back({ "Console Type", "NES/Famicom" });
            break;
        case ConsoleType::VS_SYSTEM:
            values.push_back({ "Console Type", "Nintendo Vs. System" });
            break;
        case ConsoleType::PLAYCHOICE_10:
            values.push_back({ "Console Type", "Playchoise 10" });
            break;
        case ConsoleType::FAMICLONE_DECIMAL:
            values.push_back({ "Console Type", "Regular Famiclone (CPU supports Decimal Mode)" });
            break;
        case ConsoleType::NES_EPSM:
            values.push_back({ "Console Type", "Regular NES/Famicom (EPSM module or plug-through cartridge)" });
            break;
        case ConsoleType::VR_VT01:
            values.push_back({ "Console Type", "V.R. Technology VT01" });
            break;
        case ConsoleType::VR_VT02:
            values.push_back({ "Console Type", "V.R. Technology VT02" });
            break;
        case ConsoleType::VR_VT03:
            values.push_back({ "Console Type", "V.R. Technology VT03" });
            break;
        case ConsoleType::VR_VT09:
            values.push_back({ "Console Type", "V.R. Technology VT09" });
            break;
        case ConsoleType::VR_VT32:
            values.push_back({ "Console Type", "V.R. Technology VT32" });
            break;
        case ConsoleType::VR_VT369:
            values.push_back({ "Console Type", "V.R. Technology VT369" });
            break;
        case ConsoleType::UMC_UM6578:
            values.push_back({ "Console Type", "UMC UM6578" });
            break;
        case ConsoleType::FNS:
            values.push_back({ "Console Type", "Famicom Network System" });
            break;
        default:
            values.push_back({ "Console Type", "UNIMPLEMENTED TYPE" });
            break;
    }
    switch (nes->cart->region) {
        case ConsoleRegion::NTSC:
            values.push_back({ "Console Region", "NTSC (US/JP)" });
            break;
        case ConsoleRegion::PAL:
            values.push_back({ "Console Region", "PAL (EU)" });
            break;
        case ConsoleRegion::MULTI:
            values.push_back({ "Console Region", "MULTI (WORLD)" });
            break;
        case ConsoleRegion::DENDY:
            values.push_back({ "Console Region", "DENDY (RU)" });
            break;
        default:
            values.push_back({ "Console Region", "UNKNOWN REGION (?\?)" });
            break;
    }
    values.push_back({ "512B Trainer Present", nes->cart->hasTrainer ? "true" : "false" });
    if (nes->cart->prgMemory.size() > 0)
        values.push_back({ "PRG-ROM Size", to_string(nes->cart->prgMemory.size()) });
    if (nes->cart->prgRamNonVolatile.size() > 0)
        values.push_back({ "PRG-RAM (Non-Volatile) Size", to_string(nes->cart->prgRamNonVolatile.size()) });
    if (nes->cart->prgRamVolatile.size() > 0)
        values.push_back({ "PRG-RAM (Volatile) Size", to_string(nes->cart->prgRamVolatile.size()) });
    if (nes->cart->chrMemory.size() > 0)
        values.push_back({ "CHR-ROM Size", to_string(nes->cart->chrMemory.size()) });
    if (nes->cart->chrRamNonVolatile.size() > 0)
        values.push_back({ "CHR-RAM (Non-Volatile) Size", to_string(nes->cart->chrRamNonVolatile.size()) });
    if (nes->cart->chrRamVolatile.size() > 0)
        values.push_back({ "CHR-RAM (Volatile) Size", to_string(nes->cart->chrRamVolatile.size()) });
    switch (nes->cart->mirror) {
        case MIRROR::HORIZONTAL:
            values.push_back({ "Hardware Mirror", "Horizontal" });
            break;
        case MIRROR::VERTICAL:
            values.push_back({ "Hardware Mirror", "Vertical" });
            break;
        case MIRROR::FOUR_SCREEN:
            values.push_back({ "Hardware Mirror", "Four-Screen (or mapper controlled)" });
            break;
        default:
            values.push_back({ "Hardware Mirror", "Unknown Mirror Type" });
            break;
    }
    if (nes->cart->hFormat == HeaderFormat::NES2) {
        if (nes->cart->cType == ConsoleType::VS_SYSTEM) {
            string ppuType = "Unknown Vs. System PPU Type";
            switch (nes->cart->vsPPU) {
                case VsPPU::PPU2C0X: ppuType = "Any RP2C02/RC2C03 Variant"; break;
                case VsPPU::PPU2C04A: ppuType = "RP2C04-0001"; break;
                case VsPPU::PPU2C04B: ppuType = "RP2C04-0002"; break;
                case VsPPU::PPU2C04C: ppuType = "RP2C04-0003"; break;
                case VsPPU::PPU2C04D: ppuType = "RP2C04-0004"; break;
                case VsPPU::PPU2C05A: ppuType = "RC2C05-01 (signature unknown)"; break;
                case VsPPU::PPU2C05B: ppuType = "RC2C05-02 ($2002 AND $3F = $3D)"; break;
                case VsPPU::PPU2C05C: ppuType = "RC2C05-03 ($2002 AND $1F = $1C)"; break;
                case VsPPU::PPU2C05D: ppuType = "RC2C05-04 ($2002 AND $1F = $1B)"; break;
            }
            values.push_back({ "Vs. System PPU Type", ppuType });
        }
        string expDev = "Unspecified Default Expansion Device";
        switch (nes->cart->expDev) {
            case ExpansionDevice::STANDARD_CONTROLLERS:
                expDev = "Standard NES/Famicom Controllers";
                break;
            default:
                expDev = "Displayable Values Still In Prgress";
                break;
        }
        values.push_back({ "Default Expansion (Input) Device", expDev });
    }

    return values;
}

vector<array<string, 2>> NesDebugger::GetPakMapper() const {
    if (!enabled || !initialized) return {};
    return nes->cart->mapper->getDebugData();
}

bool NesDebugger::BeginTrace() {
    if (!enabled || !initialized) return false;
    if (traceFile.is_open()) traceFile.close();
    if (traceCfg.filePath.empty()) return false;

    auto mode = traceCfg.append ? (ios::out | ios::app) : ios::out;
    traceFile.open(traceCfg.filePath, mode);
    if (!traceFile.is_open()) return false;

    traceFrame = 0;
    traceRecords.clear();
    traceTextBuffer.clear();
    traceCfg.enabled = true;
    return true;
}

void NesDebugger::EndTrace() {
    FlushTrace();
    if (traceFile.is_open()) traceFile.close();
}

void NesDebugger::PushTraceRecord(const TraceRecord& rec) {
    if (!enabled || !initialized) return;
    if (!traceFile.is_open()) return;
    traceRecords.push_back(rec);

    if (traceRecords.size() >= traceCfg.flushThresholdRecords)
        FlushTrace();
}

void NesDebugger::FlushTrace() {
    if (!enabled || !initialized) return;
    if (!traceFile.is_open() || traceRecords.empty()) return;

    traceTextBuffer.clear();
    for (const TraceRecord& rec : traceRecords)
        AppendFormattedTraceLine(traceTextBuffer, rec);

    if (traceCfg.traceToFile && traceFile.is_open()) {
        traceFile.write(traceTextBuffer.data(), static_cast<std::streamsize>(traceTextBuffer.size()));
    }

    if (traceCfg.traceToConsole) {
        /// TODO: In the future, the main application will not have an interactible console window by default.
        /// Maybe this should do something other than printf?
        printf("%s", traceTextBuffer.c_str());
    }

    traceRecords.clear();
}

void NesDebugger::AppendFormattedTraceLine(string& out, const TraceRecord& rec) const {
    if (!enabled || !initialized) return;
    out += "F=" + to_string(rec.frame);
    out += " CPUC=" + to_string(rec.cpuCycle);
    out += " PPU=" + to_string(rec.scanline) + "," + to_string(rec.dot);

    if (rec.kind == TraceKind::Instruction) {
        out += " PC=$" + hex(rec.pc, 4);
        out += " OP=$" + hex(rec.opcode, 2);
        out += " :: " + DisassembleInstruction(rec.pc);
        out += " | A:" + hex(rec.a, 2);
        out += " X:" + hex(rec.x, 2);
        out += " Y:" + hex(rec.y, 2);
        out += " SP:" + hex(rec.sp, 2);
        out += " P:" + hex(rec.status, 2);
    } else {
        out += " EVT flags=" + to_string(rec.flags);
        out += " A=$" + hex(rec.address, 4);
        out += " V=$" + hex(rec.value, 2);
        out += " D=" + to_string(rec.detailID);
    }
    out += "\n";
}

void NesDebugger::OnInstructionExecute(u16 pc, u8 oc, u8 a, u8 x, u8 y, u8 sp, u8 status, u64 cycle) {
    if (!enabled || !initialized) return;
    if (!HasTraceMode(traceCfg.mode, DebugTraceMode::INSTRUCTIONS)) return;

    PushTraceRecord({
        .kind = TraceKind::Instruction,
        .frame = traceFrame,
        .cpuCycle = cycle,
        .scanline = nes->ppu->scanline,
        .dot = nes->ppu->cycle,
        .pc = pc,
        .opcode = oc,
        .a = a,
        .x = x,
        .y = y,
        .sp = sp,
        .status = status
    });
}

void NesDebugger::PushEventRecord(DebugEventRecord ev) {
    if (!enabled || !initialized) return;
    evrActiveFrame.push_back(ev);

    if (HasTraceMode(traceCfg.mode, DebugTraceMode::EVENTS)) {
        PushTraceRecord({
            .kind = TraceKind::Event,
            .frame = traceFrame,
            .cpuCycle = nes->cpu->totalCycles,
            .scanline = ev.scanline,
            .dot = ev.cycle,
            .address = static_cast<u16>(ev.address),
            .value = static_cast<u8>(ev.value),
            .flags = ev.flags,
            .detailID = ResolveEventDetailID(ev)
        });
    }
}

u16 NesDebugger::ResolveEventDetailID(const DebugEventRecord& ev) const {
    if (!enabled || !initialized) return 0x0000;
    return 0x0000;
}