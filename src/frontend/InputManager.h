#pragma once

#include <array>
#include <SDL3/SDL.h>

#include "./AppState.h"
#include "./Proteus.h"

struct Inputs {
    bool A_BUTTON = false;          //  0 - SOUTH
    bool B_BUTTON = false;          //  1 - EAST
    bool X_BUTTON = false;          //  2 - WEST
    bool Y_BUTTON = false;          //  3 - NORTH
    bool SELECT = false;            //  4 - BACK
    bool MENU = false;              //  5 - GUIDE
    bool START = false;             //  6 - START
    bool L3_BUTTON = false;         //  7 - 
    bool R3_BUTTON = false;         //  8
    bool L1_BUTTON = false;         //  9
    bool R1_BUTTON = false;         // 10
    bool DPAD_UP = false;           // 11
    bool DPAD_DOWN = false;         // 12
    bool DPAD_LEFT = false;         // 13
    bool DPAD_RIGHT = false;        // 14
    int16_t AXIS_LEFTX = 0;
    int16_t AXIS_LEFTY = 0;
    int16_t AXIS_RIGHTX = 0;
    int16_t AXIS_RIGHTY = 0;
    int16_t AXIS_LEFT_TRIGGER = 0;
    int16_t AXIS_RIGHT_TRIGGER = 0;

    Inputs() = default;
    Inputs(bool a, bool b, bool x, bool y, bool se, bool m, bool st,
           bool l3, bool r3, bool l1, bool r1, bool du, bool dd, bool dl,
           bool dr, short lx, short ly, short rx, short ry, short lt, short rt) :
        A_BUTTON(a), B_BUTTON(b), X_BUTTON(x), Y_BUTTON(y), SELECT(se), MENU(m),
        START(st), L3_BUTTON(l3), R3_BUTTON(r3), L1_BUTTON(l1), R1_BUTTON(r1),
        DPAD_UP(du), DPAD_DOWN(dd), DPAD_LEFT(dl), DPAD_RIGHT(dr), AXIS_LEFTX(lx),
        AXIS_LEFTY(ly), AXIS_RIGHTX(rx), AXIS_RIGHTY(ry), AXIS_LEFT_TRIGGER(lt),
        AXIS_RIGHT_TRIGGER(rt) {}
    Inputs(std::array<bool, 15> btns, std::array<short, 6> axes) :
        A_BUTTON(btns[0]), B_BUTTON(btns[1]), X_BUTTON(btns[2]), Y_BUTTON(btns[3]), SELECT(btns[4]), MENU(btns[5]),
        START(btns[6]), L3_BUTTON(btns[7]), R3_BUTTON(btns[8]), L1_BUTTON(btns[9]), R1_BUTTON(btns[10]),
        DPAD_UP(btns[11]), DPAD_DOWN(btns[12]), DPAD_LEFT(btns[13]), DPAD_RIGHT(btns[14]), AXIS_LEFTX(axes[0]),
        AXIS_LEFTY(axes[1]), AXIS_RIGHTX(axes[2]), AXIS_RIGHTY(3), AXIS_LEFT_TRIGGER(axes[4]),
        AXIS_RIGHT_TRIGGER(axes[5]) {}

    void copy(const Inputs& other) {
        A_BUTTON = other.A_BUTTON;
        B_BUTTON = other.B_BUTTON;
        X_BUTTON = other.X_BUTTON;
        Y_BUTTON = other.Y_BUTTON;
        SELECT = other.SELECT;
        MENU = other.MENU;
        START = other.START;
        L3_BUTTON = other.L3_BUTTON;
        R3_BUTTON = other.R3_BUTTON;
        L1_BUTTON = other.L1_BUTTON;
        R1_BUTTON = other.R1_BUTTON;
        DPAD_UP = other.DPAD_UP;
        DPAD_DOWN = other.DPAD_DOWN;
        DPAD_LEFT = other.DPAD_LEFT;
        DPAD_RIGHT = other.DPAD_RIGHT;
        AXIS_LEFTX = other.AXIS_LEFTX;
        AXIS_LEFTY = other.AXIS_LEFTY;
        AXIS_RIGHTX = other.AXIS_RIGHTX;
        AXIS_RIGHTY = other.AXIS_RIGHTY;
        AXIS_LEFT_TRIGGER = other.AXIS_LEFT_TRIGGER;
        AXIS_RIGHT_TRIGGER = other.AXIS_RIGHT_TRIGGER;
    }

    bool getButton(int btn) const {
        switch (btn) {
            case 0:
                return A_BUTTON;
            case 1:
                return B_BUTTON;
            case 2:
                return X_BUTTON;
            case 3:
                return Y_BUTTON;
            case 4:
                return SELECT;
            case 5:
                return MENU;
            case 6:
                return START;
            case 7:
                return L3_BUTTON;
            case 8:
                return R3_BUTTON;
            case 9:
                return L1_BUTTON;
            case 10:
                return R1_BUTTON;
            case 11:
                return DPAD_UP;
            case 12:
                return DPAD_DOWN;
            case 13:
                return DPAD_LEFT;
            case 14:
                return DPAD_RIGHT;
            default:
                SDL_LogError(SDL_LOG_CATEGORY_INPUT, "That button (%d) shouldn't exist...\n", btn);
                return false;
        }
    }

    void setButton(int button, bool value) {
        switch (button) {
            case 0:
                A_BUTTON = value; return;
            case 1:
                B_BUTTON = value; return;
            case 2:
                X_BUTTON = value; return;
            case 3:
                Y_BUTTON = value; return;
            case 4:
                SELECT = value; return;
            case 5:
                MENU = value; return;
            case 6:
                START = value; return;
            case 7:
                L3_BUTTON = value; return;
            case 8:
                R3_BUTTON = value; return;
            case 9:
                L1_BUTTON = value; return;
            case 10:
                R1_BUTTON = value; return;
            case 11:
                DPAD_UP = value; return;
            case 12:
                DPAD_DOWN = value; return;
            case 13:
                DPAD_LEFT = value; return;
            case 14:
                DPAD_RIGHT = value; return;
            default:
                SDL_LogError(SDL_LOG_CATEGORY_INPUT, "That button (%d) shouldn't exist...\n", button);
                return;
        }
    }

    short getAxis(int axis) const {
        switch (axis) {
            case 0: return AXIS_LEFTX;
            case 1: return AXIS_LEFTY;
            case 2: return AXIS_RIGHTX;
            case 3: return AXIS_RIGHTY;
            case 4: return AXIS_LEFT_TRIGGER;
            case 5: return AXIS_RIGHT_TRIGGER;
            default:
                SDL_LogError(SDL_LOG_CATEGORY_INPUT, "That axis (%d) shouldn't exist...\n", axis);
                return 0;
        }
    }

    void setAxis(int axis, short value) {
        switch (axis) {
            case 0:
                AXIS_LEFTX = value; return;
            case 1:
                AXIS_LEFTY = value; return;
            case 2:
                AXIS_RIGHTX = value; return;
            case 3:
                AXIS_RIGHTY = value; return;
            case 4:
                AXIS_LEFT_TRIGGER = value; return;
            case 5:
                AXIS_RIGHT_TRIGGER = value; return;
            default:
                SDL_LogError(SDL_LOG_CATEGORY_INPUT, "That axis (%d) shouldn't exist...\n", axis);
                return;
        }
    }
};

struct Gamepad {
    SDL_JoystickID id = 0;          // ID value provided by SDL upon detection of gamepad
    SDL_Gamepad* gamepad = nullptr; // Actual Gamepad object created via SDL_OpenGamepad
    std::unique_ptr<Inputs> state = nullptr;
    Inputs lastInputs;
    std::array<bool, 21> repeats;
    std::array<Uint64, 21> lastChecks;

    enum AxisDirection {
        POSITIVE = 1,
        NEGATIVE = -1,
        INACTIVE = 0
    };

    static const int REPEAT_DELAY = 250;
    static const int REPEAT_RATE = 250;
    static const int AXIS_DEADZONE = 8000;

    Gamepad(int num);
    ~Gamepad();

    Inputs* read(bool ui);

    void processButton(int btn, bool ui);

    void processAxis(int index, bool ui);

    AxisDirection GetDirection(short value);
};

class InputManager {
    public:
        InputManager(Proteus* proteus, bool debug = false);
        ~InputManager() { Deinit(); }

        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;
        InputManager(InputManager&&) = delete;
        InputManager& operator=(InputManager&&) = delete;

        void Init();
        void Deinit();

        Inputs* ReadInputs(int gp = 0, bool ui = false);
        void TranslateInputs(std::shared_ptr<IConsole>& station, CONSOLE_ID console);

        void Connect(SDL_JoystickID id);
        void Disconnect(SDL_JoystickID id);

    private:
        Proteus* proteus;
        bool debug;

        const int MAX_PLAYERS = 4;
        int numPlayers = 0;
        std::array<Gamepad*, 4> gamepads = { nullptr, nullptr, nullptr, nullptr };

        void DisconnectGP0();
        void DisconnectGP1();
        void DisconnectGP2();
        void DisconnectGP3();
};