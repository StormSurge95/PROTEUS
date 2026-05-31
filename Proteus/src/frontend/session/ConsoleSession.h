#pragma once

#include "../FrontendPCH.h"
#include "./SessionTypes.h"
#include "../logging/Logger.h"
#include "../../shared/IConsole.h"
#include "../../shared/IDebugger.h"

namespace NS_Proteus {
    class Logger;

    class ConsoleSession {
        private:
            ConsoleID currentConsole = ConsoleID::NONE;
            ConsoleSessionState currentState = ConsoleSessionState::EMPTY;
            sptr<IConsole> station = nullptr;
            sptr<IDebugger> debugger = nullptr;
            Logger* logger = nullptr;
            ConsoleSessionErrorCode lastErrorCode = ConsoleSessionErrorCode::NONE;
            string lastError;

            high_resolution_clock::time_point runtimeStartedAt;
            high_resolution_clock::time_point loadRomStart;

            u64 resumeCount = 0;

            string loadedRomName = "";
            path loadedRomPath;

            SessionResult Failure(ConsoleSessionErrorCode code, ConsoleSessionState state, string message);
            SessionResult Success(ConsoleSessionState state, string message);
        public:
            ConsoleSession(Logger* l) : logger(l) {}
            ~ConsoleSession() = default;

            SessionResult CreateSession(ConsoleID console);
            SessionResult LoadROM(path romPath, string romName = "");
            SessionResult Start();
            SessionResult Pause();
            SessionResult Reset();
            SessionResult Shutdown();
            SessionResult PlayPause();

            bool IsActive() const;
            const ConsoleID CurrentConsoleID() const { return currentConsole; }
            const ConsoleSessionState GetState() const { return currentState; }
            const sptr<IConsole>& GetConsole() const { return station; }
            const sptr<IDebugger>& GetDebugger() const { return debugger; }
            const string GetError() const { return lastError; }
    };
}