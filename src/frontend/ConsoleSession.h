#pragma once

#include "./FrontendPCH.h"
#include "../backend/shared/IConsole.h"
#include "../backend/shared/IDebugger.h"

namespace NS_Proteus {
    class ConsoleSession {
        private:
            ConsoleID currentConsole = ConsoleID::NONE;
            SessionState currentState = SessionState::EMPTY;
            sptr<IConsole> station = nullptr;
            sptr<IDebugger> debugger = nullptr;
            string lastError;

            high_resolution_clock::time_point runtimeStartedAt;

            u64 resumeCount = 0;

            string loadedRomName = "";
            path loadedRomPath;

            SessionResult Failure(SessionErrorCode code, SessionState state, string message);
            SessionResult Success(SessionState state, string message);
        public:
            ConsoleSession() = default;
            ~ConsoleSession() = default;

            SessionResult CreateSession(ConsoleID console);
            SessionResult LoadROM(path romPath, string romName);
            SessionResult Start();
            SessionResult Pause();
            SessionResult Reset();
            SessionResult Shutdown();

            bool IsActive() const;
            const ConsoleID CurrentConsoleID() const { return currentConsole; }
            const SessionState GetState() const { return currentState; }
            const sptr<IConsole>& GetConsole() const { return station; }
            const sptr<IDebugger>& GetDebugger() const { return debugger; }
            const string GetError() const { return lastError; }
    };
}