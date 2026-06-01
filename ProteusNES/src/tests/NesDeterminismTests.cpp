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

/**
 * @brief Helper function to create a formatted string upon mismatch of tested values.
 * @param reg The register being tested.
 * @param first The expected value of the register.
 * @param now The actual value of the register.
 * @param len The nibble-length of the value (e.g.: 2 for 8-bit values, 4 for 16-bit values)
 * @return A formatted string explaining the register that mismatched and the expected/actual values.
 */
static string Mismatch(string reg, u16 first, u16 now, u8 len = 2) {
    string f = hex(first, len);
    string n = hex(now, len);
    return format("Register {} mismatch! first: {}; now: {}", reg, f, n);
}

/// @brief Helper function to get string representation of the various interrupt types.
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
            return "UNKNOWN";
    }
}

/**
 * @brief Helper function to determine equivalence of two CPU_STATE objects.
 * @param now The actual CPU_STATE
 * @param first The expected CPU_STATE
 * @details Rather than return the state of equivalence, this function will
 *          simply throw an exception if the two values are not equal. This
 *          allows us imply equivalence and only "return" a log for bug-fixing
 *          if there is an actual mismatch.
 * @note This function should only be called within a try/catch that will use
 *       the thrown error (if one occurs) to log the reason for mismatch and
 *       aid in the triage of bugs/errors within the emulator.
 */
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

/**
 * @brief Helper function to calculate the FNV-1a hash of a provided set of integer values.
 * @param buf Pointer to an array of integer values representing screen pixels produced by the emulator.
 * @return Calculated 64-bit hash value.
 * @details This function calculates a 64-bit hash value by accepting an array of 32-bit
 *          integer values and then processing each byte of the array indiviually.
 */
static u64 Hash(const u32* buf) {
    const u8* bytes = reinterpret_cast<const u8*>(buf);
    size_t count = static_cast<size_t>(PIXEL_COUNT) * sizeof(u32);
    u64 hash = 0xCBF2'9CE4'8422'2325;
    for (size_t i = 0; i < count; i++) {
        hash ^= bytes[i];
        hash *= 0x0100'0000'01B3;
    }
    return hash;
}

/**
 * @brief Helper function to calculate the FNV-1a hash of a provided set of float values.
 * @param vec Vector object reference containing float values that represent audio output produced by the emulator.
 * @return Calculated 64-bit hash value.
 * @details This function calculates a 64-bit hash value by accepting a vector of float values
 *          and then processing each byte of the corresponding data array individually.
 * @note Due to the possibility of floating-point noise produced by various operating system platforms,
 *       modification of this function may be required in the event of producing and testing against
 *       some form of "golden" result log.
 */
static u64 Hash(const vector<float>& vec) {
    const u8* bytes = reinterpret_cast<const u8*>(vec.data());
    size_t count = vec.size() * sizeof(float);
    u64 hash = 0xCBF2'9CE4'8422'2325;
    for (size_t i = 0; i < count; i++) {
        hash ^= bytes[i];
        hash *= 0x0100'0000'01B3;
    }
    return hash;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Test requires exactly two arguments:\nNesDeterminismTests <test-type> <path-to-ROM>";
        return 2;
    }

    string testType(argv[1]);
    string romPath(argv[2]);

    size_t TOTAL_FRAMES = 300;
    size_t TEST_RUNS = 3;

    if (testType == "cpu") {
        // get initial snapshots for testing against
        vector<CPU_STATE> initRun;
        initRun.reserve(TOTAL_FRAMES);
        NES* nes = new NES();
        if (!nes->loadROM(romPath)) {
            cout << "Failed to load ROM from '" << romPath << "'";
            return 3;
        }
        for (size_t i = 0; i < TOTAL_FRAMES; i++) {
            nes->clockFrame();
            initRun.push_back(nes->GetSnapshotCPU());
        }
        nes->shutdown();
        delete nes;

        #ifdef SELF_TEST
        initRun[42].pc ^= 0x1010101010101010;
        #endif

        // replay runs
        for (int run = 0; run < TEST_RUNS; run++) {
            nes = new NES();
            if (!nes->loadROM(romPath)) {
                cout << "Failed to load ROM from '" << romPath << "'";
                return 3;
            }
            for (size_t i = 0; i < TOTAL_FRAMES; i++) {
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
                    nes->shutdown();
                    delete nes;
                    return 1;
                }
            }
            nes->shutdown();
            delete nes;
        }
    } else if (testType == "frame") {
        // create container for frame hashes
        vector<u64> initRun;
        initRun.reserve(TOTAL_FRAMES);
        NES* nes = new NES();
        if (!nes->loadROM(romPath)) {
            cout << "Failed to load ROM from '" << romPath << "'";
            return 3;
        }
        for (size_t i = 0; i < TOTAL_FRAMES; i++) {
            nes->clockFrame();
            initRun.push_back(Hash(nes->getFrameBuffer()));
        }
        nes->shutdown();
        delete nes;

        #ifdef SELF_TEST
        initRun[42] ^= 0x1010101010101010;
        #endif

        // replay runs
        for (int run = 0; run < TEST_RUNS; run++) {
            nes = new NES();
            if (!nes->loadROM(romPath)) {
                cout << "Failed to load ROM from '" << romPath << "'";
                return 3;
            }
            for (size_t i = 0; i < TOTAL_FRAMES; i++) {
                nes->clockFrame();
                u64 h = Hash(nes->getFrameBuffer());
                if (h != initRun[i]) {
                    cout << "Frame hash mismatch! First: " << hex(initRun[i], 16) << "; now: " << hex(h, 16);
                    cout << "\nFailure at run #" << run << " frame #" << i << endl;
                    nes->shutdown();
                    delete nes;
                    return 1;
                }
            }
            nes->shutdown();
            delete nes;
        }
    } else if (testType == "audio") {
        vector<u64> initRun;
        initRun.reserve(TOTAL_FRAMES);
        size_t ASSERT_START = 120;
        TOTAL_FRAMES += ASSERT_START + 1;
        NES* nes = new NES();
        if (!nes->loadROM(romPath)) {
            cout << "Failed to load ROM from '" << romPath << "'";
            return 3;
        }
        for (size_t i = 0; i < TOTAL_FRAMES; i++) {
            nes->clockFrame();
            if (i > ASSERT_START) {
                vector<float> audio;
                nes->collectAudio(audio);
                initRun.push_back(Hash(audio));
            } else if (i == ASSERT_START) {
                bool btns[8] = { true, false, false, false, false, false, false, false };
                nes->update(0, btns);
            }
        }
        nes->shutdown();
        delete nes;

        #ifdef SELF_TEST
        initRun[42] ^= 0x1010101010101010;
        #endif

        for (int run = 0; run < TEST_RUNS; run++) {
            nes = new NES();
            if (!nes->loadROM(romPath)) {
                cout << "Failed to load ROM from '" << romPath << "'";
                return 3;
            }
            for (size_t i = 0; i < TOTAL_FRAMES; i++) {
                nes->clockFrame();
                if (i > ASSERT_START) {
                    vector<float> audio;
                    nes->collectAudio(audio);
                    u64 h = Hash(audio);
                    size_t index = i - ASSERT_START - 1;
                    if (h != initRun[index]) {
                        cout << "Audio hash mismatch! First: " << hex(initRun[index], 16) << "; now: " << hex(h, 16);
                        cout << "\nFailure at run #" << run << " frame #" << i << "(hash index: " << index << ")" << endl;
                        nes->shutdown();
                        delete nes;
                        return 1;
                    }
                } else if (i == ASSERT_START) {
                    bool btns[8] = { true, false, false, false, false, false, false, false };
                    nes->update(0, btns);
                }
            }
            nes->shutdown();
            delete nes;
        }
    } else {
        cout << argv[1] << " is an invalid argument; use 'cpu' 'frame' or 'audio' to select a test type.";
        return 2;
    }

    return 0;
}