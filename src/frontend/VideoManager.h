#pragma once

#include "./FrontendPCH.h"
#include "./Types.h"
#include "./Proteus.h"

namespace NS_Proteus {
    class VideoManager {
        public:
            /**
             * @brief Explicit Constructor
             * @param proteus Pointer to the Proteus instance.
             * @param debug Whether or not debug is enabled
             * @todo Is `debug` even necessary anymore?
             */
            VideoManager(Proteus* proteus);
            /**
             * @brief Explicit Destructor (simply calls `Deinit()`)
             */
            ~VideoManager() { Deinit(); }

            /// Remove all copy constructors; VideoManager is meant to be singleton
            VideoManager(const VideoManager&) = delete;
            VideoManager& operator=(const VideoManager&) = delete;
            VideoManager(VideoManager&&) = delete;
            VideoManager& operator=(VideoManager&&) = delete;

            /// @brief Initilize VideoManager
            void Init();
            /**
             * @brief Initialize GameView texture
             * @param title The new window title to use
             * @param width The width of the game window
             * @param height The heigh of the game window
             * @details
             * Allows us to change the name of the window to something more relevant
             * and prepare a texture for rendering the pixels provided by the emulated
             * game station to the screen.
             * 
             * The provided `title` argument is appended to the default window title
             * of "PROTEUS" in the format "PROTEUS: <title>"
             * 
             * The `width` and `height` provided are meant to be the width and
             * height of the natural game screen as viewed on its original console.
             */
            void InitGameTexture(std::string title, size_t width, size_t height);

            /// @brief Deinitialize VideoManager and clean up memory
            void Deinit();

            /// @brief Render necessary image data to the screen
            void Render(const AppState& state);

            /// @brief Handles the resizing of our application window
            void OnResize(size_t width, size_t height);

            /// @brief Toggles whether or not to display the Overlay Menu
            inline void ToggleOverlay() { overlayActive = !overlayActive; }

            /// @brief Toggles whether or not to display the Debug Menu
            inline void ToggleDebug() { debugActive = !debugActive; }

            SDL_Window* GetWindow() const { return window; }
            const bool OverlayActive() const { return overlayActive; }

        private:
            bool debugActive = false;
            bool overlayActive = false;
            Proteus* proteus = nullptr;

            DisplayInfo dispInfo;

            SDL_Window* window = nullptr;
            SDL_Renderer* renderer = nullptr;
            SDL_Texture* gameTexture = nullptr;

            ImGuiViewport* vp = nullptr;

            struct Fonts {
                ImFont* UI = nullptr;
                ImFont* Debug = nullptr;
                //ImFont* Nintendo = nullptr;
                //ImFont* Sony = nullptr;
                //ImFont* Microsoft = nullptr;
            } fonts = {};
            float fontSize = 64.0f;
            float ConsoleTextWidth = 275.0f;
            float GameTextWidth = 400.0f;
            float GetFontSize(ImVec2 space, const float& base) const;
            //ImFont* GetConsoleFont(ConsoleID console);

            PageCounts pages = {};
            u16 currentMAXpage = 0;
            u16 currentPage = 0;

            MenuSelection selectedItem;

            u8 ramPage = 0x00;

            void RenderConsoleSelection();
            void RenderGameSelection(ConsoleID console);
            void RenderGameView(bool debug);
            void RenderDebug();
            void RenderOverlay();

            void PrepViewport(ImGuiViewport* vp);
            void PrepUI(ImGuiViewport* vp, MenuType type);
            void DeprepUI();

            const ImGuiWindowFlags ImMenuFlags =
                ImGuiWindowFlags_NoDecoration |         // no titlebar, resize, scrollbar, or collapse
                ImGuiWindowFlags_NoMove |               // prevent moving the window
                ImGuiWindowFlags_NoSavedSettings |      // prevent .ini modification
                ImGuiWindowFlags_NoBringToFrontOnFocus; // prevent automatically bringing window to front on focus

            const ImGuiWindowFlags ImOverlayFlags =
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings;

            const ImGuiWindowFlags ImDebugFlags =
                ImGuiWindowFlags_MenuBar                // Has a menu-bar
                | ImGuiWindowFlags_AlwaysAutoResize     // make sure content always fits in window
                | ImGuiWindowFlags_NoNav;               // make sure that gamepad inputs only effect gameplay

            void FinalizeFrame(bool clear = true);
    };
}