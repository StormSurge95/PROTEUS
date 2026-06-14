#include "../../shared/plugin_utils/PluginExports.h"
#include "./NES.h"
#include "./shared/NesDebugger.h"

namespace NS_NES {
    // static manifect for this core
    static const PluginManifest NesPluginManifest = {
        1, 0, 0,
        ICONSOLE_CONTRACT_VERSION,
        IDEBUGGER_CONTRACT_VERSION,

        "NES",
        "Nintendo Entertainment System",
        "Proteus NES Emulation Core",
        "Storm Cassidy",
        "Cycle-accurate NES/Famicom emulator",
        __DATE__,
        "GPL-3.0",
        DevStatusValue::FUNCTIONAL,
        DevStatusDescription.at(DevStatusValue::FUNCTIONAL)
    };

    // === REQUIRED CORE FUNCTIONS ===

    CORE_CREATE(CreateCore) {
        try {
            return new NES();
        } catch (...) {
            return nullptr;
        }
    }

    CORE_DESTROY(DestroyCore) {
        if (core) {
            core->shutdown();
            delete reinterpret_cast<NES*>(core);
        }
    }

    // === OPTIONAL DEBUGGER FUNCTIONS ===

    DEBUGGER_CREATE(CreateDebugger) {
        if (!core) return nullptr;
        try {
            return new NesDebugger(reinterpret_cast<NES*>(core));
        } catch (...) {
            return nullptr;
        }
    }

    DEBUGGER_DESTROY(DestroyDebugger) {
        if (debugger)
            delete reinterpret_cast<NesDebugger*>(debugger);
    }

    // === MANIFEST EXPORT ===

    PLUGIN_MANIFEST(GetPluginManifest) {
        return &NesPluginManifest;
    }
}