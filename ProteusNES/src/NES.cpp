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

bool NES::loadROM(const string& path) {
    cart = make_shared<Gamepak>(path);

    if (cart->isValid()) {
        cpu->connectCART(cart);
        ppu->connectCART(cart);

        cpu->start();

        return true;
    }
    return false;
}

void NES::reset() {
    cpu->reset();
    ppu->reset();
    masterClock = 0x00;
}

void NES::clockCyclePPU() {
    ppu->clock();

    cpu->nmiTrigger |= ppu->nmiRequested;
    ppu->nmiRequested = false;
}

void NES::clockCycleCPU() {
    cart->mapper->cpuclock(cpu->totalCycles);
    cpu->clock();
    apu->clock();

    cpu->irqTrigger |= apu->irqRequested;
    apu->irqRequested = false;
}

void NES::clockMaster() {
    if ((masterClock & 3) == 0) clockCyclePPU();
    if ((masterClock % 12) == 2) clockCycleCPU();
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