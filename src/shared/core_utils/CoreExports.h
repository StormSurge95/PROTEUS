#pragma once

#include "../SharedPCH.h"
#include "../Types.h"
#include "../IConsole.h"
#include "../IDebugger.h"

struct CoreManifest {
    // === VERSION & COMPATIBILITY ===
    u16 coreVersion_major;
    u16 coreVersion_minor;
    u16 coreVersion_patch;

    // MUST MATCH ICONSOLE_CONTRACT_VERSION
    u16 iConsoleContractVersion;

    // MUST MATCH IDEBUGGER_CONTRACT_VERSION
    u16 iDebuggerContractVersion;

    // === IDENTIFICATION ===
    const char* consoleName;        // "NES"
    const char* consoleFullName;    // "Nintendo Entertainment System"
    const char* coreName;           // "NES Emulation Core"
    ConsoleID id;                   // ConsoleID::NES
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
    CORE_EXPORT const CoreManifest* CORE_INVOKE funcName();