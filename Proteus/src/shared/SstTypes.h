#pragma once

#include <SharedPCH.h>

struct SingleStateTest {
    string name;
    struct State {
        u16 pc;
        u8 s;
        u8 a;
        u8 x;
        u8 y;
        u8 p;
        vector<array<u16, 2>> ram;

        State(json data) {
            pc = data["pc"];
            s = data["s"];
            a = data["a"];
            x = data["x"];
            y = data["y"];
            p = data["p"];
            for (int i = 0; i < data["ram"].size(); i++) {
                ram.push_back({ data["ram"][i][0], data["ram"][i][1] });
            }
        }
    } initState, finalState;
    struct Cycle {
        u16 addr;
        u8 data;
        string op;

        Cycle(json d) {
            addr = d[0];
            data = d[1];
            op = d[2];
        }
    };
    vector<Cycle> cycles;

    SingleStateTest(json data) :
        name(data["name"]),
        initState(data["initial"]),
        finalState(data["final"]) {
        for (int i = 0; i < data["cycles"].size(); i++) {
            cycles.push_back(Cycle(data["cycles"][i]));
        }
    }
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