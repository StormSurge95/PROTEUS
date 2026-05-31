#include "./InputTypes.h"
#include "./InputManager.h"

using namespace NS_Proteus;

void InputManager::Init() {
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL Input! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    } else SDL_LogMessage(SDL_LOG_CATEGORY_INPUT, SDL_LOG_PRIORITY_INFO, "Input SubSystem successfully initialized.");

    if (!SDL_InitSubSystem(SDL_INIT_HAPTIC))
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_WARN, "Failed to initialize SDL Haptic Feedback! Error: %s\n", SDL_GetError());

    int numGamepads;
    SDL_JoystickID* ids = SDL_GetGamepads(&numGamepads);

    if (numGamepads > 0) {
        for (int i = 0; i < numGamepads; i++) {
            Connect(ids[i]);
        }
    }

    SDL_free(ids);
}

void InputManager::Deinit() {
    for (Gamepad* gp : gamepads)
        delete gp;
}

void InputManager::Connect(SDL_JoystickID id) {
    useKB = false;
    for (u8 i = 0; i < 4; i++) {
        if (gamepads[i] == nullptr) {
            gamepads[i] = new Gamepad(id);
            numPlayers++;
            logger->EmitInputEvent(LogEventName::INPUT_GAMEPAD_CONNECTED,
                { .device = gamepads[i]->id, .player = (++i) } // TODO: get frame in here somehow
            );
            return;
        }
    }
}

void InputManager::Disconnect(SDL_JoystickID id) {
    for (u8 i = 0; i < 4; i++) {
        if (gamepads[i]->id == id) {
            switch (i) {
                case 0:
                    DisconnectGP0();
                    numPlayers--;
                    return;
                case 1:
                    DisconnectGP1();
                    numPlayers--;
                    return;
                case 2:
                    DisconnectGP2();
                    numPlayers--;
                    return;
                case 3:
                    DisconnectGP3();
                    numPlayers--;
                    return;
            }
            logger->EmitInputEvent(LogEventName::INPUT_GAMEPAD_DISCONNECTED,
                { .device = id, .player = (++i) } // TODO: get frame in here somehow
            );
        }
    }
}

void InputManager::DisconnectGP0() {
    if (gamepads[1] != nullptr) { // GP0 disconnecting while GP1 is active
        Gamepad* temp = gamepads[0];
        gamepads[0] = gamepads[1];
        gamepads[1] = temp;
        DisconnectGP1();            // swap GP0 and GP1, then disconnect GP1
    } else { // no other gamepads, safe to disconnect GP0
        delete gamepads[0]; gamepads[0] = nullptr;
        useKB = true;
    }
}

void InputManager::DisconnectGP1() {
    if (gamepads[2] != nullptr) { // GP1 disconnecting while GP2 is active
        Gamepad* temp = gamepads[1];
        gamepads[1] = gamepads[2];
        gamepads[2] = temp;
        DisconnectGP2();            // swap GP1 and GP2, then disconnect GP2
    } else { // no other gamepads, safe to disconnect GP2
        delete gamepads[1]; gamepads[1] = nullptr;
    }
}

void InputManager::DisconnectGP2() {
    if (gamepads[3] != nullptr) { // GP2 disconnecting while GP3 is active
        Gamepad* temp = gamepads[2];
        gamepads[2] = gamepads[3];
        gamepads[3] = temp;
        DisconnectGP3();            // swap GP2 and GP3, then disconnect GP3
    } else { // no other gamepads, safe to disconnect GP3
        delete gamepads[2]; gamepads[2] = nullptr;
    }
}

void InputManager::DisconnectGP3() {
    delete gamepads[3]; gamepads[3] = nullptr;
}

Inputs* InputManager::ReadInputs(int gp, bool ui) {
    Inputs* i = nullptr;
    if (ui) gp = 0;
    if (gamepads[gp] != nullptr)
        i = gamepads[gp]->read(ui);
    else if (gp == 0) i = ReadKeyboard(ui);
    
    return i;
}

Inputs* InputManager::ReadKeyboard(bool ui) {
    const bool* kb = SDL_GetKeyboardState(nullptr);
    bool btns[15] = {
        kb[kbs.A_BUTTON], kb[kbs.B_BUTTON], kb[kbs.X_BUTTON], kb[kbs.Y_BUTTON],
        kb[kbs.SELECT], kb[kbs.MENU], kb[kbs.START],
        kb[kbs.LEFT_SHOULDER], kb[kbs.RIGHT_SHOULDER],
        kb[kbs.DPAD_UP], kb[kbs.DPAD_DOWN], kb[kbs.DPAD_LEFT], kb[kbs.DPAD_RIGHT]
    };
    kbState->SetButtons(btns);

    return kbState.get();
}

void InputManager::TranslateInputs(const sptr<IConsole>& station, ConsoleID console) {
    for (int i = 0; i < numPlayers; i++) {
        Inputs* inputs = ReadInputs(i);
        if (inputs == nullptr) throw std::exception();

        switch (console) {
            case ConsoleID::NES:
                std::array<bool, 8> btns = { inputs->A_BUTTON, inputs->B_BUTTON, inputs->SELECT, inputs->START,
                    inputs->DPAD_UP, inputs->DPAD_DOWN, inputs->DPAD_LEFT, inputs->DPAD_RIGHT };
                station->update(i, btns.data());
                break;
            default:
                break;
        }
    }
}