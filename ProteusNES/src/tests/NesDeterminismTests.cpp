#include <iostream>
#include <string>
#include <vector>

#include "../../../Proteus/src/shared/Utilities.h"
#include "../NES.h"
#include "../shared/NesTypes.h"

using std::cout;
using std::stoi;
using std::runtime_error;

using namespace NS_NES;

static string Mismatch(string reg, u16 first, u16 now, u8 bits = 2) {
    string f = hex(first, bits);
    string n = hex(now, bits);
    return format("Register {} mismatch! first: {}; now: {}", reg, f, n);
}

static string Mismatch(string reg, string first, string now) {
    return format("");
}

static string to_string(INTERRUPT i) {
    switch (i) {
        case INTERRUPT::NONE:
            return "NONE";
        case INTERRUPT::RST:
            return "RESET";
        case INTERRUPT::NMI:
            return "NMI";
        case INTERRUPT::IRQ:
            return "IRQ";
        case INTERRUPT::BRK:
            return "BRK";
        default:
            return "UKNOWN";
    }
}

static void ExpectEqual(const CPU_STATE& now, const CPU_STATE& first) {
    if (now.pc != first.pc) throw runtime_error(Mismatch("PC", first.pc, now.pc, 4));
    if (now.a != first.a) throw runtime_error(Mismatch("A", first.a, now.a));
    if (now.x != first.x) throw runtime_error(Mismatch("X", first.x, now.x));
    if (now.y != first.y) throw runtime_error(Mismatch("Y", first.y, now.y));
    if (now.sp != first.sp) throw runtime_error(Mismatch("SP", first.sp, now.sp));
    if (now.status != first.status) throw runtime_error(Mismatch("STATUS", first.status, now.status));
    if (now.cycle != first.cycle) throw runtime_error(Mismatch("CYCLE", first.cycle, now.cycle, 8));
    if (now.irqLatch != first.irqLatch) throw runtime_error(Mismatch("IRQ LATCH", first.irqLatch, now.irqLatch, 1));
    if (now.nmiLatch != first.nmiLatch) throw runtime_error(Mismatch("NMI LATCH", first.nmiLatch, now.nmiLatch, 1));
    if (now.interrupt != first.interrupt) {
        string f = to_string(first.interrupt);
        string n = to_string(now.interrupt);
        string e = format("Interrupt mismatch! first: {}; now: {}", f, n);
        throw runtime_error(e);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Test requires exactly two arguments:\nNesDeterminismTests <test-type> <path-to-ROM>";
        return 2;
    }

    string testType(argv[1]);
    string romPath(argv[2]);

    if (testType == "cpu") {
        // get initial snapshots for testing against
        vector<CPU_STATE> initRun;
        initRun.reserve(300);
        NES* nes = new NES();
        nes->loadROM(romPath);
        for (size_t i = 0; i < 300; i++) {
            nes->clockFrame();
            initRun.push_back(nes->GetSnapshotCPU());
        }
        nes->shutdown();
        delete nes;

        // replay runs
        for (int run = 0; run < 5; run++) {
            // reset to known start state before each run
            nes = new NES();
            nes->loadROM(romPath);
            for (size_t i = 0; i < 300; i++) {
                // clock an entire single frame
                nes->clockFrame();
                // collect snapshot
                CPU_STATE now = nes->GetSnapshotCPU();
                // compare to same snapshot from first run
                try {
                    ExpectEqual(now, initRun[i]); // throw error if snapshots don't match
                } catch (runtime_error& e) {
                    cout << e.what() << endl;
                    cout << "Failure at run #" << run << " frame #" << i << endl;
                    delete nes;
                    return 1;
                }
            }
            nes->shutdown();
            delete nes;
        }
    } else if (testType == "frame") {
        
    } else if (testType == "audio") {

    } else {
        cout << argv[1] << " is an invalid argument; use 'cpu' 'frame' or 'audio' to select a test type.";
        return 2;
    }

    return 0;
}