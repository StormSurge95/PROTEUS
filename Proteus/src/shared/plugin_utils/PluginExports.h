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
    // === VERSION & COMPATIBILITY ===

    /**
     * @brief Plugin version (major component)
     * @details Typically incremented for major feature releases
     */
    u16 pluginVersion_major;

    /**
     * @brief Plugin version (minor component)
     * @details Typically incremented for minor features or fixes
     */
    u16 pluginVersion_minor;

    /**
     * @brief Plugin version (patch component)
     * @details Typically increment for bug fixes only
     */
    u16 pluginVersion_patch;

    /**
     * @brief IConsole contract version this plugin implements
     * @details Must match ICONSOLE_CONTRACT_VERSION in frontend
     *          If mismatch detected, plugin will be rejected as incompatible
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
     * @details Used for plugin discovery and identification
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
     * @brief Plugin display name
     * @example "Proteus NES Emulation Plugin"
     * @details Used in UI to identify the specific plugin
     */
    const char* pluginName;

    /**
     * @brief Author or organization name
     * @details Credit line for the plugin creator(s)
     */
    const char* authorName;

    /**
     * @brief Plugin description
     * @details Brief description of what this plugin emulates
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
     * @brief License type identifier
     * @example "GPL-3.0"
     */
    const char* licenseType;

    // === STATUS ===

    /**
     * @brief Development status of this plugin
     */
    DevStatusValue devStatus;

    /**
     * @brief Human-readable status description
     */
    const char* statusDescription;
};

// Platform specific export declarations
#ifdef _WIN32
    #define PLUGIN_EXPORT extern "C" __declspec(dllexport)
    #define PLUGIN_INVOKE __cdecl
#elif defined(__GNUC__)
    #define PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
    #define PLUGIN_INVOKE __cdecl
#else
    #define PLUGIN_EXPORT extern "C"
    #define PLUGIN_INVOKE
#endif

/**
 * @def CORE_CREATE
 * @brief Declares the core factory function
 * @details REQUIRED EXPORT - Every plugin must provide:
 * 
 *   CORE_CREATE(CreateCore) {
 *       return new XXX();
 *   }
 * 
 * Signature: IConsole* CreateCore()
 */
#define CORE_CREATE(funcName) \
    PLUGIN_EXPORT IConsole* PLUGIN_INVOKE funcName()

/**
 * @def CORE_DESTROY
 * @brief Declares the core destructor function
 * @details REQUIRED EXPORT - Every plugin must provide:
 * 
 *   CORE_DESTROY(DestroyCore) {
 *       if (core)
             delete reinterpret_cast<XXX*>(core);
 *   }
 * 
 * Signature: void DestroyCore(IConsole* core)
 */
#define CORE_DESTROY(funcName) \
    PLUGIN_EXPORT void PLUGIN_INVOKE funcName(IConsole* core)

/**
 * @def DEBUGGER_CREATE
 * @brief Declares the debugger factory function
 * @details OPTIONAL EXPORT - Plugins may provide debugger support:
 * 
 *   DEBUGGER_CREATE(CreateDebugger) {
 *       return new XXXDebugger();
 *   }
 * 
 * Signature: IDebugger* CreateDebugger(IConsole* core)
 * 
 * @note while debugger implementation is pending, the
 * function provided by the plugin may return `nullptr`
 */
#define DEBUGGER_CREATE(funcName) \
    PLUGIN_EXPORT IDebugger* PLUGIN_INVOKE funcName(IConsole* core)

/**
 * @def DEBUGGER_DESTROY
 * @brief Declares the debugger destructor function
 * @details OPTIONAL EXPORT - Must exist if DEBUGGER_CREATE exists
 * 
 *   DEBUGGER_DESTROY(DestroyDebugger) {
 *       if (debugger)
 *           delete reinterpret_cast<XXXDebugger*>(debugger);
 *   }
 * 
 * Signature: void DestroyDebugger(IDebugger* debugger);
 */
#define DEBUGGER_DESTROY(funcName) \
    PLUGIN_EXPORT void PLUGIN_INVOKE funcName(IDebugger* debugger)

/**
 * @def PLUGIN_MANIFEST
 * @brief Declares the manifest export function
 * @details REQUIRED EXPORT - Every plugin must provide:
 * 
 *   PLUGIN_MANIFEST(GetPluginManifest) {
 *       static PluginManifest manifest = { ... };
 *       return &manifest;
 *   }
 * 
 * Signature: const PluginManifest* GetPluginManifest()
 */
#define PLUGIN_MANIFEST(funcName) \
    PLUGIN_EXPORT const PluginManifest* PLUGIN_INVOKE funcName()