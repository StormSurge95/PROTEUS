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
            RenderGameView(proteus->InDebug());
            break;
    }
    if (overlayActive) RenderOverlay();

    FinalizeFrame(proteus->GetState().currentView != AppView::GAME_VIEW);
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
            proteus->SetState(AppView::GAME_LIST, console);
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
    if (!ConsoleEmuStarted.at(console)) {
        char msg[100];
        sprintf_s(msg, 100, "EMULATION DEVELOPMENT OF THE %s HAS NOT STARTED YET; PLEASE CHECK BACK LATER", ConsoleNamesShort.at(console).c_str());
        ImGui::TextWrappedCentered(msg);
    } else {
        // get list of games
        vector<ROM_DATA> games = proteus->GetGameList(console);
        if (games.size() != 0) { // only attempt to render gamelist buttons if there are actually games in the list
            // disable following ImGui items if the overlay is active
            ImGui::BeginDisabled(overlayActive);
            // calculate button dimensions
            ImVec2 btnSize(vp->WorkSize.x / 5.0f, vp->WorkSize.y / 4.0f);
            // push new font size based on window size
            ImGui::PushFont(nullptr, GetFontSize(btnSize, GameTextWidth));
            // render the gamelist buttons
            for (u16 i = 0; i < 20; i++) {
                string name = FormatDisplayName(games[i].gameName, true);
                if (ImGui::ButtonCentered(name.c_str(), btnSize)) {
                    // if this button is clicked, proceed and attempt to launch the selected ROM
                    proteus->LaunchGame(i);
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
            sprintf_s(msg, 50, "NO %s GAMES FOUND", ConsoleNamesShort.at(console).c_str());
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
        RenderDebug(scale);
    }
}

/* TODO: FIGURE OUT THESE BUGS
 *      BUG 1) When starting the application, console selection menu opens as expected.
 *              When opening the overlay menu from within the console selection, none of
 *              the buttons respond; but the overlay itself still disappears as expected
 *              when it is toggled via button input. No matter what, this bug persists.
 *      BUG 2) When starting the application and proceeding to the game selection menu,
 *              two separate and opposite outcomes are possible:
 *              - When Bug #1 is performed, the overlay menu acts as expected from
 *                within the game selection menu.
 *              - When Bug #1 is NOT performed, the same bug now happens here as well,
 *                and then persists throughout the lifetime of the application.
 */
void VideoManager::RenderOverlay() {
    // prepare viewport for overlay menu
    vp = ImGui::GetMainViewport();
    PrepUI(vp, MenuType::OVERLAY);
    ImGui::Begin("OVERLAY", &overlayActive, ImOverlayFlags);
    AppView view = proteus->GetState().currentView;
    // calculate button size
    float numBtns = view == AppView::GAME_VIEW ? 8.0f : 5.0f;
    ImVec2 btnSize(vp->WorkSize.x, vp->WorkSize.y / numBtns);
    // add buttons to window
    if (ImGui::Button("RESUME", btnSize)) {
        // this button simply turns the overlay back off
        ToggleOverlay();
    }
    if (view == AppView::GAME_VIEW) {
        if (ImGui::Button("RESTART", btnSize)) {
            // this button will reset whatever console is currently running
            // turn overlay back off first to ensure that the console resets properly
            ToggleOverlay();
            proteus->ResetConsole();
        }
        if (ImGui::Button("CLOSE GAME", btnSize)) {
            // this button will close the current ROM and return us to the game list
            // turn overlay back off first to ensure the console shuts down properly
            ToggleOverlay();
            proteus->ShutDownConsole();
        }
        if (ImGui::Button("SAVE STATES", btnSize)) { /// TODO
            // this button will allow us to access the save states created for the
            // currently running ROM.
            // NOTE: THIS DOES NOT LEAD TO GAME SAVES; ONLY EMULATOR SAVE STATES
            // TODO: Maybe from GAME_LIST we should access a list of game saves
            //       similar to the gamecube and playstation consoles' abilities
            //       to browse memory card data?
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
        if (ImGui::BeginMenu("CPU")) {
            if (ImGui::MenuItem("REGISTERS", nullptr, &debugViews.at(DebugView::CPU_REGS_DISASM))) {
                SetDebugView(DebugView::CPU_REGS_DISASM);
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
        ImGui::EndMenuBar();
    }
    switch (currentDebugView) {
        case DebugView::CPU_REGS_DISASM:
            {
                // add table to display cpu registers
                if (ImGui::BeginTable("CPU REGISTERS", 2, ImGuiTableFlags_Borders)) {
                    u8 numRegs;
                    string** cpuState = proteus->Debugger()->GetStateCPU(numRegs);
                    ImGui::TableSetupColumn("REGISTER");
                    ImGui::TableSetupColumn("VALUE");
                    ImGui::TableHeadersRow();
                    for (u8 row = 0; row < numRegs; row++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", cpuState[row][0].c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", cpuState[row][1].c_str());
                        // call delete because each row was created using new
                        delete[] cpuState[row];
                    }
                    // call delete because main pointer was also created using new
                    delete[] cpuState;
                    ImGui::EndTable();
                }
                // add program disassembly
                string* disasm = proteus->Debugger()->GetDisassembly();
                for (u8 line = 0; line < 25; line++) {
                    if (line == 12) ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", disasm[line].c_str());
                    else ImGui::Text("%s", disasm[line].c_str());
                }
                delete[] disasm;
            }
            break;
        case DebugView::CPU_MEMORY:
            // TODO: Add RAM display
            {
                float tgtW = ImGui::GetContentRegionAvail().x;
                float chrW = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, 0.0f, "A").x;
                float curW = chrW * 89;
                float scale = tgtW / curW;
                ImGui::SetWindowFontScale(scale);
                u64 numLines;
                string* lines = proteus->Debugger()->GetStateRAM(numLines);
                for (u8 l = 0; l < numLines; l++) {
                    ImGui::Text("%s", lines[l].c_str());
                }
                delete[] lines;
                ImGui::SetWindowFontScale(1.0f);
            }
            break;
        case DebugView::PPU_REGS:
            {
                if (ImGui::BeginTable("PPU REGISTERS", 2, ImGuiTableFlags_Borders)) {
                    u8 numRegs;
                    string** ppuState = proteus->Debugger()->GetStatePPU(numRegs);
                    ImGui::TableSetupColumn("REGISTER");
                    ImGui::TableSetupColumn("VALUE");
                    ImGui::TableHeadersRow();
                    for (u8 row = 0; row < numRegs; row++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", ppuState[row][0].c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", ppuState[row][1].c_str());
                        delete[] ppuState[row];
                    }
                    delete[] ppuState;
                    ImGui::EndTable();
                }
            }
            break;
        case DebugView::PPU_PATTERNTABLES:
            // TODO: Add display with SELECTABLE palette entries and render pattern tables using whatever palette entry is selected
            
            break;
        case DebugView::PPU_NAMETABLES:
            // TODO: Add current PPU nametables display
            break;
        case DebugView::APU_REGISTERS:
            // TODO: Add APU register display
            break;
        case DebugView::APU_CHANNELS:
            // TODO: Add APU channel display with toggles for (de)activating various channels and waveform graphs for display
            break;
    }
    ImGui::End();

    ImGui::PopFont();
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