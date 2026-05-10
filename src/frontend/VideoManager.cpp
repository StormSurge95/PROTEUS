#include "./Proteus.h"
#include "./VideoManager.h"

#include "../../resources/inter_font.h"
#include "../../resources/debug_font.h"
#include "../../resources/NintendoFont.h"
#include "../../resources/SonyFont.h"
#include "../../resources/MicrosoftFont.h"

using namespace NS_Proteus;

VideoManager::VideoManager(Proteus* p) {
    proteus = p;
}

void VideoManager::Init() {
    // ensure SDL was init
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL Video! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // get display resolution and scale
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

    // get display information (ignore size if equal to default)
    dispInfo.scale = dm->pixel_density;
    if (dispInfo.screenWidth != dm->w || dispInfo.screenHeight != dm->h) {
        dispInfo.screenWidth = dm->w;
        dispInfo.screenHeight = dm->h;
        dispInfo.dispWidth = u32(dm->w * (2.0 / 3.0));
        dispInfo.dispHeight = u32(dm->h * (2.0 / 3.0));
    }

    // create window
    window = SDL_CreateWindow(SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING), dispInfo.dispWidth, dispInfo.dispHeight, WindowFlags);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    // create renderer
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateRenderer() failed! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_SetRenderVSync(renderer, 1);
    SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // get io object
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // enabled keyboard & gamepad navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // set dark theme cuz we aren't psychotic
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(dispInfo.scale);
    style.FontScaleDpi = dispInfo.scale;

    // Setup platform/renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // create the various fonts for use in the application
    style.FontSizeBase = fontSize;
    fonts.UI = io.Fonts->AddFontFromMemoryCompressedTTF(InterFont_data, InterFont_size);
    fonts.Debug = io.Fonts->AddFontFromMemoryCompressedTTF(DebugFont_data, DebugFont_size);
    //fonts.Nintendo = io.Fonts->AddFontFromMemoryCompressedTTF(NintendoFont_data, NintendoFont_size);
    //fonts.Sony = io.Fonts->AddFontFromMemoryCompressedTTF(SonyFont_data, SonyFont_size);
    //fonts.Microsoft = io.Fonts->AddFontFromMemoryCompressedTTF(MicrosoftFont_data, MicrosoftFont_size);
}

void VideoManager::Deinit() {

    // remove fonts from ImGui
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->RemoveFont(fonts.UI);
    io.Fonts->RemoveFont(fonts.Debug);
    //io.Fonts->RemoveFont(fonts.Nintendo);
    //io.Fonts->RemoveFont(fonts.Sony);
    //io.Fonts->RemoveFont(fonts.Microsoft);

    // Shutdown ImGui
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // destroy created texture
    if (gameTexture != nullptr) {
        SDL_DestroyTexture(gameTexture);
        gameTexture = nullptr;
    }

    // destroy renderer
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    // destroy window
    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void VideoManager::PageLeft() {
    if (currentPage == 0) currentPage = currentMAXpage;
    else currentPage--;
}

void VideoManager::PageRight() {
    if (currentPage == currentMAXpage) currentPage = 0;
    else currentPage++;
}

void VideoManager::OnResize(size_t width, size_t height) {
    dispInfo.dispWidth = (int)width;
    dispInfo.dispHeight = (int)height;
}

void VideoManager::InitGameTexture(string title, size_t width, size_t height) {
    SDL_SetWindowTitle(window, FormatDisplayName(title).c_str());

    dispInfo.gameWidth = (u32)width;
    dispInfo.gameHeight = (u32)height;

    if (gameTexture) {
        SDL_DestroyTexture(gameTexture);
    }

    gameTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, dispInfo.gameWidth, dispInfo.gameHeight);
    if (!gameTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create game texture; error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

//ImFont* VideoManager::GetConsoleFont(ConsoleID console) {
//    // TODO: Get better font for NES
//    switch (console) {
//        default:
//        case ConsoleID::NONE:
//            return fonts.UI;
//        case ConsoleID::NES:
//        case ConsoleID::GBA:
//        case ConsoleID::GBC:
//        case ConsoleID::NGC:
//        case ConsoleID::N64:
//        case ConsoleID::NDS:
//        case ConsoleID::SNS:
//            return fonts.Nintendo;
//        case ConsoleID::PS1:
//        case ConsoleID::PS2:
//        case ConsoleID::PS3:
//            return fonts.Sony;
//        case ConsoleID::XBX:
//        case ConsoleID::XB3:
//            return fonts.Microsoft;
//    }
//}

void VideoManager::PrepViewport(ImGuiViewport* vp) {
    // set next window to cover entire main viewport
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);

    // push styles to make it 'invisible'
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

}

void VideoManager::PrepUI(ImGuiViewport* vp, ConsoleID console) {
    // base viewport prep
    PrepViewport(vp);

    // push styles to actually style our UI view
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));           // no bg color
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));             // no default button color
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.10f));  // hovered buttons dim background a little
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0.25f));   // active buttons dim background more
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));

    // push font for rendering text on buttons
    ImGui::PushFont(fonts.UI); // get font based on console
}

void VideoManager::DeprepUI() {
    // pop font (we're done with it)
    ImGui::PopFont();

    // pop style colors & vars to return to normal
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(4);
}

void VideoManager::Render(const AppState& state) {
    switch (state.currentView) {
        default:
        case AppView::CONSOLE_SELECT:
            RenderConsoleSelection();
            break;
        case AppView::GAME_LIST:
            RenderGameSelection(state.selectedConsole);
            break;
        case AppView::GAME_VIEW:
            return RenderGameView(proteus->InDebug());
            break;
    }
    if (overlayActive) RenderOverlay();
}

void VideoManager::RenderConsoleSelection() {
    // start the imgui frame
    StartGUI(MenuType::MAIN, "CONSOLE_SELECTION");

    // calculate button dimensions
    ImVec2 btnSize(vp->WorkSize.x / 4.0f, vp->WorkSize.y / 3.0f);
    ImGui::PushFont(nullptr, GetFontSize(btnSize, ConsoleTextWidth));
    // render the console buttons
    for (u8 i = 0; i < (u8)ConsoleID::TOTAL; i++) {
        ConsoleID console = ConsoleID(i);
        if (ImGui::ButtonCentered(ConsoleNamesShort.at(console).c_str(), btnSize)) {
            proteus->SetState(AppView::GAME_LIST, console);
        }
        // only 4 buttons per row
        if (i % 4 != 3) ImGui::SameLine();
    }
    ImGui::PopFont();
    // end subwindow
    EndGUI(MenuType::MAIN);
}

void VideoManager::RenderGameSelection(ConsoleID console) {
    // start the imgui frame
    StartGUI(MenuType::MAIN, "GAME_SELECTION");

    // get list of games
    vector<ROM_DATA> games = proteus->GetGameList(console);
    // only attempt to render gamelist buttons if there are actually games in the list
    if (games.size() != 0) {
        // calculate button dimensions
        ImVec2 btnSize(vp->WorkSize.x / 5.0f, vp->WorkSize.y / 4.0f);
        ImGui::PushFont(nullptr, GetFontSize(btnSize, GameTextWidth));
        // render the gamelist buttons
        for (u16 i = 0; i < 20; i++) {
            string name = FormatDisplayName(games[i].gameName, true);
            if (ImGui::ButtonCentered(name.c_str(), btnSize)) {
                proteus->LaunchGame(i);
            }
            // only 5 buttons per row
            if (i % 5 != 4) ImGui::SameLine();
        }
        ImGui::PopFont();
    } else if (!ConsoleEmuStarted.at(console)) {
        char msg[100];
        sprintf_s(msg, 100, "EMULATION DEVELOPMENT OF THE %s HAS NOT STARTED YET; PLEASE CHECK BACK LATER", ConsoleNamesShort.at(console).c_str());
        ImGui::TextWrappedCentered(msg);
    } else {
        char msg[50];
        sprintf_s(msg, 50, "NO %s GAMES FOUND", ConsoleNamesShort.at(console).c_str());
        ImGui::TextWrappedCentered(msg);
    }


    EndGUI(MenuType::MAIN);
}

void VideoManager::RenderGameView(bool dbg) {
    float tgtAspect = (float)dispInfo.gameWidth / (float)dispInfo.gameHeight;
    float lglAspect = (float)dispInfo.dispWidth / (float)dispInfo.dispHeight;

    SDL_FRect dest = {};

    float scale1 = 4.0f / 7.0f;
    float scale2 = 3.0f / 7.0f;

    if (dbg) {
        lglAspect *= scale2;
    }

    if (lglAspect > tgtAspect) {
        // pillarbox
        dest.h = (float)dispInfo.dispHeight;
        dest.w = dest.h * tgtAspect;
        if (dbg) {
            dest.h *= scale2;
            dest.w *= scale2;
        }
        dest.x = dbg ? 0.0f : (dispInfo.dispWidth - dest.w) / 2.0f;
        dest.y = 0.0f;
    } else {
        // letterbox
        dest.w = (float)dispInfo.dispWidth;
        dest.h = dest.w / tgtAspect;
        if (dbg) {
            dest.h *= scale2;
            dest.w *= scale2;
        }
        dest.x = 0.0f;
        dest.y = (dispInfo.dispHeight - dest.h) / 2.0f;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    const u32* buffer = proteus->GetFrameBuffer();
    void* pixels;
    int pitch;
    SDL_LockTexture(gameTexture, NULL, &pixels, &pitch);
    for (u32 y = 0; y < dispInfo.gameHeight; y++) {
        memcpy(
            (u8*)pixels + y * pitch,
            (u8*)buffer + y * (dispInfo.gameWidth * sizeof(u32)),
            dispInfo.gameWidth * sizeof(u32)
        );
    }
    SDL_UnlockTexture(gameTexture);
    SDL_RenderTexture(renderer, gameTexture, NULL, &dest);

    if (dbg) {
        RenderDebug();
    }

    SDL_RenderPresent(renderer);
}

void VideoManager::RenderOverlay() {
    StartGUI(MenuType::OVERLAY, "OVERLAY");

    ImGui::Text("THIS IS THE OVERLAY");

    EndGUI(MenuType::OVERLAY);
}

void VideoManager::OnMouseScroll(s32 dir) {
    // TODO: Handle debug input (maybe)
    const AppState state = proteus->GetState();
    if (dir == 1) {
        PageLeft();
    } else if (dir == -1) {
        PageRight();
    }
}

float VideoManager::GetFontSize(ImVec2 space, const float& base) const {
    float r = space.x / base;

    return fontSize * r * 0.75f * dispInfo.scale;
}

void VideoManager::RenderDebug() {
    StartGUI(MenuType::DEBUG, "DEBUG MENU");

    ImGui::Text("THIS IS THE DEBUG WINDOW");

    EndGUI(MenuType::DEBUG);
}

void VideoManager::StartGUI(MenuType type, const char* name) {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    vp = ImGui::GetMainViewport();

    switch (type) {
        default:
        case MenuType::MAIN:
            PrepUI(vp);
            ImGui::Begin(name, nullptr, MenuFlags);
            break;
        case MenuType::DEBUG:
            ImGui::Begin(name, &debugActive, DebugWindowFlags);
            break;
        case MenuType::OVERLAY:
            ImGui::Begin(name, &overlayActive, MenuFlags);
            break;
    }
}

void VideoManager::EndGUI(MenuType type) {
    ImGui::End();

    if (type == MenuType::MAIN)
        DeprepUI();

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);

    if (type == MenuType::MAIN) {
        SDL_SetRenderDrawColorFloat(renderer, 0, 0.55f, 0, 1);
        SDL_RenderClear(renderer);
    }

    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);
}