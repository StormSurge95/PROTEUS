#pragma once

#include "../FrontendPCH.h"

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

    const map<ConsoleID, bool> ConsoleEmuStarted = {
        { ConsoleID::NES, true },
        { ConsoleID::SNS, false },
        { ConsoleID::PS1, false },
        { ConsoleID::N64, false },
        { ConsoleID::PS2, false },
        { ConsoleID::GBC, false },
        { ConsoleID::GBA, false },
        { ConsoleID::NGC, false },
        { ConsoleID::NDS, false },
        { ConsoleID::XBX, false },
        { ConsoleID::XB3, false },
        { ConsoleID::PS3, false },
        { ConsoleID::WII, false }
    };
}