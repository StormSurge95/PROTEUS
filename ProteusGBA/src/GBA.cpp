#include "./GBA.h"

using namespace NS_GBA;

GBA::GBA() {
    // TODO: construct permanent devices

    // TODO: establish permanent devices/bus connections
}

GBA::~GBA() {
    shutdown();
    // TODO: destroy the various uptr objects
}

bool GBA::loadROM(const string& path) {
    // TODO: construct/validate gamepak
    
    // TODO: connect gamepak

    // TODO: derive deterministic seed
    deriveSeed("GBA", path);

    return poweron();
}

bool GBA::poweron() {
    if (powered) return true;

    if (!pak) return false;

    // TODO: validate game pak

    // TODO: validate required BIOS config

    // TODO: verify that all permanent devices exist

    masterCycle = 0;
    hardwareMode = GB_Version::GBA;

    // TODO: cartridge-slot switch established before BIOS execution

    // TODO: initialize memory, bus, peripherals and clock phases

    // TODO: map boot firmware

    // TODO: hold SM83 inactive

    // TODO: place ARM7TDMI in reset state last
    
    // powered = true;
    return powered;
}

void GBA::reset() {
    masterCycle = 0;
    hardwareMode = GB_Version::GBA;

    // TODO: ARM7TDMI active at the ARM BIOS reset vector
    
    // TODO: SM83 held inactive/reset

    // TODO: cartridge-slot switch re-established before BIOS execution

    // TODO: bus and boot-ROM mapping reinitialized

    // TODO: all divider phases and pending signals reinitialized

    // TODO: all undefined RAM generated deterministically from the seed

    // TODO: all devices reset in a fixed order
}

bool GBA::shutdown() {
    // TODO: disconnect and power down all the GBA devices
    masterCycle = 0;
    powered = false;
    return true;
}

void GBA::clock() {
    steady_clock::time_point start = steady_clock::now();
    clockFrame();
    steady_clock::time_point end = steady_clock::now();
    double sleep = MILLIS_PER_FRAME - duration<double, milli>(end - start).count();
    if (sleep > 0.0) {
        sleep_for(duration<double, milli>(sleep));
    }
}

void GBA::clockFrame() {
    const u64 end = masterCycle + GBA_TICKS_PER_FRAME;
    do {
        clockMaster();
    } while (masterCycle < end); // TODO: replace with ppu "frame complete" flag check

    // TODO: clear the ppu "frame complete" flag
}

void GBA::clockMaster() {
    if (hardwareMode == GB_Version::GBA) {
        clockARM7cycle();
    } else {
        clockSM83cycle();
    }

    masterCycle++;
}

const u32* GBA::getFrameBuffer() const {
    // TODO: complete this once PPU is working
    return nullptr;
}

void GBA::collectAudio(vector<float>& audioData) {
    // TODO: complete this once APU is working
}

void GBA::update(u8 p, const bool* btns) {
    // TODO: complete this once we are capable of receiving button input
}

void GBA::initSST(SingleStateTest::State s) {
    // TODO: complete this once CPU is ready for testing
}

void GBA::runSST() {
    // TODO: complete this once CPU is ready for testing
}

bool GBA::checkSST(SingleStateTest::State s, string& str) {
    // TODO: complete this once CPU is ready for testing
    return false;
}

void GBA::clockARM7cycle() {
    // TODO: complete this once ARM7TDMI cpu has a `clock` function
}

void GBA::clockSM83cycle() {
    // TODO: complete this once SM83 cpu has a `clock` function
}