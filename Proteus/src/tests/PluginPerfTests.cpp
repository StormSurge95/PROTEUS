#include "../shared/plugin_utils/PluginRegistry.h"
#include "../shared/IConsole.h"

#include <functional>

using std::function;

struct TimingEntry {
    string name;
    u64 min, avg, max;

    TimingEntry(string n, u64 m, u64 a, u64 x) : name(n), min(m), avg(a), max(x) {}
};

static TimingEntry Measure(string name, u32 maxCount, const function<void()>& fn) {
    fn(); // warmup
    u64 min = 0xFFFFFFFF;
    u64 max = 0;
    u64 sum = 0;
    u32 count = 0;
    for (int x = 0; x < maxCount; x++) {
        steady_clock::time_point t = steady_clock::now();
        fn();
        u64 d = (steady_clock::now() - t).count();
        if (d < min) min = d;
        if (d > max) max = d;
        sum += d;
    }
    u64 avg = sum / maxCount;
    return { name, min, avg, max };
}

int main() {
    PluginRegistry::DiscoverPlugins();

    vector<TimingEntry> results = {};

    #ifdef _DEBUG
        int N = 20;
    #else
        int N = 50;
    #endif

    // benchmark DiscoverPlugins
    results.push_back(Measure("DiscoverPlugins", N, []{
        PluginRegistry::DiscoverPlugins();
    }));

    // benchmark LoadPlugin/UnloadPlugin
    results.push_back(Measure("Load/UnloadPlugin", N, []{
        PluginRegistry::LoadPlugin("nes");
        PluginRegistry::UnloadPlugin("nes");
    }));

    // benchmark CreateCore/DestroyCore
    results.push_back(Measure("Create/DestroyCore", N, []{
        IConsole* core = PluginRegistry::CreateCore("nes");
        PluginRegistry::DestroyCore(core);
    }));
    const char* filler = "=========================";
    printf("Timing Results:\n");
    for (const TimingEntry& t : results) {
        int len = strlen(filler) - t.name.size();
        printf("%s %.*s> min: %luns\tmax: %luns\tavg: %luns\n", t.name.c_str(), len, filler, t.min, t.max, t.avg);
    }

    return 0;
}