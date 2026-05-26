# Proteus Plugin Architecture Restructuring Checklist

## Phase 0: Foundation & Preparation

### 0.1 Establish IConsole and IDebugger as Plugin Contracts
- [x] 0.1.1 Review and Document Current IConsole Interface
  - [x] Update `src/backend/shared/IConsole.h` with contract version
  - [x] Document all required methods
  - [x] Add contract stability notes
  - [x] Verify existing implementations align with interface

- [x] 0.1.2 Review and Document Current IDebugger Interface
  - [x] Update `src/backend/shared/IDebugger.h` with contract version
  - [x] Document all methods
  - [x] Mark as optional for plugins
  - [x] Verify backwards compatibility

- [x] 0.1.3 Create Plugin Manifest Structure
  - [x] Create `src/shared/core_utils/CoreExports.h`
  - [x] Define version fields (major, minor, patch)
  - [x] Define contract version references
  - [x] Add metadata fields (name, author, description, etc.)
  - [x] Add development status enum

- [x] 0.1.4 Define Plugin Export Macros
  - [x] Define `CORE_EXPORT` macro (platform-specific)
  - [x] Define `CORE_INVOKE` macro
  - [x] Create `CORE_CREATE` macro
  - [x] Create `CORE_DESTROY` macro
  - [x] Create `DEBUGGER_CREATE` macro (optional)
  - [x] Create `DEBUGGER_DESTROY` macro (optional)
  - [x] Create `CORE_MANIFEST` macro
  - [x] Test macros compile on all platforms

**Deliverable:** Interfaces and export macros documented. No functional changes.

---

### 0.2 Create Cross-Platform Core Loading System

- [x] 0.2.1 Define Core Loader
  - [x] Create `src/shared/core_utils/CoreLoader.h`
  - [x] Define function pointer types for exports
  - [x] Create `LoadedCore` struct
  - [x] Define `LoadCore()` method
  - [x] Define `UnloadCore()` method
  - [x] Define `GetExpectedFilename()` helper
  - [x] Define `GetCoreExtension()` method
  - [x] Define `GetLibraryPrefix()` method
  - [x] Define `GetLastError()` method
  - [x] Declare platform-specific implementations

- [ ] 0.2.2 Create Windows Core Loader
  - [x] Create `src/shared/core_utils/platform/CoreLoader.cpp`
  - [x] Implement `LoadLibraryInternal()` using `LoadLibraryA()`
  - [x] Implement `UnloadLibraryInternal()` using `FreeLibrary()`
  - [x] Implement `GetSymbolInternal()` using `GetProcAddress()`
  - [x] Implement `GetCoreExtension()` returning ".dll"
  - [x] Implement `GetLibraryPrefix()` returning ""
  - [x] Implement `GetLastError()` with proper error messages
  - [ ] Test with real DLL loading

- [ ] 0.2.3 Create Linux Core Loader
  - [x] Implement `LoadLibraryInternal()` using `dlopen()`
  - [x] Implement `UnloadLibraryInternal()` using `dlclose()`
  - [x] Implement `GetSymbolInternal()` using `dlsym()`
  - [x] Implement `GetCoreExtension()` returning ".so"
  - [x] Implement `GetLibraryPrefix()` returning "lib"
  - [x] Implement `GetLastError()` using `dlerror()`
  - [ ] Test with real SO loading

- [ ] 0.2.4 Create macOS Core Loader
  - [x] Create `src/shared/core_utils/platform/CoreLoaderMacOS.cpp`
  - [x] Implement `LoadLibraryInternal()` using `dlopen()`
  - [x] Implement `UnloadLibraryInternal()` using `dlclose()`
  - [x] Implement `GetSymbolInternal()` using `dlsym()`
  - [x] Implement `GetCoreExtension()` returning ".dylib"
  - [x] Implement `GetLibraryPrefix()` returning "lib"
  - [x] Implement `GetLastError()` using `dlerror()`
  - [ ] Test with real DYLIB loading
  - [ ] Handle Apple Silicon and Intel architectures

- [ ] 0.2.5 Create Platform-Agnostic Core Loader Implementation
  - [x] Create `src/shared/core_utils/CoreLoader.cpp`
  - [x] Implement `LoadCore()` with validation
  - [x] Implement manifest loading and version checking
  - [x] Implement factory function resolution
  - [x] Implement optional debugger function resolution
  - [x] Implement `UnloadCore()` with cleanup
  - [x] Implement `GetExpectedFilename()` helper
  - [ ] Add comprehensive error handling

**Deliverable:** Cross-platform plugin loader compiles and links. Cores can be loaded/unloaded dynamically.

---

## Phase 1: NES Core as First Library

### 1.1 Create NES Plugin Structure

- [ ] 1.1.1 Create NES CMakeLists.txt (SHARED Library)
  - [x] Create `src/backends/nes/CMakeLists.txt`
  - [x] Define shared library target `ProteusNES`
  - [ ] Add plugin interface source files
  - [ ] Add core emulation source files
  - [ ] Add CPU, PPU, APU source files
  - [ ] Add cartridge and input source files
  - [ ] Set include directories
  - [ ] Configure compiler settings (C++17)
  - [ ] Set platform-specific output properties
  - [ ] Configure install targets
  - [ ] Set output to `${CMAKE_BINARY_DIR}/plugins`

- [x] 1.1.2 Create NES Plugin Manifest and Exports
  - [x] Create `src/backends/nes/core_utils/NESCoreExports.cpp`
  - [x] Define `CoreManifest` static instance
  - [x] Populate all manifest fields
  - [x] Implement `CreateCore()` factory function
  - [x] Implement `DestroyCore()` cleanup function
  - [x] Implement `CreateDebugger()` optional function
  - [x] Implement `DestroyDebugger()` optional function
  - [x] Implement `GetPluginManifest()` export
  - [x] Verify all exports use correct macros

**Deliverable:** NES compiles as plugin DLL/SO/DYLIB. Implements IConsole contract.

---

## Phase 2: Plugin Registry & Discovery

### 2.1 Create Plugin Registry

- [ ] 2.1.1 Plugin Registry Header
  - [ ] Create `src/shared/core_utils/PluginRegistry.h`
  - [ ] Define `PluginRegistry` class
  - [ ] Define `RegistryEntry` struct
  - [ ] Declare `DiscoverPlugins()` method
  - [ ] Declare `LoadPlugin()` method
  - [ ] Declare `LoadAllPlugins()` method
  - [ ] Declare `CreateCore()` method
  - [ ] Declare `DestroyCore()` method
  - [ ] Declare `IsPluginLoaded()` check
  - [ ] Declare `IsPluginDiscovered()` check
  - [ ] Declare `GetManifest()` accessor
  - [ ] Declare `GetDiscoveredPlugins()` lister
  - [ ] Declare `GetLoadedCores()` lister
  - [ ] Declare `UnloadAllPlugins()` method
  - [ ] Document all methods

- [ ] 2.1.2 Plugin Registry Implementation
  - [ ] Create `src/shared/core_utils/PluginRegistry.cpp`
  - [ ] Implement `GetSearchPaths()` helper
    - [ ] Add Windows search paths
    - [ ] Add Linux search paths
    - [ ] Add macOS search paths
  - [ ] Implement `DiscoverPlugins()`
    - [ ] Scan all search paths
    - [ ] Find all known console plugins
    - [ ] Populate registry
    - [ ] Log discoveries
  - [ ] Implement `LoadPlugin()`
    - [ ] Use PluginLoader to load DLL/SO/DYLIB
    - [ ] Validate manifest
    - [ ] Mark as loaded
    - [ ] Handle errors
  - [ ] Implement `LoadAllPlugins()`
    - [ ] Load each discovered plugin
    - [ ] Collect errors but continue
  - [ ] Implement `CreateCore()`
    - [ ] Find plugin in registry
    - [ ] Call factory function
    - [ ] Return IConsole instance
  - [ ] Implement `DestroyCore()`
    - [ ] Find correct plugin
    - [ ] Call destroy function
  - [ ] Implement status check methods
  - [ ] Implement getter methods
  - [ ] Implement `UnloadAllPlugins()`

**Deliverable:** Plugin registry complete. Can discover and load all plugins.

---

## Phase 3: Update Frontend to Use Plugin System

### 3.1 Create Plugin Manager for Frontend

- [ ] 3.1.1 Plugin Manager Header
  - [ ] Create `src/frontend/core_utils/PluginManager.h`
  - [ ] Define `PluginManager` class
  - [ ] Declare `Initialize()` method
  - [ ] Declare `Shutdown()` method
  - [ ] Declare `IsConsoleAvailable()` check
  - [ ] Declare `GetAvailableConsoles()` lister
  - [ ] Declare `CreateConsole()` factory
  - [ ] Declare `CreateDebugger()` factory
  - [ ] Declare `DestroyConsole()` cleanup
  - [ ] Declare `DestroyDebugger()` cleanup
  - [ ] Declare `GetConsoleMetadata()` accessor
  - [ ] Declare `LogPluginStatus()` helper
  - [ ] Document all methods

- [ ] 3.1.2 Plugin Manager Implementation
  - [ ] Create `src/frontend/core_utils/PluginManager.cpp`
  - [ ] Implement `Initialize()`
    - [ ] Call `PluginRegistry::DiscoverPlugins()`
    - [ ] Call `PluginRegistry::LoadAllPlugins()`
    - [ ] Handle partial load failures
    - [ ] Log plugin status
  - [ ] Implement `Shutdown()`
    - [ ] Call `PluginRegistry::UnloadAllPlugins()`
    - [ ] Handle cleanup errors
  - [ ] Implement `IsConsoleAvailable()`
    - [ ] Check registry status
  - [ ] Implement `GetAvailableConsoles()`
    - [ ] Return loaded plugins list
  - [ ] Implement `CreateConsole()`
    - [ ] Delegate to PluginRegistry
  - [ ] Implement `CreateDebugger()`
    - [ ] Find plugin's debugger function
    - [ ] Call if available
  - [ ] Implement `DestroyConsole()` and `DestroyDebugger()`
  - [ ] Implement `GetConsoleMetadata()`
  - [ ] Implement `LogPluginStatus()`

- [ ] 3.1.3 Update ConsoleFactory to Use Plugins
  - [ ] Update `src/frontend/session/ConsoleFactory.cpp`
  - [ ] Create `GetConsoleNameString()` mapper
  - [ ] Modify `Create()` to use PluginManager
  - [ ] Update error handling
  - [ ] Add logging

- [ ] 3.1.4 Update DebuggerFactory to Use Plugins
  - [ ] Update `src/frontend/session/DebuggerFactory.cpp`
  - [ ] Modify `Create()` to use PluginManager
  - [ ] Handle missing debugger support gracefully
  - [ ] Update error handling

- [ ] 3.1.5 Update Proteus::Init() to Use Plugin Manager
  - [ ] Modify `src/frontend/app/Proteus.cpp`
  - [ ] Call `PluginManager::Initialize()` early
  - [ ] Create `UpdateConsoleAvailability()` method
  - [ ] Populate `ConsoleEmuStarted` from loaded plugins
  - [ ] Add error handling

- [ ] 3.1.6 Update Proteus::Deinit() to Use Plugin Manager
  - [ ] Call `PluginManager::Shutdown()` at exit
  - [ ] Handle cleanup errors
  - [ ] Add logging

**Deliverable:** Frontend uses plugin system. Consoles loaded dynamically at runtime.

---

## Phase 4: Build System Configuration

### 4.1 Root CMakeLists.txt

- [ ] 4.1.1 Create Root CMakeLists.txt
  - [ ] Create `CMakeLists.txt` in project root
  - [ ] Set minimum version to 2.16
  - [ ] Set project name and version
  - [ ] Set C++ standard to 17
  - [ ] Add platform detection
  - [ ] Add build options
    - [ ] `BUILD_NES_PLUGIN` (default ON)
    - [ ] `BUILD_GBA_PLUGIN` (default ON)
    - [ ] `BUILD_STUB_PLUGINS` (default ON)
    - [ ] `ENABLE_DEBUG_FEATURES` (default OFF)
  - [ ] Set install directories per platform
  - [ ] Add src subdirectory

### 4.2 src/CMakeLists.txt

- [ ] 4.2.1 Create src/CMakeLists.txt
  - [ ] Add common subdirectory
  - [ ] Add backend/shared subdirectory
  - [ ] Add backends subdirectory (conditional)
  - [ ] Add frontend subdirectory

### 4.3 src/shared/CMakeLists.txt

- [ ] 4.3.1 Create src/shared/CMakeLists.txt
  - [ ] Create `ProteusCommon` static library
  - [ ] Add plugin source files
  - [ ] Add platform-specific loaders
  - [ ] Set include directories
  - [ ] Set compiler settings
  - [ ] Enable position-independent code (PIC)

### 4.4 src/backends/CMakeLists.txt

- [ ] 4.4.1 Create src/backends/CMakeLists.txt
  - [ ] Conditionally add NES subdirectory
  - [ ] Conditionally add GBA subdirectory
  - [ ] Conditionally add stub subdirectories
  - [ ] Configure install for all plugins

### 4.5 src/frontend/CMakeLists.txt

- [ ] 4.5.1 Create src/frontend/CMakeLists.txt
  - [ ] Find SDL3 package
  - [ ] Create `Proteus` executable
  - [ ] Add all frontend source files
  - [ ] Set include directories
  - [ ] Link against ProteusCommon and SDL3
  - [ ] Set compiler settings
  - [ ] Configure install target

**Deliverable:** Build system complete and functional. Projects compile cleanly on all platforms.

---

## Phase 5: Multi-Instance & Inter-Core Communication

### 5.1 Design Multi-Instance Session Management

- [ ] 5.1.1 Multi-Core Session Header
  - [ ] Create `src/frontend/session/MultiCoreSession.h`
  - [ ] Define `MultiCoreSession` class
  - [ ] Add primary session member
  - [ ] Add secondary session member
  - [ ] Declare `CreatePrimarySession()` method
  - [ ] Declare `LinkSecondaryConsole()` method
  - [ ] Declare `UnlinkSecondaryConsole()` method
  - [ ] Declare `IsSecondaryActive()` check
  - [ ] Declare session accessors
  - [ ] Declare `SynchronizeConsoles()` method
  - [ ] Declare `ShutdownAll()` method
  - [ ] Add synchronization state members

- [ ] 5.1.2 Multi-Core Session Implementation
  - [ ] Create `src/frontend/session/MultiCoreSession.cpp`
  - [ ] Implement constructor
  - [ ] Implement destructor
  - [ ] Implement `CreatePrimarySession()`
  - [ ] Implement `LinkSecondaryConsole()`
    - [ ] Validate no existing secondary
    - [ ] Create new ConsoleSession
    - [ ] Initialize with specified console
  - [ ] Implement `UnlinkSecondaryConsole()`
  - [ ] Implement `SynchronizeConsoles()`
    - [ ] Frame-lock both consoles
    - [ ] Handle clock ratio differences
    - [ ] Simulate link cable data exchange
  - [ ] Implement `ShutdownAll()`
  - [ ] Add error handling

### 5.2 Link Cable Simulation (Phase 5.2)

- [ ] 5.2.1 Design Link Cable Protocol
  - [ ] Create `src/frontend/session/LinkCableSimulator.h`
  - [ ] Define data transfer format
  - [ ] Define handshake protocol
  - [ ] Support multiple console pairs

- [ ] 5.2.2 Implement Link Cable
  - [ ] Create `src/frontend/session/LinkCableSimulator.cpp`
  - [ ] Implement data transfer
  - [ ] Implement synchronization
  - [ ] Add logging/debugging

**Deliverable:** Multi-instance architecture designed. Single-core mode still works.

---

## Phase 6: Cross-Platform Build Scripts

### 6.1 Create Windows Build Script

- [ ] 6.1.1 Create build_windows.ps1
  - [ ] Create `build_windows.ps1` script
  - [ ] Accept BuildType parameter (Release/Debug)
  - [ ] Accept Architecture parameter (x64/x86)
  - [ ] Create build directory
  - [ ] Run cmake with appropriate flags
  - [ ] Build project
  - [ ] Handle errors
  - [ ] Report output location

### 6.2 Create Linux Build Script

- [ ] 6.2.1 Create build_linux.sh
  - [ ] Create `build_linux.sh` script
  - [ ] Accept BuildType parameter
  - [ ] Create build directory
  - [ ] Run cmake with appropriate flags
  - [ ] Build with parallel jobs
  - [ ] Handle errors
  - [ ] Report output location

### 6.3 Create macOS Build Script

- [ ] 6.3.1 Create build_macos.sh
  - [ ] Create `build_macos.sh` script
  - [ ] Accept BuildType parameter
  - [ ] Accept Architecture parameter (x86_64/arm64)
  - [ ] Create build directory
  - [ ] Run cmake with architecture flags
  - [ ] Build with parallel jobs
  - [ ] Handle errors
  - [ ] Report output location

**Deliverable:** One-command builds work for all platforms.

---

## Phase 7: Testing & Validation

### 7.1 Create Plugin Load Tests

- [ ] 7.1.1 Create Plugin Load Test Suite
  - [ ] Create `tests/core_utils/PluginLoadTest.cpp`
  - [ ] Add `DiscoverPlugins` test
  - [ ] Add `LoadNESPlugin` test
  - [ ] Add `LoadGBAPlugin` test
  - [ ] Add `CreateNESCore` test
  - [ ] Add `CreateGBACore` test
  - [ ] Add `PluginManifest` validation test
  - [ ] Add `ContractVersion` compatibility test
  - [ ] Test error handling

### 7.2 Create Plugin Integration Tests

- [ ] 7.2.1 Create Plugin Integration Test Suite
  - [ ] Create `tests/core_utils/PluginIntegrationTest.cpp`
  - [ ] Test load ROM through plugin
  - [ ] Test emulation cycles
  - [ ] Test frame buffer access
  - [ ] Test audio collection
  - [ ] Test input handling
  - [ ] Test for each loaded plugin

### 7.3 Create Platform-Specific Tests

- [ ] 7.3.1 Test Windows Build
  - [ ] Build on Windows
  - [ ] Run plugin tests
  - [ ] Verify DLL loading
  - [ ] Test all plugins load

- [ ] 7.3.2 Test Linux Build
  - [ ] Build on Ubuntu/Debian
  - [ ] Run plugin tests
  - [ ] Verify SO loading
  - [ ] Test all plugins load

- [ ] 7.3.3 Test macOS Build
  - [ ] Build on macOS (Intel)
  - [ ] Build on macOS (Apple Silicon)
  - [ ] Run plugin tests
  - [ ] Verify DYLIB loading
  - [ ] Test all plugins load

**Deliverable:** Automated tests pass. All platforms verified.

---

## Phase 8: Documentation & Release

### 8.1 Create Developer Documentation

- [ ] 8.1.1 Create Architecture Documentation
  - [ ] Create `docs/ARCHITECTURE.md`
  - [ ] Document plugin system design
  - [ ] Document IConsole contract
  - [ ] Document IDebugger interface
  - [ ] Document manifest format
  - [ ] Document factory pattern
  - [ ] Add diagrams

- [ ] 8.1.2 Create Plugin Development Guide
  - [ ] Create `docs/PLUGIN_DEVELOPMENT.md`
  - [ ] Step-by-step guide for new console
  - [ ] Template plugin structure
  - [ ] Manifest example
  - [ ] Export functions example
  - [ ] Build system example
  - [ ] Debugging tips

- [ ] 8.1.3 Create Plugin API Reference
  - [ ] Create `docs/PLUGIN_API.md`
  - [ ] Document IConsole interface
  - [ ] Document IDebugger interface
  - [ ] Document PluginManifest structure
  - [ ] Document export macros
  - [ ] Document error codes

- [ ] 8.1.4 Create Build System Documentation
  - [ ] Create `docs/BUILD_SYSTEM.md`
  - [ ] Document CMake structure
  - [ ] Document build options
  - [ ] Document platform differences

### 8.2 Create User Documentation

- [ ] 8.2.1 Create Installation Guide
  - [ ] Create `docs/INSTALLATION.md`
  - [ ] Windows installation steps
  - [ ] Linux installation steps
  - [ ] macOS installation steps
  - [ ] System requirements
  - [ ] Troubleshooting

- [ ] 8.2.2 Create Custom Plugins Guide
  - [ ] Create `docs/CUSTOM_PLUGINS.md`
  - [ ] Explain plugin search paths
  - [ ] Document environment variables
  - [ ] Explain plugin discovery
  - [ ] Document plugin directories

### 8.3 Create Release Artifacts

- [ ] 8.3.1 Create Release Notes Template
  - [ ] Create `RELEASE_NOTES.md` template
  - [ ] Document version changes
  - [ ] Document new plugins
  - [ ] Document breaking changes
  - [ ] Document known issues

**Deliverable:** Comprehensive documentation complete.

---

## Phase 9: Final Validation & Release

### 9.1 Full Integration Testing

- [ ] 9.1.1 Test Complete Application Flow
  - [ ] Test startup and plugin loading
  - [ ] Test console selection
  - [ ] Test ROM loading per console
  - [ ] Test emulation execution
  - [ ] Test save/load states
  - [ ] Test input handling
  - [ ] Test audio output
  - [ ] Test video rendering
  - [ ] Test UI interactions
  - [ ] Test error conditions
  - [ ] Test console switching
  - [ ] Test linked mode (GBA<->GBA, GCN+GBA)

### 9.2 Performance Benchmarking

- [ ] 9.2.1 Benchmark Plugin Performance
  - [ ] Measure plugin load time
  - [ ] Measure core instantiation time
  - [ ] Benchmark NES emulation speed
  - [ ] Benchmark GBA emulation speed
  - [ ] Compare to static linking baseline
  - [ ] Profile memory usage
  - [ ] Check for memory leaks
  - [ ] Verify performance parity

### 9.3 Cleanup & Optimization

- [ ] 9.3.1 Code Cleanup
  - [ ] Remove old monolithic code
  - [ ] Remove deprecated #ifdefs
  - [ ] Clean up old factory patterns
  - [ ] Remove unused includes
  - [ ] Update all includes to use plugins

- [ ] 9.3.2 Final Optimization
  - [ ] Profile bottlenecks
  - [ ] Optimize critical paths
  - [ ] Reduce compile times
  - [ ] Minimize binary sizes

### 9.4 Create Release Builds

- [ ] 9.4.1 Generate Windows Release
  - [ ] Build Release configuration
  - [ ] Create distribution package
  - [ ] Include all DLLs
  - [ ] Include resources
  - [ ] Generate checksums
  - [ ] Sign binaries

- [ ] 9.4.2 Generate Linux Release
  - [ ] Build Release configuration
  - [ ] Create distribution package
  - [ ] Include all SOs
  - [ ] Include resources
  - [ ] Generate checksums
  - [ ] Create .deb/.rpm if applicable

- [ ] 9.4.3 Generate macOS Release
  - [ ] Build Release configuration
  - [ ] Create app bundle
  - [ ] Include all DYLIBs
  - [ ] Include resources
  - [ ] Code sign
  - [ ] Create DMG
  - [ ] Notarize for Gatekeeper

- [ ] 9.4.4 Publish Release
  - [ ] Create GitHub release
  - [ ] Upload all packages
  - [ ] Publish release notes
  - [ ] Announce release

**Deliverable:** Production release ready for all platforms.

---

## Post-Launch Maintenance

### 10.1 Monitor & Support

- [ ] Set up issue tracking
- [ ] Monitor crash reports
- [ ] Track performance metrics
- [ ] Support user issues

### 10.2 Future Plugin Development

- [ ] Document lessons learned
- [ ] Create plugin templates
- [ ] Support community plugins
- [ ] Plan for additional consoles

---

## Legend

- ✅ = Completed
- ⏳ = In Progress
- ⚠️ = Blocked/Needs Review
- ❌ = Failed/Rollback Required

## Summary Statistics

- **Total Phases:** 11
- **Total Steps:** 60+
- **Total Substeps:** 400+
- **Estimated Duration:** 15-23 weeks
- **Critical Path:** 0.0 → 0.1 → 0.2 → 1.1 → 1.1 → 3.1 → 4.1 → 5.x → 8.x → 10.x