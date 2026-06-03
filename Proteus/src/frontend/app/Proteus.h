#pragma once

#include "../FrontendPCH.h"
#include "../session/ConsoleSession.h"
#include "./IManagerContexts.h"

namespace NS_Proteus {
    class AudioManager;
    class InputManager;
    class VideoManager;
    class RomLibrary;
    class Logger;

    /**
     * Singleton class for maintaining the entire application.
     * This class will be responsible for owning and controlling
     * all SDL objects.
     */
    class Proteus : public IVideoContext, public IInputContext, public IAudioContext
    {
        public:
            Proteus();
            ~Proteus();

            Proteus(const Proteus&) = delete;
            Proteus& operator=(const Proteus&) = delete;
            Proteus(Proteus&&) = delete;
            Proteus& operator=(Proteus&&) = delete;

            void Init();
            void Deinit();

            void Run();
            void RunSST();

            // overrides for IManagerContext
            ConsoleSessionState GetSessionState() const override { return session->GetState(); }
            AppView GetAppView() const override { return state.currentView; }

            // overrides for IVideoContext
            const AppState& GetAppState() const override { return state; }
            const std::vector<ROM_DATA>& GetGameList(ConsoleID console) const override;
            void LaunchGame(u32 index) override;
            IDebugger* GetDebugger() const override { return session->GetDebugger().get();  }
            void ResetConsole() override;
            void ShutdownConsole() override;
            void Resume() override;
            const u32* GetFrameBuffer() const override;
            void OnConsoleSelected(ConsoleID console) override { SetState(AppView::GAME_LIST, console); }

            // overrides for IInputContext
            ConsoleID GetSelectedConsole() const override { return state.selectedConsole; }
            void ToggleOverlay() override;

            // overrides for IAudioContext
            bool IsAudioMuted() const override { return false; }

            inline void SetState(AppView view, ConsoleID console = ConsoleID::NONE) {
                state.currentView = view;
                if (view == AppView::GAME_LIST) state.selectedConsole = console;
            }

            Logger* GetLogger() const override { return logger.get(); }

        private:
            static std::shared_ptr<Proteus> instance;
            std::shared_ptr<AudioManager> audioManager;
            std::shared_ptr<VideoManager> videoManager;
            std::shared_ptr<InputManager> inputManager;
            sptr<ConsoleSession> session;
            uptr<RomLibrary> lib;
            sptr<Logger> logger;

            AppState state;
            RuntimeStats stats;

            bool quit = false;
            bool pausedByOverlay = false;
            SDL_Event event = {};

            void ToggleDebug();

            void SetMetadata();

            void ProcessEvents(FrameContext& ctx);
            bool ProcessQuitEvent(FrameContext& ctx);
            bool ProcessWindowEvent(FrameContext& ctx);
            bool ProcessKeyboardEvent(FrameContext& ctx);
            bool ProcessGamepadEvent(FrameContext& ctx);
            bool ProcessMouseEvent(FrameContext& ctx);
            bool ProcessButtonInput(u8 button);

            FrameContext BeginFrame();
            void PhaseInput(FrameContext& ctx);
            void PhaseComputeFrameState(FrameContext& ctx);
            void PhaseThrottleFrame(FrameContext& ctx);
            void PhaseEmuDebug(FrameContext& ctx);
            void PhaseRender(FrameContext& ctx);
            void PhaseAudio(FrameContext& ctx);
            void EndFrame(FrameContext& ctx);
    };
}