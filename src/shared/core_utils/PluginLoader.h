#pragma once

#include "./CoreExports.h"

/**
 * @class CoreLoader
 * @brief Cross-platform dynamic library loader
 * @details Handles loading, unloading, and symbol resolution for plugins
 *          on Windows, Linux, and macOS. Uses platform specific implementation.
 */
class PluginLoader {
    public:
        // Function pointer types matching plugin exports
        using CreateCoreFunc = IConsole* (*)();
        using DestroyCoreFunc = void (*)(IConsole*);
        using CreateDebuggerFunc = IDebugger * (*)(IConsole*);
        using DestroyDebuggerFunc = void (*)(IDebugger*);
        using GetManifestFunc = const PluginManifest* (*)();

        /**
         * @struct LoadedCore
         * @brief Information about a loaded core in memory
         */
        struct LoadedPlugin {
            string filePath;            // Full path to DLL/SO/DYLIB
            string consoleName;         // Short name ("NES", "GBA", "N64", etc.)

            void* libHandle;            // Platform-specific library handle

            CreateCoreFunc CreateCore;
            DestroyCoreFunc DestroyCore;
            CreateDebuggerFunc CreateDebugger;   // may return nullptr
            DestroyDebuggerFunc DestroyDebugger;
            GetManifestFunc GetManifest;

            PluginManifest manifest;
        };

        /**
         * @brief Load a core from file path
         * @param filePath Full path to core library
         * @param core [out] Populated with loaded core info
         * @return true if load was successful
         */
        static bool LoadPlugin(const string& filePath, LoadedPlugin& core);

        /**
         * @brief Unload a previously loaded core
         * @param core The core to unload
         * @return true if unload was successful
         */
        static bool UnloadPlugin(LoadedPlugin& core);

        /**
         * @brief Get the expected core filename for current platform
         * @param consoleName Console name (e.g., "nes", "gba", etc.)
         * @return Expected filename (e.g., "nes.dll" on Windows)
         */
        static string GetExpectedFilename(const string& consoleName);

        /**
         * @brief Get platform-specific file extension
         * @return ".dll" on windows, ".so" on Linux, ".dylib" on macOS
         */
        static string GetPluginExtension();

        /**
         * @brief Get platform-specific library prefix
         * @return "" on Windows, "lib" on Unix-like systems
         */
        static string GetLibraryPrefix();

        /**
         * @brief Get the last error message from core operations
         * @return Human-readable error message
         */
        static string GetLastError() { return lastError; }
    private:
        inline static string lastError = "";
        
        // Platform-specific implementations
        static void* LoadLib(const string& filePath);
        static bool UnloadLib(void* handle);
        static void* GetSym(void* handle, const string& symName);
};