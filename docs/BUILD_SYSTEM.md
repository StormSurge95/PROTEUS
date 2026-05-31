# Build System

## Overview

Proteus uses a CMake-first build flow from repository root:
- `PROTEUS/CMakeLists.txt`
- `PROTEUS/CMakePresets.json`

Source graph is assembled through:
- `PROTEUS/Proteus/src/CMakeLists.txt`

## Dependency Strategy

Dependencies are resolved via `find_package` first, with targeted fallback behavior where required.

Current dependencies include:
- SDL3
- OpenSSL
- nlohmann/json

Recommended dependency provider for CI and reproducibility:
- vcpkg via toolchain file in presets

## Preset Usage

Presets are defined in `PROTEUS/CMakePresets.json`.

Core commands:

```bash
cmake --preset <configure-preset>
cmake --build --preset <build-preset>
ctest --preset <test-preset>
```

Current presets are Ninja-based (`Ninja Multi-Config`) with Debug/Release and tests-on variants.

## Platform Scripts

Helper scripts in repo root:
- `build_windows.ps1`
- `build_linux.sh`
- `build_macos.sh`

Each script supports:
- config selection (debug/release)
- optional test execution

## Windows

Example:

```powershell
.\build_windows.ps1 -dbg -t
.\build_windows.ps1 -rel
```

## Linux/macOS

Examples:

```bash
./build_linux.sh -D -T
./build_macos.sh -R
```

## CI

CI workflow:
- `.github/workflows/ci.yml`

It runs matrix builds/tests across Windows/Linux/macOS with vcpkg and cache-enabled dependency reuse.

## Output Layout

Current convention:
- app executable in build output `bin/<config>/`
- plugin binaries in `bin/<config>/plugins/`
- test binaries in `bin/<config>/tests/`

This layout aligns runtime plugin discovery with executable-relative plugin search.

## Troubleshooting

1. Generator mismatch:
   - delete stale build directory or use a fresh `binaryDir`.

2. Toolchain not applied:
   - ensure `VCPKG_ROOT` is available to configure process.

3. Presets not visible in IDE:
   - open repository root (`PROTEUS`) in editor.

4. No tests found:
   - configure with tests-enabled preset and verify `add_test(...)` targets were generated.