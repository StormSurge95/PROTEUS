#pragma once

#include "./FrontendPCH.h"
#include "./Proteus.h"

namespace NS_Proteus {
    class InputManager {
        public:
            bool useKB = true;

            InputManager(Proteus* proteus, bool debug = false);
            ~InputManager() { Deinit(); }

            InputManager(const InputManager&) = delete;
            InputManager& operator=(const InputManager&) = delete;
            InputManager(InputManager&&) = delete;
            InputManager& operator=(InputManager&&) = delete;

            void Init();
            void Deinit();

            Inputs* ReadInputs(int gp = 0, bool ui = false);
            Inputs* ReadKeyboard(bool ui);
            void TranslateInputs(std::shared_ptr<IConsole>& station, ConsoleID console);

            void Connect(SDL_JoystickID id);
            void Disconnect(SDL_JoystickID id);

        private:
            Proteus* proteus;
            bool debug;

            const int MAX_PLAYERS = 4;
            int numPlayers = 0;
            std::array<Gamepad*, 4> gamepads = { nullptr, nullptr, nullptr, nullptr };
            KeyBinds kbs;
            std::unique_ptr<Inputs> kbState;

            void DisconnectGP0();
            void DisconnectGP1();
            void DisconnectGP2();
            void DisconnectGP3();
    };
}