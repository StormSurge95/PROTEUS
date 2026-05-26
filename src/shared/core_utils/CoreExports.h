#pragma once

#include "../SharedPCH.h"
#include "../Types.h"
#include "../IConsole.h"
#include "../IDebugger.h"

enum class DevStatusValue {
    STUB,
    WIP,
    FUNCTIONAL,
    PARTIAL,
    COMPLETE
};

const map<DevStatusValue, const char*> DevStatusDescription = {
    { DevStatusValue::STUB, "Stub implementation only, no actual functionality" },
    { DevStatusValue::WIP, "Early development, only test roms are expected to run (even if they aren't expected to pass)" },
    { DevStatusValue::FUNCTIONAL, "Core gameplay working; some (but not all) officially licensed ROMs should be playable" },
    { DevStatusValue::PARTIAL, "Full \"official\" implementation; any officially licensed ROM should be playable" },
    { DevStatusValue::COMPLETE, "Full implementation with debug tool support, all properly developed ROMs should be playable" }
};

/**
 * @struct CoreManifest
 * @brief Metadata exported by each core
 * @details Every core must export GetCoreManifest() returning
 *          a valid manifest. The frontend uses this to validate
 *          compatibility and display core information.
 * 
 * The manifest serves several purposes:
 * - Validates API contract compatibility before loading
 * - Provides metadata for UI display and logging
 * - Allows versioning and future extensibility
 * - Documents plugin capabilities and status
 */
struct CoreManifest {
    // === VERSION & COMPATIBILITY ===

    /**
     * @brief Core version (major component)
     * @details Typically incremented for major feature releases
     */
    u16 coreVersion_major;

    /**
     * @brief Core version (minor component)
     * @details Typically incremented for minor features or fixes
     */
    u16 coreVersion_minor;

    /**
     * @brief Core version (patch component)
     * @details Typically increment for bug fixes only
     */
    u16 coreVersion_patch;

    /**
     * @brief IConsole contract version this core implements
     * @details Must match ICONSOLE_CONTRACT_VERSION in frontend
     *          If mismatch detected, core will be rejected as incompatible
     */
    u16 iConsoleContractVersion;

    /**
     * @brief IDebugger contract version (if applicable)
     * @details Must match IDEBUGGER_CONTRACT_VERSION if debugger is provided
     *          Only validated if debugger functions are exported
     */
    u16 iDebuggerContractVersion;

    // === IDENTIFICATION ===

    /**
     * @brief Short console name (lowercase, no spaces)
     * @example "nes"
     * @details Used for core discovery and identification
     * @todo Should we just use our ConsoleID enum values?
     */
    const char* consoleName;

    /**
     * @brief Full console name (display friendly)
     * @example "Nintendo Entertainment System"
     * @details Used in UI and logging
     */
    const char* consoleFullName;

    /**
     * @brief Core display name
     * @example "Proteus NES Emulation Core"
     * @details Used in UI to identify the specific plugin
     */
    const char* coreName;

    /**
     * @brief Author or organization name
     * @details Credit line for the core creator(s)
     */
    const char* authorName;

    /**
     * @brief Core description
     * @details Brief description of what this core emulates
     *          and any notable features
     */
    const char* description;

    // === METADATA ===

    /**
     * @brief Build date in ISO 8601 format (YYYY-MM-DD)
     * @example "2026-05-26"
     */
    const char* buildDate;

    /**
     * @brief Licent type identifier
     * @example "GPL-3.0"
     */
    const char* licenseType;

    // === STATUS ===

    /**
     * @brief Development status of this core
     */
    DevStatusValue devStatus;

    /**
     * @brief Human-readable status description
     */
    const char* statusDescription;
};

// Platform specific export declarations
#ifdef _WIN32
    #define CORE_EXPORT extern "C" __declspec(dllexport)
    #define CORE_INVOKE __cdecl
#elif defined(__GNUC__)
    #define CORE_EXPORT extern "C" __attribute__((visibility("default")))
    #define CORE_INVOKE __cdecl
#else
    #define CORE_EXPORT extern "C"
    #define CORE_INVOKE
#endif

/**
 * @def CORE_CREATE
 * @brief Declares the core factory function
 * @details REQUIRED EXPORT - Every core must provide:
 * 
 *   CORE_CREATE(CreateXXXCore) {
 *       return new XXX();
 *   }
 * 
 * Signature: IConsole* CreateXXXCore()
 */
#define CORE_CREATE(funcName) \
    CORE_EXPORT IConsole* CORE_INVOKE funcName()

/**
 * @def CORE_DESTROY
 * @brief Declares the core destructor function
 * @details REQUIRED EXPORT - Every core must provide:
 * 
 *   CORE_DESTROY(DestroyXXXCore) {
 *       if (core)
             delete core;
 *   }
 * 
 * Signature: void DestroyXXXCore(IConsole* core)
 */
#define CORE_DESTROY(funcName) \
    CORE_EXPORT void CORE_INVOKE funcName(IConsole* core)

/**
 * @def DEBUGGER_CREATE
 * @brief Declares the debugger factory function
 * @details OPTIONAL EXPORT - Cores may provide debugger support:
 * 
 *   DEBUGGER_CREATE(CreateXXXDebugger) {
 *       return new XXXDebugger();
 *   }
 * 
 * Signature: IDebugger* CreateXXXDebugger(IConsole* core)
 * 
 * @note while debugger implementation is pending, the
 * function provided by the core may return `nullptr`
 */
#define DEBUGGER_CREATE(funcName) \
    CORE_EXPORT IDebugger* CORE_INVOKE funcName(IConsole* core)

/**
 * @def DEBUGGER_DESTROY
 * @brief Declares the debugger destructor function
 * @details OPTIONAL EXPORT - Must exist if DEBUGGER_CREATE exists
 * 
 *   DEBUGGER_DESTROY(DestroyXXXDebugger) {
 *       if (debugger)
 *           delete debugger;
 *   }
 * 
 * Signature: void DestroyXXXDebugger(IDebugger* debugger);
 */
#define DEBUGGER_DESTROY(funcName) \
    CORE_EXPORT void CORE_INVOKE funcName(IDebugger* debugger)

/**
 * @def CORE_MANIFEST
 * @brief Declares the manifest export function
 * @details REQUIRED EXPORT - Every core must provide:
 * 
 *   CORE_MANIFEST(GetXXXCoreManifest) {
 *       static CoreManifest manifest = { ... };
 *       return &manifest;
 *   }
 * 
 * Signature: const CoreManifest* GetXXXCoreManifest()
 */
#define CORE_MANIFEST(funcName) \
    CORE_EXPORT const CoreManifest* CORE_INVOKE funcName()