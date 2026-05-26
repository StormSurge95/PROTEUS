# Proteus Plugin Architecture Restructuring Plan

Complete Outline with Code Examples

**Document Version:** 1.0
**Last Updated:** May 26, 2026
**Status:** Ready for Implementation

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Phase 0: Foundation & Preparation](#phase-0-foundation-preparation)
4. [Phase 1: NES Core as First Plugin](#phase-2-nes-core-as-first-plugin)
5. [Phase 2: Plugin Registry & Discovery](#phase-3-plugin-registry-discovery)
6. [Phase 3: Update Frontend to Use Plugin System](#phase-4-update-frontend-to-use-plugin-system)
7. [Phase 4: Build System Configuration](#phase-5-build-system-configuration)
8. [Phase 5: Multi-Instance & Inter-Core Communication](#phase-6-multi-instance-inter-core-communication)
9. [Phase 6: Cross-Platform Build Scripts](#phase-7-cross-platform-build-scripts)
10. [Phase 7: Testing & Validation](#phase-8-testing-validation)
11. [Phase 8: Documentation & Release](#phase-9-documentation-release)
12. [Phase 9: Final Validation & Release](#phase-10-final-validation-release)
13. [Timeline & Success Criteria](#timeline-success-criteria)

---

## Architecture Overview

The restructuring transforms Proteus from a monolithic application into a plugin-based architecture:

```
┌─────────────────────────────────────────────────────────────────┐
│                      PROTEUS FRONTEND                           │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │  Main Application (Proteus.cpp)                          │   │
│  │  - UI/Input/Audio/Video Management                       │   │
│  │  - ConsoleSession (orchestrator)                         │   │
│  └──────────────────────────────────────────────────────────┘   │
│                           ▲                                     │
│                           │ uses                                │
│                    ┌──────┴──────┐                              │
│                    │             │                              │
│             ┌──────▼──┐   ┌──────▼────┐                         │
│             │IConsole*│   │IDebugger* │  ◄─ BRIDGE INTERFACES   │
│             └──────┬──┘   └──────┬────┘                         │
└────────────────────┼─────────────┼──────────────────────────────┘
                     │             │
                     │             │ implemented by
                     │             │
┌────────────────────┼─────────────┼────────────────────────────────┐
│             PLUGIN LOADING LAYER                                  │
│  ┌────────────────────────────────────────────────────────────┐   │
│  │  PluginRegistry & PluginLoader                             │   │
│  │  - Discover plugins in standard paths                      │   │
│  │  - Load DLL/SO/DYLIB at runtime                            │   │
│  │  - Resolve factory functions                               │   │
│  └────────────────────────────────────────────────────────────┘   │
└─────────────────────┬────────────┬────────────────────────────────┘
                      │            │
                      │ returns    │
                      │            │
   ┌──────────────────┴────────────┴──────────────────┐
   │                                                  │
┌──▼──────────────┐  ┌──────────────┐  ┌──────────────▼──┐
│ nes.dll/so/dylib│  │ gba.dll/so/  │  │ snes.dll/so/    │
│                 │  │ dylib        │  │ dylib (stub)    │
│ ┌─────────────┐ │  │ ┌──────────┐ │  │ ┌──────────────┐│
│ │ NESCore     │ │  │ │ GBACore  │ │  │ │ SNESCoreStub ││
│ │implements   │ │  │ │implements│ │  │ │ implements   ││
│ │IConsole     │ │  │ │IConsole  │ │  │ │ IConsole     ││
│ └─────────────┘ │  │ └──────────┘ │  │ └──────────────┘│
└─────────────────┘  └──────────────┘  └─────────────────┘
```

Key Design Principles:

1. IConsole and IDebugger are the primary bridge between frontend and plugins
2. Each console core is compiled as a separate shared library
3. Plugins are discovered at runtime from standard search paths
4. Factory functions (CreateCore, DestroyCore) manage instance lifecycle
5. Manifest structure validates plugin compatibility before loading
6. Platform-specific loading abstracted behind PluginLoader
7. Frontend remains console-agnostic through factory pattern

---

## Phase 0: Foundation & Preparation

### Overview

This phase establishes the foundational interfaces and plugin loading infrastructure that all subsequent work will build upon. The goal is to transform existing IConsole and IDebugger interfaces into formalized plugin contracts, and implement a robust cross-platform plugin loader.

**Deliverable:** Cross-platform plugin loader fully implemented and tested on Windows, Linux, and macOS.

**Duration:** 1-2 weeks

**Dependencies:** None (foundation phase)

---

### 0.1 Establish IConsole and IDebugger as Plugin Contracts

**Goal:** Clarify and document existing interfaces as the primary plugin API contract

#### 0.1.1 Review and Document Current IConsole Interface

**File to Update:** `src/backend/shared/IConsole.h`

**Actions:**

1. Add contract version constant at the top of the file
2. Add comprehensive documentation explaining this is the primary plugin contract
3. Ensure all methods are clearly documented with their purpose
4. Verify method signatures will remain stable

**Updated Code:**

```
#pragma once

#include "./BackendPCH.h"

/**
 * @interface IConsole
 * @brief Primary bridge interface between frontend and emulator cores
 * @details This interface is the CONTRACT that all console emulation
 *          plugins MUST implement. Frontend code ONLY interacts with
 *          console cores through this interface.
 *
 * IMPORTANT: Changes to this interface are BREAKING CHANGES for all
 *            existing plugins. Version carefully and increment
 *            ICONSOLE_CONTRACT_VERSION when modifications occur.
 *
 * Plugin developers should implement this interface for their console
 * emulation core. The frontend will instantiate cores through factory
 * functions and interact exclusively through this interface.
 */
class IConsole {
public:
    virtual ~IConsole() = default;

    // === Core Lifecycle ===
    
    /**
     * @brief Initialize the console
     * @details Called after construction to set up internal state.
     *          Safe to call multiple times; should return true if
     *          already initialized.
     * @return true if successful, false on failure
     */
    virtual bool Initialize() { return true; }

    /**
     * @brief Shutdown the console and release resources
     * @details Called before destruction to clean up resources.
     *          Should be safe to call multiple times.
     * @return true if successful, false on failure
     */
    virtual bool Shutdown() { return true; }

    // === ROM Management ===
    
    /**
     * @brief Load a ROM file into the console
     * @param romPath Full file system path to the ROM file
     * @return true if loaded successfully, false on error
     */
    virtual bool loadROM(const string&) = 0;

    // === Emulation Control ===
    
    /**
     * @brief Reset the console to its initial power-on state
     * @details Clears RAM, resets CPU/GPU/APU, but keeps ROM loaded
     */
    virtual void reset() = 0;

    /**
     * @brief Execute one master clock cycle
     * @details Advances emulation by one clock pulse. The frontend
     *          calls this repeatedly to run the emulation.
     */
    virtual void clock() = 0;

    // === Video Output ===
    
    /**
     * @brief Get the framebuffer containing rendered pixels
     * @return Pointer to ARGB8888 framebuffer data (do not free)
     *         or nullptr if no frame is ready
     */
    virtual const u32* getFrameBuffer() const = 0;

    /**
     * @brief Get native screen width in pixels
     * @return Screen width for this console (e.g., 256 for NES)
     */
    virtual const int SCREEN_WIDTH() const = 0;

    /**
     * @brief Get native screen height in pixels
     * @return Screen height for this console (e.g., 240 for NES)
     */
    virtual const int SCREEN_HEIGHT() const = 0;

    // === Audio Output ===
    
    /**
     * @brief Collect audio samples from the console
     * @param outSamples Vector to fill with float audio data
     *                   Values should be normalized to [-1.0, 1.0]
     */
    virtual void collectAudio(vector<float>&) = 0;

    // === Input ===
    
    /**
     * @brief Send input state to the console
     * @param playerIndex Player index (0-3 for multiplayer support)
     * @param buttonStates Array of 8 boolean button states
     *                     Order: Up, Down, Left, Right, A, B, Select, Start
     */
    virtual void update(u8 playerIndex, bool* buttonStates) = 0;

    // === Debugging Support (Optional) ===
    
    /**
     * @brief Initialize single-step testing mode
     * @param state The step state to initialize
     */
    virtual void initSST(SSTstate) = 0;

    /**
     * @brief Execute a single step in SST mode
     */
    virtual void runSST() = 0;

    /**
     * @brief Check SST mode state
     * @param state The state to check
     * @param outMessage Output message describing the state
     * @return true if state is valid, false otherwise
     */
    virtual bool checkSST(SSTstate, string&) = 0;
};

// Plugin contract version - increment on breaking changes
// Current version: 1
// If you modify IConsole, increment this and update all plugins
#define ICONSOLE_CONTRACT_VERSION 1
```

[^](#table-of-contents)
---

#### 0.1.2 Review and Document Current IDebugger Interface

**File to Update:** `src/backend/shared/IDebugger.h`

**Actions:**

1. Add contract version constant
2. Add documentation explaining debugger is optional
3. Clearly mark all debugging methods
4. Document forward declaration needs

**Updated Code:**

```
#pragma once

#include "./BackendPCH.h"

// Forward declaration
class IConsole;

/**
 * @interface IDebugger
 * @brief Debugging interface for console cores
 * @details Provides debugging capabilities for console emulation.
 *          Debuggers are OPTIONAL - not all console plugins need to
 *          implement this. The frontend gracefully handles plugins
 *          that do not provide debugger support.
 *
 * Debuggers are typically created alongside IConsole instances
 * via factory functions and are specific to each console type.
 * A single debugger instance is associated with one console instance.
 */
class IDebugger {
public:
    virtual ~IDebugger() = default;

    // === Breakpoint Management ===

    /**
     * @brief Set a breakpoint on the next frame boundary
     * @details Execution will pause when the next frame is rendered
     */
    virtual void breakOnNextFrame() = 0;

    /**
     * @brief Set a conditional breakpoint
     * @param condition Expression to evaluate (format is debugger-specific)
     * @details Execution will pause when condition becomes true
     */
    virtual void breakOnCondition(const string& condition) = 0;

    /**
     * @brief Check if a breakpoint is currently set
     * @return true if any breakpoint is active
     */
    virtual bool isBreakpointSet() const = 0;

    // Additional debugging methods as needed for specific consoles
};

// Debugger interface contract version - increment on breaking changes
// Current version: 1
#define IDEBUGGER_CONTRACT_VERSION 1
```

[^](#table-of-contents)
---

#### 0.1.3 Create Plugin Manifest Structure

**File to Create:** `src/common/plugin/PluginManifest.h`

**Actions:**

1. Create new directory `src/common/plugin/` if it doesn't exist
2. Create the PluginManifest.h file with the structure below

**New Code:**

```
#pragma once

#include <cstdint>

/**
 * @struct PluginManifest
 * @brief Metadata exported by each plugin
 * @details Every plugin must export GetPluginManifest() returning
 *          a valid manifest. The frontend uses this to validate
 *          compatibility and display plugin information.
 *
 * The manifest serves several purposes:
 * - Validates API contract compatibility before loading
 * - Provides metadata for UI display and logging
 * - Allows versioning and future extensibility
 * - Documents plugin capabilities and status
 */
struct PluginManifest {
    // === Version & Compatibility ===
    
    /**
     * @brief Plugin version (major component)
     * @details Typically incremented for major feature releases
     */
    uint16_t pluginVersion_major;
    
    /**
     * @brief Plugin version (minor component)
     * @details Typically incremented for minor features or fixes
     */
    uint16_t pluginVersion_minor;
    
    /**
     * @brief Plugin version (patch component)
     * @details Typically incremented for bug fixes only
     */
    uint16_t pluginVersion_patch;
    
    /**
     * @brief IConsole contract version this plugin implements
     * @details Must match ICONSOLE_CONTRACT_VERSION in frontend
     *          If mismatch detected, plugin will be rejected as incompatible
     */
    uint16_t iConsoleContractVersion;
    
    /**
     * @brief IDebugger contract version (if applicable)
     * @details Must match IDEBUGGER_CONTRACT_VERSION if debugger is provided
     *          Only validated if debugger functions are exported
     */
    uint16_t iDebuggerContractVersion;

    // === Identification ===
    
    /**
     * @brief Short console name (lowercase, no spaces)
     * @details Examples: "nes", "gba", "snes"
     *          Used for plugin discovery and identification
     */
    const char* consoleName;
    
    /**
     * @brief Full console name (display friendly)
     * @details Examples: "Nintendo Entertainment System"
     *          Used in UI and logging
     */
    const char* consoleFullName;
    
    /**
     * @brief Plugin display name
     * @details Examples: "Proteus NES Emulation Core"
     *          Used in UI to identify the specific plugin
     */
    const char* pluginName;
    
    /**
     * @brief Author or organization name
     * @details Credit line for the plugin creator
     */
    const char* authorName;
    
    /**
     * @brief Plugin description
     * @details Brief description of what this plugin emulates
     *          and any notable features
     */
    const char* description;

    // === Metadata ===
    
    /**
     * @brief Build date in ISO 8601 format (YYYY-MM-DD)
     * @details Example: "2026-05-26"
     */
    const char* buildDate;
    
    /**
     * @brief License type identifier
     * @details Examples: "MIT", "GPL-3.0", "Proprietary"
     */
    const char* licenseType;

    // === Status ===
    
    /**
     * @brief Development status of this plugin
     * @details Values:
     *   0 = Stub (placeholder, not functional)
     *   1 = Work in Progress (early development)
     *   2 = Partial (core features working, incomplete)
     *   3 = Complete (fully functional)
     *   4+ = Reserved for future use
     */
    uint8_t developmentStatus;
    
    /**
     * @brief Human-readable status description
     * @details Examples:
     *   "Stub implementation only"
     *   "Early development, expect crashes"
     *   "Core gameplay working, debugging incomplete"
     *   "Full implementation with debug support"
     */
    const char* statusDescription;
};
```

[^](#table-of-contents)
---

#### 0.1.4 Define Plugin Export Macros

**File to Create:** `src/common/plugin/PluginExports.h`

**Actions:**

1. Create the file in `src/common/plugin/`
2. Define all platform-specific export macros
3. Define all plugin function declaration macros

**New Code:**

```
#pragma once

#include "PluginManifest.h"

// ============================================================================
// Platform-Specific Export Declarations
// ============================================================================

/**
 * @def PLUGIN_EXPORT
 * @brief Platform-specific DLL/SO/DYLIB export declaration
 * @details Ensures symbols are visible when loading plugin at runtime
 */
#ifdef _WIN32
    #define PLUGIN_EXPORT extern "C" __declspec(dllexport)
    #define PLUGIN_CALLING_CONVENTION __cdecl
#elif defined(__GNUC__)
    #define PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
    #define PLUGIN_CALLING_CONVENTION
#else
    #define PLUGIN_EXPORT extern "C"
    #define PLUGIN_CALLING_CONVENTION
#endif

// Forward declarations from backend headers
class IConsole;
class IDebugger;

// ============================================================================
// Plugin Function Declaration Macros
// ============================================================================

/**
 * @def PLUGIN_FACTORY_CREATE
 * @brief Declares the core factory function
 * @details REQUIRED EXPORT - Every plugin must provide exactly one
 *
 * Function signature: IConsole* CreateXXXCore(void)
 *
 * Usage example:
 *   PLUGIN_FACTORY_CREATE(CreateNESCore) {
 *       return new NESCoreImpl();
 *   }
 *
 * The function name (e.g., "CreateNESCore") can be anything, but should
 * follow the pattern "Create[ConsoleType]Core" for clarity.
 */
#define PLUGIN_FACTORY_CREATE(funcName) \
    PLUGIN_EXPORT IConsole* PLUGIN_CALLING_CONVENTION funcName(void)

/**
 * @def PLUGIN_FACTORY_DESTROY
 * @brief Declares the core destructor function
 * @details REQUIRED EXPORT - Must exist if PLUGIN_FACTORY_CREATE exists
 *
 * Function signature: void DestroyXXXCore(IConsole* core)
 *
 * Usage example:
 *   PLUGIN_FACTORY_DESTROY(DestroyNESCore) {
 *       delete reinterpret_cast<NESCoreImpl*>(core);
 *   }
 *
 * The frontend will use this to properly clean up core instances.
 * Do NOT assume the pointer type - always delete through IConsole*.
 */
#define PLUGIN_FACTORY_DESTROY(funcName) \
    PLUGIN_EXPORT void PLUGIN_CALLING_CONVENTION funcName(IConsole* core)

/**
 * @def PLUGIN_DEBUGGER_CREATE
 * @brief Declares the debugger factory function
 * @details OPTIONAL EXPORT - Plugins may provide debugger support
 *
 * Function signature: IDebugger* CreateXXXDebugger(IConsole* core)
 *
 * Usage example:
 *   PLUGIN_DEBUGGER_CREATE(CreateNESDebugger) {
 *       return new NESDebuggerImpl(core);
 *   }
 *
 * If not implemented, omit this macro entirely. Frontend handles
 * missing debugger support gracefully. If implemented, must also
 * implement PLUGIN_DEBUGGER_DESTROY.
 */
#define PLUGIN_DEBUGGER_CREATE(funcName) \
    PLUGIN_EXPORT IDebugger* PLUGIN_CALLING_CONVENTION funcName(IConsole* core)

/**
 * @def PLUGIN_DEBUGGER_DESTROY
 * @brief Declares the debugger destructor function
 * @details OPTIONAL EXPORT - Must exist if PLUGIN_DEBUGGER_CREATE exists
 *
 * Function signature: void DestroyXXXDebugger(IDebugger* debugger)
 *
 * Usage example:
 *   PLUGIN_DEBUGGER_DESTROY(DestroyNESDebugger) {
 *       delete reinterpret_cast<NESDebuggerImpl*>(debugger);
 *   }
 *
 * Required pairing: PLUGIN_DEBUGGER_CREATE and PLUGIN_DEBUGGER_DESTROY
 * must both exist or both be omitted.
 */
#define PLUGIN_DEBUGGER_DESTROY(funcName) \
    PLUGIN_EXPORT void PLUGIN_CALLING_CONVENTION funcName(IDebugger* debugger)

/**
 * @def PLUGIN_MANIFEST_EXPORT
 * @brief Declares the manifest export function
 * @details REQUIRED EXPORT - Every plugin must provide exactly one
 *
 * Function signature: const PluginManifest* GetPluginManifest(void)
 *
 * Usage example:
 *   PLUGIN_MANIFEST_EXPORT(GetPluginManifest) {
 *       static PluginManifest manifest = {
 *           .pluginVersion_major = 1,
 *           .pluginVersion_minor = 0,
 *           .pluginVersion_patch = 0,
 *           .iConsoleContractVersion = ICONSOLE_CONTRACT_VERSION,
 *           .iDebuggerContractVersion = IDEBUGGER_CONTRACT_VERSION,
 *           .consoleName = "nes",
 *           // ... other fields
 *       };
 *       return &manifest;
 *   }
 *
 * The manifest provides the frontend with metadata about the plugin
 * and validates API compatibility before loading.
 */
#define PLUGIN_MANIFEST_EXPORT(funcName) \
    PLUGIN_EXPORT const PluginManifest* PLUGIN_CALLING_CONVENTION funcName(void)
```

[^](#table-of-contents)
---

### 0.2 Create Cross-Platform Plugin Loading System

**Goal:** Implement platform-agnostic plugin discovery and loading

#### 0.2.1 Create Core Plugin Loader Header

**File to Create:** `src/common/plugin/PluginLoader.h`

**Actions:**

1. Create the PluginLoader class definition
2. Define function pointer types for plugin exports
3. Define LoadedPlugin structure
4. Declare all public methods

**New Code:**

```
#pragma once

#include "PluginManifest.h"
#include <string>
#include <memory>

class IConsole;
class IDebugger;

/**
 * @class PluginLoader
 * @brief Cross-platform dynamic library loader
 * @details Handles loading, unloading, and symbol resolution for plugins
 *          on Windows, Linux, and macOS. Uses platform-specific implementation.
 */
class PluginLoader {
public:
    // Function pointer types matching plugin exports
    using CreateCoreFunc = IConsole* (*)();
    using DestroyCoreFunc = void (*)(IConsole*);
    using CreateDebuggerFunc = IDebugger* (*)(IConsole*);
    using DestroyDebuggerFunc = void (*)(IDebugger*);
    using GetManifestFunc = const PluginManifest* (*)();

    /**
     * @struct LoadedPlugin
     * @brief Information about a loaded plugin in memory
     */
    struct LoadedPlugin {
        std::string filePath;           // Full path to DLL/SO/DYLIB
        std::string consoleName;        // Short name (nes, gba, etc)
        
        void* libraryHandle;            // Platform-specific library handle
        
        CreateCoreFunc createCoreFunc;
        DestroyCoreFunc destroyCoreFunc;
        CreateDebuggerFunc createDebuggerFunc;  // May be null
        DestroyDebuggerFunc destroyDebuggerFunc; // May be null
        GetManifestFunc getManifestFunc;
        
        PluginManifest manifest;        // Cached manifest data
    };

    /**
     * @brief Load a plugin from file path
     * @param filePath Full path to plugin library
     * @param outPlugin [out] Populated with loaded plugin info
     * @return true if loaded successfully
     */
    static bool LoadPlugin(const std::string& filePath, LoadedPlugin& outPlugin);

    /**
     * @brief Unload a previously loaded plugin
     * @param plugin The plugin to unload
     * @return true if unloaded successfully
     */
    static bool UnloadPlugin(LoadedPlugin& plugin);

    /**
     * @brief Get the expected plugin filename for current platform
     * @param consoleName Console name (e.g., "nes", "gba")
     * @return Expected filename (e.g., "nes.dll" on Windows)
     */
    static std::string GetExpectedFilename(const std::string& consoleName);

    /**
     * @brief Get platform-specific file extension
     * @return ".dll" on Windows, ".so" on Linux, ".dylib" on macOS
     */
    static std::string GetPluginExtension();

    /**
     * @brief Get platform-specific library prefix
     * @return "" on Windows, "lib" on Unix-like systems
     */
    static std::string GetLibraryPrefix();

    /**
     * @brief Get the last error message from plugin operations
     * @return Human-readable error message
     */
    static std::string GetLastError();

private:
    // Platform-specific implementations
    static void* LoadLibraryInternal(const std::string& filePath);
    static bool UnloadLibraryInternal(void* handle);
    static void* GetSymbolInternal(void* handle, const std::string& symbolName);
};
```

[^](#table-of-contents)---

#### 0.2.2 Create Windows Plugin Loader

**File to Create:** `src/common/plugin/platform/PluginLoaderWindows.cpp`

**New Code:**

```
#ifdef _WIN32

#include "../PluginLoader.h"
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

static std::string g_lastError;

void* PluginLoader::LoadLibraryInternal(const std::string& filePath) {
    SetLastError(0);
    HMODULE handle = LoadLibraryA(filePath.c_str());
    
    if (!handle) {
        DWORD error = GetLastError();
        char buffer[1024];
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      buffer, sizeof(buffer), nullptr);
        g_lastError = std::string("LoadLibrary failed: ") + buffer;
        return nullptr;
    }
    
    g_lastError.clear();
    return reinterpret_cast<void*>(handle);
}

bool PluginLoader::UnloadLibraryInternal(void* handle) {
    if (!handle) return false;
    
    BOOL success = FreeLibrary(reinterpret_cast<HMODULE>(handle));
    
    if (!success) {
        DWORD error = GetLastError();
        char buffer[1024];
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      buffer, sizeof(buffer), nullptr);
        g_lastError = std::string("FreeLibrary failed: ") + buffer;
    } else {
        g_lastError.clear();
    }
    
    return success != 0;
}

void* PluginLoader::GetSymbolInternal(void* handle, const std::string& symbolName) {
    if (!handle) {
        g_lastError = "Invalid library handle";
        return nullptr;
    }
    
    FARPROC symbol = GetProcAddress(reinterpret_cast<HMODULE>(handle), symbolName.c_str());
    
    if (!symbol) {
        g_lastError = "Symbol not found: " + symbolName;
        return nullptr;
    }
    
    g_lastError.clear();
    return reinterpret_cast<void*>(symbol);
}

std::string PluginLoader::GetPluginExtension() {
    return ".dll";
}

std::string PluginLoader::GetLibraryPrefix() {
    return "";
}

std::string PluginLoader::GetLastError() {
    return g_lastError;
}

#endif
```

[^](#table-of-contents)---

#### 0.2.3 Create Linux Plugin Loader

**File to Create:** `src/common/plugin/platform/PluginLoaderLinux.cpp`

**New Code:**

```
#ifdef __linux__

#include "../PluginLoader.h"
#include <dlfcn.h>
#include <cstring>

static std::string g_lastError;

void* PluginLoader::LoadLibraryInternal(const std::string& filePath) {
    void* handle = dlopen(filePath.c_str(), RTLD_LAZY | RTLD_LOCAL);
    
    if (!handle) {
        const char* error = dlerror();
        g_lastError = error ? std::string(error) : "Unknown dlopen error";
        return nullptr;
    }
    
    g_lastError.clear();
    return handle;
}

bool PluginLoader::UnloadLibraryInternal(void* handle) {
    if (!handle) return false;
    
    int result = dlclose(handle);
    
    if (result != 0) {
        const char* error = dlerror();
        g_lastError = error ? std::string(error) : "Unknown dlclose error";
    } else {
        g_lastError.clear();
    }
    
    return result == 0;
}

void* PluginLoader::GetSymbolInternal(void* handle, const std::string& symbolName) {
    if (!handle) {
        g_lastError = "Invalid library handle";
        return nullptr;
    }
    
    dlerror();
    void* symbol = dlsym(handle, symbolName.c_str());
    
    const char* error = dlerror();
    if (error) {
        g_lastError = std::string(error);
        return nullptr;
    }
    
    g_lastError.clear();
    return symbol;
}

std::string PluginLoader::GetPluginExtension() {
    return ".so";
}

std::string PluginLoader::GetLibraryPrefix() {
    return "lib";
}

std::string PluginLoader::GetLastError() {
    return g_lastError;
}

#endif
```

[^](#table-of-contents)---

#### 0.2.4 Create macOS Plugin Loader

**File to Create:** `src/common/plugin/platform/PluginLoaderMacOS.cpp`

**New Code:**

```
#ifdef __APPLE__

#include "../PluginLoader.h"
#include <dlfcn.h>
#include <cstring>

static std::string g_lastError;

void* PluginLoader::LoadLibraryInternal(const std::string& filePath) {
    void* handle = dlopen(filePath.c_str(), RTLD_LAZY | RTLD_LOCAL);
    
    if (!handle) {
        const char* error = dlerror();
        g_lastError = error ? std::string(error) : "Unknown dlopen error";
        return nullptr;
    }
    
    g_lastError.clear();
    return handle;
}

bool PluginLoader::UnloadLibraryInternal(void* handle) {
    if (!handle) return false;
    
    int result = dlclose(handle);
    
    if (result != 0) {
        const char* error = dlerror();
        g_lastError = error ? std::string(error) : "Unknown dlclose error";
    } else {
        g_lastError.clear();
    }
    
    return result == 0;
}

void* PluginLoader::GetSymbolInternal(void* handle, const std::string& symbolName) {
    if (!handle) {
        g_lastError = "Invalid library handle";
        return nullptr;
    }
    
    dlerror();
    void* symbol = dlsym(handle, symbolName.c_str());
    
    const char* error = dlerror();
    if (error) {
        g_lastError = std::string(error);
        return nullptr;
    }
    
    g_lastError.clear();
    return symbol;
}

std::string PluginLoader::GetPluginExtension() {
    return ".dylib";
}

std::string PluginLoader::GetLibraryPrefix() {
    return "lib";
}

std::string PluginLoader::GetLastError() {
    return g_lastError;
}

#endif
```

[^](#table-of-contents)---

#### 0.2.5 Create Platform-Agnostic Plugin Loader Implementation

**File to Create:** `src/common/plugin/PluginLoader.cpp`

**New Code:**

```
#include "PluginLoader.h"
#include <filesystem>

namespace fs = std::filesystem;

bool PluginLoader::LoadPlugin(const std::string& filePath, LoadedPlugin& outPlugin) {
    if (!fs::exists(filePath)) {
        return false;
    }

    void* handle = LoadLibraryInternal(filePath);
    if (!handle) {
        return false;
    }

    auto getManifestFunc = reinterpret_cast<GetManifestFunc>(
        GetSymbolInternal(handle, "GetPluginManifest"));
    
    if (!getManifestFunc) {
        UnloadLibraryInternal(handle);
        return false;
    }

    const PluginManifest* manifest = getManifestFunc();
    if (!manifest) {
        UnloadLibraryInternal(handle);
        return false;
    }

    if (manifest->iConsoleContractVersion != ICONSOLE_CONTRACT_VERSION) {
        UnloadLibraryInternal(handle);
        return false;
    }

    auto createCoreFunc = reinterpret_cast<CreateCoreFunc>(
        GetSymbolInternal(handle, "CreateCore"));
    auto destroyCoreFunc = reinterpret_cast<DestroyCoreFunc>(
        GetSymbolInternal(handle, "DestroyCore"));

    if (!createCoreFunc || !destroyCoreFunc) {
        UnloadLibraryInternal(handle);
        return false;
    }

    auto createDebuggerFunc = reinterpret_cast<CreateDebuggerFunc>(
        GetSymbolInternal(handle, "CreateDebugger"));
    auto destroyDebuggerFunc = reinterpret_cast<DestroyDebuggerFunc>(
        GetSymbolInternal(handle, "DestroyDebugger"));

    if ((createDebuggerFunc && !destroyDebuggerFunc) ||
        (!createDebuggerFunc && destroyDebuggerFunc)) {
        UnloadLibraryInternal(handle);
        return false;
    }

    outPlugin.filePath = filePath;
    outPlugin.consoleName = manifest->consoleName;
    outPlugin.libraryHandle = handle;
    outPlugin.createCoreFunc = createCoreFunc;
    outPlugin.destroyCoreFunc = destroyCoreFunc;
    outPlugin.createDebuggerFunc = createDebuggerFunc;
    outPlugin.destroyDebuggerFunc = destroyDebuggerFunc;
    outPlugin.getManifestFunc = getManifestFunc;
    outPlugin.manifest = *manifest;

    return true;
}

bool PluginLoader::UnloadPlugin(LoadedPlugin& plugin) {
    if (!plugin.libraryHandle) {
        return false;
    }

    if (!UnloadLibraryInternal(plugin.libraryHandle)) {
        return false;
    }

    plugin.libraryHandle = nullptr;
    return true;
}

std::string PluginLoader::GetExpectedFilename(const std::string& consoleName) {
    return GetLibraryPrefix() + consoleName + GetPluginExtension();
}
```

---

**Phase 0 Deliverable:** Cross-platform plugin loader fully implemented and tested

[^](#table-of-contents)---

## Phase 1: NES Core as First Plugin

### 1.1 Create NES Plugin Structure

**Goal:** Transform NES from monolithic component to standalone plugin library

#### 1.1.1 Create NES CMakeLists.txt

**File to Create:** `src/backends/nes/CMakeLists.txt`

**New Code:**

```
cmake_minimum_required(VERSION 3.16)

project(ProteusNESPlugin VERSION 1.0.0 LANGUAGES CXX)

# Build as shared library (plugin)
add_library(ProteusNES SHARED
    # Plugin interface implementation
    plugin/NESCoreImpl.cpp
    plugin/NESPluginExports.cpp
    plugin/NESDebuggerImpl.cpp
    
    # Core emulation (moved from src/backend/NES)
    NES.cpp
    cpu/CPU.cpp
    cpu/CPUOpcodes.cpp
    ppu/NesPPU.cpp
    apu/NesAPU.cpp
    cartridge/Cartridge.cpp
    cartridge/Mappers/NesMapper.cpp
    input/Controller.cpp
)

# Include directories
target_include_directories(ProteusNES
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}/src
)

# Compiler settings
set_target_properties(ProteusNES PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Platform-specific settings
if(WIN32)
    set_target_properties(ProteusNES PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
    )
elseif(APPLE)
    set_target_properties(ProteusNES PROPERTIES
        PREFIX "lib"
        SUFFIX ".dylib"
        MACOSX_RPATH ON
    )
else()
    set_target_properties(ProteusNES PROPERTIES
        PREFIX "lib"
        SUFFIX ".so"
    )
endif()

# Set output directory for plugins
set_target_properties(ProteusNES PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Install plugin
install(TARGETS ProteusNES DESTINATION ${INSTALL_PLUGIN_DIR})
```

[^](#table-of-contents)---

#### 1.1.2 Create NES Plugin Exports

**File to Create:** `src/backends/nes/plugin/NESPluginExports.cpp`

**New Code:**

```
#include "../../common/plugin/PluginExports.h"
#include "NESCoreImpl.h"
#include "NESDebuggerImpl.h"

namespace NS_NESplugin {

// Static manifest for this plugin
static const PluginManifest g_nesPluginManifest = {
    .pluginVersion_major = 1,
    .pluginVersion_minor = 0,
    .pluginVersion_patch = 0,
    .iConsoleContractVersion = ICONSOLE_CONTRACT_VERSION,
    .iDebuggerContractVersion = IDEBUGGER_CONTRACT_VERSION,
    
    .consoleName = "nes",
    .consoleFullName = "Nintendo Entertainment System",
    .pluginName = "Proteus NES Emulation Core",
    .authorName = "Proteus Development Team",
    .description = "Cycle-accurate NES/Famicom emulator with full PPU and APU support",
    
    .buildDate = __DATE__,
    .licenseType = "MIT",
    
    .developmentStatus = 3,
    .statusDescription = "Full implementation with debugging support"
};

// === Required Factory Functions ===

PLUGIN_FACTORY_CREATE(CreateCore) {
    try {
        return new NESCoreImpl();
    } catch (...) {
        return nullptr;
    }
}

PLUGIN_FACTORY_DESTROY(DestroyCore) {
    delete reinterpret_cast<NESCoreImpl*>(core);
}

// === Optional Debugger Functions ===

PLUGIN_DEBUGGER_CREATE(CreateDebugger) {
    if (!core) return nullptr;
    try {
        return new NESDebuggerImpl(core);
    } catch (...) {
        return nullptr;
    }
}

PLUGIN_DEBUGGER_DESTROY(DestroyDebugger) {
    delete reinterpret_cast<NESDebuggerImpl*>(debugger);
}

// === Manifest Export ===

PLUGIN_MANIFEST_EXPORT(GetPluginManifest) {
    return &g_nesPluginManifest;
}

}
```

[^](#table-of-contents)---

#### 1.1.3 Create NES Core Implementation Header

**File to Create:** `src/backends/nes/plugin/NESCoreImpl.h`

**New Code:**

```
#pragma once

#include "../../common/plugin/PluginExports.h"
#include "../NES.h"

namespace NS_NESplugin {

/**
 * @class NESCoreImpl
 * @implements IConsole
 * @brief NES emulation core implementing the IConsole plugin contract
 * @details Wraps the existing NS_NES::NES class and adapts it to the IConsole interface.
 */
class NESCoreImpl : public IConsole {
public:
    NESCoreImpl();
    virtual ~NESCoreImpl();

    // === IConsole Implementation ===
    bool Initialize() override;
    bool Shutdown() override;
    bool loadROM(const string& path) override;
    void reset() override;
    void clock() override;
    const u32* getFrameBuffer() const override;
    const int SCREEN_WIDTH() const override { return 256; }
    const int SCREEN_HEIGHT() const override { return 240; }
    void collectAudio(vector<float>& outSamples) override;
    void update(u8 playerIndex, bool* buttonStates) override;
    void initSST(SSTstate state) override;
    void runSST() override;
    bool checkSST(SSTstate state, string& outMessage) override;

private:
    std::unique_ptr<NS_NES::NES> m_nesCore;
    bool m_initialized;
};

}
```

[^](#table-of-contents)---

#### 1.1.4 Create NES Core Implementation

**File to Create:** `src/backends/nes/plugin/NESCoreImpl.cpp`

**New Code:**

```
#include "NESCoreImpl.h"

namespace NS_NESplugin {

NESCoreImpl::NESCoreImpl()
    : m_nesCore(nullptr), m_initialized(false) {
}

NESCoreImpl::~NESCoreImpl() {
    if (m_initialized) {
        Shutdown();
    }
}

bool NESCoreImpl::Initialize() {
    if (m_initialized) return true;

    try {
        m_nesCore = std::make_unique<NS_NES::NES>();
        m_initialized = true;
        return true;
    } catch (...) {
        return false;
    }
}

bool NESCoreImpl::Shutdown() {
    if (!m_initialized) return true;
    
    m_nesCore.reset();
    m_initialized = false;
    return true;
}

bool NESCoreImpl::loadROM(const string& path) {
    if (!m_initialized || !m_nesCore) return false;
    return m_nesCore->loadROM(path);
}

void NESCoreImpl::reset() {
    if (m_initialized && m_nesCore) {
        m_nesCore->reset();
    }
}

void NESCoreImpl::clock() {
    if (m_initialized && m_nesCore) {
        m_nesCore->clock();
    }
}

const u32* NESCoreImpl::getFrameBuffer() const {
    if (!m_nesCore) return nullptr;
    return m_nesCore->getFrameBuffer();
}

void NESCoreImpl::collectAudio(vector<float>& outSamples) {
    if (m_nesCore) {
        m_nesCore->collectAudio(outSamples);
    }
}

void NESCoreImpl::update(u8 playerIndex, bool* buttonStates) {
    if (m_nesCore) {
        m_nesCore->update(playerIndex, buttonStates);
    }
}

void NESCoreImpl::initSST(SSTstate state) {
    if (m_nesCore) {
        m_nesCore->initSST(state);
    }
}

void NESCoreImpl::runSST() {
    if (m_nesCore) {
        m_nesCore->runSST();
    }
}

bool NESCoreImpl::checkSST(SSTstate state, string& outMessage) {
    if (!m_nesCore) return false;
    return m_nesCore->checkSST(state, outMessage);
}

}
```

[^](#table-of-contents)---

#### 1.1.5 Create NES Debugger Implementation

**File to Create:** `src/backends/nes/plugin/NESDebuggerImpl.h`

**New Code:**

```
#pragma once

#include "../../backend/shared/IDebugger.h"

class IConsole;

namespace NS_NESplugin {

/**
 * @class NESDebuggerImpl
 * @implements IDebugger
 * @brief Debugging interface for NES cores
 */
class NESDebuggerImpl : public IDebugger {
public:
    explicit NESDebuggerImpl(IConsole* core);
    virtual ~NESDebuggerImpl() = default;

    void breakOnNextFrame() override;
    void breakOnCondition(const string& condition) override;
    bool isBreakpointSet() const override;

private:
    IConsole* m_core;
};

}
```

**File to Create:** `src/backends/nes/plugin/NESDebuggerImpl.cpp`

**New Code:**

```
#include "NESDebuggerImpl.h"

namespace NS_NESplugin {

NESDebuggerImpl::NESDebuggerImpl(IConsole* core)
    : m_core(core) {
}

void NESDebuggerImpl::breakOnNextFrame() {
    // TODO: Implement frame-level breakpoint
}

void NESDebuggerImpl::breakOnCondition(const string& condition) {
    // TODO: Implement conditional breakpoint
}

bool NESDebuggerImpl::isBreakpointSet() const {
    // TODO: Implement breakpoint query
    return false;
}

}
```

---

**Phase 1 Deliverable:** NES compiles as plugin DLL/SO/DYLIB with proper exports

[^](#table-of-contents)---

## Phase 2: Plugin Registry & Discovery

### 2.1 Create Plugin Registry

#### 2.1.1 Create Plugin Registry Header

**File to Create:** `src/common/plugin/PluginRegistry.h`

**New Code:**

```
#pragma once

#include "PluginLoader.h"
#include <map>
#include <vector>

/**
 * @class PluginRegistry
 * @brief Central repository for plugin discovery and lifecycle management
 * @details Maintains a map of loaded plugins and provides methods to discover
 *          and instantiate console cores.
 */
class PluginRegistry {
public:
    /**
     * @brief Discover plugins in standard search paths
     * @return true if discovery succeeded
     */
    static bool DiscoverPlugins();

    /**
     * @brief Load a specific plugin by console name
     * @param consoleName Short name (e.g., "nes", "gba")
     * @return true if loaded successfully
     */
    static bool LoadPlugin(const std::string& consoleName);

    /**
     * @brief Load all discovered plugins
     * @return true if all loaded
     */
    static bool LoadAllPlugins();

    /**
     * @brief Create a core instance from a loaded plugin
     * @param consoleName Short name (e.g., "nes", "gba")
     * @return IConsole* instance, or nullptr on failure
     */
    static IConsole* CreateCore(const std::string& consoleName);

    /**
     * @brief Destroy a core instance
     * @param core Instance to destroy
     */
    static void DestroyCore(IConsole* core);

    /**
     * @brief Check if a plugin is loaded
     * @param consoleName Short name
     * @return true if loaded
     */
    static bool IsPluginLoaded(const std::string& consoleName);

    /**
     * @brief Check if a plugin is discovered
     * @param consoleName Short name
     * @return true if available
     */
    static bool IsPluginDiscovered(const std::string& consoleName);

    /**
     * @brief Get metadata for a plugin
     * @param consoleName Short name
     * @return Manifest pointer or nullptr
     */
    static const PluginManifest* GetManifest(const std::string& consoleName);

    /**
     * @brief Get list of discovered plugins
     * @return Vector of console names
     */
    static std::vector<std::string> GetDiscoveredPlugins();

    /**
     * @brief Get list of loaded plugins
     * @return Vector of console names
     */
    static std::vector<std::string> GetLoadedPlugins();

    /**
     * @brief Unload all plugins
     * @return true if successful
     */
    static bool UnloadAllPlugins();

private:
    struct RegistryEntry {
        std::string consoleName;
        std::string filePath;
        PluginLoader::LoadedPlugin loadedPlugin;
        bool isLoaded;
    };

    static std::map<std::string, RegistryEntry> s_registry;
    static bool s_discovered;

    static std::vector<std::string> GetSearchPaths();
};
```

[^](#table-of-contents)---

#### 2.1.2 Create Plugin Registry Implementation

**File to Create:** `src/common/plugin/PluginRegistry.cpp`

**New Code:**

```
#include "PluginRegistry.h"
#include <filesystem>
#include <cstdlib>
#include <iostream>

namespace fs = std::filesystem;

std::map<std::string, PluginRegistry::RegistryEntry> PluginRegistry::s_registry;
bool PluginRegistry::s_discovered = false;

std::vector<std::string> PluginRegistry::GetSearchPaths() {
    std::vector<std::string> paths;

    // Relative paths
    paths.push_back("./plugins");
    paths.push_back("plugins");

    // Environment variable
    const char* envPath = std::getenv("PROTEUS_PLUGIN_PATH");
    if (envPath) {
        paths.push_back(envPath);
    }

#ifdef _WIN32
    const char* appData = std::getenv("APPDATA");
    if (appData) {
        paths.push_back(std::string(appData) + "\\Proteus\\plugins");
    }
#elif defined(__linux__)
    const char* home = std::getenv("HOME");
    if (home) {
        paths.push_back(std::string(home) + "/.proteus/plugins");
    }
    paths.push_back("/usr/local/share/proteus/plugins");
    paths.push_back("/usr/share/proteus/plugins");
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (home) {
        paths.push_back(std::string(home) + 
                       "/Library/Application Support/Proteus/plugins");
    }
#endif

    return paths;
}

bool PluginRegistry::DiscoverPlugins() {
    auto searchPaths = GetSearchPaths();
    
    // List of known consoles
    const std::vector<std::string> knownConsoles = {
        "nes", "snes", "ps1", "n64", "ps2", "gbc", "gba", 
        "ngc", "xbox", "nds", "x360", "ps3", "wii"
    };

    for (const auto& consoleName : knownConsoles) {
        std::string expectedFilename = PluginLoader::GetExpectedFilename(consoleName);
        
        for (const auto& searchPath : searchPaths) {
            std::string fullPath = searchPath + "/" + expectedFilename;
            
            if (fs::exists(fullPath)) {
                RegistryEntry entry;
                entry.consoleName = consoleName;
                entry.filePath = fullPath;
                entry.isLoaded = false;
                
                s_registry[consoleName] = entry;
                std::cout << "[PluginRegistry] Discovered: " << consoleName 
                         << " at " << fullPath << std::endl;
                break;
            }
        }
    }

    s_discovered = true;
    return true;
}

bool PluginRegistry::LoadPlugin(const std::string& consoleName) {
    auto it = s_registry.find(consoleName);
    if (it == s_registry.end()) {
        return false;
    }

    if (it->second.isLoaded) {
        return true;
    }

    if (!PluginLoader::LoadPlugin(it->second.filePath, it->second.loadedPlugin)) {
        std::cerr << "[PluginRegistry] Failed to load " << consoleName << ": " 
                 << PluginLoader::GetLastError() << std::endl;
        return false;
    }

    it->second.isLoaded = true;
    std::cout << "[PluginRegistry] Loaded: " << consoleName << std::endl;
    return true;
}

bool PluginRegistry::LoadAllPlugins() {
    bool allSuccess = true;
    for (auto& [consoleName, entry] : s_registry) {
        if (!LoadPlugin(consoleName)) {
            allSuccess = false;
        }
    }
    return allSuccess;
}

IConsole* PluginRegistry::CreateCore(const std::string& consoleName) {
    auto it = s_registry.find(consoleName);
    if (it == s_registry.end() || !it->second.isLoaded) {
        return nullptr;
    }

    return it->second.loadedPlugin.createCoreFunc();
}

void PluginRegistry::DestroyCore(IConsole* core) {
    if (!core) return;

    for (auto& [consoleName, entry] : s_registry) {
        if (entry.isLoaded) {
            entry.loadedPlugin.destroyCoreFunc(core);
            return;
        }
    }
}

bool PluginRegistry::IsPluginLoaded(const std::string& consoleName) {
    auto it = s_registry.find(consoleName);
    return it != s_registry.end() && it->second.isLoaded;
}

bool PluginRegistry::IsPluginDiscovered(const std::string& consoleName) {
    return s_registry.find(consoleName) != s_registry.end();
}

const PluginManifest* PluginRegistry::GetManifest(const std::string& consoleName) {
    auto it = s_registry.find(consoleName);
    if (it == s_registry.end() || !it->second.isLoaded) {
        return nullptr;
    }
    return &it->second.loadedPlugin.manifest;
}

std::vector<std::string> PluginRegistry::GetDiscoveredPlugins() {
    std::vector<std::string> result;
    for (const auto& [consoleName, entry] : s_registry) {
        result.push_back(consoleName);
    }
    return result;
}

std::vector<std::string> PluginRegistry::GetLoadedPlugins() {
    std::vector<std::string> result;
    for (const auto& [consoleName, entry] : s_registry) {
        if (entry.isLoaded) {
            result.push_back(consoleName);
        }
    }
    return result;
}

bool PluginRegistry::UnloadAllPlugins() {
    bool allSuccess = true;
    for (auto& [consoleName, entry] : s_registry) {
        if (entry.isLoaded) {
            if (!PluginLoader::UnloadPlugin(entry.loadedPlugin)) {
                allSuccess = false;
            }
            entry.isLoaded = false;
        }
    }
    return allSuccess;
}
```

---

**Phase 2 Deliverable:** Plugin registry system complete

[^](#table-of-contents)---

## Phase 3: Update Frontend to Use Plugin System

### 3.1 Create Plugin Manager for Frontend

#### 3.1.1 Create Plugin Manager Header

**File to Create:** `src/frontend/plugin/PluginManager.h`

**New Code:**

```
#pragma once

#include "../../common/plugin/PluginRegistry.h"
#include "../../backend/shared/IConsole.h"
#include "../../backend/shared/IDebugger.h"

namespace NS_Proteus {

/**
 * @class PluginManager
 * @brief Frontend interface to the plugin system
 * @details Manages plugin lifecycle from the frontend's perspective
 */
class PluginManager {
public:
    /**
     * @brief Initialize the plugin system
     * @details Discovers and loads all available plugins
     * @return true if initialization successful
     */
    static bool Initialize();

    /**
     * @brief Shutdown the plugin system
     * @return true if successful
     */
    static bool Shutdown();

    /**
     * @brief Check if a console is available
     * @param consoleName Console name (e.g., "nes")
     * @return true if loaded and ready
     */
    static bool IsConsoleAvailable(const std::string& consoleName);

    /**
     * @brief Get list of available consoles
     * @return Vector of console names (e.g., ["nes", "gba"])
     */
    static std::vector<std::string> GetAvailableConsoles();

    /**
     * @brief Create a console instance
     * @param consoleName Console to instantiate
     * @return New IConsole instance, or nullptr on failure
     */
    static IConsole* CreateConsole(const std::string& consoleName);

    /**
     * @brief Create a debugger for a console
     * @param consoleName Console name
     * @param console Associated console instance
     * @return New IDebugger instance, or nullptr if no debugger available
     */
    static IDebugger* CreateDebugger(const std::string& consoleName, IConsole* console);

    /**
     * @brief Destroy a console instance
     * @param console Instance to destroy
     */
    static void DestroyConsole(IConsole* console);

    /**
     * @brief Destroy a debugger instance
     * @param consoleName Console name (used to find correct destructor)
     * @param debugger Instance to destroy
     */
    static void DestroyDebugger(const std::string& consoleName, IDebugger* debugger);

    /**
     * @brief Get metadata for a plugin
     * @param consoleName Console name
     * @return Plugin manifest, or nullptr
     */
    static const PluginManifest* GetConsoleMetadata(const std::string& consoleName);

    /**
     * @brief Log current plugin status
     */
    static void LogPluginStatus();

private:
    static bool s_initialized;
};

}
```

[^](#table-of-contents)---

#### 3.1.2 Create Plugin Manager Implementation

**File to Create:** `src/frontend/plugin/PluginManager.cpp`

**New Code:**

```
#include "PluginManager.h"
#include <iostream>

namespace NS_Proteus {

bool PluginManager::s_initialized = false;

bool PluginManager::Initialize() {
    if (s_initialized) {
        return true;
    }

    std::cout << "\n=== Proteus Plugin System Initialization ===" << std::endl;

    if (!PluginRegistry::DiscoverPlugins()) {
        std::cerr << "Plugin discovery failed" << std::endl;
        return false;
    }

    if (!PluginRegistry::LoadAllPlugins()) {
        std::cout << "Warning: Some plugins failed to load" << std::endl;
    }

    s_initialized = true;
    LogPluginStatus();
    std::cout << "=============================================\n" << std::endl;

    return true;
}

bool PluginManager::Shutdown() {
    if (!s_initialized) {
        return true;
    }

    if (!PluginRegistry::UnloadAllPlugins()) {
        std::cerr << "Warning: Plugin unload had issues" << std::endl;
    }

    s_initialized = false;
    return true;
}

bool PluginManager::IsConsoleAvailable(const std::string& consoleName) {
    return PluginRegistry::IsPluginLoaded(consoleName);
}

std::vector<std::string> PluginManager::GetAvailableConsoles() {
    return PluginRegistry::GetLoadedPlugins();
}

IConsole* PluginManager::CreateConsole(const std::string& consoleName) {
    return PluginRegistry::CreateCore(consoleName);
}

IDebugger* PluginManager::CreateDebugger(const std::string& consoleName, IConsole* console) {
    if (!console) return nullptr;

    // Would need to access registry to get debugger factory
    // For now, this is a simplified version
    auto manifest = PluginRegistry::GetManifest(consoleName);
    if (!manifest) return nullptr;

    // Access internal registry entry to get debugger function
    // This requires making registry data accessible or providing a helper method
    return nullptr;
}

void PluginManager::DestroyConsole(IConsole* console) {
    if (!console) return;
    PluginRegistry::DestroyCore(console);
}

void PluginManager::DestroyDebugger(const std::string& consoleName, IDebugger* debugger) {
    if (!debugger) return;
    // Would need access to debugger destroy function
    // Delete is called by shared_ptr in ConsoleSession
}

const PluginManifest* PluginManager::GetConsoleMetadata(const std::string& consoleName) {
    return PluginRegistry::GetManifest(consoleName);
}

void PluginManager::LogPluginStatus() {
    auto discovered = PluginRegistry::GetDiscoveredPlugins();
    auto loaded = PluginRegistry::GetLoadedPlugins();

    std::cout << "Discovered " << discovered.size() << " plugins:" << std::endl;
    for (const auto& name : discovered) {
        auto manifest = PluginRegistry::GetManifest(name);
        std::cout << "  [" << (IsConsoleAvailable(name) ? "OK" : "FAIL") << "] "
                 << name << " - " << (manifest ? manifest->consoleFullName : "Unknown")
                 << std::endl;
    }
}

}
```

[^](#table-of-contents)---

#### 3.1.3 Update ConsoleFactory

**File to Update:** `src/frontend/session/ConsoleFactory.cpp`

**New Code (Add to existing file):**

```
#include "ConsoleFactory.h"
#include "../plugin/PluginManager.h"

namespace NS_Proteus {

std::string ConsoleFactory::GetConsoleNameString(ConsoleID console) {
    switch (console) {
        case ConsoleID::NES: return "nes";
        case ConsoleID::SNS: return "snes";
        case ConsoleID::PS1: return "ps1";
        case ConsoleID::N64: return "n64";
        case ConsoleID::PS2: return "ps2";
        case ConsoleID::GBC: return "gbc";
        case ConsoleID::GBA: return "gba";
        case ConsoleID::NGC: return "ngc";
        case ConsoleID::XBX: return "xbox";
        case ConsoleID::NDS: return "nds";
        case ConsoleID::XB3: return "x360";
        case ConsoleID::PS3: return "ps3";
        case ConsoleID::WII: return "wii";
        default: return "";
    }
}

std::shared_ptr<IConsole> ConsoleFactory::Create(ConsoleID console) {
    std::string consoleName = GetConsoleNameString(console);
    if (consoleName.empty()) {
        return nullptr;
    }

    IConsole* corePtr = PluginManager::CreateConsole(consoleName);
    if (!corePtr) {
        return nullptr;
    }

    return std::shared_ptr<IConsole>(corePtr, [consoleName](IConsole* core) {
        PluginManager::DestroyConsole(core);
    });
}

}
```

[^](#table-of-contents)---

#### 3.1.4 Update Proteus::Init()

**File to Update:** `src/frontend/app/Proteus.cpp`

**Code to Add at Start of Init Method:**

```
#include "../plugin/PluginManager.h"

void Proteus::Init() {
    SetMetadata();

    // Initialize plugin system BEFORE creating sessions
    if (!PluginManager::Initialize()) {
        std::cerr << "Failed to initialize plugin system" << std::endl;
        // Continue anyway - some plugins may have loaded
    }

    videoManager->Init();
    inputManager->Init();
    audioManager->Init();

    UpdateConsoleAvailability();
}

void Proteus::UpdateConsoleAvailability() {
    auto availableConsoles = PluginManager::GetAvailableConsoles();
    
    ConsoleEmuStarted.clear();
    
    for (const auto& name : availableConsoles) {
        ConsoleID id = GetConsoleIDFromName(name);
        if (id != ConsoleID::NONE) {
            ConsoleEmuStarted[id] = true;
        }
    }
}

void Proteus::Deinit() {
    videoManager.reset();
    inputManager.reset();

    if (!PluginManager::Shutdown()) {
        std::cerr << "Warning: Plugin shutdown had issues" << std::endl;
    }

    SDL_Quit();
}
```

---

**Phase 3 Deliverable:** Frontend uses plugin system dynamically at runtime

[^](#table-of-contents)---

## Phase 4: Build System Configuration

### 4.1 Root CMakeLists.txt

**File to Create:** `CMakeLists.txt` (at project root)

**New Code:**

```
cmake_minimum_required(VERSION 2.16)

project(Proteus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    message(STATUS "Building for Windows")
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    message(STATUS "Building for macOS")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
    message(STATUS "Building for Linux")
endif()

# Build options
option(BUILD_NES_PLUGIN "Build NES emulation plugin" ON)
option(BUILD_GBA_PLUGIN "Build GBA emulation plugin" ON)
option(BUILD_STUB_PLUGINS "Build stub plugins for unsupported consoles" ON)
option(ENABLE_DEBUG_FEATURES "Enable debug features" OFF)

# Install directories
if(WIN32)
    set(INSTALL_BIN_DIR "bin")
    set(INSTALL_PLUGIN_DIR "plugins")
    set(INSTALL_RESOURCE_DIR "resources")
elseif(APPLE)
    set(INSTALL_BIN_DIR ".")
    set(INSTALL_PLUGIN_DIR "Proteus.app/Contents/Resources/plugins")
    set(INSTALL_RESOURCE_DIR "Proteus.app/Contents/Resources")
else()
    set(INSTALL_BIN_DIR "bin")
    set(INSTALL_PLUGIN_DIR "lib/proteus/plugins")
    set(INSTALL_RESOURCE_DIR "share/proteus")
endif()

# Add source directory
add_subdirectory(src)
```

[^](#table-of-contents)---

### 4.2 src/CMakeLists.txt

**File to Create:** `src/CMakeLists.txt`

**New Code:**

```
# Common library (plugin system)
add_subdirectory(common)

# Backend shared interfaces
add_subdirectory(backend/shared)

# Plugins
if(BUILD_NES_PLUGIN OR BUILD_GBA_PLUGIN OR BUILD_STUB_PLUGINS)
    add_subdirectory(backends)
endif()

# Frontend (main application)
add_subdirectory(frontend)
```

[^](#table-of-contents)---

### 4.3 src/common/CMakeLists.txt

**File to Create:** `src/common/CMakeLists.txt`

**New Code:**

```
add_library(ProteusCommon STATIC
    plugin/PluginLoader.cpp
    plugin/PluginRegistry.cpp
    plugin/platform/PluginLoaderWindows.cpp
    plugin/platform/PluginLoaderLinux.cpp
    plugin/platform/PluginLoaderMacOS.cpp
)

target_include_directories(ProteusCommon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(ProteusCommon PROPERTIES
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE ON
)
```

[^](#table-of-contents)---

### 4.4 src/backends/CMakeLists.txt

**File to Create:** `src/backends/CMakeLists.txt`

**New Code:**

```
if(BUILD_NES_PLUGIN)
    add_subdirectory(nes)
endif()

if(BUILD_GBA_PLUGIN)
    add_subdirectory(gba)
endif()

if(BUILD_STUB_PLUGINS)
    add_subdirectory(snes)
    add_subdirectory(ps1)
    add_subdirectory(n64)
    add_subdirectory(ps2)
    add_subdirectory(gbc)
    add_subdirectory(ngc)
    add_subdirectory(xbox)
    add_subdirectory(nds)
    add_subdirectory(x360)
    add_subdirectory(ps3)
    add_subdirectory(wii)
endif()
```

[^](#table-of-contents)---

### 4.5 src/frontend/CMakeLists.txt

**File to Create:** `src/frontend/CMakeLists.txt`

**New Code:**

```
find_package(SDL3 REQUIRED)

add_executable(Proteus
    main.cpp
    app/Proteus.cpp
    session/ConsoleSession.cpp
    session/ConsoleFactory.cpp
    session/DebuggerFactory.cpp
    plugin/PluginManager.cpp
    video/VideoManager.cpp
    input/InputManager.cpp
    audio/AudioManager.cpp
    rom_library/RomLibrary.cpp
    logging/Logger.cpp
)

target_include_directories(Proteus
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${SDL3_INCLUDE_DIRS}
)

target_link_libraries(Proteus
    PRIVATE
        ProteusCommon
        SDL3::SDL3
)

set_target_properties(Proteus PROPERTIES
    CXX_STANDARD 17
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

install(TARGETS Proteus DESTINATION ${INSTALL_BIN_DIR})
```

---

**Phase 4 Deliverable:** Build system complete and functional on all platforms

[^](#table-of-contents)---

## Phase 5: Multi-Instance & Inter-Core Communication

### 5.1 Design Multi-Instance Session Management

#### 5.1.1 Create Multi-Core Session Header

**File to Create:** `src/frontend/session/MultiCoreSession.h`

**New Code:**

```
#pragma once

#include "./ConsoleSession.h"
#include "../../backend/shared/IConsole.h"
#include <vector>

namespace NS_Proteus {

/**
 * @class MultiCoreSession
 * @brief Manages multiple simultaneous console instances
 * @details Allows primary console with optional secondary console(s)
 *          for linked gameplay (e.g., GBA<->GBA, GCN+GBA)
 */
class MultiCoreSession {
public:
    MultiCoreSession(Logger* logger);
    ~MultiCoreSession();

    /**
     * @brief Create primary console session
     * @param console Console type for primary
     * @return Session result
     */
    SessionResult CreatePrimarySession(ConsoleID console);

    /**
     * @brief Link a secondary console
     * @param console Console type for secondary
     * @return Session result
     */
    SessionResult LinkSecondaryConsole(ConsoleID console);

    /**
     * @brief Unlink secondary console
     * @return Session result
     */
    SessionResult UnlinkSecondaryConsole();

    /**
     * @brief Check if secondary console is linked
     * @return true if secondary active
     */
    bool IsSecondaryActive() const { return m_secondarySession != nullptr; }

    /**
     * @brief Get primary console session
     * @return ConsoleSession pointer
     */
    ConsoleSession* GetPrimarySession() { return m_primarySession.get(); }

    /**
     * @brief Get secondary console session (may be null)
     * @return ConsoleSession pointer or nullptr
     */
    ConsoleSession* GetSecondarySession() { return m_secondarySession.get(); }

    /**
     * @brief Synchronize both cores (called each frame)
     * @details Ensures both cores remain in sync clock-wise
     */
    void SynchronizeConsoles();

    /**
     * @brief Shut down all sessions
     * @return Session result
     */
    SessionResult ShutdownAll();

private:
    std::unique_ptr<ConsoleSession> m_primarySession;
    std::unique_ptr<ConsoleSession> m_secondarySession;
    Logger* m_logger;

    u64 m_primaryClockCycles;
    u64 m_secondaryClockCycles;
    float m_clockRatioAdjustment;
};

}
```

[^](#table-of-contents)---

#### 5.1.2 Create Multi-Core Session Implementation

**File to Create:** `src/frontend/session/MultiCoreSession.cpp`

**New Code:**

```
#include "MultiCoreSession.h"

namespace NS_Proteus {

MultiCoreSession::MultiCoreSession(Logger* logger)
    : m_logger(logger), m_primaryClockCycles(0), m_secondaryClockCycles(0),
      m_clockRatioAdjustment(1.0f) {
    m_primarySession = std::make_unique<ConsoleSession>(logger);
}

MultiCoreSession::~MultiCoreSession() {
    ShutdownAll();
}

SessionResult MultiCoreSession::CreatePrimarySession(ConsoleID console) {
    return m_primarySession->CreateSession(console);
}

SessionResult MultiCoreSession::LinkSecondaryConsole(ConsoleID console) {
    if (m_secondarySession) {
        return SessionResult{ false, ConsoleSessionState::ERROR, 
                             ConsoleSessionErrorCode::INVALID_TRANSITION,
                             "Secondary console already linked" };
    }

    m_secondarySession = std::make_unique<ConsoleSession>(m_logger);
    return m_secondarySession->CreateSession(console);
}

SessionResult MultiCoreSession::UnlinkSecondaryConsole() {
    if (!m_secondarySession) {
        return SessionResult{ true, ConsoleSessionState::EMPTY,
                             ConsoleSessionErrorCode::NONE, "No secondary to unlink" };
    }

    SessionResult result = m_secondarySession->Shutdown();
    m_secondarySession.reset();
    return result;
}

void MultiCoreSession::SynchronizeConsoles() {
    if (!m_secondarySession || !m_secondarySession->IsActive()) {
        return;
    }

    // Keep both consoles in frame sync
    // This would involve frame-locking logic and potentially data exchange
    // via link cable simulation
}

SessionResult MultiCoreSession::ShutdownAll() {
    SessionResult result1 = m_primarySession->Shutdown();
    SessionResult result2{ true, ConsoleSessionState::EMPTY, 
                          ConsoleSessionErrorCode::NONE, "" };
    
    if (m_secondarySession) {
        result2 = m_secondarySession->Shutdown();
        m_secondarySession.reset();
    }

    if (!result1.success || !result2.success) {
        return SessionResult{ false, ConsoleSessionState::ERROR,
                             ConsoleSessionErrorCode::INTERNAL_ERROR, 
                             "Shutdown had errors" };
    }

    return result1;
}

}
```

---

**Phase 5 Deliverable:** Multi-instance architecture designed and implemented

[^](#table-of-contents)---

## Phase 6: Cross-Platform Build Scripts

### 6.1 Windows Build Script

**File to Create:** `build_windows.ps1`

**New Code:**

```
param(
    [string]$BuildType = "Release",
    [string]$Architecture = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Proteus Windows Build ===" -ForegroundColor Cyan
Write-Host "Build Type: $BuildType" -ForegroundColor Green
Write-Host "Architecture: $Architecture" -ForegroundColor Green

$BuildDir = "build_windows_$Architecture"
if (Test-Path $BuildDir) {
    Remove-Item -Recurse $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null

Write-Host "`nGenerating CMake files..." -ForegroundColor Yellow
cmake -B $BuildDir -DCMAKE_BUILD_TYPE=$BuildType `
    -DBUILD_NES_PLUGIN=ON `
    -DBUILD_GBA_PLUGIN=ON `
    -DBUILD_STUB_PLUGINS=ON

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake generation failed" -ForegroundColor Red
    exit 1
}

Write-Host "`nBuilding..." -ForegroundColor Yellow
cmake --build $BuildDir --config $BuildType --parallel 4

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed" -ForegroundColor Red
    exit 1
}

Write-Host "`nBuild successful!" -ForegroundColor Green
Write-Host "Output: $BuildDir\$BuildType" -ForegroundColor Cyan
```

[^](#table-of-contents)---

### 6.2 Linux Build Script

**File to Create:** `build_linux.sh`

**New Code:**

```
#!/bin/bash

BUILD_TYPE=${1:-Release}
BUILD_DIR="build_linux_$(uname -m)"

echo "=== Proteus Linux Build ==="
echo "Build Type: $BUILD_TYPE"

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"

echo "Generating CMake files..."
cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_NES_PLUGIN=ON \
    -DBUILD_GBA_PLUGIN=ON \
    -DBUILD_STUB_PLUGINS=ON

if [ $? -ne 0 ]; then
    echo "CMake generation failed"
    exit 1
fi

echo "Building..."
cmake --build "$BUILD_DIR" --parallel $(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

echo "Build successful!"
echo "Output: $BUILD_DIR"
```

[^](#table-of-contents)---

### 6.3 macOS Build Script

**File to Create:** `build_macos.sh`

**New Code:**

```
#!/bin/bash

BUILD_TYPE=${1:-Release}
ARCH=${2:-$(uname -m)}
BUILD_DIR="build_macos_$ARCH"

echo "=== Proteus macOS Build ==="
echo "Build Type: $BUILD_TYPE"
echo "Architecture: $ARCH"

if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"

echo "Generating CMake files..."
cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_OSX_ARCHITECTURES="$ARCH" \
    -DBUILD_NES_PLUGIN=ON \
    -DBUILD_GBA_PLUGIN=ON \
    -DBUILD_STUB_PLUGINS=ON

if [ $? -ne 0 ]; then
    echo "CMake generation failed"
    exit 1
fi

echo "Building..."
cmake --build "$BUILD_DIR" --parallel $(sysctl -n hw.ncpu)

if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

echo "Build successful!"
echo "Output: $BUILD_DIR"
```

---

**Phase 6 Deliverable:** One-command builds functional on all platforms

[^](#table-of-contents)---

## Phase 7: Testing & Validation

### 7.1 Create Plugin Load Tests

**File to Create:** `tests/plugin/PluginLoadTest.cpp`

**New Code:**

```
#include <gtest/gtest.h>
#include "common/plugin/PluginLoader.h"
#include "common/plugin/PluginRegistry.h"

class PluginLoadTest : public ::testing::Test {
protected:
    void SetUp() override {
        PluginRegistry::DiscoverPlugins();
    }
};

TEST_F(PluginLoadTest, DiscoverPlugins) {
    auto discovered = PluginRegistry::GetDiscoveredPlugins();
    EXPECT_GT(discovered.size(), 0);
    EXPECT_TRUE(PluginRegistry::IsPluginDiscovered("nes"));
}

TEST_F(PluginLoadTest, LoadNESPlugin) {
    EXPECT_TRUE(PluginRegistry::LoadPlugin("nes"));
    EXPECT_TRUE(PluginRegistry::IsPluginLoaded("nes"));
}

TEST_F(PluginLoadTest, LoadGBAPlugin) {
    EXPECT_TRUE(PluginRegistry::LoadPlugin("gba"));
    EXPECT_TRUE(PluginRegistry::IsPluginLoaded("gba"));
}

TEST_F(PluginLoadTest, CreateNESCore) {
    EXPECT_TRUE(PluginRegistry::LoadPlugin("nes"));

    IConsole* core = PluginRegistry::CreateCore("nes");
    EXPECT_NE(core, nullptr);

    PluginRegistry::DestroyCore(core);
}

TEST_F(PluginLoadTest, CreateGBACore) {
    EXPECT_TRUE(PluginRegistry::LoadPlugin("gba"));

    IConsole* core = PluginRegistry::CreateCore("gba");
    EXPECT_NE(core, nullptr);

    PluginRegistry::DestroyCore(core);
}

TEST_F(PluginLoadTest, PluginManifest) {
    EXPECT_TRUE(PluginRegistry::LoadPlugin("nes"));

    auto manifest = PluginRegistry::GetManifest("nes");
    EXPECT_NE(manifest, nullptr);
    EXPECT_STREQ(manifest->consoleName, "nes");
    EXPECT_EQ(manifest->iConsoleContractVersion, ICONSOLE_CONTRACT_VERSION);
}

TEST_F(PluginLoadTest, ContractVersionCompatibility) {
    EXPECT_TRUE(PluginRegistry::LoadPlugin("nes"));

    auto manifest = PluginRegistry::GetManifest("nes");
    EXPECT_EQ(manifest->iConsoleContractVersion, ICONSOLE_CONTRACT_VERSION);
}
```

[^](#table-of-contents)
---

### 7.2 Create Plugin Integration Tests

**File to Create:** `tests/plugin/PluginIntegrationTest.cpp`

**New Code:**

```
#include <gtest/gtest.h>
#include "common/plugin/PluginRegistry.h"

class PluginIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        PluginRegistry::DiscoverPlugins();
        PluginRegistry::LoadAllPlugins();
    }

    void TearDown() override {
        PluginRegistry::UnloadAllPlugins();
    }
};

TEST_F(PluginIntegrationTest, NESCoreLifecycle) {
    IConsole* core = PluginRegistry::CreateCore("nes");
    EXPECT_NE(core, nullptr);

    EXPECT_TRUE(core->Initialize());
    EXPECT_NE(core->getFrameBuffer(), nullptr);
    EXPECT_EQ(core->SCREEN_WIDTH(), 256);
    EXPECT_EQ(core->SCREEN_HEIGHT(), 240);
    EXPECT_TRUE(core->Shutdown());

    PluginRegistry::DestroyCore(core);
}

TEST_F(PluginIntegrationTest, GBACoreLifecycle) {
    IConsole* core = PluginRegistry::CreateCore("gba");
    EXPECT_NE(core, nullptr);

    EXPECT_TRUE(core->Initialize());
    EXPECT_NE(core->getFrameBuffer(), nullptr);
    EXPECT_EQ(core->SCREEN_WIDTH(), 240);
    EXPECT_EQ(core->SCREEN_HEIGHT(), 160);
    EXPECT_TRUE(core->Shutdown());

    PluginRegistry::DestroyCore(core);
}

TEST_F(PluginIntegrationTest, AudioCollection) {
    IConsole* core = PluginRegistry::CreateCore("nes");
    EXPECT_TRUE(core->Initialize());

    std::vector<float> samples;
    EXPECT_NO_THROW(core->collectAudio(samples));

    core->Shutdown();
    PluginRegistry::DestroyCore(core);
}

TEST_F(PluginIntegrationTest, InputHandling) {
    IConsole* core = PluginRegistry::CreateCore("nes");
    EXPECT_TRUE(core->Initialize());

    bool buttonStates[8] = {false, false, false, false, false, false, false, false};
    EXPECT_NO_THROW(core->update(0, buttonStates));

    buttonStates[4] = true;  // Press A button
    EXPECT_NO_THROW(core->update(0, buttonStates));

    core->Shutdown();
    PluginRegistry::DestroyCore(core);
}
```

---

**Phase 7 Deliverable:** Comprehensive automated tests on all platforms

[^](#table-of-contents)
---

## Phase 8: Documentation & Release

### 8.1 Create Developer Documentation

Create the following documentation files:

**File to Create:** `docs/ARCHITECTURE.md`

Contents:
- High-level system architecture overview
- Plugin system design and flow
- IConsole and IDebugger contract specifications
- Plugin discovery and loading process
- Architecture diagrams

**File to Create:** `docs/PLUGIN_DEVELOPMENT.md`

Contents:
- Step-by-step guide for creating new console plugins
- Template plugin project structure
- CMakeLists.txt example
- Manifest configuration example
- Export functions implementation
- Testing guidelines

**File to Create:** `docs/PLUGIN_API.md`

Contents:
- Complete IConsole interface reference
- Complete IDebugger interface reference
- PluginManifest structure reference
- Export macro reference
- Error handling guidelines

**File to Create:** `docs/BUILD_SYSTEM.md`

Contents:
- Overall build system structure
- CMake configuration options
- Platform-specific build notes
- Plugin build configuration

### 8.2 Create User Documentation

**File to Create:** `docs/INSTALLATION.md`

Contents:
- Windows installation steps
- Linux installation steps
- macOS installation steps
- System requirements
- Common troubleshooting

**File to Create:** `docs/CUSTOM_PLUGINS.md`

Contents:
- Plugin search path explanation
- Environment variable configuration
- Plugin compatibility checking
- Custom plugin loading

### 8.3 Create Release Documentation

**File to Create:** `RELEASE_NOTES.md`

Template:
- Version number
- Release date
- New features
- Bug fixes
- Breaking changes
- Known issues
- Installation instructions
- Supported consoles

**Phase 8 Deliverable:** Complete documentation set

[^](#table-of-contents)
---

## Phase 9: Final Validation & Release

### 9.1 Full Integration Testing

Tasks:
- Test complete application flow on Windows, Linux, macOS
- Test all console cores individually
- Test multi-instance/linked mode operations
- Test error conditions and recovery
- Test console switching without crashes
- Test ROM loading/unloading cycles
- Test save/load state functionality
- Test input handling across all platforms

### 9.2 Performance Benchmarking

Tasks:
- Measure plugin discovery time
- Measure plugin load time
- Measure core instantiation time
- Profile NES emulation speed on each platform
- Profile GBA emulation speed on each platform
- Compare performance to monolithic baseline
- Check memory usage across platforms
- Verify no memory leaks

### 9.3 Cleanup & Optimization

Tasks:
- Remove old monolithic code
- Remove deprecated #ifdefs
- Clean up old factory patterns
- Update all includes to plugin system
- Remove unused dependencies
- Optimize critical paths
- Minimize binary sizes

### 9.4 Create Release Builds

Tasks:
- Build Release configuration on Windows
- Create Windows distribution package
- Build Release configuration on Linux
- Create Linux distribution package
- Build Release configuration on macOS (Intel)
- Build Release configuration on macOS (Apple Silicon)
- Create macOS distribution package
- Generate checksums for all packages
- Sign binaries appropriately
- Create GitHub releases

**Phase 9 Deliverable:** Production release ready for all platforms

[^](#table-of-contents)
---

## Timeline & Success Criteria

### Estimated Duration by Phase

- Phase 0: 1-2 weeks
- Phase 1: 1 week
- Phase 2: 1-2 weeks
- Phase 3: 1-2 weeks
- Phase 4: 1-2 weeks
- Phase 5: 2-3 weeks
- Phase 6: 1 week
- Phase 7: 1-2 weeks
- Phase 8: 1-2 weeks
- Phase 9: 1-2 weeks

**Total Estimated Duration: 15-23 weeks**

### Critical Path

Phase 0.0 → 0.1 → 0.2 → 1.1 → 2.1 → 3.1 → 4.x → 7.x → 9.x

### Success Criteria

1. Frontend application is the "main" executable
2. Each console core is a separate plugin (DLL/SO/DYLIB)
3. Plugins load dynamically at runtime from standard search paths
4. Multi-platform support (Windows, Linux, macOS)
5. Multi-instance support (linked cores, multiplayer)
6. Performance parity with monolithic version
7. Clear plugin API for future expansion
8. Comprehensive developer and user documentation
9. Automated testing on all platforms passes
10. Professional release packages available for all platforms

### Key Architecture Decisions

1. IConsole & IDebugger as primary bridge
2. Factory pattern for instance creation
3. Manifest-based version validation
4. Optional debugger support
5. Multi-platform dynamic loading abstraction
6. Registry pattern for plugin management
7. Frontend remains console-agnostic

[^](#table-of-contents)
---