#pragma once

#include "../FrontendPCH.h"
#include "./RomLibraryTypes.h"

namespace NS_Proteus {
    class RomLibrary {
        private:
            path baseRomPath;
            path libraryPath;

            map<ConsoleID, vector<ROM_DATA>> library;

            /**
             * @brief Get the hash value for a ROM file pointed to by `filePath`
             * 
             * @param filepath Path to the ROM file
             * @return string Hash value produced from ROM
             */
            string GetHash(const string& filepath);

            /**
             * @brief Lookup the provided `hash` to find the normalized title of
             *        a ROM for a provided `console`
             * 
             * @example <hash for "Metroid (USA).nes"> ---> "Metroid"
             * 
             * @param console The ConsoleID representing the original console the ROM is meant for.
             * @param hash The hash produced by the requested ROM
             * @return string The normalized title of the provided ROM.
             */
            string Lookup(ConsoleID console, const string& hash);

            /**
             * @brief Save the runtime value(s) of the rom library to a readable file.
             */
            void Save();

            /**
             * @brief Attempt to load the rom library from a readable file.
             *        If file does not exist, calls `Create()` and returns
             */
            void Load();

            /**
             * @brief Attempts to create a library to use for locating and opening various ROM files.
             * 
             * @details
             * If `baseRomPath` does not exist, then the directory is created before this function returns.
             * If the directory exists, `Refresh()` is called with each ConsoleID key
             */
            void Create();

            string NormPathToKey(path rawPath);
            ROM_DATA NewData(const ConsoleID id, const directory_entry& entry);
            bool Unchanged(const ROM_DATA& oldData, const directory_entry& newData);
        public:
            RomLibrary();
            inline RomLibrary(path libPath) { libraryPath = libPath; Load(); }
            inline ~RomLibrary() { Save(); }

            /**
             * @brief 
             * 
             * @param console 
             */
            void Refresh(ConsoleID console = ConsoleID::NONE);
            inline vector<ROM_DATA>& GetGameList(ConsoleID console) { return library[console]; }
    };
}