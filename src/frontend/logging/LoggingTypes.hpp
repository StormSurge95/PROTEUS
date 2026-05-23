#pragma once

#include "../Types.hpp"

namespace NS_Proteus {
    enum class AppPhaseName { INPUT, EMUDEB, RENDER, AUDIO, THROTTLE };
    static const map<AppPhaseName, string> AppPhaseNames = {
        { AppPhaseName::INPUT, "INPUT" },
        { AppPhaseName::EMUDEB, "EMULATION/DEBUGGING" },
        { AppPhaseName::RENDER, "RENDER" },
        { AppPhaseName::AUDIO, "AUDIO" },
        { AppPhaseName::THROTTLE, "THROTTLE" }
    };
    enum class AppPhaseStatus { BEGIN, END, SKIPPED };
    static const map<AppPhaseStatus, string> AppPhaseStatuses = {
        { AppPhaseStatus::BEGIN, "BEGIN" },
        { AppPhaseStatus::END, "END" },
        { AppPhaseStatus::SKIPPED, "SKIPPED" }
    };

    enum class LogLevel {
        TRACE, DEBUG, INFO, WARN, ERROR
    };
    static const map<LogLevel, string> LogLevels = {
        { LogLevel::TRACE, "TRACE" },
        { LogLevel::INFO, "INFO" },
        { LogLevel::DEBUG, "DEBUG" },
        { LogLevel::WARN, "WARN" },
        { LogLevel::ERROR, "ERROR" }
    };

    enum class LogCategory {
        APP_LOOP, APP_INPUT,
        SESSION,
        ROM_LIBRARY,
        NES_CPU, NES_PPU, NES_APU,
        DEBUG_TRACE,
        LOGGING
    };
    static const map<LogCategory, string> LogCategories = {
        { LogCategory::APP_LOOP, "APP:LOOP" },
        { LogCategory::APP_INPUT, "APP:INPUT" },
        { LogCategory::SESSION, "SESSION" },
        { LogCategory::ROM_LIBRARY, "ROM:LIBRARY" },
        { LogCategory::NES_CPU, "NES:CPU" },
        { LogCategory::NES_PPU, "NES:PPU" },
        { LogCategory::NES_APU, "NES:APU" },
        { LogCategory::DEBUG_TRACE, "DEBUG:TRACE" }
    };

    struct LogRecord {
        u64 timestamp;
        LogLevel level;
        LogCategory category;
        string message;
        map<string, string> fields;
        // threadID? 
    };

    struct LogEventPayload {
        LogLevel level;
        LogCategory category;
        vector<string> required_fields;
        vector<string> optional_fields;
        string message_template;
    };

    enum class LogEventName {
        APP_LOOP_PHASE_BEGIN, APP_LOOP_PHASE_END, APP_LOOP_PHASE_SKIPPED,
        SESSION_CREATE_REQUESTED, SESSION_CREATE_SUCCEEDED, SESSION_CREATE_FAILED, SESSION_STATE_TRANSITION,
        ROM_LIBRARY_REFRESH_STARTED, ROM_LIBRARY_REFRESH_COMPLETED, ROM_LIBRARY_FILE_NOT_FOUND_FALLBACK_SCAN,
        APP_INPUT_GAMEPAD_CONNECTED, APP_INPUT_GAMEPAD_DISCONNECTED
    };

    const map<LogEventName, LogEventPayload> LogEventCatalogue = {
        { LogEventName::APP_LOOP_PHASE_BEGIN, {
                LogLevel::TRACE, LogCategory::APP_LOOP, { "phase", "frame", "session", "overlay", "minimized" }, { "console" }, "Phase began."
            }
        },
        { LogEventName::APP_LOOP_PHASE_END, {
                LogLevel::TRACE, LogCategory::APP_LOOP, { "phase", "frame", "session", "time" }, { "console" }, "Phase ended."
            }
        },
        { LogEventName::APP_LOOP_PHASE_SKIPPED, {
                LogLevel::DEBUG, LogCategory::APP_LOOP, { "phase", "frame", "session", "reason" }, { "console" }, "Phase skipped."
            }
        },
        { LogEventName::SESSION_CREATE_REQUESTED, {
                LogLevel::INFO, LogCategory::SESSION, { "console" }, { "frame", "session" }, "Session creation requested."
            }
        },
        { LogEventName::SESSION_CREATE_SUCCEEDED, {
                LogLevel::INFO, LogCategory::SESSION, { "console", "post_state" }, { "frame" }, "Session created successfully."
            }
        },
        { LogEventName::SESSION_CREATE_FAILED, {
                LogLevel::ERROR, LogCategory::SESSION, { "console", "error_code", "reason" }, { "frame", "post_state" }, "Session creation failed."
            }
        },
        { LogEventName::SESSION_STATE_TRANSITION, {
                LogLevel::INFO, LogCategory::SESSION, { "from_state", "to_state" }, { "frame", "console" }, "Session state changed."
            }
        },
        { LogEventName::ROM_LIBRARY_REFRESH_STARTED, {
                LogLevel::INFO, LogCategory::ROM_LIBRARY, { "console" }, { "frame" }, "ROM library refresh started."
            }
        },
        { LogEventName::ROM_LIBRARY_REFRESH_COMPLETED, {
                LogLevel::INFO, LogCategory::ROM_LIBRARY, { "console", "rom_count", "time" }, {"frame"}, "ROM library refresh completed."
            }
        },
        { LogEventName::ROM_LIBRARY_FILE_NOT_FOUND_FALLBACK_SCAN, {
                LogLevel::WARN, LogCategory::ROM_LIBRARY, { "path" }, { "console", "frame" }, "ROM library file not found; scanning files directly."
            }
        },
        { LogEventName::APP_INPUT_GAMEPAD_CONNECTED, {
                LogLevel::INFO, LogCategory::APP_INPUT, { "device_id", "player_index" }, { "frame" }, "Gamepad connected."
            }
        },
        { LogEventName::APP_INPUT_GAMEPAD_DISCONNECTED, {
                LogLevel::INFO, LogCategory::APP_INPUT, { "device_id", "player_index" }, { "frame" }, "Gamepad disconnected."
            }
        }
    };
}