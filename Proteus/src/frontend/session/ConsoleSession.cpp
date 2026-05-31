#include <ConsoleSession.h>
#include <DebuggerFactory.h>
#include <ConsoleFactory.h>

using namespace NS_Proteus;

SessionResult ConsoleSession::Failure(ConsoleSessionErrorCode code, ConsoleSessionState state, string message) {
    lastErrorCode = code;
    lastError = message;
    currentState = state;
    return { false, currentState, code, message };
}

SessionResult ConsoleSession::Success(ConsoleSessionState state, string message) {
    currentState = state;
    return { true, currentState, ConsoleSessionErrorCode::NONE, message };
}

/**
 * @brief Creates the necessary objects for running a particular console within the application.
 * @param console The `ConsoleID` to start a session with.
 * @return `SessionResult` object with the result of the session creation 
 */
SessionResult ConsoleSession::CreateSession(ConsoleID console) {
    logger->EmitSessionCreateEvent(LogEventName::SESSION_CREATE_REQUESTED,
        { .console = console, .session = currentState }
    );
    
    SessionResult result;

    if (console == ConsoleID::NONE) {
        // handle invalid argument
        result = Failure(ConsoleSessionErrorCode::INVALID_ARGUMENT, currentState, "No Console Selected");
    } else if (currentState != ConsoleSessionState::EMPTY && currentState != ConsoleSessionState::SHUTDOWN) {
        // handle starting state; can only create a session if an active one does not already exist
        result = Failure(ConsoleSessionErrorCode::INVALID_TRANSITION, currentState, "Session Already Active");
    } else if (!PluginManager::IsConsoleAvailable(console)) {
        // handle unsupported consoles
        result = Failure(ConsoleSessionErrorCode::UNSUPPORTED_CONSOLE, currentState, "Console Selection Currently Unsupported");
    } else {
        // try to create console
        station = ConsoleFactory::Create(console);
        if (!station) {
            // handle console creation failure
            result = Failure(ConsoleSessionErrorCode::CONSOLE_CREATE_FAILED, ConsoleSessionState::ERROR, "Console Creation Failed");
        } else {
            // try to create debugger
            debugger = DebuggerFactory::Create(console, station);
            if (!debugger) {
                // handle debugger creation failure
                station.reset();
                result = Failure(ConsoleSessionErrorCode::DEBUGGER_CREATE_FAILED, ConsoleSessionState::ERROR, "Debugger Creation Failed");
            } else {
                // all steps successful; apply `currentConsole`
                currentConsole = console;
                // session creation successful
                result = Success(ConsoleSessionState::CREATED, "Session Created");
            }
        }
    }

    if (result.success) {
        logger->EmitSessionCreateEvent(LogEventName::SESSION_CREATE_SUCCEEDED,
            { .console = console, .session = currentState }
        );
    } else {
        logger->EmitSessionCreateEvent(LogEventName::SESSION_CREATE_FAILED,
            { .console = console, .session = currentState, .code = lastErrorCode, .reason = lastError }
        );
    }

    return result;
}

/**
 * @brief Loads a particular ROM into the console.
 * @param romPath The path to the ROM file to be loaded.
 * @param romName [optional] The display name (game title) of the ROM to be loaded.
 * @return `SessionResult` object with the result of the attempted ROM load.
 */
SessionResult ConsoleSession::LoadROM(path p, string n) {
    logger->EmitRomLoadEvent(LogEventName::ROM_LOAD_REQUESTED,
        { .console = currentConsole, .romName = n, .romPath = p, .session = currentState }
    );
    loadRomStart = high_resolution_clock::now();

    SessionResult result;

    if (p == path()) {
        result = Failure(ConsoleSessionErrorCode::INVALID_ARGUMENT, currentState, "ROM Path is Empty");
    } else if (!station) {
        result = Failure(ConsoleSessionErrorCode::NO_ACTIVE_CONSOLE, ConsoleSessionState::ERROR, "No Active Console Session");
    } else if (currentState != ConsoleSessionState::CREATED) {
        result = Failure(ConsoleSessionErrorCode::INVALID_TRANSITION, currentState, "Invalid SessionState for LoadROM()");
    } else {
        bool success = station->loadROM(p.string());

        if (!success) {
            result = Failure(ConsoleSessionErrorCode::ROM_LOAD_FAILED, currentState, "Console Failed to Load ROM");
        } else {
            result = Success(ConsoleSessionState::ROM_LOADED, "ROM Loaded Successfully");
        }
    }

    if (result.success) {
        high_resolution_clock::duration d(high_resolution_clock::now() - loadRomStart);
        logger->EmitRomLoadEvent(LogEventName::ROM_LOAD_SUCCEEDED,
            { .console = currentConsole, .romName = n, .romPath = p, .session = currentState, .time = d });
        loadedRomPath = p;
        loadedRomName = n;
    } else {
        logger->EmitRomLoadEvent(LogEventName::ROM_LOAD_FAILED,
            { .console = currentConsole, .romName = n, .romPath = p, .code = lastErrorCode, .reason = lastError, .session = currentState });
    }

    return result;
}

/**
 * @brief 
 * @return 
 */
SessionResult ConsoleSession::Start() {
    if (!station)
        return Failure(ConsoleSessionErrorCode::NO_ACTIVE_CONSOLE, ConsoleSessionState::ERROR, "No Console Available");

    switch (currentState) {
        case ConsoleSessionState::ROM_LOADED:
            // begin runtime exec
            runtimeStartedAt = high_resolution_clock::now();
            return Success(ConsoleSessionState::RUNNING, "Session Started");
        case ConsoleSessionState::PAUSED:
            // resume runtime exec
            resumeCount++;
            return Success(ConsoleSessionState::RUNNING, "Session Resumed");
        case ConsoleSessionState::RUNNING:
            // can't fail to run if we're already running
            return Success(ConsoleSessionState::RUNNING, "Session Already Running");
        default:
            // all other states simply fail; but we don't want to overwrite the error message cuz it might explain this failure
            return Failure(ConsoleSessionErrorCode::INVALID_TRANSITION, currentState, lastError);
    }
}

SessionResult ConsoleSession::Pause() {
    switch (currentState) {
        case ConsoleSessionState::PAUSED:
            // can't fail to pause if we're already paused
            return Success(currentState, "Session Already Paused");
        case ConsoleSessionState::RUNNING:
            // pause currently running session
            return Success(ConsoleSessionState::PAUSED, "Session Successfully Paused");
        default:
            // all other states simply fail; but we don't want to overwrite the error message cuz it might explain this failure
            return Failure(ConsoleSessionErrorCode::INVALID_TRANSITION, currentState, lastError);
    }
}

SessionResult ConsoleSession::Reset() {
    switch (currentState) {
        case ConsoleSessionState::RUNNING:
        case ConsoleSessionState::PAUSED:
        case ConsoleSessionState::ROM_LOADED:
            station->reset();
            return Success(ConsoleSessionState::ROM_LOADED, "ROM Reset Successfully");
        default:
            return Failure(ConsoleSessionErrorCode::INVALID_TRANSITION, currentState, lastError);
    }
}

SessionResult ConsoleSession::Shutdown() {
    currentConsole = ConsoleID::NONE;
    debugger.reset();
    debugger = nullptr;
    station.reset();
    station = nullptr;
    resumeCount = 0;
    loadedRomName = "";
    loadedRomPath = path();
    return Success(ConsoleSessionState::SHUTDOWN, "Session Shutdown Successful");
}

SessionResult ConsoleSession::PlayPause() {
    if (currentState == ConsoleSessionState::PAUSED)
        return Start();
    else
        return Pause();
}

bool ConsoleSession::IsActive() const {
    switch (currentState) {
        case ConsoleSessionState::ERROR:
        case ConsoleSessionState::SHUTDOWN:
        case ConsoleSessionState::CREATED:
            return false;
        default:
            return true;
    }
}