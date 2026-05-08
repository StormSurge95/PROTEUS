#pragma once

#include "./FrontendPCH.h"
#include "./InputManager.h"
#include "./Proteus.h"

namespace NS_Proteus {
    class VideoManager {
        public:
            VideoManager(Proteus* proteus, bool debug = false);
            ~VideoManager() { Deinit(); }

            VideoManager(const VideoManager&) = delete;
            VideoManager& operator=(const VideoManager&) = delete;
            VideoManager(VideoManager&&) = delete;
            VideoManager& operator=(VideoManager&&) = delete;

            void Init();
            void InitGameTexture(std::string title, size_t width, size_t height);
            void Deinit();

            void Render();

            void OnInput(Inputs* i);
            void OnMouseMove(float, float);
            void OnMouseScroll(int);
            void OnSelect() const;
            void OnCancel();
            void OnResize(size_t width, size_t height);

            void ToggleOverlay() { overlayActive = !overlayActive; }

            void LoadCaches();
            void ClearCaches();

        private:
            bool debug = false;
            bool overlayActive = false;
            Proteus* proteus = nullptr;

            SDL_Window* window = nullptr;
            SDL_Renderer* renderer = nullptr;
            SDL_Texture* texture = nullptr;
            SDL_Texture* gameTexture = nullptr;
            TTF_TextEngine* engine = nullptr;
            Font fontR, fontS, fontM;

            CacheList consoleCache = {};
            GameCache gameCache = {};
            PageCounts pages = {};
            unsigned int currentMAXpage = 0;
            unsigned int currentPage = 0;

            MenuSelection selectedItem;

            int screenWidth = 0;
            int screenHeight = 0;
            int dispWidth = 0;
            int dispHeight = 0;
            int gameWidth = 0;
            int gameHeight = 0;

            int ramPage = 0x00;

            void PageUp();
            void PageDown();
            void PageLeft();
            void PageRight();

            void RenderConsoleList();
            void RenderGameList(std::string console, unsigned int page);
            void RenderGameView(bool dbg = false);
            void RenderGradientBackground();
            void RenderOverlay();
            void RenderView();

            void RenderDataCPU(SDL_FRect&);
            void RenderDataRAM(SDL_FRect&);

            void RenderDataPPU(SDL_FRect&);
            void RenderPalettes(SDL_FRect&);
            void RenderPatternTables(SDL_FRect&);

            FontChoice CONSOLE_LIST, GAME_LIST, DEBUG;

            float MeasureLineWidth(TTF_Font* font, const string& line) const;
            TextSize MeasureWrappedText(TTF_Font* font, const string& text, int wrapWidth) const;
            /**
             * @brief Get an ordered list of the fonts of the provided family.
             * @param family The font family to get an ordered list for.
             * @return An array of font choices, ordered from largest to smallest.
             */
            array<FontChoice, 4> GetOrderedFonts(const Font& family) const;
            FontChoice PickMenuFont(const Font& family, const string& label) const;
            FontChoice PickTextboxFont(const Font& family, const string& text, float maxW, float maxH, int wrapW) const;
            FontChoice PickDebugFont(const vector<string>& lines, float maxW, float maxH) const;
            TextCache* CreateTextCacheForMenuLabel(const string& label, const Font& family, SDL_Color color) const;

            float CellWidth() const { return static_cast<float>(dispWidth) / (proteus->GetState().currentView == AppView::CONSOLE_SELECT ? 3.0f : 4.0f); }
            float CellHeight() const { return static_cast<float>(dispHeight) / (proteus->GetState().currentView == AppView::CONSOLE_SELECT ? 4.0f : 5.0f); }

            void LoadConsoleCache();
            void LoadGameCache();

            void RenderSelector();

            std::string FormatDisplayName(const std::string& name);
    };
}