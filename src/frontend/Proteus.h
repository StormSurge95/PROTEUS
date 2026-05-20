#pragma once

#include "./FrontendPCH.h"
#include "./ConsoleSession.h"
#include "../backend/shared/IConsole.h"

namespace NS_Proteus {
    class RomLibrary;

    /**
     * Singleton class for maintaining the entire application.
     * This class will be responsible for owning and controlling
     * all SDL objects.
     */
    class Proteus
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
            //void RunSST();

            const AppState GetState() const { return state; }

            void ProcessKeyInput(SDL_Keycode key);
            void ProcessButtonInput(u8 button);

            inline void SetState(AppView view, ConsoleID console = ConsoleID::NONE) {
                state.currentView = view;
                if (view == AppView::GAME_LIST) state.selectedConsole = console;
            }

            void LaunchGame(int index);
            void ResetConsole();
            void ShutDownConsole();
            void Resume();

            //DebugView GetDebugView() { return debugManager->currentView; }
            IDebugger* GetDebugger() const { return session->GetDebugger().get();  }

            std::vector<ROM_DATA> GetGameList(ConsoleID console);
            const u32* GetFrameBuffer();
        private:
            static std::shared_ptr<Proteus> instance;
            std::shared_ptr<AudioManager> audioManager;
            std::shared_ptr<VideoManager> videoManager;
            std::shared_ptr<InputManager> inputManager;
            sptr<ConsoleSession> session;
            uptr<RomLibrary> lib;

            AppState state;
            RuntimeStats stats;

            bool quit = false;
            SDL_Event event = {};

            void ToggleDebug();

            void ToggleOverlay();

            void SetMetadata();

            void ProcessEvents(FrameContext& ctx);

            FrameContext BeginFrame();
            void EndFrame(FrameContext& ctx);
            void ComputeFrameState(FrameContext& ctx);
    };
}