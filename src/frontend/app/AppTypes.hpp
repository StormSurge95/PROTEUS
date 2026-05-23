#pragma once

#include "../FrontendPCH.hpp"
#include "../session/SessionTypes.hpp"

namespace NS_Proteus {
    struct FrameState {
        bool windowMinimized = false;
        bool inGameView = false;
        bool overlayActive = false;
        bool sessionRunning = false;
        bool sessionPaused = false;
        bool suppressInput = false;

        bool runGameplay = false;
        bool runRender = false;
        bool runAudio = false;
        bool throttleFrame = false;
    };

    struct AppState {
        AppView currentView = AppView::CONSOLE_SELECT;
        ConsoleID selectedConsole = ConsoleID::NONE;
        string selectedGame = "";
        bool isLoading = false;
    };

    struct RuntimeStats {
        u64 frameCount = 0;
        double fps = 0.0;
        RingBuffer<double, 1000> frameTimes = {};
    };

    struct TickStats {
        high_resolution_clock::time_point frameStart;
        high_resolution_clock::time_point frameEnd;
        duration<double, milli> frameDuration;

        u32 eventsPolled;
        u64 frameCount;
    };

    struct FrameContext {
        ConsoleSessionState sessionState;
        ConsoleID currentConsole;
        FrameState state;
        TickStats stats;
        SDL_Event event;

        bool suppressInput = false;
        bool quitRequested = false;
        bool overlayToggledOff = false;
    };

    static SDL_EventType WINDOW_EVENTS[] = { SDL_EVENT_WINDOW_CLOSE_REQUESTED, SDL_EVENT_WINDOW_RESIZED };
    static SDL_EventType GAMEPAD_EVENTS[] = { SDL_EVENT_GAMEPAD_ADDED, SDL_EVENT_GAMEPAD_REMOVED, SDL_EVENT_GAMEPAD_BUTTON_DOWN };
}