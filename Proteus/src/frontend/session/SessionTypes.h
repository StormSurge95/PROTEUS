#pragma once

#include <FrontendPCH.h>

namespace NS_Proteus {
    enum class ConsoleSessionState {
        EMPTY,
        CREATED,
        ROM_LOADED,
        RUNNING,
        PAUSED,
        ERROR,
        SHUTDOWN
    };
    static const map<ConsoleSessionState, string> ConsoleSessionStates = {
        { ConsoleSessionState::EMPTY, "EMPTY" },
        { ConsoleSessionState::CREATED, "CREATED" },
        { ConsoleSessionState::ROM_LOADED, "ROM LOADED" },
        { ConsoleSessionState::RUNNING, "RUNNING" },
        { ConsoleSessionState::PAUSED, "PAUSED" },
        { ConsoleSessionState::ERROR, "ERROR" },
        { ConsoleSessionState::SHUTDOWN, "SHUTDOWN" }
    };

    enum class ConsoleSessionErrorCode {
        NONE,
        INVALID_ARGUMENT,
        UNSUPPORTED_CONSOLE,
        CONSOLE_CREATE_FAILED,
        DEBUGGER_CREATE_FAILED,
        INVALID_TRANSITION,
        NO_ACTIVE_CONSOLE,
        ROM_LOAD_FAILED,
        INTERNAL_ERROR
    };
    static const map<ConsoleSessionErrorCode, string> ConsoleSessionErrors = {
        { ConsoleSessionErrorCode::NONE, "NONE" },
        { ConsoleSessionErrorCode::INVALID_ARGUMENT, "INVALID ARGUMENT" },
        { ConsoleSessionErrorCode::UNSUPPORTED_CONSOLE, "UNSUPPORTED CONSOLE" },
        { ConsoleSessionErrorCode::CONSOLE_CREATE_FAILED, "CONSOLE CREATION FAILED" },
        { ConsoleSessionErrorCode::DEBUGGER_CREATE_FAILED, "DEBUGGER CREATION FAILED" },
        { ConsoleSessionErrorCode::INVALID_TRANSITION, "INVALID STATE TRANSITION" },
        { ConsoleSessionErrorCode::NO_ACTIVE_CONSOLE, "NO CONSOLE SESSION ACTIVE" },
        { ConsoleSessionErrorCode::ROM_LOAD_FAILED, "ROM LOAD FAILED" },
        { ConsoleSessionErrorCode::INTERNAL_ERROR, "INTERNAL ERROR" },
    };

    struct SessionResult {
        bool success;
        ConsoleSessionState postState;
        ConsoleSessionErrorCode code;
        string message;
    };

    inline const SessionResult INVALID_ARGUMENT_SESSION_RESULT {
        .success = false,
        .postState = ConsoleSessionState::ERROR,
        .code = ConsoleSessionErrorCode::INVALID_ARGUMENT,
        .message = ConsoleSessionErrors.at(ConsoleSessionErrorCode::INVALID_ARGUMENT)
    };

    inline const SessionResult UNSUPPORTED_CONSOLE_SESSION_RESULT {
        .success = false,
        .postState = ConsoleSessionState::ERROR,
        .code = ConsoleSessionErrorCode::UNSUPPORTED_CONSOLE,
        .message = ConsoleSessionErrors.at(ConsoleSessionErrorCode::UNSUPPORTED_CONSOLE)
    };

    inline const SessionResult CONSOLE_CREATE_FAILED_SESSION_RESULT {
        .success = false,
        .postState = ConsoleSessionState::ERROR,
        .code = ConsoleSessionErrorCode::CONSOLE_CREATE_FAILED,
        .message = ConsoleSessionErrors.at(ConsoleSessionErrorCode::CONSOLE_CREATE_FAILED)
    };

    inline const SessionResult DEBUGGER_CREATE_FAILED_SESSION_RESULT {
        .success = false,
        .postState = ConsoleSessionState::ERROR,
        .code = ConsoleSessionErrorCode::DEBUGGER_CREATE_FAILED,
        .message = ConsoleSessionErrors.at(ConsoleSessionErrorCode::DEBUGGER_CREATE_FAILED)
    };

    inline const SessionResult INVALID_TRANSITION_SESSION_RESULT {
        .success = false,
        .postState = ConsoleSessionState::ERROR,
        .code = ConsoleSessionErrorCode::INVALID_TRANSITION,
        .message = ConsoleSessionErrors.at(ConsoleSessionErrorCode::INVALID_TRANSITION)
    };

    inline const SessionResult NO_ACTIVE_CONSOLE_SESSION_RESULT {
        .success = false,
        .postState = ConsoleSessionState::ERROR,
        .code = ConsoleSessionErrorCode::NO_ACTIVE_CONSOLE,
        .message = ConsoleSessionErrors.at(ConsoleSessionErrorCode::NO_ACTIVE_CONSOLE)
    };

    inline const SessionResult ROM_LOAD_FAILED_SESSION_RESULT {
        .success = false,
        .postState = ConsoleSessionState::ERROR,
        .code = ConsoleSessionErrorCode::ROM_LOAD_FAILED,
        .message = ConsoleSessionErrors.at(ConsoleSessionErrorCode::ROM_LOAD_FAILED)
    };

    inline const SessionResult INTERNAL_ERROR_SESSION_RESULT {
        .success = false,
        .postState = ConsoleSessionState::ERROR,
        .code = ConsoleSessionErrorCode::INTERNAL_ERROR,
        .message = ConsoleSessionErrors.at(ConsoleSessionErrorCode::INTERNAL_ERROR)
    };
}