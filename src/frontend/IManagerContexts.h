#pragma once

#include "./FrontendPCH.h"
#include "./ConsoleSession.h"

namespace NS_Proteus {
    class IManagerContext {
        public:
            virtual ConsoleSessionState GetSessionState() const = 0;
            virtual AppView GetAppView() const = 0;
    };

    class IVideoContext : public IManagerContext {
        public:
            virtual const AppState& GetAppState() const = 0;
            virtual const vector<ROM_DATA>& GetGameList(ConsoleID console) const = 0;
            virtual void LaunchGame(u32 index) = 0;
            virtual IDebugger* GetDebugger() const = 0;
            virtual void ResetConsole() = 0;
            virtual void ShutdownConsole() = 0;
            virtual void Resume() = 0;
            virtual void OnConsoleSelected(ConsoleID console) = 0;
            virtual const u32* GetFrameBuffer() const = 0;
    };

    class IInputContext : IManagerContext {
        public:
            virtual ConsoleID GetSelectedConsole() const = 0;
            virtual void ToggleOverlay() = 0;
    };

    class IAudioContext : IManagerContext {
        public:
            virtual bool IsAudioMuted() const = 0;
    };
}