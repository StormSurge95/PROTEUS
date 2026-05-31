#include "InputTypes.h"

using namespace NS_Proteus;

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
    u64 now = SDL_GetTicks();

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
    s32 axis = index - 15;
    s16 dir = SDL_GetGamepadAxis(gamepad, (SDL_GamepadAxis)axis);
    if (ui) dir = (s16)GetDirection(dir);
    u64 now = SDL_GetTicks();

    if (ui) {
        if (dir != (s16)AxisDirection::INACTIVE) {
            if (lastInputs.getAxis(axis) != (s16)AxisDirection::INACTIVE) {
                if (repeats[index]) {
                    if (now - lastChecks[index] >= REPEAT_RATE) {
                        state->setAxis(axis, dir);
                        lastChecks[index] = now;
                    } else
                        state->setAxis(axis, (s16)AxisDirection::INACTIVE);
                }
            } else {
                if (now - lastChecks[index] >= REPEAT_DELAY) {
                    repeats[index] = true;
                    state->setAxis(axis, dir);
                    lastChecks[index] = now;
                } else
                    state->setAxis(axis, (s16)AxisDirection::INACTIVE);
            }
            lastInputs.setAxis(axis, dir);
        } else {
            lastChecks[index] = 0;
            repeats[index] = false;
            lastInputs.setAxis(axis, (s16)AxisDirection::INACTIVE);
            state->setAxis(axis, (s16)AxisDirection::INACTIVE);
        }
    } else {
        lastChecks[index] = now;
        repeats[index] = false;
        lastInputs.setAxis(axis, dir);
        state->setAxis(axis, dir);
    }
}

AxisDirection Gamepad::GetDirection(short value) {
    if (value > AXIS_DEADZONE) return AxisDirection::POSITIVE;
    if (SDL_abs(value) > AXIS_DEADZONE) return AxisDirection::NEGATIVE;
    return AxisDirection::INACTIVE;
}