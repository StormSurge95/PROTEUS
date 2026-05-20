#include "./ConsoleSession.h"
#include "../backend/Consoles.h"
#include "./DebuggerFactory.h"
#include "./ConsoleFactory.h"

using namespace NS_Proteus;

SessionResult ConsoleSession::Failure(SessionErrorCode code, SessionState state, string message) {
    lastError = message;
    currentState = state;
    return { false, currentState, code, message };
}

SessionResult ConsoleSession::Success(SessionState state, string message) {
    currentState = state;
    return { true, currentState, SessionErrorCode::NONE, message };
}

/**
 * @brief Creates the necessary objects for running a particular console within the application.
 * @param console The `ConsoleID` to start a session with.
 * @return `SessionResult` object with the result of the session creation 
 */
SessionResult ConsoleSession::CreateSession(ConsoleID console) {
    // handle invalid argument
    if (console == ConsoleID::NONE)
        return Failure(SessionErrorCode::INVALID_ARGUMENT, currentState, "No Console Selected");

    // handle starting state; can only create a session if an active one does not already exist
    if (currentState != SessionState::EMPTY && currentState != SessionState::SHUTDOWN)
        return Failure(SessionErrorCode::INVALID_TRANSITION, currentState, "Session Already Active");

    // handle unsupported consoles
    if (!ConsoleEmuStarted.at(console))
        return Failure(SessionErrorCode::UNSUPPORTED_CONSOLE, currentState, "Console Selection Currently Unsupported");

    // create console and handle any failures
    station = ConsoleFactory::Create(console);
    if (!station)
        return Failure(SessionErrorCode::CONSOLE_CREATE_FAILED, SessionState::ERROR, "Console Creation Failed");

    // create debugger and handle any failures
    debugger = DebuggerFactory::Create(console, station);
    if (!debugger) {
        station.reset();
        return Failure(SessionErrorCode::DEBUGGER_CREATE_FAILED, SessionState::ERROR, "Debugger Creation Failed");
    }

    // session creation successful
    return Success(SessionState::CREATED, "Session Created");
}

/**
 * @brief Loads a particular ROM into the console.
 * @param romPath The path to the ROM file to be loaded.
 * @param romName [optional] The display name (game title) of the ROM to be loaded.
 * @return `SessionResult` object with the result of the attempted ROM load.
 */
SessionResult ConsoleSession::LoadROM(path romPath, string romName = "") {
    if (romPath == path())
        return Failure(SessionErrorCode::INVALID_ARGUMENT, currentState, "ROM Path is Empty");

    if (!station)
        return Failure(SessionErrorCode::NO_ACTIVE_CONSOLE, SessionState::ERROR, "No Active Console Session");

    if (currentState != SessionState::CREATED)
        return Failure(SessionErrorCode::INVALID_TRANSITION, currentState, "Invalid SessionState for LoadROM()");

    bool success = station->loadROM(romPath.string());

    if (!success)
        return Failure(SessionErrorCode::ROM_LOAD_FAILED, currentState, "Console Failed to Load ROM");

    loadedRomPath = romPath;
    loadedRomName = romName;
    return Success(SessionState::ROM_LOADED, "ROM Loaded Successfully");
}

/**
 * @brief 
 * @return 
 */
SessionResult ConsoleSession::Start() {
    if (!station)
        return Failure(SessionErrorCode::NO_ACTIVE_CONSOLE, SessionState::ERROR, "No Console Available");

    switch (currentState) {
        case SessionState::ROM_LOADED:
            // begin runtime exec
            runtimeStartedAt = high_resolution_clock::now();
            return Success(SessionState::RUNNING, "Session Started");
        case SessionState::PAUSED:
            // resume runtime exec
            resumeCount++;
            return Success(SessionState::RUNNING, "Session Resumed");
        case SessionState::RUNNING:
            // can't fail to run if we're already running
            return Success(SessionState::RUNNING, "Session Already Running");
        default:
            // all other states simply fail; but we don't want to overwrite the error message cuz it might explain this failure
            return Failure(SessionErrorCode::INVALID_TRANSITION, currentState, lastError);
    }
}

SessionResult ConsoleSession::Pause() {
    switch (currentState) {
        case SessionState::PAUSED:
            // can't fail to pause if we're already paused
            return Success(currentState, "Session Already Paused");
        case SessionState::RUNNING:
            // pause currently running session
            return Success(SessionState::PAUSED, "Session Successfully Paused");
        default:
            // all other states simply fail; but we don't want to overwrite the error message cuz it might explain this failure
            return Failure(SessionErrorCode::INVALID_TRANSITION, currentState, lastError);
    }
}

SessionResult ConsoleSession::Reset() {
    switch (currentState) {
        case SessionState::RUNNING:
        case SessionState::PAUSED:
        case SessionState::ROM_LOADED:
            station->reset();
            return Success(SessionState::ROM_LOADED, "ROM Reset Successfully");
        default:
            return Failure(SessionErrorCode::INVALID_TRANSITION, currentState, lastError);
    }
}

SessionResult ConsoleSession::Shutdown() {
    currentConsole = ConsoleID::NONE;
    station.reset();
    station = nullptr;
    debugger.reset();
    debugger = nullptr;
    resumeCount = 0;
    loadedRomName = "";
    loadedRomPath = path();
    return Success(SessionState::SHUTDOWN, "Session Shutdown Successful");
}

bool ConsoleSession::IsActive() const {
    switch (currentState) {
        case SessionState::ERROR:
        case SessionState::SHUTDOWN:
        case SessionState::CREATED:
            return false;
        default:
            return true;
    }
}