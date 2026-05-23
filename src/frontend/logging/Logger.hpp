#pragma once

#include "../FrontendPCH.hpp"
#include "./LoggingTypes.hpp"
#include "../app/AppTypes.hpp"
#include <iostream>

using std::cout;

namespace NS_Proteus {
    class Logger {
        protected:
            bool logToConsole = true;
            bool logToFile = false;
            path logPath;
            ofstream logFile;

            high_resolution_clock::time_point lastPhaseStart;

            static map<LogLevel, bool> LevelEnabled;
            static map<LogCategory, bool> CategoryEnabled;
        public:
            ~Logger();

            void Log(LogLevel level, LogCategory category, string message, map<string, string> fields = {});
            void LogEvent(LogEventName event, map<string, string> fields);
            void Trace(LogCategory category, string message, map<string, string> fields = {});
            void Debug(LogCategory category, string message, map<string, string> fields = {});
            void Info(LogCategory category, string message, map<string, string> fields = {});
            void Warn(LogCategory category, string message, map<string, string> fields = {});
            void Error(LogCategory category, string message, map<string, string> fields = {});
            inline bool IsEnabled(LogLevel level, LogCategory category) { return LevelEnabled[level] && CategoryEnabled[category]; }

            void EmitPhaseHook(FrameContext& ctx, AppPhaseName phase, AppPhaseStatus status, string reason = "");

            void SetLogPath(path filepath);
            inline void EnableFileLogging() { logToFile = true; }
            inline void DisableFileLogging() { logToFile = false; }
            inline void EnableConsoleLogging() { logToConsole = true; }
            inline void DisableConsoleLogging() { logToConsole = false; }

            bool ValidateReq(const vector<string>& req, const map<string, string>& fields);
            void ValidateOpt(const vector<string>& req, const vector<string>& opt, map<string, string>& fields);
    };
}