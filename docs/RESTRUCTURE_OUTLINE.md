# Proteus Plugin Architecture: Cleaned Restructure Outline

Document Version: 2.0 (Cleaned)
Last Updated: 2026-05-26
Status: Planning Baseline

## 1. Purpose

This outline replaces outdated or ambiguous parts of the original restructure plan and aligns with the current project layout under `src/shared`, `src/backend`, and `src/frontend`.

Primary goal: move from compile-time console binding to runtime plugin loading, while preserving Visual Studio-first development on Windows and enabling cross-platform builds as a follow-up.

## 2. Current-State Snapshot

Implemented or partially implemented:
- `IConsole` and `IDebugger` contract version constants exist in `src/shared`.
- Core export macros and `CoreManifest` exist in `src/shared/core_utils/CoreExports.h`.
- A cross-platform `CoreLoader` implementation exists in `src/shared/core_utils/CoreLoader.*`.
- NES export unit exists (`src/backend/NES/NesCoreExports.cpp`).
- CMake fragments exist for core utils and backend plugins.

Not implemented or incomplete:
- No `PluginRegistry` implementation.
- No frontend `PluginManager`.
- Frontend factories still instantiate/link compile-time NES types directly.
- No root CMake build graph (`CMakeLists.txt` at project root and key subdirs missing).
- No plugin test suite.
- No release/docs package set described in the original outline.

## 3. Scope Corrections vs Original Outline

1. Path corrections:
- Use `src/shared/...` (not `src/backend/shared/...`).
- Keep `src/backend/NES` and `src/backend/GBA` naming unless intentionally renamed later.

2. Critical-path corrections:
- Multi-instance/link-cable work is not on the plugin migration critical path.
- Cross-platform scripting is downstream of stable CMake + plugin runtime.

3. Acceptance corrections:
- "Loader complete" requires real plugin load tests, not compile-only status.

## 4. Phased Plan to Completion

## Phase 0: Foundation Hardening

Objective:
- Make the existing contract + loader foundation reliable and internally consistent.

Work:
- Validate and normalize naming conventions between manifest/export functions and loader symbol resolution.
- Fix loader edge cases and platform details (error handling and platform naming/extension behavior).
- Ensure manifest compatibility checks include all relevant contract checks.

Exit criteria:
- Windows build can load/unload a real plugin DLL and resolve required symbols.
- Loader diagnostics are clear enough for frontend-facing error messages.

## Phase 1: NES as First Runtime Plugin

Objective:
- Make NES truly loadable as a runtime plugin artifact.

Work:
- Complete NES plugin target source wiring and output conventions.
- Ensure plugin export unit and core implementation are consistently compiled into plugin artifact.
- Validate output filename compatibility with loader expectations.

Exit criteria:
- NES can be created and destroyed only through exported plugin functions.

## Phase 2: Plugin Registry and Discovery

Objective:
- Centralize discovery, load state, manifests, and factory dispatch.

Work:
- Add `PluginRegistry` in `src/shared/core_utils`.
- Implement discovery paths, load/unload lifecycle, manifest access, and core/debugger creation routing.
- Add robust handling for partial failures (one plugin fails, others still usable).

Exit criteria:
- Registry can discover/load supported plugins and create cores by console id/name.

## Phase 3: Frontend Migration to Plugin System

Objective:
- Remove compile-time frontend dependency on concrete backend console classes.

Work:
- Add frontend `PluginManager` wrapper over `PluginRegistry`.
- Refactor session factories to delegate create/destroy to plugin manager/registry.
- Initialize and shutdown plugin layer from app lifecycle.
- Update session/UI availability state based on loaded plugins.

Exit criteria:
- Console and debugger instances are created dynamically via plugin system.
- Frontend does not need direct concrete NES type construction for runtime usage.

## Phase 4: Build System Consolidation (Visual Studio-first)

Objective:
- Introduce complete CMake topology without breaking Visual Studio workflow.

Work:
- Add root and subdirectory `CMakeLists.txt` files for app, shared, backend plugins, and tests.
- Keep MSBuild project usable during migration; stage deprecation later.
- Standardize output directories for executable and plugins.

Exit criteria:
- One CMake configure/build on Windows produces app + plugin outputs with expected layout.

## Phase 5: Testing and Validation

Objective:
- Establish confidence in plugin runtime behavior.

Work:
- Add plugin load tests (discover/load/unload/manifest compatibility).
- Add plugin integration tests (create core, lifecycle, basic framebuffer/audio/input calls).
- Add regression tests for failure behavior (missing symbols, version mismatch, bad manifests).

Exit criteria:
- Automated test suite passes on Windows.

## Phase 6: Cross-Platform Enablement

Objective:
- Expand proven Windows flow to Linux and macOS.

Work:
- Validate loader behavior and plugin naming on Linux/macOS.
- Add build scripts for each platform after CMake parity.
- Execute smoke tests for plugin load and app startup per platform.

Exit criteria:
- Linux/macOS builds and plugin load smoke tests pass.

## Phase 7: Documentation and Release Preparation

Objective:
- Publish stable developer/user docs for plugin architecture.

Work:
- Architecture, plugin API, plugin development guide, and build guide.
- Installation and custom plugin location/config docs.
- Release notes template and packaging checklist.

Exit criteria:
- Documentation set is complete and consistent with shipped behavior.

## Phase 8: Optional Advanced Architecture

Objective:
- Implement multi-instance and inter-core communication after core plugin migration is stable.

Work:
- Multi-core session orchestration.
- Optional link-cable/data bridge simulation for supported console pairs.

Exit criteria:
- Single-core flow remains stable; optional linked flow has bounded scope and tests.

## 5. Updated Critical Path

Phase 0 -> Phase 1 -> Phase 2 -> Phase 3 -> Phase 4 -> Phase 5 -> Phase 6 -> Phase 7

Phase 8 is intentionally off the critical path.

## 6. Definition of Done (Restructure Complete)

Restructure is complete when all are true:
- Frontend runtime console/debugger creation is plugin-driven.
- At least NES ships as a runtime plugin artifact.
- Registry/discovery/load lifecycle is implemented and tested.
- Build system supports Windows-first CMake workflow in Visual Studio.
- Automated plugin tests pass on Windows, with Linux/macOS smoke validation.
- Architecture and plugin API docs reflect real implementation.
