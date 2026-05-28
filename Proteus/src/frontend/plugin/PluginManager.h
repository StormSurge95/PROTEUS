#pragma once

#include "../FrontendPCH.h"
#include "../../shared/plugin_utils/PluginExports.h"

namespace NS_Proteus {
    /// @brief mapping table from ConsoleID values to plugin registry IDs
    static map<ConsoleID, string> ConsolePluginMap = {
        { ConsoleID::NES, "nes" }
    };

    class PluginManager {
        public:
            /**
             * @brief Initialize the plugin manager context
             * @details Discovers available plugins through PluginRegistry.
             * @return true if discovery succeeds; false otherwise
             */
            static bool Initialize();

            /**
             * @brief Shut down plugin manager context
             * @details Unloads all currently loaded plugins and marks the
             *          manager as uninitialized.
             */
            static void Shutdown();

            /**
             * @brief Check whether a console is currently available
             * @param id The console to check
             * @return true if a mapped plugin exists and has been discovered
             */
            static bool IsConsoleAvailable(ConsoleID id);

            /**
             * @brief Acquire all currently available consoles
             * @return A vector containing a ConsoleID for each available console
             */
            static vector<ConsoleID> GetAvailableConsoles();

            /**
             * @brief Create a console core instance for the requested console
             * @param id The console to create
             * @return Dynamic pointer (owned by caller) to created IConsole instance
             * @important On failure, returns nullptr and updates `lastError`
             */
            static IConsole* CreateConsole(ConsoleID id);

            /**
             * @brief Destroy a previously created console core instance
             * @param core The core instance to destroy
             * @return true if successful destruction; false otherwise
             */
            static bool DestroyConsole(IConsole* core);

            /**
             * @brief Create a debugger instance for a console/core context
             * @param id The console owning the target core
             * @param core The core instance to attach to debugger
             * @return Dynamic pointer (owned by caller) to created debugger
             * @important On failure, returns nullptr and updates `lastError`
             */
            static IDebugger* CreateDebugger(ConsoleID id, IConsole* core);

            /**
             * @brief Destroy a previously created debugger instance
             * @param dbg The debugger instance to destroy
             * @return true if successful destruction; false otherwise
             */
            static bool DestroyDebugger(IDebugger* dbg);

            /**
             * @brief Acquire plugin metadata for a console
             * @param id The console to get metadata for
             * @return Pointer reference (owned by plugin) to PluginManifest
             * @important On failure, returns nullptr and updates `lastError`
             */
            static const PluginManifest* GetConsoleMetadata(ConsoleID id);

            /**
             * @brief Get the last recorded error of the PluginManager context
             */
            static string GetLastError() { return lastError; }

        private:
            /// @brief human-readable error message
            inline static string lastError = "";
            /// @brief tracks whether manager has been initialized
            inline static bool initialized = false;

            /**
             * @brief Map ConsoleID to plugin registry ID
             * @param id Console identifier
             * @return plugin ID string; empty string if unsupported
             */
            static string ToPluginID(ConsoleID id);

            /**
             * @brief Map plugin registry ID to ConsoleID
             * @param id Plugin ID string
             * @return matching ConsoleID; ConsoleID::NONE if no mapping exists
             */
            static ConsoleID ToConsoleID(const string& id);
    };
}
