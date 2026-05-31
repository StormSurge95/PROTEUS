#include <cstdint>
#include <PluginExports.h>

static PluginManifest badManifest = {
    0, 0, 1,
    ICONSOLE_CONTRACT_VERSION,
    IDEBUGGER_CONTRACT_VERSION,

    "bad_symbols",
    "Bad Symbols Test Console",
    "Bad Symbols Fixture",
    "Storm Cassidy",
    "Intentionally missing required core create export.",
    __DATE__,
    "",
    DevStatusValue::STUB,
    "Fixture plugin for loader negative testing"
};

PLUGIN_MANIFEST(GetPluginManifest) {
    return &badManifest;
}

CORE_DESTROY(DestroyCore) {
    (void)core;
}

DEBUGGER_CREATE(CreateDebugger) {
    (void)core;
    return nullptr;
}

DEBUGGER_DESTROY(DestroyDebugger) {
    (void)debugger;
}