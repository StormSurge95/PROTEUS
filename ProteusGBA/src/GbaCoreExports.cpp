#include "../../shared/plugin_utils/PluginExports.h"
#include "./GBA.h"

namespace NS_GBA {
    // static manifest for this core
    static const PluginManifest GbaPluginManifest = {
        1, 0, 0,
        ICONSOLE_CONTRACT_VERSION,
        IDEBUGGER_CONTRACT_VERSION,

        "GBA",
        "Gameboy Advance",
        "Proteus GBA Emulation Core",
        "Storm Cassidy",
        "Cycle-accurate GBA emulator",
        __DATE__,
        "GPL-3.0",
        DevStatusValue::WIP,
        DevStatusDescription.at(DevStatusValue::WIP)
    };

    // === REQUIRED CORE FUNCTIONS ===
    CORE_CREATE(CreateCore) {
        try {
            return new GBA();
        } catch (...) {
            return nullptr;
        }
    }

    CORE_DESTROY(DestroyCore) {
        if (core) {
            core->shutdown();
            delete reinterpret_cast<GBA*>(core);
        }
    }

    // === OPTIONAL DEBUGGER FUNCTIONS ===

    DEBUGGER_CREATE(CreateDebugger) {
        if (!core) return nullptr;
        // TODO: return actual debugger once one is implemented
        return nullptr;
    }

    DEBUGGER_DESTROY(DestroyDebugger) {
        // TODO: destroy actual debugger once one is implemented
    }

    // === MANIFEST EXPORT ===

    PLUGIN_MANIFEST(GetPluginManifest) {
        return &GbaPluginManifest;
    }
}