#include "./Proteus.h"
#include "FrontendPCH.h"
#include "../../resources/NES_DB.h"
#include "../backend/NES/NES.h"
#include "./AudioManager.h"
#include "./InputManager.h"
#include "./VideoManager.h"
#include "./DebugManager.h"
#include "./RomLibrary.h"

#include <openssl/evp.h>
#include <openssl/md5.h>

using namespace NS_Proteus;

Proteus::Proteus() {
    videoManager = std::make_shared<VideoManager>(this);
    inputManager = std::make_shared<InputManager>(this);
    audioManager = std::make_shared<AudioManager>(this);
    debugManager = std::make_shared<DebugManager>();

    lib = make_unique<RomLibrary>();
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
        ProcessEvents();

        if (SDL_GetWindowFlags(videoManager->GetWindow()) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        videoManager->Render(state);
        if (state.currentView == AppView::GAME_VIEW && !videoManager->OverlayActive()) {
            inputManager->TranslateInputs(station, state.selectedConsole);
            if (debugManager->GetDebugger()->logToFile) debugManager->GetDebugger()->LogTrace();
            if (!dbgPause) station->clock();
        }
        audioManager->Update(station);
    }
}

void Proteus::RunSST() {
    station = make_shared<NES_NS::NES>();
    for (u16 i = 0; i <= 0xFF; i++) {
        printf("Instruction 0x%02x...", i);
        // get sst data from json file
        ifstream f(format("C:\\devenv\\SSTs\\NES\\{}.json", hex(i, 2)));
        json data = json::parse(f);
        f.close();
        // convert json object data into a format more usable by our program
        vector<SSTtest> SST;
        for (int i = 0; i < data.size(); i++)
            SST.push_back(SSTtest(data[i]));
        // run our tests
        for (const SSTtest& test : SST) {
            station->initSST(test.initState);
            station->runSST();
            string result;
            bool pass = station->checkSST(test.finalState, result);
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
    debug = !debug;
    if (station != nullptr) {
        videoManager->ToggleDebug();
    }
}

void Proteus::ProcessEvents() {
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
                //if (static_cast<MouseButton>(event.button.button) == MouseButton::LEFT)
                //    videoManager->OnSelect();
                if (static_cast<MouseButton>(event.button.button) == MouseButton::RIGHT) {
                    if (state.currentView == AppView::GAME_LIST && !videoManager->OverlayActive())
                        SetState(AppView::CONSOLE_SELECT);
                    else if (videoManager->OverlayActive()) videoManager->ToggleOverlay();
                }
                else if (static_cast<MouseButton>(event.button.button) == MouseButton::MIDDLE)
                    videoManager->ToggleOverlay();
                break;
        }
    }
}

void Proteus::ProcessKeyInput(SDL_Keycode key) {
    switch (key) {
        case SDLK_F5:
            // TODO: backstep/rewind
            break;
        case SDLK_F6:
            dbgPause = !dbgPause;
            break;
        case SDLK_F7:
            if (dbgPause) {
                if ((SDL_GetModState() & SDL_KMOD_SHIFT) != 0)
                    debugManager->GetDebugger()->StepCycle();
                else
                    debugManager->GetDebugger()->StepInstruction();
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
            if (state.currentView == AppView::GAME_VIEW)
                videoManager->ToggleOverlay();
            break;
        case SDL_GAMEPAD_BUTTON_EAST:
            if (state.currentView == AppView::GAME_LIST && !videoManager->OverlayActive())
                SetState(AppView::CONSOLE_SELECT);
            else if (videoManager->OverlayActive()) videoManager->ToggleOverlay();
            break;
    }
}

std::vector<ROM_DATA> Proteus::GetGameList(ConsoleID console) {
    return lib->GetGameList(console);
}

void Proteus::LaunchGame(int index) {
    StartConsole();

    ROM_DATA game = lib->GetGameList(state.selectedConsole)[index];

    state.selectedGame = game.gameName;
    std::string path = game.path;

    std::string title = "PROTEUS: " + game.gameName;

    videoManager->InitGameTexture(title, station->SCREEN_WIDTH(), station->SCREEN_HEIGHT());

    if (station->loadROM(path)) {
        state.currentView = AppView::GAME_VIEW;
        ROMactive = true;
    } else {
        ROMactive = false;
    }
}

void Proteus::StartConsole() {
    switch (state.selectedConsole) {
        case ConsoleID::NES:
            station = std::make_shared<NES_NS::NES>();
            debugManager->SetDebugger(ConsoleID::NES, station);
            return;
        default:
            exit(EXIT_FAILURE);
    }
}

void Proteus::ResetConsole() {
    station->reset();
}

void Proteus::ShutDownConsole() {
    station.reset();
    ROMactive = false;
    SetState(AppView::GAME_LIST, state.selectedConsole);
}

const u32* Proteus::GetFrameBuffer() {
    if (station.get() != nullptr) {
        return station->getFrameBuffer();
    }
    return nullptr;
}