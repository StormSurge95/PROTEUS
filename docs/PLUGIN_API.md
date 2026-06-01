# Plugin API

## Purpose
This document defines the runtime interface that emulator core plugins must implement to be discoverable and loadable by Proteus.

Header of record:
- `PROTEUS/Proteus/src/shared/plugin_utils/PluginExports.h`

## ABI Surface

Plugin exports use C linkage and platform-specific export declarations via macros:
- `PLUGIN_EXPORT`
- `PLUGIN_INVOKE`

Helper macros for required/optional exports:
- `CORE_CREATE(funcName)`
- `CORE_DESTROY(funcName)`
- `DEBUGGER_CREATE(funcName)` (optional; required if debugger destroy exists)
- `DEBUGGER_DESTROY(funcName)` (optional; required if debugger create exists)
- `PLUGIN_MANIFEST(funcName)`

## Manifest Contract

Each plugin must return a valid `PluginManifest` from:
- `GetPluginManifest()`

Important fields:
- plugin version triplet
- `iConsoleContractVersion`
- `iDebuggerContractVersion`
- console/plugin identity metadata
- status metadata (`DevStatusValue`)

Loader compatibility checks reject plugins whose contract versions do not match host interfaces.

## Required Exports

Each plugin must export:

1. `GetPluginManifest() -> const PluginManifest*`
2. `CreateCore() -> IConsole*`
3. `DestroyCore(IConsole*) -> void`

## Optional Debugger Exports

Debugger support is optional, but must be all-or-nothing:

- `CreateDebugger(IConsole*) -> IDebugger*`
- `DestroyDebugger(IDebugger*) -> void`

If one exists and the other is missing, plugin load is rejected.

## Symbol Names

Current loader expects exact symbol names:
- `GetPluginManifest`
- `CreateCore`
- `DestroyCore`
- `CreateDebugger` (optional)
- `DestroyDebugger` (optional)

These names must match exported function names exactly.

## Plugin Filename Conventions

Expected filename is built from:
- platform library prefix (`""` on Windows, `"lib"` on Unix-like)
- plugin library base name from registry mapping
- platform extension (`.dll`, `.so`, `.dylib`)

Current known plugin id -> basename map is defined in `PluginRegistry`.

## Error Handling Semantics

Load failures should surface actionable diagnostics through `PluginLoader::GetLastError()` and `PluginRegistry::GetLastError()`:
- missing file
- missing required symbol
- manifest null
- contract mismatch
- debugger export pairing mismatch

## Lifecycle Expectations

Ownership model:
- `CreateCore`/`CreateDebugger` return raw pointers owned by caller
- destruction must be performed through registry/manager layer, not directly in caller code
- plugin unload is blocked while live core/debugger instances associated with that plugin exist

