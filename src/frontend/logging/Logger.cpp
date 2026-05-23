#include "./Logger.hpp"
#include "../app/AppTypes.hpp"

namespace NS_Proteus {
    map<LogLevel, bool> Logger::LevelEnabled = {
        { LogLevel::TRACE, false },
        { LogLevel::INFO, false },
        { LogLevel::DEBUG, false },
        { LogLevel::WARN, true },
        { LogLevel::ERROR, true }
    };
    map<LogCategory, bool> Logger::CategoryEnabled = {
        { LogCategory::APP_LOOP, true },
        { LogCategory::APP_INPUT, false },
        { LogCategory::SESSION, false },
        { LogCategory::ROM_LIBRARY, false },
        { LogCategory::NES_CPU, false },
        { LogCategory::NES_PPU, false },
        { LogCategory::NES_APU, false },
        { LogCategory::DEBUG_TRACE, false }
    };

    Logger::~Logger() {
        if (logFile.is_open()) logFile.close();
    }

    void Logger::Log(LogLevel level, LogCategory category, string message, map<string, string> fields) {
        if (IsEnabled(level, category)) {
            stringstream ss;
            ss << GetTimestamp();
            ss << " [" << LogLevels.at(level) << " - ";
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
}