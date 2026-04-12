#include "../frontend/Proteus.h"
#include "InputManager.h"

Gamepad::Gamepad(int i) : id(i) {
    gamepad = SDL_OpenGamepad(id);
    state = std::make_unique<Inputs>();
    lastInputs.copy(*state.get());
    repeats.fill(false);
    lastChecks.fill(0);
}

Gamepad::~Gamepad() {
    id = 0;
    SDL_CloseGamepad(gamepad);
    state.release();
}

Inputs* Gamepad::read(bool ui) {
    for (int i = 0; i < 21; i++) {
        if (i < 15) processButton(i, ui);
        else processAxis(i, ui);
    }

    return state.get();
}

void Gamepad::processButton(int btn, bool ui) {
    bool pressed = SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)btn);
    uint64_t now = SDL_GetTicks();

    if (ui) {
        if (pressed) {
            if (lastInputs.getButton(btn)) {
                if (repeats[btn]) {
                    if (now - lastChecks[btn] >= REPEAT_RATE) {
                        state->setButton(btn, true);
                        lastChecks[btn] = now;
                    } else
                        state->setButton(btn, false);
                } else {
                    if (now - lastChecks[btn] >= REPEAT_DELAY) {
                        repeats[btn] = true;
                        state->setButton(btn, true);
                        lastChecks[btn] = now;
                    } else
                        state->setButton(btn, false);
                }
            }
            lastInputs.setButton(btn, true);
        } else {
            lastChecks[btn] = 0;
            repeats[btn] = false;
            lastInputs.setButton(btn, false);
            state->setButton(btn, false);
        }
    } else {
        lastChecks[btn] = now;
        repeats[btn] = false;
        lastInputs.setButton(btn, pressed);
        state->setButton(btn, pressed);
    }
}

void Gamepad::processAxis(int index, bool ui) {
    int axis = index - 15;
    short dir = SDL_GetGamepadAxis(gamepad, (SDL_GamepadAxis)axis);
    if (ui) dir = GetDirection(dir);
    uint64_t now = SDL_GetTicks();

    if (ui) {
        if (dir != INACTIVE) {
            if (lastInputs.getAxis(axis) != INACTIVE) {
                if (repeats[index]) {
                    if (now - lastChecks[index] >= REPEAT_RATE) {
                        state->setAxis(axis, dir);
                        lastChecks[index] = now;
                    } else
                        state->setAxis(axis, INACTIVE);
                }
            } else {
                if (now - lastChecks[index] >= REPEAT_DELAY) {
                    repeats[index] = true;
                    state->setAxis(axis, dir);
                    lastChecks[index] = now;
                } else
                    state->setAxis(axis, INACTIVE);
            }
            lastInputs.setAxis(axis, dir);
        } else {
            lastChecks[index] = 0;
            repeats[index] = false;
            lastInputs.setAxis(axis, INACTIVE);
            state->setAxis(axis, INACTIVE);
        }
    } else {
        lastChecks[index] = now;
        repeats[index] = false;
        lastInputs.setAxis(axis, dir);
        state->setAxis(axis, dir);
    }
}

Gamepad::AxisDirection Gamepad::GetDirection(short value) {
    if (value > AXIS_DEADZONE) return POSITIVE;
    if (SDL_abs(value) > AXIS_DEADZONE) return NEGATIVE;
    return INACTIVE;
}

InputManager::InputManager(Proteus* p, bool d) {
    proteus = p;
    debug = d;
}

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
    for (int i = 0; i < 4; i++) {
        if (gamepads[i] == nullptr) {
            gamepads[i] = new Gamepad(id);
            numPlayers++;
            return;
        }
    }
}

void InputManager::Disconnect(SDL_JoystickID id) {
    for (int i = 0; i < 4; i++) {
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
    if (ui) gp = 0;
    if (gamepads[gp] != nullptr)
        return gamepads[gp]->read(ui);
    else return nullptr;
}

void InputManager::TranslateInputs(std::shared_ptr<IConsole>& station, CONSOLE_ID console) {
    for (int i = 0; i < numPlayers; i++) {
        Inputs* inputs = ReadInputs(i);
        if (inputs == nullptr) throw std::exception();

        switch (console) {
            case NES:
                std::array<bool, 8> btns = { inputs->A_BUTTON, inputs->B_BUTTON, inputs->SELECT, inputs->START,
                    inputs->DPAD_UP, inputs->DPAD_DOWN, inputs->DPAD_LEFT, inputs->DPAD_RIGHT };
                station->update(i, btns.data());
                break;
        }
    }
}