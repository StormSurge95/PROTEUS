#pragma once

#include <FrontendPCH.h>
#include <LoggingTypes.h>
#include <AppTypes.h>

namespace NS_Proteus {
    class Logger {
        protected:
            bool logToConsole = true;
            bool logToFile = false;
            path logPath;
            ofstream logFile;

            high_resolution_clock::time_point lastPhaseStart;

            inline static u16 LevelsEnabled = LogLevel::ALL_LVLS;
            inline static u16 CategoriesEnabled = LogCategory::SESSION | LogCategory::LOGGING;
        public:
            ~Logger();

            void Log(LogLevel level, LogCategory category, string message, map<string, string> fields = {});
            void LogEvent(LogEventName event, map<string, string> fields);
            void Trace(LogCategory category, string message, map<string, string> fields = {});
            void Debug(LogCategory category, string message, map<string, string> fields = {});
            void Info(LogCategory category, string message, map<string, string> fields = {});
            void Warn(LogCategory category, string message, map<string, string> fields = {});
            void Error(LogCategory category, string message, map<string, string> fields = {});
            bool IsEnabled(LogLevel level, LogCategory category);

            void EmitPhaseHook(FrameContext& ctx, AppPhaseName phase, AppPhaseStatus status, string reason = "");
            void EmitSessionCreateEvent(LogEventName event, SessionCreateEvent data);
            void EmitRomLoadEvent(LogEventName event, RomLoadEvent data);
            void EmitInputEvent(LogEventName event, InputEvent data);
            void EmitCpuStepEvent(CpuStepEvent data);
            void EmitPpuStateEvent(PpuStateEvent data);

            void SetLogPath(path filepath);
            inline void EnableFileLogging() { logToFile = true; }
            inline void DisableFileLogging() { logToFile = false; }
            inline void EnableConsoleLogging() { logToConsole = true; }
            inline void DisableConsoleLogging() { logToConsole = false; }

            bool ValidateReq(const vector<string>& req, const map<string, string>& fields);
            void ValidateOpt(const vector<string>& req, const vector<string>& opt, map<string, string>& fields);
    };
}