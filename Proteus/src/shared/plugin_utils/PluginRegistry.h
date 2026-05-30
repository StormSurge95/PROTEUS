#pragma once

#include "./PluginLoader.h"
#include "../Types.h"

class PluginRegistry {
    public:
        struct RegistryEntry {
            string id;
            string filePath;
            bool discovered = false;
            bool loaded = false;
            PluginLoader::LoadedPlugin plugin{};
        };

        /**
         * @brief Scan valid search paths for applicable plugin DLL files
         * @details This function can be used to both initialize and update
         *          the `registry` map of plugins.
         * @note In the event that no plugins are discovered, this function
         *          updates `lastError`.
         * @return true if at least one plugin is discovered; false otherwise
         */
        static bool DiscoverPlugins();

        /**
         * @brief Load a particular plugin library
         * @param id The id of the plugin to load
         * @return true of load was successful; false otherwise
         */
        static bool LoadPlugin(const string& id);

        /**
         * @brief Load all currently discovered plugin libraries
         * @note Upon failure to load any library, this function updates `lastError`
         */
        static void LoadAllPlugins();

        /**
         * @brief Unload a particular plugin library
         * @param id the id of the plugin to unload
         * @return true if unload was successful; false otherwise
         */
        static bool UnloadPlugin(const string& id);

        /**
         * @brief Unload all currently loaded plugin libraries
         * @note Upon failure to unload any library, this function updates `lastError`
         */
        static void UnloadAllPlugins();

        /**
         * @brief Create the IConsole emulation core of the requested plugin
         * @param id The id of the plugin to create a core from.
         * @return Dynamic pointer (owned by caller) to the created core
         * @note If `id` does not link to a currently loaded plugin, then this function will attempt to load said plugin
         * @important In the event of any failure, this function will simply return `nullptr` and update `lastError`
         */
        static IConsole* CreateCore(const string& id);

        /**
         * @brief Destroy the provided IConsole emulator core
         * @param core The core instance to destroy
         * @return true if successful destruction; false otherwise
         */
        static bool DestroyCore(IConsole* core);

        /**
         * @brief Create the IDebugger implementation core of the requested plugin for the provided core.
         * @param id The id of the plugin to use for debugger creation
         * @param core The core to attach to the created debugger
         * @return Dynamic pointer (ownded by caller) to the created debugger
         * @note If `id` does not link to a currently loaded plugin, then this function will attempt to load said plugin
         * @important In the event of any failure, this function will simply return `nullptr` and update `lastError`
         * @important If the referenced library does not implement IDebugger; will record it as an error; but Debugger implementation
         *          is optional and therefore the outcome of it not being implemented must be properly handled by the caller.
         */
        static IDebugger* CreateDebugger(const string& id, IConsole* core);

        /**
         * @brief Destroy the provided IDebugger instance
         * @param dbg The IDebugger instance to be destroyed
         * @return true if successful destruction; false otherwise
         */
        static bool DestroyDebugger(IDebugger* dbg);

        /**
         * @brief Determine whether a particular plugin has been discovered
         * @param id The id of the plugin to check
         * @return true if discovered; false otherwise
         */
        static bool IsPluginDiscovered(const string& id);

        /**
         * @brief Determine whether a particular plugin is currently loaded
         * @param id The id of the plugin to check
         * @return true if loaded; false otherwise
         */
        static bool IsPluginLoaded(const string& id);

        /**
         * @brief Acquire the core manifest associated with a particular plugin
         * @param id The plugin to acquire the manifest for
         * @return Pointer reference (owned by the emulation core of the plugin) to the related CoreManifest
         * @note If `id` does not link to a currently loaded plugin, then this function will attempt to load said plugin
         */
        static const PluginManifest* GetManifest(const string& id);

        /**
         * @brief Acquire a list of all discovered plugins
         * @return A vector containing a string ID for each discovered plugin
         */
        static vector<string> GetDiscoveredPlugins();

        /**
         * @brief Acquire a list of all currently loaded plugins
         * @return A vector containing a string ID for each currently loaded plugin
         */
        static vector<string> GetLoadedPlugins();

        /**
         * @brief Get the last recorded error of the PluginRegistry context 
         */
        static string GetLastError() { return lastError; }

    private:
        /// @brief map of RegistryEntry objects keyed via their string IDs
        inline static unordered_map<string, RegistryEntry> registry{};

        /// @brief map of plugin ID strings used as keys for library base names
        inline static unordered_map<string, string> libBaseNames = {
            { "nes", "ProteusNES" },
            { "gba", "ProteusGBA" },
            #ifndef NDEBUG
            { "bad_symbols", "BadSymbolsPlugin" },
            #endif
        };

        /// @brief map of IConsole pointers used as keys to determine the ID of the RegistryEntry context used to create them
        /// @important DO NOT MANUALLY FREE THESE POINTERS. ALL CREATE/DESTROY MUST BE DONE THROUGH THE REGISTRY
        inline static unordered_map<IConsole*, string> consoleCreators{};

        /// @brief map of IDebugger pointers used as keys to determine the ID of the RegistryEntry context used to create them
        /// @important DO NOT MANUALLY FREE THESE POINTERS. ALL CREATE/DESTROY MUST BE DONE THROUGH THE REGISTRY
        inline static unordered_map<IDebugger*, string> debuggerCreators{};

        /// @brief human-readable error message
        inline static string lastError = "";

        /**
         * @brief Acquire all valid plugin search paths
         * @return A vector containing a single string for each valid search path
         */
        static vector<string> GetSearchPaths();

        /**
         * @brief Acquire a list of all known plugin IDs
         * @return A vector containing a string ID for each known plugin
         */
        static vector<string> GetKnownPluginIDs();

        /**
         * @brief Collect a mutable RegistryEntry reference for a particular plugin ID
         * @param id The ID to collect a reference for
         * @return A mutable reference to the requested entry
         */
        static RegistryEntry* FindEntryMutable(const string& id);

        /**
         * @brief Collect an immutable RegistryEntry reference for a particular plugin ID
         * @param id The ID to collect a reference for
         * @return An immutable reference to the requested entry
         */
        static const RegistryEntry* FindEntry(const string& id);
};