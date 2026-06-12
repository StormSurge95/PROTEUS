#pragma once

#include "../plugin/PluginManager.h"

using namespace NS_Proteus;

class Headless {
    public:
        struct InputEvent {
            u64 frame = 0;
            u8 player = 0;
            size_t button = 0;
            bool pressed = false;
        };

        static void Run(ConsoleID id, const path& rom, const string& inputSpec = "");
};