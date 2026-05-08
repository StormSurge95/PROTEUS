#include <filesystem>

#include "./AppState.h"
#include "./Proteus.h"
#include "./VideoManager.h"

#include "../../resources/font.regular.h"
#include "../../resources/font.schatten.h"
#include "../../resources/font.mono.h"

using namespace NS_Proteus;

VideoManager::VideoManager(Proteus* p, bool d) {
    proteus = p;
    debug = d;
}

void VideoManager::Init() {
    // ensure SDL was init
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL Video! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int mainDisp = SDL_GetPrimaryDisplay();
    if (mainDisp == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to get main display ID! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    const SDL_DisplayMode* dm = SDL_GetDesktopDisplayMode(mainDisp);
    if (!dm) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to get display mode! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
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

    SDL_IOStream* rwM = SDL_IOFromConstMem(font_mono_h, font_mono_h_len);
    if (!rwM) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_IOFromConstMem() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    fontM.SM = TTF_OpenFontIO(rwM, false, 16);
    SDL_SeekIO(rwM, 0, SDL_IO_SEEK_SET);
    fontM.MD = TTF_OpenFontIO(rwM, false, 24);
    SDL_SeekIO(rwM, 0, SDL_IO_SEEK_SET);
    fontM.LG = TTF_OpenFontIO(rwM, false, 32);
    SDL_SeekIO(rwM, 0, SDL_IO_SEEK_SET);
    fontM.XL = TTF_OpenFontIO(rwM, true, 48);
    if (!fontM.SM || !fontM.MD || !fontM.LG || !fontM.XL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_OpenFontIO() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    TTF_SetFontWrapAlignment(fontM.SM, TTF_HORIZONTAL_ALIGN_LEFT);
    TTF_SetFontWrapAlignment(fontM.MD, TTF_HORIZONTAL_ALIGN_LEFT);
    TTF_SetFontWrapAlignment(fontM.LG, TTF_HORIZONTAL_ALIGN_LEFT);
    TTF_SetFontWrapAlignment(fontM.XL, TTF_HORIZONTAL_ALIGN_LEFT);
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
    TTF_CloseFont(fontM.SM); TTF_CloseFont(fontM.MD); TTF_CloseFont(fontM.LG); TTF_CloseFont(fontM.XL);

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
        RenderGameView(proteus->InDebug());
    } else {
        RenderGradientBackground();
        if (state.currentView == CONSOLE_SELECT)
            RenderConsoleList();
        else
            RenderGameList(CONSOLES[state.selectedConsole].first, currentPage);
        RenderSelector();
    }
    if (overlayActive) RenderOverlay();
}

void VideoManager::RenderOverlay() {
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

void VideoManager::RenderGameList(std::string console, unsigned int page) {
    currentMAXpage = pages[console];

    int rows = 4;
    int cols = 5;
    int rowH = dispHeight / rows;
    int colW = dispWidth / cols;

    int row = 0; int col = 0;
    int start = page * 12;
    int end = start + 12;
    for (int i = start; i < end && i < gameCache[console].size(); i++) {
        TextCache* cache = gameCache[console].at(i).second;

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

void VideoManager::RenderGameView(bool dbg) {
    float tgtAspect = (float)gameWidth / (float)gameHeight;
    float lglAspect = (float)dispWidth / (float)dispHeight;

    SDL_FRect dest = {};

    float scale1 = 4.0f / 7.0f;
    float scale2 = 3.0f / 7.0f;

    if (dbg) {
        lglAspect *= scale2;
    }

    if (lglAspect > tgtAspect) {
        // pillarbox
        dest.h = (float)dispHeight;
        dest.w = dest.h * tgtAspect;
        if (dbg) {
            dest.h *= scale2;
            dest.w *= scale2;
        }
        dest.x = dbg ? 0.0f : (dispWidth - dest.w) / 2.0f;
        dest.y = 0.0f;
    } else {
        // letterbox
        dest.w = (float)dispWidth;
        dest.h = dest.w / tgtAspect;
        if (dbg) {
            dest.h *= scale2;
            dest.w *= scale2;
        }
        dest.x = 0.0f;
        dest.y = (dispHeight - dest.h) / 2.0f;
    }

    const u32* buffer = proteus->GetFrameBuffer();
    void* pixels;
    int pitch;
    SDL_LockTexture(gameTexture, NULL, &pixels, &pitch);
    for (int y = 0; y < gameHeight; y++) {
        memcpy(
            (u8*)pixels + y * pitch,
            (u8*)buffer + y * (gameWidth * sizeof(u32)),
            gameWidth * sizeof(u32)
        );
    }
    SDL_UnlockTexture(gameTexture);
    SDL_RenderTexture(renderer, gameTexture, NULL, &dest);

    if (dbg) {
        SDL_FRect dbgDest = { dest.w, 0.0f, (float)dispWidth - dest.w, (float)dispHeight };

        if (proteus->GetDebugView() == DebugView::CPU) {
            // render cpu data text to screen
            RenderDataCPU(dbgDest);
        } else if (proteus->GetDebugView() == DebugView::PPU) {
            // render ppu image data to screen
            RenderDataPPU(dbgDest);
        }
    }
}

void VideoManager::RenderDataCPU(SDL_FRect& dest) {
    SDL_FRect cpuDest = { dest.x, dest.y, dest.w * (2.0f / 5.0f), dest.h };
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &cpuDest);

    // render cpu data text to screen
    std::istringstream cpuData(proteus->GetDebugInfoCPU());
    std::string line;
    u8 lineNum = 0;
    SDL_FRect lineDest = {};
    lineDest.x = cpuDest.x;
    lineDest.y = cpuDest.y;
    SDL_Color white = { 0xFF, 0xFF, 0xFF, 0xFF };
    SDL_Color green = { 0x00, 0xFF, 0x00, 0xFF };
    SDL_Surface* s;
    SDL_Texture* t;
    while (std::getline(cpuData, line)) {
        if (lineNum == 19) {
            s = TTF_RenderText_Solid(fontM.MD, line.c_str(), 0, green);
        } else {
            s = TTF_RenderText_Solid(fontM.MD, line.c_str(), 0, white);
        }
        t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_DestroySurface(s);
        float w, h;
        SDL_GetTextureSize(t, &w, &h);
        lineDest.w = w;
        lineDest.h = h;
        SDL_RenderTexture(renderer, t, NULL, &lineDest);
        lineDest.y += lineDest.h;
        SDL_DestroyTexture(t);
        lineNum++;
    }

    cpuDest.x += cpuDest.w;
    cpuDest.w = dest.w * (3.0f / 5.0f);
    RenderDataRAM(cpuDest);
}

void VideoManager::RenderDataRAM(SDL_FRect& dest) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &dest);
    std::istringstream ramData(proteus->GetDebugInfoRAM());
    std::string line;
    u8 lineNum = 0;
    u32 lineStart = ramPage * 16;
    SDL_FRect lineDest = {};
    lineDest.x = dest.x;
    lineDest.y = dest.y;
    SDL_Color white = { 0xFF, 0xFF, 0xFF, 0xFF };
    std::string head = "";
    SDL_Surface* s;
    SDL_Texture* t;
    u32 x = 0;
    while (x < lineStart) {
        std::getline(ramData, line);
        x++;
    }
    line.resize(53);
    // render current page
    sprintf_s(line.data(), 53, " PG%d 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", ramPage);
    do {
        s = TTF_RenderText_Solid(fontM.MD, line.c_str(), 0, white);
        t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_DestroySurface(s);
        float w, h;
        SDL_GetTextureSize(t, &w, &h);
        lineDest.w = w;
        lineDest.h = h;
        SDL_RenderTexture(renderer, t, NULL, &lineDest);
        lineDest.y += lineDest.h;
        SDL_DestroyTexture(t);
        lineNum++;
    } while (std::getline(ramData, line) && lineNum < 17);
    lineDest.y += (lineDest.h / 2.0f); // add blank line for spacing
    lineNum = 0; // reset lineNum so we don't have to do math
    // render next page
    sprintf_s(line.data(), 53, " PG%d 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", ramPage + 1);
    do {
        s = TTF_RenderText_Solid(fontM.MD, line.c_str(), 0, white);
        t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_DestroySurface(s);
        float w, h;
        SDL_GetTextureSize(t, &w, &h);
        lineDest.w = w;
        lineDest.h = h;
        SDL_RenderTexture(renderer, t, NULL, &lineDest);
        lineDest.y += lineDest.h;
        SDL_DestroyTexture(t);
        lineNum++;
    } while (std::getline(ramData, line) && lineNum < 17);
}

void VideoManager::RenderDataPPU(SDL_FRect& dest) {
    SDL_FRect paletteDest = { dest.x, dest.y, dest.w, (dest.h / 5) };
    RenderPalettes(paletteDest);

    SDL_FRect pattTableDest = { dest.x, dest.y + paletteDest.h, dest.w / 2.0f, dest.w / 2.0f };
    RenderPatternTables(pattTableDest);
}

void VideoManager::RenderPalettes(SDL_FRect& dest) {
    std::vector<u32> colors = proteus->GetDebugPaletteColors();
    float w = dest.w / 17;
    float h = w;
    for (int i = 0; i < colors.size() && i < 16; ++i) {
        SDL_FRect swatch = { dest.x + i * (w + 4), dest.y, w, h };
        u32 color = colors[i];
        u8 b = (color >> 16) & 0xFF;
        u8 g = (color >> 8) & 0xFF;
        u8 r = color & 0xFF;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderFillRect(renderer, &swatch);
    }
}

void VideoManager::RenderPatternTables(SDL_FRect& dest) {
    SDL_FRect ptDest = { dest.x, dest.y, dest.h, dest.h };
    for (u8 i = 0; i < 2; i++) {
        std::vector<u32> pixels = proteus->GetDebugPatternTable(i);
        SDL_Surface* s = SDL_CreateSurfaceFrom(128, 128, SDL_PIXELFORMAT_ABGR8888, pixels.data(), 128 * sizeof(u32));
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_DestroySurface(s);
        SDL_RenderTexture(renderer, t, nullptr, &ptDest);
        SDL_DestroyTexture(t);
        ptDest.x += ptDest.w;
    }
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

array<FontChoice, 4> VideoManager::GetOrderedFonts(const Font& family) const {
    return { {
        { family.XL, FontSize::XL },
        { family.LG, FontSize::LG },
        { family.MD, FontSize::MD },
        { family.SM, FontSize::SM }
    } };
}

TextSize VideoManager::MeasureWrappedText(TTF_Font* font, const string& text, int wrapWidth) const {
    SDL_Color transparentWhite = { 255, 255, 255, 255 };

    SDL_Surface* s = TTF_RenderText_Blended_Wrapped(
        font,
        text.c_str(),
        0,
        transparentWhite,
        wrapWidth
    );

    // if we fail to create a surface, return absurdly large values.
    if (!s) return { FLT_MAX, FLT_MAX };

    // create TextSize object for returning
    TextSize size = {
        static_cast<float>(s->w),
        static_cast<float>(s->h)
    };

    // destroy SDL_Surface for memory management
    SDL_DestroySurface(s);

    // return the calculated size
    return size;
}

FontChoice VideoManager::PickTextboxFont(const Font& family, const string& text, float maxW, float maxH, int wrapW) const {
    // search through the list of possible choices, starting at the largest option
    // to find the largest font size that can fit within our properties
    for (const FontChoice& choice : GetOrderedFonts(family)) {
        if (choice.font == nullptr) continue;

        // get the measured text size of the current font option
        TextSize measured = MeasureWrappedText(choice.font, text, wrapW);

        // if our measured size is not larger than our max size, return this option
        if (measured.w <= maxW && measured.h <= maxH) return choice;
    }

    // default to smallest font size.
    return { family.SM, FontSize::SM };
}

FontChoice VideoManager::PickMenuFont(const Font& family, const string& label) const {
    // calculate width of cell based on view
    float cellW = CellWidth();
    // calculate heigh of cell based on view
    float cellH = CellHeight();

    // calculate usable width/height to give text breathing room
    float uW = cellW * 0.85f;
    float uH = cellH * 0.75f;

    // calculate whole-number wrap width
    int wrapWidth = static_cast<int>(floor(uW));

    // use calculated values to select font size
    return PickTextboxFont(family, label, uW, uH, wrapWidth);
}

float VideoManager::MeasureLineWidth(TTF_Font* font, const string& line) const {
    int w = 0;
    int h = 0;

    if (!TTF_GetStringSize(font, line.c_str(), 0, &w, &h)) return 0.0f;

    return static_cast<float>(w);
}

FontChoice VideoManager::PickDebugFont(const vector<string>& lines, float maxW, float maxH) const {
    for (const FontChoice& choice : GetOrderedFonts(fontM)) {
        float lineH = static_cast<float>(TTF_GetFontHeight(choice.font));

        float totalH = lineH * lines.size();

        if (totalH > maxH) continue;

        float widestLineW = 0.0f;
        for (const string& line : lines) {
            float lineW = MeasureLineWidth(choice.font, line);
            widestLineW = max(widestLineW, lineW);
        }

        if (widestLineW <= maxW) return choice;
    }

    return { fontM.SM, FontSize::SM };
}

TextCache* VideoManager::CreateTextCacheForMenuLabel(const string& label, const Font& family, SDL_Color color) const {
    int uW = static_cast<int>(CellWidth() * 0.85f);

    // dynamically select select font size based on window size
    FontChoice c = PickMenuFont(family, label);

    SDL_Surface* s = TTF_RenderText_Blended_Wrapped(c.font, label.c_str(), 0, color, uW);
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_DestroySurface(s);
    TextSize size;
    SDL_GetTextureSize(t, &size.w, &size.h);

    return new TextCache(t, size.w, size.h);
}

void VideoManager::LoadConsoleCache() {
    consoleCache.clear();

    SDL_Color c(255, 255, 255, 255);

    for (auto it = CONSOLES.begin(); it != CONSOLES.end(); it++) {
        // Create text cache with dynamic font selection based on window size
        TextCache* t = CreateTextCacheForMenuLabel(it->second, fontR, c);
        CacheItem pair(it->first, t);
        consoleCache.push_back(pair);
    }
}

void VideoManager::LoadGameCache() {
    gameCache.clear();

    SDL_Color c(255, 255, 255, 255);

    for (auto it = CONSOLES.begin(); it != CONSOLES.end(); it++) {
        const std::vector<ROM> gameList = proteus->GetGameList(it->first);
        CacheList cache;
        cache.clear();
        for (const ROM& entry : gameList) {
            TextCache* t = CreateTextCacheForMenuLabel(FormatDisplayName(entry.gameName), fontR, c);
            CacheItem pair(entry.gameName, t);
            cache.push_back(pair);
        }
        std::pair<std::string, CacheList> consoleList(it->first, cache);
        gameCache.insert(consoleList);
        std::pair<std::string, unsigned int> numPages(it->first, (unsigned int)cache.size() / 20);
        pages.insert(numPages);
    }
}

void VideoManager::LoadCaches() {
    LoadConsoleCache();
    LoadGameCache();
}

void VideoManager::ClearCaches() {
    consoleCache.clear();
    gameCache.clear();
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

    if (i->AXIS_LEFT_TRIGGER > 0 || i->L1_BUTTON)
        PageLeft();

    if (i->AXIS_RIGHT_TRIGGER > 0 || i->R1_BUTTON)
        PageRight();

    if (i->A_BUTTON) OnSelect();
    if (i->B_BUTTON) OnCancel();
}

void VideoManager::PageLeft() {
    if (currentPage == 0) currentPage = currentMAXpage;
    else currentPage--;
}

void VideoManager::PageRight() {
    if (currentPage == currentMAXpage) currentPage = 0;
    else currentPage++;
}

void VideoManager::PageUp() {
    if (ramPage == 0) ramPage = 6;
    else ramPage -= 2;
}

void VideoManager::PageDown() {
    if (ramPage == 6) ramPage = 0;
    else ramPage += 2;
}

void VideoManager::OnMouseMove(float x, float y) {
    selectedItem.col = (int)(x / ((float)dispWidth / 4));
    selectedItem.row = (int)(y / ((float)dispHeight / 3));
}

void VideoManager::OnMouseScroll(int dir) {
    const AppState state = proteus->GetState();
    if (dir == 1) { // scroll up -> page left
        if (state.currentView == GAME_VIEW && proteus->InDebug())
            PageUp();
        else
            PageLeft();
    } else if (dir == -1) { // scroll down -> page right
        if (state.currentView == GAME_VIEW && proteus->InDebug())
            PageDown();
        else
            PageRight();
    }
}

void VideoManager::OnSelect() const {
    AppState s = proteus->GetState();

    int index = (selectedItem.row * 4) + selectedItem.col;

    if (s.currentView == CONSOLE_SELECT) {
        proteus->SetState(AppView::GAME_LIST, (CONSOLE_ID)index);
    } else if (s.currentView == AppView::GAME_LIST) {
        proteus->LaunchGame((currentPage * 12) + index);
    }
}

void VideoManager::OnCancel() {
    AppState s = proteus->GetState();

    int index = s.selectedConsole;

    if (s.currentView == AppView::GAME_LIST) {
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

    LoadCaches();
}

void VideoManager::RenderSelector() {
    AppView v = proteus->GetState().currentView;
    float w = (float)dispWidth / (v == AppView::CONSOLE_SELECT ? 4.0f : 5.0f);
    float h = (float)dispHeight / (v == AppView::CONSOLE_SELECT ? 3.0f : 4.0f);
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

string VideoManager::FormatDisplayName(const string& fullname) {
    size_t header = fullname.find(" (");
    string name = fullname.substr(0, header);

    size_t comma = name.rfind(", ");
    if (comma != string::npos) {
        string base = name.substr(0, comma);
        string suff = name.substr(comma + 2);

        if (suff == "The" || suff == "A" || suff == "An")
            name = suff.append(" ").append(base);
    }

    replace(name.begin(), name.end(), '_', ' ');

    // TODO: replace "<char>.<char>" with "<char> - <char>"

    return name;
}

void VideoManager::InitGameTexture(string title, size_t width, size_t height) {
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