# Proteus Architecture

## Overview
Proteus is split into three layers:

1. Frontend application (`PROTEUS/Proteus`)
2. Shared contracts and plugin runtime utilities (`PROTEUS/Proteus/src/shared`)
3. Console core plugins (`PROTEUS/ProteusNES`, `PROTEUS/ProteusGBA`, and future cores)

The frontend does not instantiate concrete emulator classes directly. Console and debugger instances are created through the plugin system at runtime.

## Repository Layout

- `PROTEUS/CMakeLists.txt`: top-level CMake entrypoint
- `PROTEUS/CMakePresets.json`: shared configure/build/test presets
- `PROTEUS/Proteus/src/frontend`: app/runtime/UI/session code
- `PROTEUS/Proteus/src/shared`: API contracts and shared utilities
- `PROTEUS/Proteus/src/shared/plugin_utils`: plugin loader + registry + exports
- `PROTEUS/Proteus/src/tests`: plugin runtime tests
- `PROTEUS/ProteusNES/src`: NES plugin implementation
- `PROTEUS/ProteusGBA/src`: GBA plugin implementation

## Runtime Ownership Model

## App Layer
- `Proteus` owns frontend managers and session lifecycle.
- `PluginManager` initializes discovery/shutdown and maps `ConsoleID -> plugin id`.
- `ConsoleSession` owns active `IConsole`/`IDebugger` instances for runtime use.

## Plugin Layer
- `PluginRegistry` discovers, loads, and unloads plugin libraries.
- `PluginRegistry` creates and destroys core/debugger instances through exported function pointers.
- Registry tracks ownership mappings from created pointers back to plugin id for safe destruction/unload checks.

## Contracts

Public contracts live in `src/shared`:
- `IConsole.h`
- `IDebugger.h`

Version compatibility is validated at plugin load time through `PluginManifest` fields:
- `iConsoleContractVersion`
- `iDebuggerContractVersion`

## Plugin Discovery and Loading

`PluginRegistry::DiscoverPlugins()`:
- builds expected filenames from known plugin id -> library basename mappings
- scans search paths for matching libraries
- marks discovered entries and stores resolved file paths

`PluginRegistry::LoadPlugin(id)`:
- validates discovery state
- loads dynamic library through `PluginLoader`
- resolves required exports
- validates manifest contract versions
- stores function pointers and marks plugin loaded

## Search Paths

Current search behavior includes:
- optional env override: `PROTEUS_PLUGIN_DIR`
- `<executable_dir>/plugins`
- `<cwd>/plugins`
- debug helper path from parent cwd in debug builds

## Error Model

Both `PluginLoader` and `PluginRegistry` maintain `lastError` strings with context-rich failure messages. Callers bubble these errors up to frontend/session layers.

## Test Architecture

Runtime plugin tests are under `src/tests` and cover:
- discovery
- load/unload
- core/debugger lifecycle
- negative scenarios and diagnostics
- timing baselines

Fixture plugin(s) for negative tests are built under `src/tests/fixtures`.

## Build and CI Model

Build system is CMake-first with presets, plus platform helper scripts:
- `build_windows.ps1`
- `build_linux.sh`
- `build_macos.sh`

CI executes cross-platform configure/build/test using CMake presets and vcpkg-provided dependencies.