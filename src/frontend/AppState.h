#pragma once

#include <string>
#include <vector>

enum AppView : unsigned int {
    CONSOLE_SELECT,
    GAME_LIST,
    GAME_VIEW,
    MENU
};

enum CONSOLE_ID {
    // Following systems are 100% functional
    // (though likely could use some form of improvements)
    NONE = -1,
    
    // Following systems are not yet completed, but are WIP
    NES, // only missing audio
    // Following systems are not yet implemented or in progress.
    SNES,
    PLAYSTATION,
    N64,
    PS2,
    GBA,
    GAMECUBE,
    XBOX,
    NDS,
    XBOX_360,
    PS3,
    WII,


    // Systems below are not planned to be emulated until they
    // no longer have games being officially released to them.
    
    //PS4, // this might be an exception due to bloodborne...
    
    //XBOX_ONE,
    //SWITCH,
    //XBOX_XS,
    //PS5,
    //SWITCH2
};

struct AppState {
    AppView currentView = CONSOLE_SELECT;
    CONSOLE_ID selectedConsole = NONE;
    std::string selectedGame = "";
    bool isLoading = false;
};

const std::vector<std::pair<std::string, std::string>> CONSOLES = {
            { "NES", "NES" },
            { "SNES", "SNES" },
            { "PS1", "PLAYSTATION" },
            { "N64", "NINTENDO-64" },
            { "PS2", "PLAYSTATION 2" },
            { "GBA", "GAMEBOY\nADVANCE" },
            { "XBOX", "XBOX" },
            { "NGC", "GAMECUBE" },
            { "X360", "XBOX 360" },
            { "NDS", "NINTENDO DS" },
            { "PS3", "PLAYSTATION 3" },
            { "WII", "NINTENDO WII" }
};

static std::string GetConsoleFromID(CONSOLE_ID id) {
    switch (id) {
        case NES: return "NES";
        case SNES: return "SNES";
        case PLAYSTATION: return "PS1";
        case N64: return "N64";
        case PS2: return "PS2";
        case GBA: return "GBA";
        case XBOX: return "XBOX";
        case GAMECUBE: return "NGC";
        case XBOX_360: return "X360";
        case NDS: return "NDS";
        case PS3: return "PS3";
        case WII: return "WII";
        default: return "";
    }
}