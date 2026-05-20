#include "./ConsoleSession.h"
#include "../backend/Consoles.h"
#include "./DebuggerFactory.h"
#include "./ConsoleFactory.h"

using namespace NS_Proteus;

SessionResult ConsoleSession::Failure(ConsoleSessionErrorCode code, ConsoleSessionState state, string message) {
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
    // handle invalid argument
    if (console == ConsoleID::NONE)
        return Failure(ConsoleSessionErrorCode::INVALID_ARGUMENT, currentState, "No Console Selected");

    // handle starting state; can only create a session if an active one does not already exist
    if (currentState != ConsoleSessionState::EMPTY && currentState != ConsoleSessionState::SHUTDOWN)
        return Failure(ConsoleSessionErrorCode::INVALID_TRANSITION, currentState, "Session Already Active");

    // handle unsupported consoles
    if (!ConsoleEmuStarted.at(console))
        return Failure(ConsoleSessionErrorCode::UNSUPPORTED_CONSOLE, currentState, "Console Selection Currently Unsupported");

    // create console and handle any failures
    station = ConsoleFactory::Create(console);
    if (!station)
        return Failure(ConsoleSessionErrorCode::CONSOLE_CREATE_FAILED, ConsoleSessionState::ERROR, "Console Creation Failed");

    // create debugger and handle any failures
    debugger = DebuggerFactory::Create(console, station);
    if (!debugger) {
        station.reset();
        return Failure(ConsoleSessionErrorCode::DEBUGGER_CREATE_FAILED, ConsoleSessionState::ERROR, "Debugger Creation Failed");
    }

    // session creation successful
    return Success(ConsoleSessionState::CREATED, "Session Created");
}

/**
 * @brief Loads a particular ROM into the console.
 * @param romPath The path to the ROM file to be loaded.
 * @param romName [optional] The display name (game title) of the ROM to be loaded.
 * @return `SessionResult` object with the result of the attempted ROM load.
 */
SessionResult ConsoleSession::LoadROM(path romPath, string romName = "") {
    if (romPath == path())
        return Failure(ConsoleSessionErrorCode::INVALID_ARGUMENT, currentState, "ROM Path is Empty");

    if (!station)
        return Failure(ConsoleSessionErrorCode::NO_ACTIVE_CONSOLE, ConsoleSessionState::ERROR, "No Active Console Session");

    if (currentState != ConsoleSessionState::CREATED)
        return Failure(ConsoleSessionErrorCode::INVALID_TRANSITION, currentState, "Invalid SessionState for LoadROM()");

    bool success = station->loadROM(romPath.string());

    if (!success)
        return Failure(ConsoleSessionErrorCode::ROM_LOAD_FAILED, currentState, "Console Failed to Load ROM");

    loadedRomPath = romPath;
    loadedRomName = romName;
    return Success(ConsoleSessionState::ROM_LOADED, "ROM Loaded Successfully");
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
    station.reset();
    station = nullptr;
    debugger.reset();
    debugger = nullptr;
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