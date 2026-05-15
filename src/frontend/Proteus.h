#pragma once

#include "./FrontendPCH.h"
#include "./DebugManager.h"
#include "../backend/SHARED/IConsole.h"

namespace NS_Proteus {
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

            bool InDebug() const { return debug; }

            const AppState GetState() const { return state; }

            void ProcessKeyInput(SDL_Keycode key);
            void ProcessButtonInput(u8 button);

            inline void SetState(AppView view, ConsoleID console = ConsoleID::NONE) {
                state.currentView = view;
                if (view == AppView::GAME_LIST) state.selectedConsole = console;
            }

            void LaunchGame(int index);

            DebugView GetDebugView() { return debugManager->currentView; }
            IDebugger* GetDebugger() const { return debugManager->GetDebugger().get();  }

            std::vector<ROM_DATA> GetGameList(ConsoleID console);
            const u32* GetFrameBuffer();
            void StartConsole();
            void ResetConsole();
            void ShutDownConsole();
        private:
            bool debug = false;
            bool dbgPause = false;
            bool ROMactive = false;
            std::shared_ptr<IConsole> station = nullptr;

            static std::shared_ptr<Proteus> instance;
            std::shared_ptr<AudioManager> audioManager;
            std::shared_ptr<VideoManager> videoManager;
            std::shared_ptr<InputManager> inputManager;
            std::shared_ptr<DebugManager> debugManager;

            map<ConsoleID, vector<ROM_DATA>> gameList;

            AppState state;

            SDL_Window* window = nullptr;
            SDL_Renderer* renderer = nullptr;

            int dispWidth = 0;
            int dispHeight = 0;

            bool quit = false;
            SDL_Event event = {};

            void ToggleDebug();

            void SetMetadata();

            void ProcessEvents();

            void IdentifyROMs();

            std::string Lookup(const std::string& console, const std::string& hash);

            std::string MD5(const std::string& filepath);

    };
}