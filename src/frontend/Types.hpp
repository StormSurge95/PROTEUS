#pragma once

#include "FrontendPCH.hpp"

namespace NS_Proteus {
    class AudioManager;
    class InputManager;
    class VideoManager;

    enum class AppView {
        CONSOLE_SELECT,
        GAME_LIST,
        GAME_VIEW,
    };

    enum class ConsoleID : u8 {
        NES,
        SNS,
        PS1,
        N64,
        PS2,
        GBC,
        GBA,
        NGC,
        XBX,
        NDS,
        XB3,
        PS3,
        WII,
        TOTAL,
        NONE
        //XB1,
        //NSW,
        //XXS,
        //PS5,
        //NS2
    };

    const map<ConsoleID, string> ConsoleNamesShort = {
        { ConsoleID::NES, "NES" },
        { ConsoleID::SNS, "SNES" },
        { ConsoleID::PS1, "PS1" },
        { ConsoleID::N64, "N64" },
        { ConsoleID::PS2, "PS2" },
        { ConsoleID::GBC, "GBC" },
        { ConsoleID::GBA, "GBA" },
        { ConsoleID::NGC, "GAMECUBE" },
        { ConsoleID::XBX, "XBOX" },
        { ConsoleID::NDS, "NDS" },
        { ConsoleID::XB3, "X360" },
        { ConsoleID::PS3, "PS3" },
        { ConsoleID::WII, "WII" }
    };

    const map<ConsoleID, string> ConsoleNamesLong = {
        { ConsoleID::NES, "NINTENDO ENTERNTAINMENT SYSTEM" },
        { ConsoleID::SNS, "SUPER NINTENDO ENTERTAINMENT SYSTEM" },
        { ConsoleID::PS1, "PLAYSTATION" },
        { ConsoleID::N64, "NINTENDO 64" },
        { ConsoleID::PS2, "PLAYSTATION 2" },
        { ConsoleID::GBC, "GAMEBOY COLOR" },
        { ConsoleID::GBA, "GAMEBOY ADVANCE" },
        { ConsoleID::NGC, "NINTENDO GAMECUBE" },
        { ConsoleID::NDS, "NINTENDO DS" },
        { ConsoleID::XBX, "XBOX" },
        { ConsoleID::XB3, "XBOX 360" },
        { ConsoleID::PS3, "PLAYSTATION 3" },
        { ConsoleID::WII, "NINTENTO WII" }
    };

    static ConsoleID GetIDFromName(string name) {
        for (const auto& [key, val] : ConsoleNamesShort) {
            if (strcmp(name.c_str(), val.c_str()) == 0)
                return key;
        }

        return ConsoleID::NONE;
    }
}