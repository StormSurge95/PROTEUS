#pragma once

#include "../FrontendPCH.h"
#include "../session/SessionTypes.h"

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

    enum LogLevel {
        NONE =  0x00,
        TRACE = 0x01,
        DEBUG = 0x02,
        INFO =  0x04,
        WARN =  0x08,
        ERROR = 0x10,

        ALL_LVLS =   0x1F
    };
    static const map<LogLevel, string> LogLevels = {
        { LogLevel::TRACE, "TRACE" },
        { LogLevel::INFO, "INFO" },
        { LogLevel::DEBUG, "DEBUG" },
        { LogLevel::WARN, "WARN" },
        { LogLevel::ERROR, "ERROR" }
    };

    enum LogCategory {
        APP_LOOP =      0x00,
        APP_INPUT =     0x01,
        SESSION =       0x02,
        ROM_LIBRARY =   0x04,
        CPU_LOG =       0x08,
        PPU_LOG =       0x10,
        APU_LOG =       0x20,
        DEBUG_TRACE =   0x40,
        LOGGING =       0x80,

        ALL_CATS =      0xFF
    };
    static const map<LogCategory, string> LogCategories = {
        { LogCategory::APP_LOOP, "APP:LOOP" },
        { LogCategory::APP_INPUT, "APP:INPUT" },
        { LogCategory::SESSION, "SESSION" },
        { LogCategory::ROM_LIBRARY, "ROM:LIBRARY" },
        { LogCategory::CPU_LOG, "LOG:CPU" },
        { LogCategory::PPU_LOG, "LOG:PPU" },
        { LogCategory::APU_LOG, "LOG:APU" },
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

    struct SessionCreateEvent {
        ConsoleID console = ConsoleID::NONE;
        u64 frame = 0xFFFFFFFFFFFFFFFF;
        ConsoleSessionState session = ConsoleSessionState::EMPTY;
        ConsoleSessionErrorCode code = ConsoleSessionErrorCode::NONE;
        string reason = "";
    };

    struct RomLoadEvent {
        ConsoleID console = ConsoleID::NONE;
        string romName = "";
        path romPath = path();
        ConsoleSessionErrorCode code = ConsoleSessionErrorCode::NONE;
        string reason = "";
        u64 frame = 0xFFFFFFFFFFFFFFFF;
        ConsoleSessionState session = ConsoleSessionState::EMPTY;
        high_resolution_clock::duration time = high_resolution_clock::duration(0);
    };

    struct InputEvent {
        string action = "";
        string source = "";
        SDL_JoystickID device = 0xFFFFFFFF;
        u8 player = 0xFF;
        ConsoleSessionState session = ConsoleSessionState::EMPTY;
        ConsoleID console = ConsoleID::NONE;
        SDL_Keycode key = 0xFFFFFFFF;
        SDL_GamepadButton button = SDL_GAMEPAD_BUTTON_SOUTH;
        u64 frame = 0xFFFFFFFFFFFFFFFF;
    };

    struct CpuStepEvent {
        // req
        string mode = "";
        u64 pc = 0;
        // opt
        u64 frame = 0;
        ConsoleSessionState session = ConsoleSessionState::EMPTY;
        ConsoleID console = ConsoleID::NONE;
        u64 opcode = 0;
        u64 cycles = 0;
        string disasm = "";
    };

    struct PpuStateEvent {
        // req
        u64 scanline = 0;
        u64 dot = 0;
        bool vblank = false;
        // opt
        u64 frame = 0;
        ConsoleSessionState session = ConsoleSessionState::EMPTY;
        ConsoleID console = ConsoleID::NONE;
        bool nmiPending = false;
        string registers = "";
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
        ROM_LOAD_REQUESTED, ROM_LOAD_SUCCEEDED, ROM_LOAD_FAILED,
        ROM_LIBRARY_REFRESH_STARTED, ROM_LIBRARY_REFRESH_COMPLETED, ROM_LIBRARY_FILE_NOT_FOUND_FALLBACK_SCAN,
        INPUT_ACTION, INPUT_GAMEPAD_CONNECTED, INPUT_GAMEPAD_DISCONNECTED,
        CPU_STEP, PPU_STATE_SNAPSHOT
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
                LogLevel::INFO, LogCategory::SESSION, { "console", "session" }, { "frame" }, "Session created successfully."
            }
        },
        { LogEventName::SESSION_CREATE_FAILED, {
                LogLevel::ERROR, LogCategory::SESSION, { "console", "error", "reason" }, { "frame", "session" }, "Session creation failed."
            }
        },
        { LogEventName::SESSION_STATE_TRANSITION, {
                LogLevel::INFO, LogCategory::SESSION, { "from_state", "to_state" }, { "frame", "console" }, "Session state changed."
            }
        },
        { LogEventName::ROM_LOAD_REQUESTED, {
                LogLevel::INFO, LogCategory::SESSION, { "console", "rom_name", "rom_path" }, { "frame", "session" }, "ROM load requested."
            }
        },
        { LogEventName::ROM_LOAD_SUCCEEDED, {
                LogLevel::INFO, LogCategory::SESSION, { "console", "rom_name", "rom_path" }, { "frame", "session", "time" }, "ROM loaded successfully."
            }
        },
        { LogEventName::ROM_LOAD_FAILED, {
                LogLevel::ERROR, LogCategory::SESSION, { "console", "rom_name", "rom_path", "error_code", "reason" }, { "frame", "session" }, "ROM load failed."
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
        { LogEventName::INPUT_ACTION, {
                LogLevel::DEBUG, LogCategory::APP_INPUT, { "action", "source" }, { "frame", "session", "console", "key", "button", "player" }, "Input action processed."
            }
        },
        { LogEventName::INPUT_GAMEPAD_CONNECTED, {
                LogLevel::INFO, LogCategory::APP_INPUT, { "device", "player" }, { "frame" }, "Gamepad connected."
            }
        },
        { LogEventName::INPUT_GAMEPAD_DISCONNECTED, {
                LogLevel::INFO, LogCategory::APP_INPUT, { "device", "player" }, { "frame" }, "Gamepad disconnected."
            }
        },
        { LogEventName::CPU_STEP, {
                LogLevel::TRACE, LogCategory::CPU_LOG, { "mode", "pc" }, { "frame", "session", "console", "opcode", "cycles", "disasm" }, "CPU step executed."
            }
        },
        { LogEventName::PPU_STATE_SNAPSHOT, {
                LogLevel::TRACE, LogCategory::PPU_LOG, { "scanline", "dot", "vblank" }, { "frame", "session", "console", "nmi_pending", "registers" }, "PPU state snapshot captured."
            }
        }
    };
};