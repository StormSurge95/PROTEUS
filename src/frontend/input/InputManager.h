#pragma once

#include "../FrontendPCH.h"
#include "../app/IManagerContexts.h"
#include "../logging/Logger.h"

namespace NS_Proteus {
    class Logger;

    class InputManager {
        public:
            bool useKB = true;

            InputManager(const IInputContext* c) : ctx(c), kbState(make_unique<Inputs>()) {}
            ~InputManager() { Deinit(); }

            InputManager(const InputManager&) = delete;
            InputManager& operator=(const InputManager&) = delete;
            InputManager(InputManager&&) = delete;
            InputManager& operator=(InputManager&&) = delete;

            void Init();
            void Deinit();

            Inputs* ReadInputs(int gp = 0, bool ui = false);
            Inputs* ReadKeyboard(bool ui);
            void TranslateInputs(const sptr<IConsole>& station, ConsoleID console);

            void Connect(SDL_JoystickID id);
            void Disconnect(SDL_JoystickID id);

        private:
            const IInputContext* ctx;
            Logger* logger = nullptr;

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