# Proteus Plugin Architecture Restructuring Checklist

## Phase 0: Foundation & Preparation

### 0.1 Establish IConsole and IDebugger as Plugin Contracts
- [ ] 0.1.1 Review and Document Current IConsole Interface
  - [ ] Update `src/backend/shared/IConsole.hpp` with contract version
  - [ ] Document all required methods
  - [ ] Add contract stability notes
  - [ ] Verify existing implementations align with interface

- [ ] 0.1.2 Review and Document Current IDebugger Interface
  - [ ] Update `src/backend/shared/IDebugger.hpp` with contract version
  - [ ] Document all methods
  - [ ] Mark as optional for plugins
  - [ ] Verify backwards compatibility

- [ ] 0.1.3 Create Plugin Manifest Structure
  - [ ] Create `src/common/plugin/PluginManifest.hpp`
  - [ ] Define version fields (major, minor, patch)
  - [ ] Define contract version references
  - [ ] Add metadata fields (name, author, description, etc.)
  - [ ] Add development status enum

- [ ] 0.1.4 Define Plugin Export Macros
  - [ ] Create `src/common/plugin/PluginExports.hpp`
  - [ ] Define `PLUGIN_EXPORT` macro (platform-specific)
  - [ ] Define `PLUGIN_CALLING_CONVENTION` macro
  - [ ] Create `PLUGIN_FACTORY_CREATE` macro
  - [ ] Create `PLUGIN_FACTORY_DESTROY` macro
  - [ ] Create `PLUGIN_DEBUGGER_CREATE` macro (optional)
  - [ ] Create `PLUGIN_DEBUGGER_DESTROY` macro (optional)
  - [ ] Create `PLUGIN_MANIFEST_EXPORT` macro
  - [ ] Test macros compile on all platforms

**Deliverable:** Interfaces and export macros documented. No functional changes.

---

### 0.2 Create Cross-Platform Plugin Loading System

- [ ] 0.2.1 Create Core Plugin Loader
  - [ ] Create `src/common/plugin/PluginLoader.hpp`
  - [ ] Define function pointer types for exports
  - [ ] Create `LoadedPlugin` struct
  - [ ] Define `LoadPlugin()` method
  - [ ] Define `UnloadPlugin()` method
  - [ ] Define `GetExpectedFilename()` helper
  - [ ] Define `GetPluginExtension()` method
  - [ ] Define `GetLibraryPrefix()` method
  - [ ] Define `GetLastError()` method
  - [ ] Declare platform-specific implementations

- [ ] 0.2.2 Create Windows Plugin Loader
  - [ ] Create `src/common/plugin/platform/PluginLoaderWindows.cpp`
  - [ ] Implement `LoadLibraryInternal()` using `LoadLibraryA()`
  - [ ] Implement `UnloadLibraryInternal()` using `FreeLibrary()`
  - [ ] Implement `GetSymbolInternal()` using `GetProcAddress()`
  - [ ] Implement `GetPluginExtension()` returning ".dll"
  - [ ] Implement `GetLibraryPrefix()` returning ""
  - [ ] Implement `GetLastError()` with proper error messages
  - [ ] Test with real DLL loading

- [ ] 0.2.3 Create Linux Plugin Loader
  - [ ] Create `src/common/plugin/platform/PluginLoaderLinux.cpp`
  - [ ] Implement `LoadLibraryInternal()` using `dlopen()`
  - [ ] Implement `UnloadLibraryInternal()` using `dlclose()`
  - [ ] Implement `GetSymbolInternal()` using `dlsym()`
  - [ ] Implement `GetPluginExtension()` returning ".so"
  - [ ] Implement `GetLibraryPrefix()` returning "lib"
  - [ ] Implement `GetLastError()` using `dlerror()`
  - [ ] Test with real SO loading

- [ ] 0.2.4 Create macOS Plugin Loader
  - [ ] Create `src/common/plugin/platform/PluginLoaderMacOS.cpp`
  - [ ] Implement `LoadLibraryInternal()` using `dlopen()`
  - [ ] Implement `UnloadLibraryInternal()` using `dlclose()`
  - [ ] Implement `GetSymbolInternal()` using `dlsym()`
  - [ ] Implement `GetPluginExtension()` returning ".dylib"
  - [ ] Implement `GetLibraryPrefix()` returning "lib"
  - [ ] Implement `GetLastError()` using `dlerror()`
  - [ ] Test with real DYLIB loading
  - [ ] Handle Apple Silicon and Intel architectures

- [ ] 0.2.5 Create Platform-Agnostic Plugin Loader Implementation
  - [ ] Create `src/common/plugin/PluginLoader.cpp`
  - [ ] Implement `LoadPlugin()` with validation
  - [ ] Implement manifest loading and version checking
  - [ ] Implement factory function resolution
  - [ ] Implement optional debugger function resolution
  - [ ] Implement `UnloadPlugin()` with cleanup
  - [ ] Implement `GetExpectedFilename()` helper
  - [ ] Add comprehensive error handling

**Deliverable:** Cross-platform plugin loader compiles and links. Plugins can be loaded/unloaded dynamically.

---

## Phase 1: Backend Infrastructure Consolidation

### 1.1 Consolidate Shared Backend Interfaces

- [ ] 1.1.1 Update Backend PCH
  - [ ] Create/Update `src/backend/shared/BackendPCH.hpp`
  - [ ] Define type aliases (u8, u16, u32, u64, etc.)
  - [ ] Define smart pointer aliases (sptr, uptr)
  - [ ] Include standard library headers
  - [ ] Include `IConsole.hpp`
  - [ ] Include `IDebugger.hpp`
  - [ ] Define `SSTstate` enum
  - [ ] Verify all backends can compile with this PCH

- [ ] 1.1.2 Create Shared Backend Utilities
  - [ ] Create `src/backend/shared/BackendUtils.hpp`
  - [ ] Create `GetConsoleName()` helper
  - [ ] Create `GetButtonName()` conversion
  - [ ] Create `Clamp()` template function
  - [ ] Create bit manipulation helpers (IsBitSet, SetBit, ClearBit)
  - [ ] Add utility functions used across backends
  - [ ] Document all utilities

**Deliverable:** Backend infrastructure organized. Shared utilities available for all plugins.

---

## Phase 2: NES Core as First Plugin

### 2.1 Create NES Plugin Structure

- [ ] 2.1.1 Create NES CMakeLists.txt (SHARED Library)
  - [ ] Create `src/backends/nes/CMakeLists.txt`
  - [ ] Define shared library target `ProteusNES`
  - [ ] Add plugin interface source files
  - [ ] Add core emulation source files
  - [ ] Add CPU, PPU, APU source files
  - [ ] Add cartridge and input source files
  - [ ] Set include directories
  - [ ] Configure compiler settings (C++17)
  - [ ] Set platform-specific output properties
  - [ ] Configure install targets
  - [ ] Set output to `${CMAKE_BINARY_DIR}/plugins`

- [ ] 2.1.2 Create NES Plugin Manifest and Exports
  - [ ] Create `src/backends/nes/plugin/NESPluginExports.cpp`
  - [ ] Define `PluginManifest` static instance
  - [ ] Populate all manifest fields
  - [ ] Implement `CreateCore()` factory function
  - [ ] Implement `DestroyCore()` cleanup function
  - [ ] Implement `CreateDebugger()` optional function
  - [ ] Implement `DestroyDebugger()` optional function
  - [ ] Implement `GetPluginManifest()` export
  - [ ] Verify all exports use correct macros

- [ ] 2.1.3 Create NES Core Implementation Header
  - [ ] Create `src/backends/nes/plugin/NESCoreImpl.hpp`
  - [ ] Define `NESCoreImpl` class implementing `IConsole`
  - [ ] Declare all IConsole methods
  - [ ] Add member variable for `NS_NES::NES`
  - [ ] Add initialization state tracking
  - [ ] Document class purpose and relationship to backend

- [ ] 2.1.4 Create NES Core Implementation
  - [ ] Create `src/backends/nes/plugin/NESCoreImpl.cpp`
  - [ ] Implement constructor and destructor
  - [ ] Implement `Initialize()`
  - [ ] Implement `Shutdown()`
  - [ ] Implement `loadROM()`
  - [ ] Implement `reset()`
  - [ ] Implement `clock()`
  - [ ] Implement `getFrameBuffer()`
  - [ ] Implement `collectAudio()`
  - [ ] Implement `update()`
  - [ ] Implement SST methods
  - [ ] Add error handling and logging

- [ ] 2.1.5 Create NES Debugger Implementation
  - [ ] Create `src/backends/nes/plugin/NESDebuggerImpl.hpp`
  - [ ] Define `NESDebuggerImpl` class implementing `IDebugger`
  - [ ] Declare all IDebugger methods
  - [ ] Add reference to associated console

- [ ] 2.1.6 Create NES Debugger Implementation
  - [ ] Create `src/backends/nes/plugin/NESDebuggerImpl.cpp`
  - [ ] Implement constructor
  - [ ] Implement all debugger methods
  - [ ] Add breakpoint handling
  - [ ] Add condition tracking

**Deliverable:** NES compiles as plugin DLL/SO/DYLIB. Implements IConsole contract.

---

## Phase 3: Other Console Cores as Plugins

### 3.1 Repeat NES Plugin Structure for GBA

- [ ] 3.1.1 Create GBA CMakeLists.txt
  - [ ] Create `src/backends/gba/CMakeLists.txt`
  - [ ] Follow same structure as NES plugin
  - [ ] Configure for GBA-specific sources
  - [ ] Set output directory

- [ ] 3.1.2 Create GBA Plugin Exports
  - [ ] Create `src/backends/gba/plugin/GBAPluginExports.cpp`
  - [ ] Define manifest for GBA
  - [ ] Implement factory functions
  - [ ] Implement debugger functions

- [ ] 3.1.3 Create GBA Core Implementation
  - [ ] Create `src/backends/gba/plugin/GBACoreImpl.hpp`
  - [ ] Create `src/backends/gba/plugin/GBACoreImpl.cpp`
  - [ ] Implement all IConsole methods
  - [ ] Reference GBA-specific backends

- [ ] 3.1.4 Create GBA Debugger Implementation
  - [ ] Create `src/backends/gba/plugin/GBADebuggerImpl.hpp`
  - [ ] Create `src/backends/gba/plugin/GBADebuggerImpl.cpp`
  - [ ] Implement debugger functionality

### 3.2 Create Stub Cores for Unsupported Consoles

- [ ] 3.2.1 Create SNES Stub Plugin
  - [ ] Create `src/backends/snes/CMakeLists.txt`
  - [ ] Create `src/backends/snes/plugin/SNESStubImpl.hpp`
  - [ ] Create `src/backends/snes/plugin/SNESPluginExports.cpp`
  - [ ] Set development status to "stub"

- [ ] 3.2.2 Create PS1 Stub Plugin
  - [ ] Create `src/backends/ps1/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

- [ ] 3.2.3 Create N64 Stub Plugin
  - [ ] Create `src/backends/n64/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

- [ ] 3.2.4 Create PS2 Stub Plugin
  - [ ] Create `src/backends/ps2/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

- [ ] 3.2.5 Create GBC Stub Plugin
  - [ ] Create `src/backends/gbc/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

- [ ] 3.2.6 Create NGC (GameCube) Stub Plugin
  - [ ] Create `src/backends/ngc/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

- [ ] 3.2.7 Create Xbox Stub Plugin
  - [ ] Create `src/backends/xbox/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

- [ ] 3.2.8 Create NDS Stub Plugin
  - [ ] Create `src/backends/nds/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

- [ ] 3.2.9 Create Xbox 360 Stub Plugin
  - [ ] Create `src/backends/x360/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

- [ ] 3.2.10 Create PS3 Stub Plugin
  - [ ] Create `src/backends/ps3/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

- [ ] 3.2.11 Create Wii Stub Plugin
  - [ ] Create `src/backends/wii/CMakeLists.txt`
  - [ ] Create stub implementation
  - [ ] Create plugin exports

**Deliverable:** All console cores (NES, GBA, and stubs) compile as plugins.

---

## Phase 4: Plugin Registry & Discovery

### 4.1 Create Plugin Registry

- [ ] 4.1.1 Plugin Registry Header
  - [ ] Create `src/common/plugin/PluginRegistry.hpp`
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
  - [ ] Declare `GetLoadedPlugins()` lister
  - [ ] Declare `UnloadAllPlugins()` method
  - [ ] Document all methods

- [ ] 4.1.2 Plugin Registry Implementation
  - [ ] Create `src/common/plugin/PluginRegistry.cpp`
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

## Phase 5: Update Frontend to Use Plugin System

### 5.1 Create Plugin Manager for Frontend

- [ ] 5.1.1 Plugin Manager Header
  - [ ] Create `src/frontend/plugin/PluginManager.hpp`
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

- [ ] 5.1.2 Plugin Manager Implementation
  - [ ] Create `src/frontend/plugin/PluginManager.cpp`
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

- [ ] 5.1.3 Update ConsoleFactory to Use Plugins
  - [ ] Update `src/frontend/session/ConsoleFactory.cpp`
  - [ ] Create `GetConsoleNameString()` mapper
  - [ ] Modify `Create()` to use PluginManager
  - [ ] Update error handling
  - [ ] Add logging

- [ ] 5.1.4 Update DebuggerFactory to Use Plugins
  - [ ] Update `src/frontend/session/DebuggerFactory.cpp`
  - [ ] Modify `Create()` to use PluginManager
  - [ ] Handle missing debugger support gracefully
  - [ ] Update error handling

- [ ] 5.1.5 Update Proteus::Init() to Use Plugin Manager
  - [ ] Modify `src/frontend/app/Proteus.cpp`
  - [ ] Call `PluginManager::Initialize()` early
  - [ ] Create `UpdateConsoleAvailability()` method
  - [ ] Populate `ConsoleEmuStarted` from loaded plugins
  - [ ] Add error handling

- [ ] 5.1.6 Update Proteus::Deinit() to Use Plugin Manager
  - [ ] Call `PluginManager::Shutdown()` at exit
  - [ ] Handle cleanup errors
  - [ ] Add logging

**Deliverable:** Frontend uses plugin system. Consoles loaded dynamically at runtime.

---

## Phase 6: Build System Configuration

### 6.1 Root CMakeLists.txt

- [ ] 6.1.1 Create Root CMakeLists.txt
  - [ ] Create `CMakeLists.txt` in project root
  - [ ] Set minimum version to 3.16
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

### 6.2 src/CMakeLists.txt

- [ ] 6.2.1 Create src/CMakeLists.txt
  - [ ] Add common subdirectory
  - [ ] Add backend/shared subdirectory
  - [ ] Add backends subdirectory (conditional)
  - [ ] Add frontend subdirectory

### 6.3 src/common/CMakeLists.txt

- [ ] 6.3.1 Create src/common/CMakeLists.txt
  - [ ] Create `ProteusCommon` static library
  - [ ] Add plugin source files
  - [ ] Add platform-specific loaders
  - [ ] Set include directories
  - [ ] Set compiler settings
  - [ ] Enable position-independent code (PIC)

### 6.4 src/backends/CMakeLists.txt

- [ ] 6.4.1 Create src/backends/CMakeLists.txt
  - [ ] Conditionally add NES subdirectory
  - [ ] Conditionally add GBA subdirectory
  - [ ] Conditionally add stub subdirectories
  - [ ] Configure install for all plugins

### 6.5 src/frontend/CMakeLists.txt

- [ ] 6.5.1 Create src/frontend/CMakeLists.txt
  - [ ] Find SDL3 package
  - [ ] Create `Proteus` executable
  - [ ] Add all frontend source files
  - [ ] Set include directories
  - [ ] Link against ProteusCommon and SDL3
  - [ ] Set compiler settings
  - [ ] Configure install target

**Deliverable:** Build system complete and functional. Projects compile cleanly on all platforms.

---

## Phase 7: Multi-Instance & Inter-Core Communication

### 7.1 Design Multi-Instance Session Management

- [ ] 7.1.1 Multi-Core Session Header
  - [ ] Create `src/frontend/session/MultiCoreSession.hpp`
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

- [ ] 7.1.2 Multi-Core Session Implementation
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

### 7.2 Link Cable Simulation (Phase 7.2)

- [ ] 7.2.1 Design Link Cable Protocol
  - [ ] Create `src/frontend/session/LinkCableSimulator.hpp`
  - [ ] Define data transfer format
  - [ ] Define handshake protocol
  - [ ] Support multiple console pairs

- [ ] 7.2.2 Implement Link Cable
  - [ ] Create `src/frontend/session/LinkCableSimulator.cpp`
  - [ ] Implement data transfer
  - [ ] Implement synchronization
  - [ ] Add logging/debugging

**Deliverable:** Multi-instance architecture designed. Single-core mode still works.

---

## Phase 8: Cross-Platform Build Scripts

### 8.1 Create Windows Build Script

- [ ] 8.1.1 Create build_windows.ps1
  - [ ] Create `build_windows.ps1` script
  - [ ] Accept BuildType parameter (Release/Debug)
  - [ ] Accept Architecture parameter (x64/x86)
  - [ ] Create build directory
  - [ ] Run cmake with appropriate flags
  - [ ] Build project
  - [ ] Handle errors
  - [ ] Report output location

### 8.2 Create Linux Build Script

- [ ] 8.2.1 Create build_linux.sh
  - [ ] Create `build_linux.sh` script
  - [ ] Accept BuildType parameter
  - [ ] Create build directory
  - [ ] Run cmake with appropriate flags
  - [ ] Build with parallel jobs
  - [ ] Handle errors
  - [ ] Report output location

### 8.3 Create macOS Build Script

- [ ] 8.3.1 Create build_macos.sh
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

## Phase 9: Testing & Validation

### 9.1 Create Plugin Load Tests

- [ ] 9.1.1 Create Plugin Load Test Suite
  - [ ] Create `tests/plugin/PluginLoadTest.cpp`
  - [ ] Add `DiscoverPlugins` test
  - [ ] Add `LoadNESPlugin` test
  - [ ] Add `LoadGBAPlugin` test
  - [ ] Add `CreateNESCore` test
  - [ ] Add `CreateGBACore` test
  - [ ] Add `PluginManifest` validation test
  - [ ] Add `ContractVersion` compatibility test
  - [ ] Test error handling

### 9.2 Create Plugin Integration Tests

- [ ] 9.2.1 Create Plugin Integration Test Suite
  - [ ] Create `tests/plugin/PluginIntegrationTest.cpp`
  - [ ] Test load ROM through plugin
  - [ ] Test emulation cycles
  - [ ] Test frame buffer access
  - [ ] Test audio collection
  - [ ] Test input handling
  - [ ] Test for each loaded plugin

### 9.3 Create Platform-Specific Tests

- [ ] 9.3.1 Test Windows Build
  - [ ] Build on Windows
  - [ ] Run plugin tests
  - [ ] Verify DLL loading
  - [ ] Test all plugins load

- [ ] 9.3.2 Test Linux Build
  - [ ] Build on Ubuntu/Debian
  - [ ] Run plugin tests
  - [ ] Verify SO loading
  - [ ] Test all plugins load

- [ ] 9.3.3 Test macOS Build
  - [ ] Build on macOS (Intel)
  - [ ] Build on macOS (Apple Silicon)
  - [ ] Run plugin tests
  - [ ] Verify DYLIB loading
  - [ ] Test all plugins load

**Deliverable:** Automated tests pass. All platforms verified.

---

## Phase 10: Documentation & Release

### 10.1 Create Developer Documentation

- [ ] 10.1.1 Create Architecture Documentation
  - [ ] Create `docs/ARCHITECTURE.md`
  - [ ] Document plugin system design
  - [ ] Document IConsole contract
  - [ ] Document IDebugger interface
  - [ ] Document manifest format
  - [ ] Document factory pattern
  - [ ] Add diagrams

- [ ] 10.1.2 Create Plugin Development Guide
  - [ ] Create `docs/PLUGIN_DEVELOPMENT.md`
  - [ ] Step-by-step guide for new console
  - [ ] Template plugin structure
  - [ ] Manifest example
  - [ ] Export functions example
  - [ ] Build system example
  - [ ] Debugging tips

- [ ] 10.1.3 Create Plugin API Reference
  - [ ] Create `docs/PLUGIN_API.md`
  - [ ] Document IConsole interface
  - [ ] Document IDebugger interface
  - [ ] Document PluginManifest structure
  - [ ] Document export macros
  - [ ] Document error codes

- [ ] 10.1.4 Create Build System Documentation
  - [ ] Create `docs/BUILD_SYSTEM.md`
  - [ ] Document CMake structure
  - [ ] Document build options
  - [ ] Document platform differences

### 10.2 Create User Documentation

- [ ] 10.2.1 Create Installation Guide
  - [ ] Create `docs/INSTALLATION.md`
  - [ ] Windows installation steps
  - [ ] Linux installation steps
  - [ ] macOS installation steps
  - [ ] System requirements
  - [ ] Troubleshooting

- [ ] 10.2.2 Create Custom Plugins Guide
  - [ ] Create `docs/CUSTOM_PLUGINS.md`
  - [ ] Explain plugin search paths
  - [ ] Document environment variables
  - [ ] Explain plugin discovery
  - [ ] Document plugin directories

### 10.3 Create Release Artifacts

- [ ] 10.3.1 Create Release Notes Template
  - [ ] Create `RELEASE_NOTES.md` template
  - [ ] Document version changes
  - [ ] Document new plugins
  - [ ] Document breaking changes
  - [ ] Document known issues

**Deliverable:** Comprehensive documentation complete.

---

## Phase 11: Final Validation & Release

### 11.1 Full Integration Testing

- [ ] 11.1.1 Test Complete Application Flow
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

### 11.2 Performance Benchmarking

- [ ] 11.2.1 Benchmark Plugin Performance
  - [ ] Measure plugin load time
  - [ ] Measure core instantiation time
  - [ ] Benchmark NES emulation speed
  - [ ] Benchmark GBA emulation speed
  - [ ] Compare to static linking baseline
  - [ ] Profile memory usage
  - [ ] Check for memory leaks
  - [ ] Verify performance parity

### 11.3 Cleanup & Optimization

- [ ] 11.3.1 Code Cleanup
  - [ ] Remove old monolithic code
  - [ ] Remove deprecated #ifdefs
  - [ ] Clean up old factory patterns
  - [ ] Remove unused includes
  - [ ] Update all includes to use plugins

- [ ] 11.3.2 Final Optimization
  - [ ] Profile bottlenecks
  - [ ] Optimize critical paths
  - [ ] Reduce compile times
  - [ ] Minimize binary sizes

### 11.4 Create Release Builds

- [ ] 11.4.1 Generate Windows Release
  - [ ] Build Release configuration
  - [ ] Create distribution package
  - [ ] Include all DLLs
  - [ ] Include resources
  - [ ] Generate checksums
  - [ ] Sign binaries

- [ ] 11.4.2 Generate Linux Release
  - [ ] Build Release configuration
  - [ ] Create distribution package
  - [ ] Include all SOs
  - [ ] Include resources
  - [ ] Generate checksums
  - [ ] Create .deb/.rpm if applicable

- [ ] 11.4.3 Generate macOS Release
  - [ ] Build Release configuration
  - [ ] Create app bundle
  - [ ] Include all DYLIBs
  - [ ] Include resources
  - [ ] Code sign
  - [ ] Create DMG
  - [ ] Notarize for Gatekeeper

- [ ] 11.4.4 Publish Release
  - [ ] Create GitHub release
  - [ ] Upload all packages
  - [ ] Publish release notes
  - [ ] Announce release

**Deliverable:** Production release ready for all platforms.

---

## Post-Launch Maintenance

### 12.1 Monitor & Support

- [ ] Set up issue tracking
- [ ] Monitor crash reports
- [ ] Track performance metrics
- [ ] Support user issues

### 12.2 Future Plugin Development

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

- **Total Phases:** 12
- **Total Steps:** 60+
- **Total Substeps:** 400+
- **Estimated Duration:** 15-23 weeks
- **Critical Path:** 0.0 → 0.1 → 0.2 → 1.1 → 2.1 → 4.1 → 5.1 → 6.x → 9.x → 11.x