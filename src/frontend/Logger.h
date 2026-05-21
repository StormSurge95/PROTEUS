#pragma once

#include "./FrontendPCH.h"

namespace NS_Proteus {
    class ILogger {
        private:
            map<LogLevel, bool> LevelEnabled = {
                { LogLevel::TRACE, false },
                { LogLevel::INFO, false },
                { LogLevel::DEBUG, false },
                { LogLevel::WARN, true },
                { LogLevel::ERROR, true }
            };
            map<LogCategory, bool> CategoryEnabled = {
                { LogCategory::APP_LOOP, false },
                { LogCategory::APP_INPUT, false },
                { LogCategory::SESSION, false },
                { LogCategory::ROM_LIBRARY, false },
                { LogCategory::NES_CPU, false },
                { LogCategory::NES_PPU, false },
                { LogCategory::NES_APU, false },
                { LogCategory::DEBUG_TRACE, false }
            };
        public:
            virtual void Log(LogLevel level, LogCategory category, string message, map<string, string> fields = {}) {
                if (IsEnabled(level, category)) {
                    // do logging
                }
            };
            void Trace(LogCategory category, string message, map<string, string> fields = {}) {
                Log(LogLevel::TRACE, category, message, fields);
            }
            void Debug(LogCategory category, string message, map<string, string> fields = {}) {
                Log(LogLevel::DEBUG, category, message, fields);
            }
            void Info(LogCategory category, string message, map<string, string> fields = {}) {
                Log(LogLevel::INFO, category, message, fields);
            }
            void Warn(LogCategory category, string message, map<string, string> fields = {}) {
                Log(LogLevel::WARN, category, message, fields);
            }
            void Error(LogCategory category, string message, map<string, string> fields = {}) {
                Log(LogLevel::ERROR, category, message, fields);
            }
            bool IsEnabled(LogLevel level, LogCategory category) { return LevelEnabled[level] && CategoryEnabled[category]; }
    };

    class ILogSink {
        public:
            virtual ~ILogSink() = default;

            virtual void write(LogRecord record) = 0;
            virtual void flush() = 0;
            virtual void close() = 0;

            virtual bool isHealthy() = 0;
            virtual string name() = 0;
    };

    class Logger : public ILogger {
        private:
        public:
            Logger() = default;
            ~Logger() = default;

            void EmitPhaseHook(FrameContext& ctx, AppPhaseName phase, AppPhaseStatus status, string reason = "") {
                Trace(LogCategory::APP_LOOP, "phase_hook");
            };
    };
}