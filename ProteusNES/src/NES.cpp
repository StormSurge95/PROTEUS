#include "./NES.h"
#include "./NesController.h"
#include "./APU/NesAPU.h"

using namespace NS_NES;

NES::NES() {
    cpu = make_shared<CPU>();
    ppu = make_shared<PPU>();
    apu = make_shared<APU>();
    player1 = make_shared<Controller>(1);
    player2 = make_shared<Controller>(2);
    player1->other = player2;
    player2->other = player1;

    cpu->connectPPU(ppu);
    ppu->connectCPU(cpu);

    cpu->connectAPU(apu);
    apu->connectCPU(cpu);

    cpu->connectCONT(player1, 1);
    cpu->connectCONT(player2, 2);
}

void NES::connectEventSink(NesEventSink* s) {
    eventSink = s;
    cpu->connectEventSink(s);
    ppu->connectEventSink(s);
    apu->connectEventSink(s);
    if (cart) cart->connectEventSink(s);
}

bool NES::poweron() {
    // already on: quick-return success
    if (powered) return true;

    // no valid cart: quick-return failure
    if (cart == nullptr || !cart->isValid()) return false;

    // startup timing begins at zero
    masterClock = 0;

    // clear top-level transient runtime flags
    ppu->frameComplete = false;

    // put each device into its power-on state, not reset state
    cart->powerup(seed ^ 0xC001D00D);
    ppu->powerup(seed ^ 0x50505531);
    apu->powerup(seed ^ 0x41505531);
    cpu->powerup(seed ^ 0x43505531);

    // successfully turned on the console
    powered = true;
    return powered;
}

void NES::reset() {
    cart->reset();
    ppu->reset();
    apu->reset();
    cpu->reset();
    masterClock = 0;
}

bool NES::shutdown() {
    ppu->powerdown();
    apu->powerdown();
    cpu->powerdown();
    if (cart) cart->powerdown();
    masterClock = 0;
    powered = false;
    return true;
}

bool NES::loadROM(const string& path) {
    cart = make_shared<Gamepak>(path);

    if (cart->isValid()) {
        if (eventSink) cart->connectEventSink(eventSink);

        // cart is valid; wire it to the other pieces
        cpu->connectCART(cart);
        ppu->connectCART(cart);

        // calculate the PRNG seed based on the current rom
        deriveSeed("NES", path);

        cpu->setRegion(&cart->region);
        ppu->setRegion(&cart->region);
        apu->setRegion(&cart->region);

        // return result of `poweron` sequence
        return poweron();
    }
    return false;
}

void NES::clockCyclePPU() {
    ppu->clock();
    cart->mapper->ppuclock(ppu->cycle);

    cpu->nmiPending |= ppu->nmiRequested;
    ppu->nmiRequested = false;

    cpu->setIrqLine_Mapper(cart->mapper->irqRequestActive());
}

void NES::clockCycleCPU() {
    cart->mapper->cpuclock(cpu->totalCycles);
    cpu->clock();
    apu->clock();
}

void NES::clockMaster() {
    if ((masterClock % GetPpuClockDiv(cart->region)) == 0) clockCyclePPU();
    if ((masterClock % GetCpuClockDiv(cart->region)) == 2) clockCycleCPU();
    masterClock++;
}

void NES::clockFrame() {
    do {
        clockMaster();
    } while (!ppu->frameComplete);

    ppu->frameComplete = false;
}

void NES::clock() {
    steady_clock::time_point start = steady_clock::now();
    clockFrame();
    steady_clock::time_point end = steady_clock::now();
    double sleep = 15.0 - duration<double, milli>(end - start).count();
    if (sleep > 0.0) {
        sleep_for(duration<double, milli>(sleep));
    }
}

void NES::update(u8 player, const bool* buttons) {
    if (player == 0) {
        for (int x = 0; x < 8; x++) {
            player1->update(static_cast<BUTTONS>(x), buttons[x]);
        }
    } else if (player == 1) {
        for (int x = 0; x < 8; x++) {
            player2->update(static_cast<BUTTONS>(x), buttons[x]);
        }
    }
}

void NES::collectAudio(vector<float>& samples) {
    apu->collectSamples(samples);
}

const CPU_STATE NES::GetSnapshotCPU() const {
    return cpu->GetState();
}

void NES::initSST(SingleStateTest::State state) {
    cpu->init(state);
}
void NES::runSST() {
    do {
        cpu->clock();
    } while (cpu->cycles != 0);
}
bool NES::checkSST(SingleStateTest::State state, string& result) {
    return cpu->check(state, result);
}