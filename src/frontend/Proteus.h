#pragma once

#include "./FrontendPCH.h"
#include "./AppState.h"
#include "./DebugManager.h"
#include "../backend/SHARED/IConsole.h"

class AudioManager;
class InputManager;
class VideoManager;

struct ROM {
    std::string gameName;
    std::string path;
};

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

        void ProcessInputs();

        inline void SetState(AppView view, CONSOLE_ID console = NONE) {
            state.currentView = view;
            if (view == GAME_LIST) state.selectedConsole = console;
        }

        void LaunchGame(int index);

        DebugView GetDebugView() { return debugManager->currentView; }
        std::string GetDebugInfoCPU();
        std::string GetDebugInfoRAM();
        std::vector<u32> GetDebugPaletteColors();
        std::vector<u32> GetDebugPatternTable(int);

        std::vector<ROM> GetGameList(const std::string& console);
        const u32* GetFrameBuffer();
    private:
        bool debug = false;
        bool ROMactive = false;
        std::shared_ptr<IConsole> station = nullptr;

        static std::shared_ptr<Proteus> instance;
        std::shared_ptr<AudioManager> audioManager;
        std::shared_ptr<VideoManager> videoManager;
        std::shared_ptr<InputManager> inputManager;
        std::shared_ptr<DebugManager> debugManager;

        std::map<std::string, std::vector<ROM>> gameList;

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

        void StartConsole();
        void ShutDownConsole(bool shutDownApp = false);
};