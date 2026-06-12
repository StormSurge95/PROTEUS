#include <conio.h>
// #include <consoleapi.h>

#include "./Headless.h"

using namespace NS_Proteus;

namespace {
    struct InputBuffers {
        size_t count = 0;
        uptr<bool[]> p1;
        uptr<bool[]> p2;
    };

    static string Lower(string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return s;
    }

    static vector<string> Split(const string& s, char delim) {
        vector<string> parts;
        std::stringstream ss(s);
        string part;
        while (std::getline(ss, part, delim)) {
            if (!part.empty()) parts.push_back(part);
        }
        return parts;
    }

    static InputBuffers MakeBuffers(size_t count) {
        InputBuffers out;
        out.count = count;
        out.p1 = make_unique<bool[]>(count);
        out.p2 = make_unique<bool[]>(count);
        std::fill_n(out.p1.get(), count, false);
        std::fill_n(out.p2.get(), count, false);
        return out;
    }

    static size_t ResolveButtonIndex(ConsoleID id, const string& rawName) {
        string name = Lower(rawName);

        if (id == ConsoleID::NES) {
            if (name == "a")        return 0;
            if (name == "b")        return 1;
            if (name == "select")   return 2;
            if (name == "start")    return 3;
            if (name == "up")       return 4;
            if (name == "down")     return 5;
            if (name == "left")     return 6;
            if (name == "right")    return 7;
        }

        throw std::invalid_argument(
            "Unsupported headless button '" + rawName +
            "' for console '" + GetNameFromID(id) + "'"
        );
    }

    static u8 ResolvePlayer(const string& rawPlayer) {
        string player = Lower(rawPlayer);
        if (player == "p1" || player == "1") return 0;
        if (player == "p2" || player == "2") return 1;
        throw std::invalid_argument("Unsupported player token '" + rawPlayer + "'");
    }

    static bool ResolvePressed(const string& rawState) {
        string state = Lower(rawState);
        if (state == "1" || state == "down" || state == "press" || state == "pressed")
            return true;
        if (state == "0" || state == "up" || state == "release" || state == "released")
            return false;
        throw std::invalid_argument("Unsupported input state token '" + rawState + "'");
    }

    static vector<Headless::InputEvent> GetBuiltInProfile(ConsoleID id, const string& romStem) {
        vector<Headless::InputEvent> events;
        string rom = Lower(romStem);

        if (id == ConsoleID::NES) {
            if (rom == "volumes") {
                // this input sequence causes `volumes.nes` to perfome a deterministic audio playback
                events.push_back({ 120, 0, ResolveButtonIndex(id, "A"), true });
                events.push_back({ 121, 0, ResolveButtonIndex(id, "A"), false });
            }
            if (rom == "accuracycoin") {
                // upon starting `AccuracyCoin.nes`, user cursor is at the title of page one
                // we press 'start' to cause the entire suite of tests to run
                events.push_back({ 120, 0, ResolveButtonIndex(id, "start"), true });
                events.push_back({ 121, 0, ResolveButtonIndex(id, "start"), false });
            }
        }

        return events;
    }

    static vector<Headless::InputEvent> ParseCliEvents(ConsoleID id, const string& inputSpec) {
        vector<Headless::InputEvent> events;
        if (inputSpec.empty()) return events;

        // Format:
        // "120,p1,A,1;121,p1,A,0;300,p1,START,1;301,p1,START,0"
        for (const string& record : Split(inputSpec, ';')) {
            auto parts = Split(record, ',');
            if (parts.size() != 4) {
                throw std::invalid_argument(
                    "Bad -input record '" + record +
                    "'. Expected 'frame,player,button,state'"
                );
            }

            Headless::InputEvent ev;
            ev.frame = std::stoull(parts[0]);
            ev.player = ResolvePlayer(parts[1]);
            ev.button = ResolveButtonIndex(id, parts[2]);
            ev.pressed = ResolvePressed(parts[3]);
            events.push_back(ev);
        }
        
        return events;
    }

    static vector<Headless::InputEvent> BuildSchedule(ConsoleID id, const path& rom, const string& inputSpec) {
        vector<Headless::InputEvent> events = GetBuiltInProfile(id, rom.stem().string());
        vector<Headless::InputEvent> cli = ParseCliEvents(id, inputSpec);

        events.insert(events.end(), cli.begin(), cli.end());

        std::stable_sort(events.begin(), events.end(),
            [](const Headless::InputEvent& a, const Headless::InputEvent& b) {
                if (a.frame != b.frame) return a.frame < b.frame;
                if (a.player != b.player) return a.player < b.player;
                return a.button < b.button;
            });

        return events;
    }

    static void ApplyFrameEvents(const vector<Headless::InputEvent>& schedule, size_t& nextEvent, u64 frame, InputBuffers& btns) {
        while (nextEvent < schedule.size() && schedule[nextEvent].frame == frame) {
            const auto& ev = schedule[nextEvent];
            bool* tgt = (ev.player == 0) ? btns.p1.get() : btns.p2.get();
            tgt[ev.button] = ev.pressed;
            nextEvent++;
        }
    }

    static bool PollEscape() {
        if (!_kbhit()) return false;

        int ch = _getch();
        return ch == 27;
    }

    static std::atomic<bool> gQuit = false;
}

void Headless::Run(ConsoleID id, const path& rom, const string& inputSpec) {
    if (!PluginManager::Initialize())
        throw std::runtime_error(PluginManager::GetLastError());

    uptr<IConsole> core = nullptr;
    uptr<IDebugger> dbg = nullptr;
    
    try {
        if (!PluginManager::IsConsoleAvailable(id))
            throw std::invalid_argument("'" + GetNameFromID(id) + "' is not currently available for emulation");

        IConsole* core = PluginManager::CreateConsole(id);
        if (!core) throw std::runtime_error(PluginManager::GetLastError());
        IDebugger* dbg = PluginManager::CreateDebugger(id, core);
        if (!core->loadROM(rom.string())) {
            PluginManager::DestroyConsole(core);
            throw std::invalid_argument("'" + rom.string() + "' does not appear to lead to a valid ROM file");
        }

        InputBuffers btns = MakeBuffers(core->buttonCount());
        vector<InputEvent> schedule = BuildSchedule(id, rom, inputSpec);
        size_t nextEvent = 0;
        u64 frameCount = 0;
        
        if (dbg) {
            dbg->SetTraceConfig({
                .enabled = true,
                .append = false,
                .flushEveryFrame = true,
                .flushThresholdRecords = 8192,
                .mode = DebugTraceMode::ALL,
                .filePath = "trace.log"
            });
            dbg->BeginTrace();
        }

        bool quit = false;
        while (!quit && !gQuit) {
            if (PollEscape()) {
                quit = true;
                continue;
            }

            ApplyFrameEvents(schedule, nextEvent, frameCount, btns);

            core->update(0, btns.p1.get());
            core->update(1, btns.p2.get());

            core->clockFrame();
            frameCount++;
        }
        if (dbg) {
            dbg->EndTrace();
            PluginManager::DestroyDebugger(dbg);
        }
        core->shutdown();
        PluginManager::DestroyConsole(core);
        PluginManager::Shutdown();
    } catch (...) {
        PluginManager::Shutdown();
        throw;
    }
}