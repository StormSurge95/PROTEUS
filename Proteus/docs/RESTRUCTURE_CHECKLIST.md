# Proteus Plugin Architecture: Cleaned Restructure Checklist

Document Version: 2.0 (Cleaned)
Last Updated: 2026-05-26
Status Legend: [ ] Not Started, [~] In Progress, [x] Completed, [!] Needs Verification

## Phase 0: Foundation Hardening

### 0.1 Contract Surfaces (`src/shared`)
- [x] `IConsole` has contract version constant (`ICONSOLE_CONTRACT_VERSION`).
- [x] `IDebugger` has contract version constant (`IDEBUGGER_CONTRACT_VERSION`).
- [x] Contract headers are documented for plugin/API role.
- [x] Verify contract comments and naming consistency for public-facing docs.

### 0.2 Core Manifest and Export Macros (`src/shared/plugin_utils/CoreExports.h`)
- [x] `CoreManifest` structure defined.
- [x] Export macros exist for core/debugger create/destroy and manifest.
- [x] Verify ABI/export behavior across MSVC/Clang/GCC.
- [x] Verify naming consistency between exported symbols and loader expectations.

### 0.3 Core Loader (`src/shared/plugin_utils/CoreLoader.*`)
- [x] Cross-platform load/unload/symbol resolution structure exists.
- [x] Validate Windows runtime loading with real plugin artifact.
- [ ] Validate Linux runtime loading with real plugin artifact.
- [ ] Validate macOS runtime loading with real plugin artifact.
- [ ] Improve and standardize failure diagnostics for all rejection paths.
- [ ] Confirm platform-specific filename/prefix/extension behavior is correct.

Exit gate:
- [~] Phase 0 gate: loader proven with real plugin load/unload at least on Windows.
  - Currently only tested and successful on Windows

## Phase 1: NES Runtime Plugin

### 1.1 Plugin Target and Source Wiring (`src/backend/NES`)
- [x] NES export unit exists (`NesCoreExports.cpp`).
- [x] NES shared-library CMake target exists.
- [x] Ensure NES plugin target includes all required source units for runtime viability.
- [x] Ensure output naming/layout matches loader expectations.
- [x] Verify create/destroy/debugger/manifest exports resolve at runtime.

Exit gate:
- [x] Phase 1 gate: NES core lifecycle works through plugin exports only.

## Phase 2: Plugin Registry and Discovery

### 2.1 Shared Registry Layer (`src/shared/plugin_utils`)
- [x] Create `PluginRegistry.h`.
- [x] Create `PluginRegistry.cpp`.
- [x] Implement search path policy (dev + packaged layouts).
- [x] Implement discovery map/index of candidate plugins.
- [x] Implement load/unload lifecycle and loaded-state tracking.
- [x] Implement manifest retrieval and compatibility checks.
- [x] Implement core/debugger create/destroy routing via registry entries.
- [x] Implement partial-failure handling (continue on independent plugin failures).

Exit gate:
- [x] Phase 2 gate: registry discovers and loads plugins and can instantiate cores.

## Phase 3: Frontend Plugin Migration

### 3.1 Frontend Integration (`src/frontend`)
- [x] Create frontend `PluginManager` wrapper.
- [x] Initialize plugin manager during app startup.
- [x] Shutdown plugin manager during app teardown.
- [x] Refactor `ConsoleFactory` to use plugin manager/registry.
- [x] Refactor `DebuggerFactory` to use plugin manager/registry.
- [x] Remove runtime dependency on direct concrete backend construction in frontend.
- [x] Update session/UI availability states from discovered/loaded plugins.

Exit gate:
- [x] Phase 3 gate: frontend runtime creation path is plugin-only.

## Phase 4: Build System Consolidation (Windows + Visual Studio first)

### 4.1 CMake Topology
- [x] Add root `CMakeLists.txt`.
- [x] Add `src/CMakeLists.txt`.
- [x] Add backend aggregation `CMakeLists.txt` (or equivalent).
- [x] Add frontend executable `CMakeLists.txt`.
- [x] Add tests `CMakeLists.txt`.

### 4.2 Build Behavior
- [x] Standardize plugin output directory conventions.
- [x] Ensure Visual Studio CMake workflow is fully usable.
- [x] Keep existing `.vcxproj` build path functional during migration period.

Exit gate:
- [x] Phase 4 gate: Windows CMake build emits app + plugins in expected layout.

## Phase 5: Testing and Validation

### 5.1 Plugin Runtime Tests
- [ ] Add plugin load tests (discover/load/unload/version mismatch/symbol mismatch).
- [ ] Add plugin integration tests (core lifecycle + basic APIs).
- [ ] Add regression tests for negative scenarios and error messages.

### 5.2 Execution Targets
- [ ] Run tests on Windows CI or repeatable local harness.
- [ ] Record baseline timings for plugin discovery/load.

Exit gate:
- [ ] Phase 5 gate: automated Windows plugin test suite passes.

## Phase 6: Cross-Platform Enablement

### 6.1 Linux/macOS Parity
- [ ] Validate Linux plugin load flow end-to-end.
- [ ] Validate macOS plugin load flow end-to-end.
- [ ] Add/verify `build_windows.ps1`, `build_linux.sh`, `build_macos.sh`.
- [ ] Run smoke tests for app startup + plugin discovery on each platform.

Exit gate:
- [ ] Phase 6 gate: cross-platform smoke validation completed.

## Phase 7: Documentation and Release Prep

### 7.1 Developer Documentation
- [ ] `docs/ARCHITECTURE.md`
- [ ] `docs/PLUGIN_API.md`
- [ ] `docs/PLUGIN_DEVELOPMENT.md`
- [ ] `docs/BUILD_SYSTEM.md`

### 7.2 User/Release Documentation
- [ ] `docs/INSTALLATION.md`
- [ ] `docs/CUSTOM_PLUGINS.md`
- [ ] `RELEASE_NOTES.md` template
- [ ] Packaging/checksum/signing checklist

Exit gate:
- [ ] Phase 7 gate: docs match implementation and release process is defined.

## Phase 8: Optional Advanced Work (Non-Critical Path)

### 8.1 Multi-Instance and Inter-Core Features
- [ ] Design multi-core session orchestration.
- [ ] Implement optional linked-console data bridge (where applicable).
- [ ] Add focused tests that do not regress single-core runtime.

Exit gate:
- [ ] Phase 8 gate: optional advanced features are stable and isolated.

## Restructure Completion Gate

- [ ] Frontend is runtime plugin-driven for console/debugger lifecycle.
- [ ] NES plugin ships as real runtime plugin artifact.
- [ ] Registry/discovery/load lifecycle is implemented and tested.
- [ ] Windows-first Visual Studio+CMake workflow is stable.
- [ ] Cross-platform smoke validation completed.
- [ ] Documentation and release checklist are complete and implementation-accurate.
