#pragma once

#include <map>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

#include "./InputManager.h"
#include "./Proteus.h"

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
        void OnSelect() const;
        void OnCancel();
        void OnResize(size_t width, size_t height);

        void LoadCaches();

    private:
        bool debug = false;
        Proteus* proteus = nullptr;

        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* texture = nullptr;
        SDL_Texture* gameTexture = nullptr;
        TTF_TextEngine* engine = nullptr;
        struct Font {
            TTF_Font* SM = nullptr;
            TTF_Font* MD = nullptr;
            TTF_Font* LG = nullptr;
            TTF_Font* XL = nullptr;

            Font() = default;
        } fontR, fontS;

        struct TextCache {
            SDL_Texture* texture = nullptr;
            float width, height;

            TextCache(SDL_Texture* t, float w, float h) : texture(t), width(w), height(h) {}
        };

        std::vector<std::pair<std::string, TextCache*>> consoleCache = {};
        typedef std::vector<std::pair<std::string, TextCache*>> GameCacheList;
        std::map<std::string, GameCacheList> gameCache = {};
        std::map<std::string, unsigned int> pages = {};
        unsigned int currentMAXpage = 0;
        unsigned int currentPage = 0;

        struct Selection {
            int row = 0;
            int col = 0;

            Selection() = default;
            void RowDown() {
                row = (row == 2) ? 0 : row + 1;
            }
            void RowUp() {
                row = (row == 0) ? 2 : row - 1;
            }
            void ColLeft() {
                col = (col == 0) ? 3 : col - 1;
            }
            void ColRight() {
                col = (col == 3) ? 0 : col + 1;
            }
        } selectedItem;

        int screenWidth = 0;
        int screenHeight = 0;
        int dispWidth = 0;
        int dispHeight = 0;
        int gameWidth = 0;
        int gameHeight = 0;


        void RenderConsoleList();
        void RenderGameList(std::string console, unsigned int page);
        void RenderGameView();
        void RenderGradientBackground();
        void RenderView();

        void LoadConsoleCache();
        void LoadGameCache();

        void RenderSelector();

        std::string FormatDisplayName(const std::string& name);
};