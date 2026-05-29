#include "./Proteus.h"
#include "../FrontendPCH.h"
#include "../audio/AudioManager.h"
#include "../input/InputTypes.h"
#include "../input/InputManager.h"
#include "../video/VideoManager.h"
#include "../rom_library/RomLibrary.h"
#include "../logging/Logger.h"
#include "../plugin/PluginManager.h"

using namespace NS_Proteus;

Proteus::Proteus() {
    videoManager = std::make_shared<VideoManager>(this);
    inputManager = std::make_shared<InputManager>(this);
    audioManager = std::make_shared<AudioManager>(this);
    logger = make_shared<Logger>();

    if (!PluginManager::Initialize()) {
        printf(PluginManager::GetLastError().c_str());
    }

    lib = make_unique<RomLibrary>();
    session = make_shared<ConsoleSession>(logger.get());
}

Proteus::~Proteus() {
    videoManager.reset();
    audioManager.reset();
    inputManager.reset();
    lib.reset();
    session->Shutdown();
    session.reset();
    PluginManager::Shutdown();
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
        PhaseInput(ctx);
        PhaseComputeFrameState(ctx);

        if (ctx.state.throttleFrame) {
            PhaseThrottleFrame(ctx);
            EndFrame(ctx);
            continue;
        }

        PhaseEmuDebug(ctx);
        PhaseRender(ctx);
        PhaseAudio(ctx);
        EndFrame(ctx);
    }
}

FrameContext Proteus::BeginFrame() {
    return {
        .sessionState = GetSessionState(),
        .currentConsole = session->CurrentConsoleID(),
        .state = {},
        .stats = {
            .frameStart = high_resolution_clock::now(),
            .frameCount = stats.frameCount
        },
        .suppressInput = false,
        .quitRequested = false
    };
}

void Proteus::EndFrame(FrameContext& ctx) {
    ctx.stats.frameEnd = high_resolution_clock::now();
    ctx.stats.frameDuration = ctx.stats.frameEnd - ctx.stats.frameStart;
    quit = ctx.quitRequested;

    stats.frameCount++;
    stats.frameTimes.push(ctx.stats.frameDuration.count());
    stats.fps = stats.frameTimes.avg();
}

void Proteus::PhaseInput(FrameContext& ctx) {
    logger->EmitPhaseHook(ctx, AppPhaseName::INPUT, AppPhaseStatus::BEGIN);
    ProcessEvents(ctx);
    logger->EmitPhaseHook(ctx, AppPhaseName::INPUT, AppPhaseStatus::END);
}

void Proteus::PhaseComputeFrameState(FrameContext& ctx) {
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

void Proteus::PhaseThrottleFrame(FrameContext& ctx) {
    logger->EmitPhaseHook(ctx, AppPhaseName::THROTTLE, AppPhaseStatus::BEGIN);
    SDL_Delay(10);
    logger->EmitPhaseHook(ctx, AppPhaseName::THROTTLE, AppPhaseStatus::END);
}

void Proteus::PhaseEmuDebug(FrameContext& ctx) {
    if (ctx.state.runGameplay) {
        logger->EmitPhaseHook(ctx, AppPhaseName::EMUDEB, AppPhaseStatus::BEGIN);
        if (!ctx.state.suppressInput)
            inputManager->TranslateInputs(session->GetConsole(), state.selectedConsole);
        // TODO: runtime trace(s)
        session->GetConsole()->clock();
        logger->EmitPhaseHook(ctx, AppPhaseName::EMUDEB, AppPhaseStatus::END);
    } else
        logger->EmitPhaseHook(ctx, AppPhaseName::EMUDEB, AppPhaseStatus::SKIPPED, "EmuDebug phase disabled this frame");
}

void Proteus::PhaseRender(FrameContext& ctx) {
    if (ctx.state.runRender) {
        logger->EmitPhaseHook(ctx, AppPhaseName::RENDER, AppPhaseStatus::BEGIN);
        videoManager->Render(state);
        logger->EmitPhaseHook(ctx, AppPhaseName::RENDER, AppPhaseStatus::END);
    } else
        logger->EmitPhaseHook(ctx, AppPhaseName::RENDER, AppPhaseStatus::SKIPPED, "Render phase disabled this frame");
}

void Proteus::PhaseAudio(FrameContext& ctx) {
    if (ctx.state.runAudio) {
        logger->EmitPhaseHook(ctx, AppPhaseName::AUDIO, AppPhaseStatus::BEGIN);
        audioManager->Update(session->GetConsole());
        logger->EmitPhaseHook(ctx, AppPhaseName::AUDIO, AppPhaseStatus::END);
    } else
        logger->EmitPhaseHook(ctx, AppPhaseName::AUDIO, AppPhaseStatus::SKIPPED, "Audio phase disabled this frame");
}

void Proteus::RunSST() {
    session->CreateSession(ConsoleID::NES);
    for (u16 i = 0; i <= 0xFF; i++) {
        printf("Instruction 0x%02x...", i);
        // get sst data from json file
        ifstream f(format("C:\\devenv\\SSTs\\NES\\{}.json", hex(i, 2)));
        json data = json::parse(f);
        f.close();
        // convert json object data into a format more usable by our program
        vector<SingleStateTest> SST;
        for (int i = 0; i < data.size(); i++)
            SST.push_back(SingleStateTest(data[i]));
        // run our tests
        for (const SingleStateTest& test : SST) {
            session->GetConsole()->initSST(test.initState);
            session->GetConsole()->runSST();
            string result;
            bool pass = session->GetConsole()->checkSST(test.finalState, result);
            if (!pass) {
                printf("FAIL\n%s\n", result.c_str());
                exit(EXIT_FAILURE);
            }
        }
        printf("PASS\n");
    }
}

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
    videoManager->ToggleDebug();
}

bool Proteus::ProcessQuitEvent(FrameContext& ctx) {
    if (ctx.event.type == SDL_EVENT_QUIT ||
        (ctx.event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            ctx.event.window.windowID == SDL_GetWindowID(videoManager->GetWindow())) ||
        (ctx.event.type == SDL_EVENT_KEY_DOWN && ctx.event.key.key == SDLK_ESCAPE)) {
        return ctx.quitRequested = true;
    }
    return false;
}

bool Proteus::ProcessWindowEvent(FrameContext& ctx) {
    if (ctx.event.type == SDL_EVENT_WINDOW_RESIZED) {
        videoManager->OnResize(ctx.event.window.data1, ctx.event.window.data2);
        return true;
    }
    return false;
}

bool Proteus::ProcessKeyboardEvent(FrameContext& ctx) {
    if (ctx.event.type == SDL_EVENT_KEY_DOWN) {
        switch (ctx.event.key.key) {
            case SDLK_F5:
                // TODO: backstep/rewind
                return ctx.suppressInput = true;
            case SDLK_F6:
                if (session->GetState() == ConsoleSessionState::PAUSED)
                    session->Start();
                else if (session->GetState() == ConsoleSessionState::RUNNING)
                    session->Pause();
                return ctx.suppressInput = true;
            case SDLK_F7:
                if (session->GetState() == ConsoleSessionState::PAUSED) {
                    if ((SDL_GetModState() & SDL_KMOD_SHIFT) != 0)
                        session->GetDebugger()->StepCycle();
                    else
                        session->GetDebugger()->StepInstruction();
                }
                return ctx.suppressInput = true;
            case SDLK_F8:
                ToggleDebug();
                return ctx.suppressInput = true;
        }
    }
    return false;
}

bool Proteus::ProcessGamepadEvent(FrameContext& ctx) {
    if (ctx.event.type == SDL_EVENT_GAMEPAD_ADDED) {
        inputManager->Connect(ctx.event.gdevice.which);
        return true;
    } else if (ctx.event.type == SDL_EVENT_GAMEPAD_REMOVED) {
        inputManager->Disconnect(ctx.event.gdevice.which);
        return true;
    } else if (ctx.event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        bool consumed = ProcessButtonInput(ctx.event.gbutton.button);
        ctx.suppressInput |= consumed;
        return consumed;
    }
    return false;
}

bool Proteus::ProcessMouseEvent(FrameContext& ctx) {
    if (ctx.event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (static_cast<MouseButton>(ctx.event.button.button) == MouseButton::RIGHT) {
            if (videoManager->OverlayActive())
                videoManager->ToggleOverlay();
            else if (state.currentView == AppView::GAME_LIST)
                SetState(AppView::CONSOLE_SELECT);
            return true;
        } else if (static_cast<MouseButton>(ctx.event.button.button) == MouseButton::MIDDLE) {
            videoManager->ToggleOverlay();
            return true;
        }
    }
    return false;
}

void Proteus::ProcessEvents(FrameContext& ctx) {
    while (SDL_PollEvent(&ctx.event)) {
        ctx.stats.eventsPolled++;
        ImGui_ImplSDL3_ProcessEvent(&ctx.event);
        if (ProcessQuitEvent(ctx)) continue;
        if (ProcessWindowEvent(ctx)) continue;
        if (ProcessGamepadEvent(ctx)) continue;
        if (ProcessKeyboardEvent(ctx)) continue;
        if (ProcessMouseEvent(ctx)) continue;
    }
}

bool Proteus::ProcessButtonInput(u8 button) {
    switch (button) {
        case SDL_GAMEPAD_BUTTON_GUIDE:
            ToggleOverlay();
            return true;
        case SDL_GAMEPAD_BUTTON_EAST:
            if (state.currentView == AppView::GAME_LIST && !videoManager->OverlayActive())
                SetState(AppView::CONSOLE_SELECT);
            else if (videoManager->OverlayActive()) ToggleOverlay();
            return true;
    }
    return false;
}

void Proteus::ToggleOverlay() {
    // TODO: make closing of overlay consume input used to close it rather than allow it to effect game being played

    session->PlayPause();
    videoManager->ToggleOverlay();
}

const std::vector<ROM_DATA>& Proteus::GetGameList(ConsoleID console) const {
    return lib->GetGameList(console);
}

void Proteus::LaunchGame(u32 index) {
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

const u32* Proteus::GetFrameBuffer() const {
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

void Proteus::ShutdownConsole() {
    SessionResult r = session->Shutdown();
    if (r.success) state.currentView = AppView::GAME_LIST;
    // TODO: handle failure
    // TODO: implement some way of shutting down the entire app
}

void Proteus::Resume() {
    if (session->GetState() == ConsoleSessionState::PAUSED)
        session->Start();
}