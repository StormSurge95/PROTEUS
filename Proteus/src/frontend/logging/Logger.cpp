#include <Logger.h>
#include <AppTypes.h>
#include <iostream>

using std::cout;
using namespace NS_Proteus;

Logger::~Logger() {
    if (logFile.is_open()) logFile.close();
}

bool Logger::IsEnabled(LogLevel level, LogCategory category) {
    bool lvl = (LevelsEnabled & level) > 0;
    if (!lvl) return false;
    bool cat = (CategoriesEnabled & category) > 0;
    if (!cat) return false;

    return true;
}

void Logger::Log(LogLevel level, LogCategory category, string message, map<string, string> fields) {
    if (IsEnabled(level, category)) {
        stringstream ss;
        ss << GetTimestamp() << " ";
        ss << "[" << LogLevels.at(level) << " - ";
        ss << LogCategories.at(category) << "] ";
        ss << message << " ";
        for (const auto& [name, value] : fields) {
            ss << name << ": " << value << "  ";
        }
        ss << endl;
        if (logToFile)
            logFile << ss.str();
        if (logToConsole)
            cout << ss.str();
    }
}
void Logger::Trace(LogCategory category, string message, map<string, string> fields) {
    Log(LogLevel::TRACE, category, message, fields);
}
void Logger::Debug(LogCategory category, string message, map<string, string> fields) {
    Log(LogLevel::DEBUG, category, message, fields);
}
void Logger::Info(LogCategory category, string message, map<string, string> fields) {
    Log(LogLevel::INFO, category, message, fields);
}
void Logger::Warn(LogCategory category, string message, map<string, string> fields) {
    Log(LogLevel::WARN, category, message, fields);
}
void Logger::Error(LogCategory category, string message, map<string, string> fields) {
    Log(LogLevel::ERROR, category, message, fields);
}
void Logger::LogEvent(LogEventName event, map<string, string> fields) {
    if (!LogEventCatalogue.contains(event))
        return Log(LogLevel::ERROR, LogCategory::LOGGING, "Invalid LogEventName argument!");
    LogEventPayload payload = LogEventCatalogue.at(event);
    if (!ValidateReq(payload.required_fields, fields))
        return Log(LogLevel::ERROR, LogCategory::LOGGING, "Required fields missing!");
    ValidateOpt(payload.required_fields, payload.optional_fields, fields);
    return Log(payload.level, payload.category, payload.message_template, fields);
}

void Logger::EmitPhaseHook(FrameContext& ctx, AppPhaseName phase, AppPhaseStatus status, string reason) {
    LogEventName event;
    map<string, string> fields = {
        { "phase", AppPhaseNames.at(phase) },
        { "frame", to_string(ctx.stats.frameCount) },
        { "session", ConsoleSessionStates.at(ctx.sessionState) },
    };
    if (ctx.currentConsole != ConsoleID::NONE)
        fields["console"] = ConsoleNamesShort.at(ctx.currentConsole);
    switch (status) {
        default:
        case AppPhaseStatus::BEGIN:
            event = LogEventName::APP_LOOP_PHASE_BEGIN;
            fields["overlay"] = (ctx.state.overlayActive ? "True" : "False");
            fields["minimized"] = (ctx.state.windowMinimized ? "True" : "False");
            lastPhaseStart = high_resolution_clock::now();
            break;
        case AppPhaseStatus::END:
            event = LogEventName::APP_LOOP_PHASE_END;
            fields["time"] = GetDuration(duration_cast<milliseconds>(high_resolution_clock::now() - lastPhaseStart));
            break;
        case AppPhaseStatus::SKIPPED:
            event = LogEventName::APP_LOOP_PHASE_SKIPPED;
            fields["reason"] = reason;
            break;
    }
    LogEvent(event, fields);
}

void Logger::EmitSessionCreateEvent(LogEventName event, SessionCreateEvent data) {
    map<string, string> fields = {
        { "console", ConsoleNamesShort.at(data.console) },
        { "session", ConsoleSessionStates.at(data.session) }
    };

    if (data.frame != 0xFFFFFFFFFFFFFFFF)
        fields["frame"] = to_string(data.frame);

    if (event == LogEventName::SESSION_CREATE_FAILED) {
        fields["error"] = ConsoleSessionErrors.at(data.code);
        fields["reason"] = data.reason;
    }

    LogEvent(event, fields);
}

void Logger::EmitRomLoadEvent(LogEventName event, RomLoadEvent data) {
    map<string, string> fields = {
        { "console", ConsoleNamesShort.at(data.console) },
        { "rom_name", data.romName },
        { "rom_path", data.romPath.string() },
        { "session", ConsoleSessionStates.at(data.session) }
    };

    if (data.frame != 0xFFFFFFFFFFFFFFFF)
        fields["frame"] = to_string(data.frame);

    switch (event) {
        default:
        case LogEventName::ROM_LOAD_REQUESTED:
            break;
        case LogEventName::ROM_LOAD_SUCCEEDED:
            fields["time"] = GetDuration(data.time);
            break;
        case LogEventName::ROM_LOAD_FAILED:
            fields["error_code"] = ConsoleSessionErrors.at(data.code);
            fields["reason"] = data.reason;
            break;
    }

    LogEvent(event, fields);
}

void Logger::EmitInputEvent(LogEventName event, InputEvent data) {
    map<string, string> fields;

    fields["player"] = to_string(data.player);
    fields["frame"] = to_string(data.frame);

    if (event == LogEventName::INPUT_ACTION) {
        fields["action"] = data.action;
        fields["source"] = data.source;
        if (data.source == "keyboard")
            fields["key"] = to_string(data.key);
        else if (data.source == "gamepad")
            fields["button"] = to_string(data.button);
    } else {
        fields["device"] = to_string(data.device);
    }

    LogEvent(event, fields);
}

void Logger::EmitCpuStepEvent(CpuStepEvent data) {
    map<string, string> fields{
        { "mode", data.mode },
        { "pc", hex(data.pc, 4) },
        { "frame", to_string(data.frame) },
        { "session", ConsoleSessionStates.at(data.session) },
        { "console", ConsoleNamesShort.at(data.console) },
        { "opcode", hex(data.opcode, 2) },
        { "cycles", to_string(data.cycles) },
        { "disasm", data.disasm }
    };

    LogEvent(LogEventName::CPU_STEP, fields);
}

void Logger::EmitPpuStateEvent(PpuStateEvent data) {
    map<string, string> fields{
        { "scanline", to_string(data.scanline) },
        { "dot", to_string(data.dot) },
        { "vblank", data.vblank ? "True" : "False " },
        { "frame", to_string(data.frame) },
        { "session", ConsoleSessionStates.at(data.session) },
        { "console", ConsoleNamesShort.at(data.console) },
        { "nmi_pending", data.nmiPending ? "True" : "False" },
        { "registers", data.registers }
    };

    LogEvent(LogEventName::PPU_STATE_SNAPSHOT, fields);
}

void Logger::SetLogPath(path filepath) {
    if (logFile.is_open()) logFile.close();
    logPath = path("C:/devenv/PROTEUS");
    logPath /= filepath;
    logFile.open(logPath);
    if (!logFile.is_open()) {
        DisableFileLogging();
        return;
    } else EnableFileLogging();
}

bool Logger::ValidateReq(const vector<string>& req, const map<string, string>& fields) {
    for (const string& k : req) {
        if (!fields.contains(k)) return false;
    }
    return true;
}
void Logger::ValidateOpt(const vector<string>& req, const vector<string>& opt, map<string, string>& fields) {
    for (const auto& [key, val] : fields) {
        // key exists within "required_fields"; we can skip
        if (find(req.begin(), req.end(), key) != req.end()) continue;
        // key exists within "optional_fields"; we can skip
        if (find(opt.begin(), opt.end(), key) != opt.end()) continue;
        // key is not valid for this payload event; but it's not something worth crashing over
        Log(LogLevel::WARN, LogCategory::LOGGING, "Invalid field paramater provided; removing from processing.");
        // simply remove the invalid field so we don't polute the log(s)
        fields.erase(key);
    }
}