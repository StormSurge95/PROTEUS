#pragma once

#include "./FrontendPCH.h"

namespace NS_Proteus {
    class RomLibrary {
        private:
            path baseRomPath = absolute("C:/ROMS").lexically_normal().make_preferred();
            path libraryPath = absolute("C:/ROMS/RomLibrary.json").lexically_normal().make_preferred();

            map<ConsoleID, vector<ROM_DATA>> library;

            string GetHash(const string& filepath);
            string Lookup(ConsoleID console, const string& hash);

            void Save();
            void Load();
            void Create();

            string NormPathToKey(path rawPath);
            ROM_DATA NewData(const ConsoleID id, const directory_entry& entry);
            bool Unchanged(const ROM_DATA& oldData, const directory_entry& newData);
        public:
            inline RomLibrary() { Load(); }
            inline RomLibrary(path libPath) { libraryPath = libPath; Load(); }
            inline ~RomLibrary() { Save(); }

            void Refresh(ConsoleID console = ConsoleID::NONE);
            inline vector<ROM_DATA>& GetGameList(ConsoleID console) { return library[console]; }
    };
}