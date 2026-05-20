#include "./Proteus.h"
#include "FrontendPCH.h"
#include "../backend/NES/NES.h"
#include "./AudioManager.h"
#include "./InputManager.h"
#include "./VideoManager.h"
#include "./RomLibrary.h"

using namespace NS_Proteus;

Proteus::Proteus() {
    videoManager = std::make_shared<VideoManager>(this);
    inputManager = std::make_shared<InputManager>(this);
    audioManager = std::make_shared<AudioManager>(this);

    lib = make_unique<RomLibrary>();
    session = make_shared<ConsoleSession>();
}

Proteus::~Proteus() {
    videoManager.reset();
    audioManager.reset();
    inputManager.reset();
    lib.reset();
}

void Proteus::Init() {
    SetMetadata();

    videoManager->Init();

    inputManager->Init();

    audioManager->Init();
}

void Proteus::Deinit() {
    // TODO: finish deinit tasks

    videoManager.reset();
    inputManager.reset();

    SDL_Quit();
}

void Proteus::Run() {
    while (!quit) {
        FrameContext ctx = BeginFrame();
        ProcessEvents(ctx);
        ComputeFrameState(ctx);

        if (ctx.state.throttleFrame) {
            SDL_Delay(10);
            EndFrame(ctx);
            continue;
        }

        if (ctx.state.runRender) videoManager->Render(state);

        if (ctx.state.runGameplay) {
            if (!ctx.state.suppressInput)
                inputManager->TranslateInputs(session->GetConsole(), state.selectedConsole);
            // TODO: runtime trace(s)
            session->GetConsole()->clock();
        }
        
        if (ctx.state.runAudio) audioManager->Update(session->GetConsole());

        EndFrame(ctx);
    }
}

FrameContext Proteus::BeginFrame() {
    return {
        .state = {},
        .stats = {
            .frameStart = high_resolution_clock::now()
        },
        .suppressInput = false,
        .quitRequested = false
    };
}

void Proteus::EndFrame(FrameContext& ctx) {
    ctx.stats.frameEnd = high_resolution_clock::now();
    ctx.stats.frameDuration = ctx.stats.frameEnd - ctx.stats.frameStart;

    stats.frameCount++;
    stats.frameTimes.push(ctx.stats.frameDuration.count());
    stats.fps = stats.frameTimes.avg();
}

void Proteus::ComputeFrameState(FrameContext& ctx) {
    ctx.state.windowMinimized = SDL_GetWindowFlags(videoManager->GetWindow()) & SDL_WINDOW_MINIMIZED;
    ctx.state.inGameView = state.currentView == AppView::GAME_VIEW;
    ctx.state.overlayActive = videoManager->OverlayActive();
    ctx.state.sessionRunning = session->GetState() == ConsoleSessionState::RUNNING;
    ctx.state.sessionPaused = session->GetState() == ConsoleSessionState::PAUSED;
    ImGuiIO& io = ImGui::GetIO();
    ctx.state.suppressInput = io.WantCaptureKeyboard || io.WantCaptureMouse;

    ctx.state.runGameplay = ctx.state.inGameView && ctx.state.sessionRunning && !ctx.state.windowMinimized;
    ctx.state.runRender = !ctx.state.windowMinimized;
    ctx.state.runAudio = !ctx.state.windowMinimized;
    ctx.state.throttleFrame = ctx.state.windowMinimized;
}

//void Proteus::RunSST() {
//    station = make_shared<NES_NS::NES>();
//    for (u16 i = 0; i <= 0xFF; i++) {
//        printf("Instruction 0x%02x...", i);
//        // get sst data from json file
//        ifstream f(format("C:\\devenv\\SSTs\\NES\\{}.json", hex(i, 2)));
//        json data = json::parse(f);
//        f.close();
//        // convert json object data into a format more usable by our program
//        vector<SSTtest> SST;
//        for (int i = 0; i < data.size(); i++)
//            SST.push_back(SSTtest(data[i]));
//        // run our tests
//        for (const SSTtest& test : SST) {
//            station->initSST(test.initState);
//            station->runSST();
//            string result;
//            bool pass = station->checkSST(test.finalState, result);
//            if (!pass) {
//                printf("FAIL\n%s\n", result.c_str());
//                exit(EXIT_FAILURE);
//            }
//        }
//        printf("PASS\n");
//    }
//}

void Proteus::SetMetadata() {
    SDL_SetAppMetadata(
        "PROTEUS",
        "0.0.0.1",
        "com.stormsurge.Proteus"
    );
    SDL_SetAppMetadataProperty(
        SDL_PROP_APP_METADATA_CREATOR_STRING,
        "StormSurge Productions, LLC"
    );
    SDL_SetAppMetadataProperty(
        SDL_PROP_APP_METADATA_COPYRIGHT_STRING,
        "Copyright (c) 2026 StormSurge Productions, LLC"
    );
    SDL_SetAppMetadataProperty(
        SDL_PROP_APP_METADATA_URL_STRING,
        "https://www.github.com/StormSurge95/Proteus"
    );
    SDL_SetAppMetadataProperty(
        SDL_PROP_APP_METADATA_TYPE_STRING,
        "application"
    );
}

void Proteus::ToggleDebug() {
    //debug = !debug;
    //if (station != nullptr) {
    //    videoManager->ToggleDebug();
    //}
}

void Proteus::ProcessEvents(FrameContext& ctx) {
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                    quit = true;
                else ProcessKeyInput(event.key.key);
                break;
            case SDL_EVENT_QUIT:
                quit = true;
                break;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (event.window.windowID == SDL_GetWindowID(videoManager->GetWindow())) quit = true;
                break;
            case SDL_EVENT_GAMEPAD_ADDED:
                inputManager->Connect(event.gdevice.which);
                break;
            case SDL_EVENT_GAMEPAD_REMOVED:
                inputManager->Disconnect(event.gdevice.which);
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                videoManager->OnResize(event.window.data1, event.window.data2);
                break;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                ProcessButtonInput(event.gbutton.button);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (static_cast<MouseButton>(event.button.button) == MouseButton::RIGHT) {
                    if (state.currentView == AppView::GAME_LIST && !videoManager->OverlayActive())
                        SetState(AppView::CONSOLE_SELECT);
                    else if (videoManager->OverlayActive()) videoManager->ToggleOverlay();
                }
                else if (static_cast<MouseButton>(event.button.button) == MouseButton::MIDDLE)
                    videoManager->ToggleOverlay();
                break;
        }
        ctx.stats.eventsPolled++;
    }
}

void Proteus::ProcessKeyInput(SDL_Keycode key) {
    switch (key) {
        case SDLK_F5:
            // TODO: backstep/rewind
            break;
        case SDLK_F6:
            if (session->GetState() == ConsoleSessionState::PAUSED)
                session->Start();
            else if (session->GetState() == ConsoleSessionState::RUNNING)
                session->Pause();
            break;
        case SDLK_F7:
            if (session->GetState() == ConsoleSessionState::PAUSED) {
                if ((SDL_GetModState() & SDL_KMOD_SHIFT) != 0)
                    session->GetDebugger()->StepCycle();
                else
                    session->GetDebugger()->StepInstruction();
            }
            break;
        case SDLK_F8:
            ToggleDebug();
            break;
    }
}

void Proteus::ProcessButtonInput(u8 button) {
    switch (button) {
        case SDL_GAMEPAD_BUTTON_GUIDE:
            ToggleOverlay();
            break;
        case SDL_GAMEPAD_BUTTON_EAST:
            if (state.currentView == AppView::GAME_LIST && !videoManager->OverlayActive())
                SetState(AppView::CONSOLE_SELECT);
            else if (videoManager->OverlayActive()) ToggleOverlay();
            break;
    }
}

void Proteus::ToggleOverlay() {
    // TODO: make closing of overlay consume input used to close it rather than allow it to effect game being played

    if (session->GetState() == ConsoleSessionState::RUNNING)
        session->Pause();
    else if (session->GetState() == ConsoleSessionState::PAUSED)
        session->Start();

    videoManager->ToggleOverlay();
}

std::vector<ROM_DATA> Proteus::GetGameList(ConsoleID console) {
    return lib->GetGameList(console);
}

void Proteus::LaunchGame(int index) {
    // start our console session
    SessionResult r = session->CreateSession(state.selectedConsole);

    if (!r.success) return; // TODO: handle failure

    // acquire game data from library
    ROM_DATA game = lib->GetGameList(state.selectedConsole)[index];
    std::string path = game.path;

    // load rom into console
    r = session->LoadROM(game.path, game.gameName);

    if (!r.success) return; // TODO: handle failure

    r = session->Start();

    if (!r.success) return; // TODO: handle failure

    // update app state
    state.selectedGame = game.gameName;

    // use data to initialize game texture and update display
    std::string title = "PROTEUS: " + game.gameName;
    videoManager->InitGameTexture(title, session->GetConsole()->SCREEN_WIDTH(), session->GetConsole()->SCREEN_HEIGHT());
    state.currentView = AppView::GAME_VIEW;
}

const u32* Proteus::GetFrameBuffer() {
    if (session->GetConsole().get() != nullptr) {
        return session->GetConsole()->getFrameBuffer();
    }
    return nullptr;
}

void Proteus::ResetConsole() {
    SessionResult r = session->Reset();
    if (r.success) session->Start();
    // TODO: handle failure
}

void Proteus::ShutDownConsole() {
    SessionResult r = session->Shutdown();
    if (r.success) state.currentView = AppView::GAME_LIST;
    // TODO: handle failure
    // TODO: implement some way of shutting down the entire app
}

void Proteus::Resume() {
    if (session->GetState() == ConsoleSessionState::PAUSED)
        session->Start();
}