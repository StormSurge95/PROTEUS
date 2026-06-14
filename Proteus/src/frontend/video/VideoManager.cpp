#include "./VideoManager.h"
#include "../plugin/PluginManager.h"

#include "../../../resources/FontUI.h"
#include "../../../resources/FontUI_Bold.h"
#include "../../../resources/FontUI_Italic.h"
#include "../../../resources/FontUI_BoldItalic.h"
#include "../../../resources/FontDebug.h"

using namespace NS_Proteus;

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
    ImGuiIO& io = ImGui::GetIO();

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
    fonts.UI = io.Fonts->AddFontFromMemoryCompressedTTF(FontUI_data, FontUI_size);
    fonts.UI_Bold = io.Fonts->AddFontFromMemoryCompressedTTF(FontUI_Bold_data, FontUI_Bold_size);
    fonts.UI_Italic = io.Fonts->AddFontFromMemoryCompressedTTF(FontUI_Italic_data, FontUI_Italic_size);
    fonts.UI_BoldItalic = io.Fonts->AddFontFromMemoryCompressedTTF(FontUI_BoldItalic_data, FontUI_BoldItalic_size);
    fonts.Debug = io.Fonts->AddFontFromMemoryCompressedTTF(FontDebug_data, FontDebug_size);
}

void VideoManager::Deinit() {
    // remove fonts from ImGui
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->RemoveFont(fonts.UI);
    io.Fonts->RemoveFont(fonts.Debug);

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

void VideoManager::OnResize(size_t width, size_t height) {
    dispInfo.dispWidth = (int)width;
    dispInfo.dispHeight = (int)height;
}

void VideoManager::InitGameTexture(string title, size_t width, size_t height, bool canDebug) {
    if (!canDebug) debugActive = false;

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

void VideoManager::PrepUI(ImGuiViewport* vp, MenuType type) {
    // base viewport prep
    PrepViewport(vp);

    // push styles to actually style our UI view
    if (type == MenuType::OVERLAY)
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(200, 200, 200, 0.15f));   // have overlay dim the screen
    else
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));       // have main menu(s) not effect bg color
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
    // start new ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    switch (state.currentView) {
        default:
        case AppView::CONSOLE_SELECT:
            RenderConsoleSelection();
            break;
        case AppView::GAME_LIST:
            RenderGameSelection(state.selectedConsole);
            break;
        case AppView::GAME_VIEW:
            RenderGameView(debugActive);
            break;
    }
    if (overlayActive) RenderOverlay();

    FinalizeFrame(ctx->GetAppState().currentView != AppView::GAME_VIEW);
}

void VideoManager::RenderConsoleSelection() {
    // prep Viewport for Main menu
    vp = ImGui::GetMainViewport();
    PrepUI(vp, MenuType::MAIN);

    // begin console selection menu window
    ImGui::Begin("CONSOLE_SELECTION", nullptr, ImMenuFlags);

    // disable all console selection buttons when overlay menu is active
    ImGui::BeginDisabled(overlayActive);
    // calculate button dimensions
    ImVec2 btnSize(vp->WorkSize.x / 4.0f, vp->WorkSize.y / 3.0f);
    // push new font size based on window size
    ImGui::PushFont(nullptr, GetFontSize(btnSize, ConsoleTextWidth));
    // render the console buttons
    for (u8 i = 0; i < (u8)ConsoleID::TOTAL; i++) {
        ConsoleID console = ConsoleID(i);
        if (ImGui::ButtonCentered(ConsoleNamesShort.at(console).c_str(), btnSize)) {
            // if this button is clicked, proceed to the game selection menu
            ctx->OnConsoleSelected(console);
        }
        // only 4 buttons per row
        if (i % 4 != 3) ImGui::SameLine();
    }
    // return to previous font size
    ImGui::PopFont();
    // allow any following inputs to be enabled (the only inputs drawn
    // after this should be overlay inputs and only if overlay is active)
    ImGui::EndDisabled();
    // end subwindow
    ImGui::End();
    // undo viewport modifications
    DeprepUI();
}

void VideoManager::RenderGameSelection(ConsoleID console) {
    // prep Viewport for Main menu
    vp = ImGui::GetMainViewport();
    PrepUI(vp, MenuType::MAIN);

    // begin game selection menu window
    ImGui::Begin("GAME_SELECTION", nullptr, ImMenuFlags);
    
    // if we haven't even started developing an emulator for `console`,
    // then there's no point in letting the user attempt to start a ROM
    if (!PluginManager::IsConsoleAvailable(console)) {
        char msg[100];
        snprintf(msg, 100, "EMULATION DEVELOPMENT OF THE %s HAS NOT STARTED YET; PLEASE CHECK BACK LATER", ConsoleNamesShort.at(console).c_str());
        ImGui::TextWrappedCentered(msg);
    } else {
        // get list of games
        vector<ROM_DATA> games = ctx->GetGameList(console);
        if (games.size() != 0) { // only attempt to render gamelist buttons if there are actually games in the list
            // disable following ImGui items if the overlay is active
            ImGui::BeginDisabled(overlayActive);
            // calculate button dimensions
            ImVec2 btnSize(vp->WorkSize.x / 5.0f, vp->WorkSize.y / 4.0f);
            // push new font size based on window size
            ImGui::PushFont(nullptr, GetFontSize(btnSize, GameTextWidth));
            // render the gamelist buttons
            for (u16 i = 0; i < games.size(); i++) {
                string name = FormatDisplayName(games[i].gameName, true);
                if (ImGui::ButtonCentered(name.c_str(), btnSize)) {
                    // if this button is clicked, proceed and attempt to launch the selected ROM
                    ctx->LaunchGame(i);
                }
                // only 5 buttons per row
                if (i % 5 != 4) ImGui::SameLine();
            }
            // return to previous font size
            ImGui::PopFont();
            // enable following ImGui items (only items after here should be within overlay menu)
            ImGui::EndDisabled();
        } else { // if there are no games in the list, display a message to let user know we didn't find anything
            char msg[50];
            snprintf(msg, 50, "NO %s GAMES FOUND", ConsoleNamesShort.at(console).c_str());
            ImGui::TextWrappedCentered(msg);
        }
    }

    // end subwindow
    ImGui::End();

    // undo viewport modifications
    DeprepUI();
}

void VideoManager::RenderGameView(bool dbg) {
    float tgtAspect = (float)dispInfo.gameWidth / (float)dispInfo.gameHeight;
    float lglAspect = (float)dispInfo.dispWidth / (float)dispInfo.dispHeight;
    float scale = 1.0f;

    SDL_FRect dest = {};

    if (lglAspect > tgtAspect) {
        // pillarbox
        dest.h = (float)dispInfo.dispHeight;
        dest.w = dest.h * tgtAspect;
        dest.x = dbg ? 0.0f : (dispInfo.dispWidth - dest.w) / 2.0f;
        dest.y = 0.0f;
        scale = (float)dispInfo.dispHeight / (float)dispInfo.gameHeight;
    } else {
        // letterbox
        dest.w = (float)dispInfo.dispWidth;
        dest.h = dest.w / tgtAspect;
        dest.x = 0.0f;
        dest.y = (dispInfo.dispHeight - dest.h) / 2.0f;
        scale = (float)dispInfo.dispWidth / (float)dispInfo.gameWidth;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    const u32* buffer = ctx->GetFrameBuffer();
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
        RenderDebug(scale);
    }
}

void VideoManager::RenderOverlay() {
    // prepare viewport for overlay menu
    vp = ImGui::GetMainViewport();
    PrepUI(vp, MenuType::OVERLAY);
    ImGui::Begin("OVERLAY", &overlayActive, ImOverlayFlags);
    AppView view = ctx->GetAppView();
    // calculate button size
    float numBtns = view == AppView::GAME_VIEW ? 8.0f : 5.0f;
    ImVec2 btnSize(vp->WorkSize.x, vp->WorkSize.y / numBtns);
    // add buttons to window
    if (ImGui::Button("RESUME", btnSize)) {
        // this button simply turns the overlay back off
        ToggleOverlay();
        ctx->Resume();
    }
    if (view == AppView::GAME_VIEW) {
        if (ImGui::Button("RESTART", btnSize)) {
            // this button will reset whatever console is currently running
            // turn overlay back off first to ensure that the console resets properly
            ToggleOverlay();
            ctx->ResetConsole();
        }
        if (ImGui::Button("CLOSE GAME", btnSize)) {
            // this button will close the current ROM and return us to the game list
            // turn overlay back off first to ensure the console shuts down properly
            ToggleOverlay();
            ctx->ShutdownConsole();
        }
        if (ImGui::Button("SAVE STATES", btnSize)) {
            // TODO: have button access list of saved states for currently running title
            //       Maybe from GAME_LIST we should access a list of game saves
            //       similar to the gamecube and playstation consoles' abilities
            //       to browse memory card data?
            // this button will allow us to access the save states created for the
            // currently running ROM.
            // NOTE: THIS DOES NOT LEAD TO GAME SAVES; ONLY EMULATOR SAVE STATES
        }
    }
    if (ImGui::Button("OPTIONS", btnSize)) {
        // during GAME_LIST or GAME_VIEW, this leads to console-specific options
        // during CONSOLE_SELECT, this leads to global application options
    }
    if (ImGui::Button("CONTROLS", btnSize)) {
        // during CONSOLE_SELECT or GAME_LIST, this leads to global application controls
        // during GAME_VIEW, this leads to console-specific control binding options
    }
    if (ImGui::Button("CHEATS", btnSize)) {
        // this leads to cheat list/save/create options
        // TODO: should this depend on view in some way?
        //      maybe during GAME_VIEW we lead to specific game cheats
        //      and otherwise we go to the broader application cheat options?
    }
    if (ImGui::Button("INFORMATION", btnSize)) {
        // this leads to a basic information screen
        // CONSOLE_SELECT: application information/credits
        // GAME_LIST: console information
        // GAME_VIEW: game information
    }
    // end overlay subwindow
    ImGui::End();
    // undo any viewport changes
    DeprepUI();
}

float VideoManager::GetFontSize(ImVec2 space, const float& base) const {
    float r = space.x / base;

    return fontSize * r * 0.75f * dispInfo.scale;
}

void VideoManager::RenderDebug(float scale) {
    vp = ImGui::GetMainViewport();
    float w = dispInfo.gameWidth * scale;
    ImVec2 pos(vp->WorkPos.x + w, vp->WorkPos.y);
    ImGui::SetNextWindowPos(pos);
    ImVec2 size(vp->WorkSize.x - w, vp->WorkSize.y);
    ImGui::SetNextWindowSize(size);

    float fontSize = ImGui::GetStyle().FontSizeBase * 0.5f;
    ImGui::PushFont(fonts.Debug, fontSize);

    ImGui::Begin("DEBUG_MENU", &debugActive, ImDebugFlags);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("VIEW")) {
            if (ImGui::MenuItem("EVENTS", nullptr, &debugViews.at(DebugView::EVENT_VIEWER))) {
                SetDebugView(DebugView::EVENT_VIEWER);
            }
            if (ImGui::BeginMenu("CPU")) {
                if (ImGui::MenuItem("REGISTERS", nullptr, &debugViews.at(DebugView::CPU_REGS))) {
                    SetDebugView(DebugView::CPU_REGS);
                }
                if (ImGui::MenuItem("DISASSEMBLY", nullptr, &debugViews.at(DebugView::CPU_DISASM))) {
                    SetDebugView(DebugView::CPU_DISASM);
                }
                if (ImGui::MenuItem("MEMORY", nullptr, &debugViews.at(DebugView::CPU_MEMORY))) {
                    SetDebugView(DebugView::CPU_MEMORY);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("PPU")) {
                if (ImGui::MenuItem("REGISTERS", nullptr, &debugViews.at(DebugView::PPU_REGS))) {
                    SetDebugView(DebugView::PPU_REGS);
                }
                if (ImGui::MenuItem("PATTERN TABLES", nullptr, &debugViews.at(DebugView::PPU_PATTERNTABLES))) {
                    SetDebugView(DebugView::PPU_PATTERNTABLES);
                }
                if (ImGui::MenuItem("NAMETABLES", nullptr, &debugViews.at(DebugView::PPU_NAMETABLES))) {
                    SetDebugView(DebugView::PPU_NAMETABLES);
                }
                if (ImGui::MenuItem("SPRITES", nullptr, &debugViews.at(DebugView::PPU_SPRITES))) {
                    SetDebugView(DebugView::PPU_SPRITES);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("APU")) {
                if (ImGui::MenuItem("REGISTERS", nullptr, &debugViews.at(DebugView::APU_REGISTERS))) {
                    SetDebugView(DebugView::APU_REGISTERS);
                }
                if (ImGui::MenuItem("CHANNELS", nullptr, &debugViews.at(DebugView::APU_CHANNELS))) {
                    SetDebugView(DebugView::APU_CHANNELS);
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("PAK", nullptr, &debugViews.at(DebugView::PAK_HEADER))) {
                SetDebugView(DebugView::PAK_HEADER);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("OPTIONS")) {
            static bool temp = false;;
            if (ImGui::Checkbox("LOG TO FILE", &temp)) {
                ImGui::OpenPopup("Filepath Select", ImPopupFlags);
            }
            ImGui::SetNextWindowPos(ImVec2(dispInfo.PopupX(), dispInfo.PopupY()), 0, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(dispInfo.PopupW(), dispInfo.PopupH()));
            ImGui::SetNextWindowFocus();
            if (ImGui::BeginPopup("Filepath Select", ImPopupWindowFlags)) {
                ImGui::TextWrapped("Enter the name of the log file to create:");
                static char filepath[256] = "proteus_log.txt";
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputText("##Filepath", filepath, 256);
                ImGui::PopItemWidth();
                float w = ((ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f);
                if (ImGui::Button("Confirm", ImVec2(w, 0))) {
                    // ctx->GetLogger()->SetLogPath(filepath);
                    ctx->GetDebugger()->SetTraceConfig({
                        .enabled = true,
                        .append = false,
                        .flushEveryFrame = true,
                        .flushThresholdRecords = 8192,
                        .mode = DebugTraceMode::ALL,
                        .filePath = filepath
                    });
                    ctx->GetDebugger()->BeginTrace();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(w, 0))) {
                    temp = false;
                    ctx->GetLogger()->DisableFileLogging();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    switch (currentDebugView) {
        case DebugView::EVENT_VIEWER:
            RenderDebugEVT();
            break;
        default:
        case DebugView::CPU_REGS:
            // render current state of CPU registers
            RenderDebugCPU();
            break;
        case DebugView::CPU_DISASM:
            // render disassembled machine code
            RenderDebugDIS();
            break;
        case DebugView::CPU_MEMORY:
            // render current state of RAM
            RenderDebugRAM();
            break;
        case DebugView::PPU_REGS:
            // render current state of PPU registers
            RenderDebugPPU();
            break;
        case DebugView::PPU_PATTERNTABLES:
            RenderDebugPTB();
            break;
        case DebugView::PPU_NAMETABLES:
            RenderDebugNTB();
            break;
        case DebugView::PPU_SPRITES:
            RenderDebugSPR();
            break;
        case DebugView::APU_REGISTERS:
            // render current state of APU registers
            RenderDebugAPU();
            break;
        case DebugView::APU_CHANNELS:
            // TODO: Add APU channel display with toggles for (de)activating various channels and waveform graphs for display
            break;
        case DebugView::PAK_HEADER:
            RenderDebugPAK();
            break;
    }
    ImGui::End();

    ImGui::PopFont();
}

void VideoManager::RenderDebugEVT() {
    // ensure debugger exists
    IDebugger* dbg = ctx->GetDebugger();
    if (!dbg) {
        ImGui::Text("No debugger available.");
        return;
    }

    // get the current event viewer configuration
    EventViewerConfig cfg = dbg->GetEventViewerConfig();

    // refresh button allows immediate taking of new snapshot
    if (ImGui::Button("Refresh")) {
        dbg->TakeEventViewerSnapshot(false);
    }

    // if auto refresh, then we take a new snapshot anyways
    if (cfg.autoRefresh) {
        dbg->TakeEventViewerSnapshot(true);
    }

    // observer for whether we need to change the saved config
    bool changed = false;

    // add checkboxes that affect our observer
    ImGui::SameLine();
    changed |= ImGui::Checkbox("Auto-refresh", &cfg.autoRefresh);
    ImGui::SameLine();
    changed |= ImGui::Checkbox("Show previous frame", &cfg.showPreviousFrame);
    ImGui::SameLine();
    if (ImGui::Button("Filter Events")) {
        ImGui::OpenPopup("Filter Events", ImPopupFlags);
    }
    ImGui::SetNextWindowPos(ImVec2(dispInfo.PopupX(), dispInfo.PopupY()), 0, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(dispInfo.PopupW(), dispInfo.PopupH(1.5f)));
    ImGui::SetNextWindowFocus();
    if (ImGui::BeginPopup("Filter Events", ImPopupWindowFlags)) {
        for (auto& [flag, filter] : cfg.eventFilters) {
            changed |= ImGui::Checkbox(filter.label, &filter.show);
        }
        ImGui::EndPopup();
    }

    // if config changed, update it within the debugger
    if (changed) {
        dbg->SetEventViewerConfig(cfg);
    }
    
    // get current event viewer frame size
    EventViewerDisplaySize evSize = dbg->GetEventViewerDisplaySize();
    // ensure that returned size is valid
    if (evSize.width == 0 || evSize.height == 0) {
        ImGui::Text("Event viewer returned an empty surface.");
        return;
    }

    // ensure texture used for ev frame is valid
    if (evTexture && (evTexture->w != evSize.width || evTexture->h != evSize.height)) {
        SDL_DestroyTexture(evTexture);
        evTexture = nullptr;
    }
    if (!evTexture) {
        evTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, evSize.width, evSize.height);
    }

    // get the ev pixels constructed by the debugger
    vector<u32> evPixels = dbg->GetEventViewerPixels();
    // ensure that a first snapshot has been taken without forcing user interaction
    if (evPixels.size() != size_t(evSize.width) * size_t(evSize.height)) {
        dbg->TakeEventViewerSnapshot(false);
        evPixels = dbg->GetEventViewerPixels();
    }
    // copy retrieved pixels into our ev texture
    void* pixels;
    int pitch;
    SDL_LockTexture(evTexture, nullptr, &pixels, &pitch);
    for (u32 y = 0; y < evSize.height; y++) {
        memcpy(
            reinterpret_cast<u8*>(pixels) + y * pitch,
            reinterpret_cast<u8*>(evPixels.data()) + y * (evSize.width * sizeof(u32)),
            evSize.width * sizeof(u32)
        );
    }
    SDL_UnlockTexture(evTexture);

    // calculate canvas size
    float w = ImGui::GetContentRegionAvail().x;
    float ratio = evSize.width / w;
    ImVec2 canvas(w, evSize.height / ratio);

    ImGui::InvisibleButton("EVENT_VIEWER_CANVAS", canvas);

    ImVec2 canvasMin = ImGui::GetItemRectMin();
    ImVec2 canvasMax = ImGui::GetItemRectMax();
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left) && hovered;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(canvasMin, canvasMax, 0xFF000000);

    float scale = min(canvas.x / evSize.width, canvas.y / evSize.height);

    ImVec2 gameRectMin(canvasMin.x, canvasMin.y);
    ImVec2 gameRectMax(
        canvasMin.x + (float(dispInfo.gameWidth) / float(evSize.width)) * canvas.x,
        canvasMin.y + (float(dispInfo.gameHeight) / float(evSize.height)) * canvas.y
    );

    dl->AddImage(gameTexture, gameRectMin, gameRectMax);
    dl->AddImage(evTexture, canvasMin, canvasMax);

    // create render after-effect so that user knows exactly which event pixel they are hovering over
    if (hovered) {
        // hide cursor so that user can more easily see which pixel they are about to click
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);

        // get various position related values from ImGui
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 imgMin = ImGui::GetItemRectMin();
        ImVec2 imgMax = ImGui::GetItemRectMax();

        // calculate other position values based on collected values
        ImVec2 localPos(mousePos.x - imgMin.x, mousePos.y - imgMin.y);
        ImVec2 uvPos(std::clamp(localPos.x / canvas.x, 0.0f, 0.999999f), std::clamp(localPos.y / canvas.y, 0.0f, 0.999999f));

        // calculate the appropriate cycle/scanline values for the pixel being hovered over
        u32 cycle = u32(std::floor(uvPos.x * evSize.width));
        u32 scanline = u32(std::floor(uvPos.y * evSize.height));

        DebugEventRecord event = dbg->GetEventAt(scanline, cycle);

        // determine the on-screen bounds of the in-emulator pixel
        ImVec2 cellMin(
            imgMin.x + (float(cycle) / float(evSize.width)) * canvas.x,
            imgMin.y + (float(scanline) / float(evSize.height)) * canvas.y
        );
        ImVec2 cellMax(
            imgMin.x + (float(cycle + 1) / float(evSize.width)) * canvas.x,
            imgMin.y + (float(scanline + 1) / float(evSize.height)) * canvas.y
        );

        // draw crosshairs that form a square around the pixel in question
        dl->AddLineH(imgMin.x, imgMax.x, cellMin.y, 0xFFFFFFFF);
        dl->AddLineH(imgMin.x, imgMax.x, cellMax.y, 0xFFFFFFFF);
        dl->AddLineV(cellMin.x, imgMin.y, imgMax.y, 0xFFFFFFFF);
        dl->AddLineV(cellMax.x, imgMin.y, imgMax.y, 0xFFFFFFFF);

        if (ImGui::BeginTooltip()) {
            if (event.isNull()) {
                ImGui::Text("%s", "NO EVENT");
                ImGui::Text("(%d, %d)", cycle, scanline);
                ImGui::Spacing();
                ImGui::Text("No event at this position");
            } else {
                ImGui::Text("%s", event.type.c_str());
                ImGui::Text("(%d, %d)", cycle, scanline);
                ImGui::Spacing();
                if (event.hasAddress()) ImGui::Text("Address: %04X", event.address);
                if (event.hasValue()) ImGui::Text("Value: %02X", event.value);
            }
            ImGui::EndTooltip();
        }

        // manage click on event pixel
        if (clicked) {
            if (!event.isNull()) {
                evSelectedScanline = scanline;
                evSelectedCycle = cycle;
            }
        }
    }

    // render event list via table structure
    vector<DebugEventRecord> evr = dbg->GetEventViewerEvents();
    if (ImGui::BeginChild("EVENT TABLE", ImGui::GetContentRegionAvail())) {
        if (ImGui::BeginTable("EVENT RECORDS", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Scanline");
            ImGui::TableSetupColumn("Dot/Pixel");
            ImGui::TableSetupColumn("Address");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("Details");
            ImGui::TableHeadersRow();
            // ImGuiListClipper clipper;
            // clipper.Begin(evr.size());
            // vector<DebugEventRecord>::reverse_iterator it = evr.rbegin();
            // while (clipper.Step()) {
            //     for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            //         DebugEventRecord& event = *it;
            //         ImGui::TableNextRow();
            //         u32 color = LerpColor(WithAlpha(event.color, 96), WithAlpha(0xFFFFFFFF, 96), 0.25);
            //         if (event.scanline == evSelectedScanline && event.cycle == evSelectedCycle) {
            //             color = Brighten(color, 0.5f); // TODO: figure out why this completely changes the color
            //         }
            //         ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, color);
            //         ImGui::TableNextColumn();
            //         ImGui::Text("%s", event.type.c_str());
            //         ImGui::TableNextColumn();
            //         ImGui::TextAligned(0.5f, ImGui::GetColumnWidth(), "%d", event.scanline);
            //         ImGui::TableNextColumn();
            //         ImGui::TextAligned(0.5f, ImGui::GetColumnWidth(), "%d", event.cycle);
            //         ImGui::TableNextColumn();
            //         ImGui::TextAligned(0.5f, ImGui::GetColumnWidth(), "%04X", event.address);
            //         ImGui::TableNextColumn();
            //         ImGui::TextAligned(0.5f, ImGui::GetColumnWidth(), "%02X", event.value);
            //         ImGui::TableNextColumn();
            //         ImGui::Text("%s", event.details.c_str());
            //         it++;
            //     }
            // }
            for (const DebugEventRecord& event : evr | std::views::reverse) {
                ImGui::TableNextRow();
                u32 color = LerpColor(WithAlpha(event.color, 96), WithAlpha(0xFFFFFFFF, 96), 0.25);
                if (event.scanline == evSelectedScanline && event.cycle == evSelectedCycle) {
                    color = Brighten(color, 0.5f); // TODO: figure out why this completely changes the color
                }
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, color);
                ImGui::TableNextColumn();
                ImGui::Text("%s", event.type.c_str());
                ImGui::TableNextColumn();
                ImGui::TextAligned(0.5f, ImGui::GetColumnWidth(), "%d", event.scanline);
                ImGui::TableNextColumn();
                ImGui::TextAligned(0.5f, ImGui::GetColumnWidth(), "%d", event.cycle);
                ImGui::TableNextColumn();
                ImGui::TextAligned(0.5f, ImGui::GetColumnWidth(), "%04X", event.address);
                ImGui::TableNextColumn();
                ImGui::TextAligned(0.5f, ImGui::GetColumnWidth(), "%02X", event.value);
                ImGui::TableNextColumn();
                ImGui::Text("%s", event.details.c_str());
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}

void VideoManager::RenderDebugCPU() {
    if (ImGui::BeginTable("CPU STATE", 3, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Value (Hex)");
        ImGui::TableHeadersRow();
        vector<array<string, 3>> cpuState = ctx->GetDebugger()->GetStateCPU();
        for (int i = 0; i < cpuState.size(); i++) {
            ImGui::TableNextRow();
            if (memcmp(cpuState[i][0].data(), "", 1) == 0)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.25)));
            ImGui::TableNextColumn();
            ImGui::Text("%s", cpuState[i][0].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", cpuState[i][1].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", cpuState[i][2].c_str());
        }
        ImGui::EndTable();
    }
}

void VideoManager::RenderDebugDIS() {
    vector<string> dis = ctx->GetDebugger()->GetDisassembly();
    for (u8 line = 0; line < dis.size(); line++) {
        if (line == dis.size() / 2)
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", dis[line].c_str());
        else
            ImGui::Text("%s", dis[line].c_str());
    }
}

void VideoManager::RenderDebugRAM() {
    float tgtW = ImGui::GetContentRegionAvail().x;
    float chrW = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, 0.0f, "A").x;
    float curW = chrW * 89;
    float scale = tgtW / curW;
    ImGui::SetWindowFontScale(scale);
    vector<string> lines = ctx->GetDebugger()->GetStateRAM();
    for (const string& line : lines) {
        ImGui::Text("%s", line.c_str());
    }
    ImGui::SetWindowFontScale(1.0f);
}

void VideoManager::RenderDebugPPU() {
    if (ImGui::BeginTable("PPU STATE", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Value (Hex)");
        ImGui::TableHeadersRow();
        vector<array<string, 4>> ppuState = ctx->GetDebugger()->GetStatePPU();
        for (int i = 0; i < ppuState.size(); i++) {
            ImGui::TableNextRow();
            if (memcmp(ppuState[i][0].data(), "", 1) == 0)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.25)));
            ImGui::TableNextColumn();
            ImGui::Text("%s", ppuState[i][0].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", ppuState[i][1].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", ppuState[i][2].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", ppuState[i][3].c_str());
        }
        ImGui::EndTable();
    }
}

void VideoManager::RenderPaletteSelector(const vector<u32>& colors, int paletteIndex, float itemWidth, u8 colorsPerPalette) {
    constexpr float aspect = 86.0f / 26.0f;

    float itemHeight = itemWidth / aspect;
    
    float padding = itemHeight * (4.0f / 26.0f);
    float gap = itemWidth * (2.0f / 86.0f);
    if (gap < 1.0f) gap = 1.0f;
    
    ImVec2 swatchSize(
        (itemWidth - (padding * 2.0f) - (gap * 3.0f)) / 4.0f,
        itemHeight - (padding * 2.0f)
    );

    ImDrawList* dl = ImGui::GetWindowDrawList();
    bool changed = false;

    ImGui::PushID(paletteIndex);

    ImVec2 itemMin = ImGui::GetCursorScreenPos();
    ImVec2 itemSize(itemWidth, itemHeight);

    if (ImGui::InvisibleButton("palette", itemSize)) {
        if (selectedPalette != paletteIndex) {
            selectedPalette = paletteIndex;
            patternTablesDirty = true;
        }
    }

    bool hovered = ImGui::IsItemHovered();
    bool selected = (selectedPalette == paletteIndex);

    ImVec2 itemMax(itemMin.x + itemWidth, itemMin.y + itemHeight);

    if (hovered) dl->AddRectFilled(itemMin, itemMax, IM_COL32(255, 255, 255, 12), 3.0f);

    int base = paletteIndex * colorsPerPalette;

    float x = itemMin.x + padding;
    float y = itemMin.y + padding;

    for (int i = 0; i < colorsPerPalette; i++) {
        ImVec2 swatchMin(x, y);
        ImVec2 swatchMax(x + swatchSize.x, y + swatchSize.y);

        dl->AddRectFilled(swatchMin, swatchMax, colors[base + i]);
        dl->AddRect(swatchMin, swatchMax, IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);

        x += swatchSize.x + gap;
    }

    ImVec2 groupMin(itemMin.x + padding, itemMin.y + padding);
    ImVec2 groupMax(itemMax.x - padding, itemMax.y - padding);

    if (selected) {
        dl->AddRect(groupMin, groupMax, IM_COL32(255, 220, 80, 255), 3.0f, 0, 2.0f);
    } else if (hovered) {
        dl->AddRect(groupMin, groupMax, IM_COL32(200, 200, 200, 100), 3.0f, 0, 1.0f);
    }

    ImGui::PopID();
}

void VideoManager::RenderDebugPTB() {
    // get palette data
    PaletteData pal = ctx->GetDebugger()->GetPaletteData();

    float w = ImGui::GetContentRegionAvail().x / 4.0f;

    // render the selectable palette entries
    for (u8 i = 0; i < pal.paletteCount; i++) {
        RenderPaletteSelector(pal.colors, i, w, pal.colorsPerPalette);
        if (i % 4 != 3) ImGui::SameLine(0.0f, 0.0f);
    }

    if (patternTablesDirty) {
        for (u8 i = 0; i < 2; i++) {
            vector<u32> pixels = ctx->GetDebugger()->GetPatternTable(i, selectedPalette);

            if (patternTableTextures[i] == nullptr) {
                patternTableTextures[i] = SDL_CreateTexture(
                    renderer, SDL_PIXELFORMAT_ABGR8888,
                    SDL_TEXTUREACCESS_STREAMING, 128, 128
                );
            }

            void* dots;
            int pitch;
            SDL_LockTexture(patternTableTextures[i], nullptr, &dots, &pitch);
            for (u32 y = 0; y < 128; y++) {
                memcpy(
                    reinterpret_cast<u8*>(dots) + y * pitch,
                    reinterpret_cast<u8*>(pixels.data()) + y * (128 * sizeof(u32)),
                    128 * sizeof(u32)
                );
            }
            SDL_UnlockTexture(patternTableTextures[i]);
        }
        patternTablesDirty = false;
    }

    float availH = ImGui::GetContentRegionAvail().y;
    float space = ImGui::GetStyle().ItemSpacing.y;
    float imgHeight = (availH - space) / 2.0f;
    float imgWidth = imgHeight;

    ImGui::Image(patternTableTextures[0], ImVec2(imgWidth, imgHeight));
    ImGui::Image(patternTableTextures[1], ImVec2(imgWidth, imgHeight));
}

void VideoManager::RenderDebugNTB() {
    // update our nametable textures
    for (u8 i = 0; i < 4; i++) {
        vector<u32> pixels = ctx->GetDebugger()->GetNameTable(i);

        if (nametableTextures[i] == nullptr) {
            nametableTextures[i] = SDL_CreateTexture(
                renderer, SDL_PIXELFORMAT_ABGR8888,
                SDL_TEXTUREACCESS_STREAMING, 256, 240
            );
        }

        void* dots;
        int pitch;
        SDL_LockTexture(nametableTextures[i], nullptr, &dots, &pitch);
        for (u32 y = 0; y < 240; y++) {
            memcpy(
                reinterpret_cast<u8*>(dots) + y * pitch,
                reinterpret_cast<u8*>(pixels.data()) + y * (256 * sizeof(u32)),
                256 * sizeof(u32)
            );
        }
        SDL_UnlockTexture(nametableTextures[i]);
    }

    float aspect = 256.0f / 240.0f;

    float space = ImGui::GetStyle().ItemSpacing.x;
    
    // this allows us to have a total of four nametables visible
    float imgWidth = (ImGui::GetContentRegionAvail().x - space) / 2.0f;
    float imgHeight = imgWidth / aspect;

    ImGui::Image(nametableTextures[0], ImVec2(imgWidth, imgHeight));
    ImGui::SameLine(0.0f, space);
    ImGui::Image(nametableTextures[1], ImVec2(imgWidth, imgHeight));
    ImGui::Image(nametableTextures[2], ImVec2(imgWidth, imgHeight));
    ImGui::SameLine(0.0f, space);
    ImGui::Image(nametableTextures[3], ImVec2(imgWidth, imgHeight));
}

void VideoManager::RenderDebugAPU() {
    if (ImGui::BeginTable("APU STATE", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Value (Hex)");
        ImGui::TableHeadersRow();
        vector<array<string, 4>> apuState = ctx->GetDebugger()->GetStateAPU();
        for (int i = 0; i < apuState.size(); i++) {
            ImGui::TableNextRow();
            if (memcmp(apuState[i][0].data(), "", 1) == 0)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(1, 1, 1, 0.25)));
            ImGui::TableNextColumn();
            ImGui::Text("%s", apuState[i][0].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", apuState[i][1].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", apuState[i][2].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", apuState[i][3].c_str());
        }
        ImGui::EndTable();
    }
}

void VideoManager::FinalizeFrame(bool clear) {
    // finalize ImGui draw data
    ImGui::Render();
    // set render scale based on ImGui IO
    ImGuiIO& io = ImGui::GetIO();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    
    if (clear) {
        // clear screen to a nice shade of green
        SDL_SetRenderDrawColor(renderer, 0, 188, 0, 1);
        SDL_RenderClear(renderer);
    }

    // submit ImGui draw data to renderer and render to screen
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

void VideoManager::SetDebugView(DebugView view) {
    currentDebugView = view;
    for (const pair<DebugView, bool>& p : debugViews) {
        debugViews.at(p.first) = p.first == view;
    }
}

void VideoManager::RenderDebugPAK() {
    if (ImGui::CollapsingHeader("GAMEPAK INFO")) {
        if (ImGui::BeginTable("GAMEPAK INFO", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableHeader("Gamepak Data");
            ImGui::TableSetupColumn("Data");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
            vector<array<string, 2>> pakData = ctx->GetDebugger()->GetPakHeader();
            for (size_t i = 0; i < pakData.size(); i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", pakData[i][0].c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", pakData[i][1].c_str());
            }
            ImGui::EndTable();
        }
    }
    ImGui::Separator();
    if (ImGui::CollapsingHeader("MAPPER INFO")) {
        if (ImGui::BeginTable("MAPPER INFO", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableHeader("Mapper Data");
            ImGui::TableSetupColumn("Data");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
            vector<array<string, 2>> mapData = ctx->GetDebugger()->GetPakMapper();
            for (size_t i = 0; i < mapData.size(); i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", mapData[i][0].c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", mapData[i][1].c_str());
            }
            ImGui::EndTable();
        }
    }
}

void VideoManager::RenderDebugSPR() {
    const vector<u32> pixels = ctx->GetDebugger()->GetSprites();

    const int atlW = 64;
    const int atlH = int(pixels.size() / atlW);
    if (atlH <= 0) return;

    if (sprTexture == nullptr || sprTextureHeight != atlH) {
        if (sprTexture) {
            SDL_DestroyTexture(sprTexture);
            sprTexture = nullptr;
        }

        sprTexture = SDL_CreateTexture(
            renderer, SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING, atlW, atlH
        );
        sprTextureHeight = (u16)atlH;
    }

    void* texels = nullptr;
    int pitch = 0;
    SDL_LockTexture(sprTexture, nullptr, &texels, &pitch);
    for (int y = 0; y < atlH; y++) {
        memcpy(
            reinterpret_cast<u8*>(texels) + y * pitch,
            reinterpret_cast<const u8*>(pixels.data()) + y * atlW * sizeof(u32),
            atlW * sizeof(u32)
        );
    }
    SDL_UnlockTexture(sprTexture);

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    float scale = floorf(min(avail.x / float(atlW), avail.y / float(atlH)));
    if (scale < 1.0f) scale = 1.0f;

    ImVec2 topLeft = ImGui::GetCursorScreenPos();
    ImVec2 itemMin(topLeft.x, topLeft.y);
    int sW = 8 * scale;
    int sH = (atlH / 8) * scale;

    ImGui::Image(sprTexture, ImVec2(atlW * scale, atlH * scale));

    ImDrawList* dl = ImGui::GetWindowDrawList();
    for (int i = 0; i < 64; i++) {
        ImVec2 itemMax(itemMin.x + sW, itemMin.y + sH);
        dl->AddRect(itemMin, itemMax, IM_COL32(255, 255, 255, 255));
        dl->AddText(itemMin, IM_COL32(255, 255, 255, 255), to_string(i).c_str());
        if (i % 8 == 7) {
            itemMin.x = topLeft.x;
            itemMin.y += sH;
        } else itemMin.x += sW;
    }
}