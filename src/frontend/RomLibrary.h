#pragma once

#include "./FrontendPCH.h"

namespace NS_Proteus {
    class RomLibrary {
        private:
            path baseRomPath = "C:\\ROMS";
            path libraryPath = "C:\\ROMS\\RomLibrary.json";
            map<ConsoleID, vector<ROM_DATA>> library;

            string GetHash(const string& filepath);
            string Lookup(ConsoleID console, const string& hash);
            void Save();
            void Load();
            void Create();
        public:
            inline RomLibrary() { Load(); }
            inline RomLibrary(path libPath) { libraryPath = libPath; Load(); }
            inline ~RomLibrary() { Save(); }

            void Refresh(ConsoleID console = ConsoleID::NONE);
            inline vector<ROM_DATA>& GetGameList(ConsoleID console) { return library[console]; }
    };
}