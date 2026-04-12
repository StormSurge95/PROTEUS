#include <filesystem>

#include "./AppState.h"
#include "./Proteus.h"
#include "./VideoManager.h"

#include "../../resources/font.regular.h"
#include "../../resources/font.schatten.h"

VideoManager::VideoManager(Proteus* p, bool d) {
    proteus = p;
    debug = d;
}

void VideoManager::Init() {
    // ensure SDL was init
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL Video! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    } else {
        SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "SDL Video successfully initialized\n");
    }

    int mainDisp = SDL_GetPrimaryDisplay();
    if (mainDisp == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to get main display ID! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Main Display ID: %d\n", mainDisp);
    }

    const SDL_DisplayMode* dm = SDL_GetDesktopDisplayMode(mainDisp);
    if (!dm) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to get display mode! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    } else {
        std::stringstream ss;
        ss << "SDL_DisplayMode* dm = {\n\t";
        ss << "ID: %d,\n\t";
        ss << "Format: %X,\n\t";
        ss << "Width: %d,\n\t";
        ss << "Height: %d,\n\t";
        ss << "Density: %f,\n\t";
        ss << "Refresh Rate: %f\n\t";
        ss << "Refresh Numerator: %d,\n\t";
        ss << "Refresh Denominator: %d\n}";
    
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO,
            ss.str().c_str(),
            dm->displayID,
            dm->format,
            dm->w, dm->h,
            dm->pixel_density,
            dm->refresh_rate,
            dm->refresh_rate_numerator,
            dm->refresh_rate_denominator);
    }

    screenWidth = dm->w;
    screenHeight = dm->h;
    dispWidth = screenWidth / 2;
    dispHeight = screenHeight / 2;

    window = SDL_CreateWindow(SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING), dispWidth, dispHeight, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateRenderer() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SetRenderVSync(renderer, 1);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, dispWidth, dispHeight);
    
    if (texture == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateTexture() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART);

    if (!TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_Init() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_IOStream* rwR = SDL_IOFromConstMem(font_regular_ttf, font_regular_ttf_len);
    if (!rwR) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_IOFromConstMem() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    fontR.SM = TTF_OpenFontIO(rwR, false, 16);
    SDL_SeekIO(rwR, 0, SDL_IO_SEEK_SET);
    fontR.MD = TTF_OpenFontIO(rwR, false, 24);
    SDL_SeekIO(rwR, 0, SDL_IO_SEEK_SET);
    fontR.LG = TTF_OpenFontIO(rwR, false, 32);
    SDL_SeekIO(rwR, 0, SDL_IO_SEEK_SET);
    fontR.XL = TTF_OpenFontIO(rwR, true, 48);
    if (!fontR.SM || !fontR.MD || !fontR.LG || !fontR.XL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_OpenFontIO() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    TTF_SetFontWrapAlignment(fontR.SM, TTF_HORIZONTAL_ALIGN_CENTER);
    TTF_SetFontWrapAlignment(fontR.MD, TTF_HORIZONTAL_ALIGN_CENTER);
    TTF_SetFontWrapAlignment(fontR.LG, TTF_HORIZONTAL_ALIGN_CENTER);
    TTF_SetFontWrapAlignment(fontR.XL, TTF_HORIZONTAL_ALIGN_CENTER);

    SDL_IOStream* rwS = SDL_IOFromConstMem(font_schatten_ttf, font_schatten_ttf_len);
    if (!rwS) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_IOFromConstMem() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    fontS.SM = TTF_OpenFontIO(rwS, false, 16);
    SDL_SeekIO(rwS, 0, SDL_IO_SEEK_SET);
    fontS.MD = TTF_OpenFontIO(rwS, false, 24);
    SDL_SeekIO(rwS, 0, SDL_IO_SEEK_SET);
    fontS.LG = TTF_OpenFontIO(rwS, false, 32);
    SDL_SeekIO(rwS, 0, SDL_IO_SEEK_SET);
    fontS.XL = TTF_OpenFontIO(rwS, true, 48);
    if (!fontS.SM || !fontS.MD || !fontS.LG || !fontS.XL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_OpenFontIO() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    TTF_SetFontWrapAlignment(fontS.SM, TTF_HORIZONTAL_ALIGN_CENTER);
    TTF_SetFontWrapAlignment(fontS.MD, TTF_HORIZONTAL_ALIGN_CENTER);
    TTF_SetFontWrapAlignment(fontS.LG, TTF_HORIZONTAL_ALIGN_CENTER);
    TTF_SetFontWrapAlignment(fontS.XL, TTF_HORIZONTAL_ALIGN_CENTER);
}

void VideoManager::Deinit() {
    // TODO: Destroy other created video related SDL objects
    if (gameCache.size() > 0) {
        for (auto it1 = gameCache.begin(); it1 != gameCache.end(); it1++) {
            auto &cache = (*it1).second;
            for (auto it2 = cache.begin(); it2 != cache.end(); it2++) {
                SDL_DestroyTexture((*it2).second->texture);
                delete (*it2).second;
            }
        }
    }

    if (consoleCache.size() > 0) {
        for (auto& p : consoleCache) {
            SDL_DestroyTexture(p.second->texture);
            delete p.second;
        }
        consoleCache.clear();
    }

    TTF_CloseFont(fontR.SM); TTF_CloseFont(fontR.MD); TTF_CloseFont(fontR.LG); TTF_CloseFont(fontR.XL);
    TTF_CloseFont(fontS.SM); TTF_CloseFont(fontS.MD); TTF_CloseFont(fontS.LG); TTF_CloseFont(fontS.XL);

    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void VideoManager::RenderView() {
    const AppState state = proteus->GetState();
    if (state.currentView == GAME_VIEW) {
        RenderGameView();
    } else {
        RenderGradientBackground();
        if (state.currentView == CONSOLE_SELECT)
            RenderConsoleList();
        else
            RenderGameList(CONSOLES[state.selectedConsole].first);
        RenderSelector();
    }
}

void VideoManager::RenderGradientBackground() {
    // render a gradient background
    SDL_FColor out = { .r = 0.12f, .g = 0.2f, .b = 0.12f, .a = 1.0f };
    SDL_FColor cen = { .r = 0.0f, .g = 0.2f, .b = 0.0f, .a = 1.0f };
    SDL_Vertex verts[5] = {
        {.position = {.x = 0, .y = 0}, .color = out},   // top left
        {.position = {.x = (float)dispWidth, .y = 0}, .color = out},   // top right
        {.position = {.x = 0, .y = (float)dispHeight}, .color = out},   // bottom left
        {.position = {.x = (float)dispWidth, .y = (float)dispHeight}, .color = out},   // bottom right
        {.position = {.x = (float)dispWidth / 2, .y = (float)dispHeight / 2}, .color = cen} // center
    };

    int indices[12] = { 0, 1, 4,
                       4, 1, 3,
                       3, 2, 4,
                       4, 2, 0 };

    SDL_RenderGeometry(renderer, NULL, verts, 5, indices, 12);
}

void VideoManager::RenderConsoleList() {
    // set up a 4x3 grid to hold our emulation engines
    int rows = 3;
    int cols = 4;
    int rowH = dispHeight / rows;
    int colW = dispWidth / cols;

    int row = 0; int col = 0;
    for (const std::pair<std::string, TextCache*>& p : consoleCache) {
        TextCache* cache = p.second;

        float x = col * colW + (colW - cache->width) / 2.0f;
        float y = row * rowH + (rowH - cache->height) / 2.0f;

        SDL_FRect dst(x, y, cache->width, cache->height);
        SDL_RenderTexture(renderer, cache->texture, NULL, &dst);

        col++;

        if (col >= cols) {
            col = 0;
            row++;
        }
    }
}

void VideoManager::RenderGameList(std::string console) {
    int rows = 3;
    int cols = 4;
    int rowH = dispHeight / rows;
    int colW = dispWidth / cols;

    int row = 0; int col = 0;
    for (const std::pair<std::string, TextCache*>& p : gameCache[console]) {
        TextCache* cache = p.second;

        float x = col * colW + (colW - cache->width) / 2.0f;
        float y = row * rowH + (rowH - cache->height) / 2.0f;
        
        SDL_FRect dst(x, y, cache->width, cache->height);
        SDL_RenderTexture(renderer, cache->texture, NULL, &dst);

        col++;

        if (col >= cols) {
            col = 0;
            row++;

            if (row >= rows) return;
        }
    }
}

void VideoManager::RenderGameView() {
    float tgtAspect = (float)gameWidth / (float)gameHeight;
    float lglAspect = (float)dispWidth / (float)dispHeight;

    SDL_FRect dest = {};

    if (lglAspect > tgtAspect) {
        // pillarbox
        dest.h = (float)dispHeight;
        dest.w = dest.h * tgtAspect;
        dest.x = (dispWidth - dest.w) / 2.0f;
        dest.y = 0.0f;
    } else {
        // letterbox
        dest.w = (float)dispWidth;
        dest.h = dest.w / tgtAspect;
        dest.x = 0.0f;
        dest.y = (dispHeight - dest.h) / 2.0f;
    }

    const uint32_t* buffer = proteus->GetFrameBuffer();
    void* pixels;
    int pitch;
    SDL_LockTexture(gameTexture, NULL, &pixels, &pitch);
    for (int y = 0; y < gameHeight; y++) {
        memcpy(
            (uint8_t*)pixels + y * pitch,
            (uint8_t*)buffer + y * (gameWidth * sizeof(uint32_t)),
            gameWidth * sizeof(uint32_t)
        );
    }
    SDL_UnlockTexture(gameTexture);
    SDL_RenderTexture(renderer, gameTexture, NULL, &dest);
}

void VideoManager::Render() {
    // clear the renderer to prepare for the next frame
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // render the current view to the texture
    RenderView();

    // Present the renderer to the display
    SDL_RenderPresent(renderer);
}

void VideoManager::LoadConsoleCache() {
    consoleCache.clear();

    SDL_Color c(255, 255, 255, 255);

    TTF_Font* font = fontR.XL;

    // why is this suddenly failing to create other caches?
    //for (const std::pair<std::string, std::string>& p : CONSOLES) {
    for (auto it = CONSOLES.begin(); it != CONSOLES.end(); it++) {
        SDL_Surface* s = TTF_RenderText_Blended_Wrapped(font, it->second.c_str(), 0, c, dispWidth / 4);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_DestroySurface(s);
        float w, h;
        SDL_GetTextureSize(t, &w, &h);

        std::pair<std::string, TextCache*> pair(it->first, new TextCache(t, w, h));
        consoleCache.push_back(pair);
    }
}

void VideoManager::LoadGameCache() {
    gameCache.clear();

    SDL_Color c(255, 255, 255, 255);

    TTF_Font* font = fontR.LG; // TODO: select font based on window size

    for (auto it = CONSOLES.begin(); it != CONSOLES.end(); it++) {
        //if (it->first != "NES") continue;
        const std::vector<ROM> gameList = proteus->GetGameList(it->first);
        GameCacheList cache;
        cache.clear();
        for (const ROM& entry : gameList) {
            SDL_Surface* s = TTF_RenderText_Blended_Wrapped(font, FormatDisplayName(entry.gameName).c_str(), 0, c, dispWidth / 4);
            if (!s) {
                SDL_Log("TTF_RenderText failed: %s\n", SDL_GetError());
                continue;
            }
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_DestroySurface(s);
            float w, h;
            SDL_GetTextureSize(t, &w, &h);

            std::pair<std::string, TextCache*> pair(entry.gameName, new TextCache(t, w, h));
            cache.push_back(pair);
        }
        std::pair<std::string, GameCacheList> consoleList(it->first, cache);
        gameCache.insert(consoleList);
    }
}

void VideoManager::LoadCaches() {
    LoadConsoleCache();
    LoadGameCache();
}

void VideoManager::OnInput(Inputs* i) {
    if (i == nullptr) return;

    if (i->DPAD_LEFT || i->AXIS_LEFTX < 0) {
        selectedItem.ColLeft();
    } else if (i->DPAD_RIGHT || i->AXIS_LEFTX > 0) {
        selectedItem.ColRight();
    }
    
    if (i->DPAD_UP || i->AXIS_LEFTY < 0) {
        selectedItem.RowUp();
    } else if (i->DPAD_DOWN || i->AXIS_LEFTY > 0) {
        selectedItem.RowDown();
    }

    if (i->A_BUTTON) OnSelect();
    if (i->B_BUTTON) OnCancel();
}

void VideoManager::OnSelect() const {
    AppState s = proteus->GetState();

    int index = (selectedItem.row * 4) + selectedItem.col;

    if (s.currentView == CONSOLE_SELECT) {
        proteus->SetState(GAME_LIST, (CONSOLE_ID)index);
    } else if (s.currentView == GAME_LIST) {
        proteus->LaunchGame(index);
    }
}

void VideoManager::OnCancel() {
    AppState s = proteus->GetState();

    int index = s.selectedConsole;

    if (s.currentView == GAME_LIST) {
        proteus->SetState(CONSOLE_SELECT);
        int row = index / 4;
        int col = index % 4;
        selectedItem.row = row;
        selectedItem.col = col;
    }
}

void VideoManager::OnResize(size_t width, size_t height) {
    dispWidth = (int)width;
    dispHeight = (int)height;

    SDL_SetRenderLogicalPresentation(renderer, dispWidth, dispHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

void VideoManager::RenderSelector() {
    AppView v = proteus->GetState().currentView;
    float w = (float)dispWidth / 4;
    float h = (float)dispHeight / 3;
    float x = selectedItem.col * w;
    float y = selectedItem.row * h;
    float thickness = 25;

    // top
    SDL_FRect t = { x, y, w, thickness };
    // right
    SDL_FRect r = { (x + (w - thickness)), y, thickness, h };
    // left
    SDL_FRect l = { x, y, thickness, h };
    // bottom
    SDL_FRect b = { x, (y + (h - thickness)), w, thickness };

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    SDL_RenderFillRect(renderer, &t);
    SDL_RenderFillRect(renderer, &r);
    SDL_RenderFillRect(renderer, &l);
    SDL_RenderFillRect(renderer, &b);
}

std::string VideoManager::FormatDisplayName(const std::string& fullname) {
    size_t header = fullname.find(" (");
    std::string name = fullname.substr(0, header);

    size_t comma = name.rfind(", ");
    if (comma == std::string::npos)
        return name;

    std::string base = name.substr(0, comma);
    std::string suff = name.substr(comma + 2);

    if (suff == "The" || suff == "A" || suff == "An")
        return suff.append(" ").append(base);

    return name;
}

void VideoManager::InitGameTexture(std::string title, size_t width, size_t height) {
    SDL_SetWindowTitle(window, FormatDisplayName(title).c_str());

    gameWidth = (int)width;
    gameHeight = (int)height;

    if (gameTexture) {
        SDL_DestroyTexture(gameTexture);
        gameTexture = nullptr;
    }

    gameTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, gameWidth, gameHeight);

    if (!gameTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create game texture, error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}