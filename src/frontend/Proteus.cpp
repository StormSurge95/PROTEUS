#include "./Proteus.h"

#include "FrontendPCH.h"

#include "../../resources/NES_DB.h"
#include "../backend/NES/NES.h"

#include "./AudioManager.h"
#include "./InputManager.h"
#include "./VideoManager.h"
#include "./DebugManager.h"

#include <fstream>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <SDL3/SDL.h>

using namespace NS_Proteus;

Proteus::Proteus() {
    videoManager = std::make_shared<VideoManager>(this);
    inputManager = std::make_shared<InputManager>(this);
    audioManager = std::make_shared<AudioManager>(this);
    debugManager = std::make_shared<DebugManager>();
}

Proteus::~Proteus() {
    videoManager.reset();
    audioManager.reset();
    inputManager.reset();
}

void Proteus::Init() {
    SetMetadata();

    IdentifyROMs();

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

        //ProcessInputs();
        videoManager->Render(state);
        if (state.currentView == AppView::GAME_VIEW && !videoManager->OverlayActive()) {
            inputManager->TranslateInputs(station, state.selectedConsole);
            if (!debug || !dbgPause) station->clock();
        }
        audioManager->Update(station);
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
        if (debug) {
            debugManager->SetDebugger(ConsoleID::NES, station);
            debugManager->CycleDebugViews();
        } else debugManager->CycleDebugViews(false);
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
        case SDLK_TAB:
            debugManager->CycleDebugViews();
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

std::string Proteus::MD5(const std::string& filepath) {
    unsigned char result[MD5_DIGEST_LENGTH];
    std::ifstream file(filepath, std::ios::binary);

    if (!file.is_open()) {
        SDL_Log("Failed to open %s file for hashing!", filepath.c_str());
        exit(EXIT_FAILURE);
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);

    char buffer[4096];
    file.seekg(16, file.beg);
    while (file.read(buffer, sizeof(buffer))) {
        EVP_DigestUpdate(ctx, buffer, (size_t)file.gcount());
    }
    EVP_DigestUpdate(ctx, buffer, (size_t)file.gcount());

    unsigned int len = 0;
    EVP_DigestFinal_ex(ctx, result, &len);

    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];

    return ss.str();
}

void Proteus::IdentifyROMs() {
    const path base = "C:/ROMS/";
    if (!exists(base)) create_directory(base);

    for (const pair<ConsoleID, string>& p : ConsoleNamesShort) {
        string console = p.second;
        if (console != "NES") continue;
        path path = base.string() + console + "/";
        if (!exists(path)) {
            create_directory(path);
            continue;
        }
        vector<ROM_DATA> games = {};
        for (const auto& entry : directory_iterator(path)) {
            // get text data
            string file = entry.path().string();
            string filename = file.substr(file.find_last_of('/') + 1);
            
            // get hash
            string hash = MD5(file);

            string gameName = Lookup(console, hash);
            if (gameName == "Unknown") gameName = filename.substr(0, filename.length() - 4);
            games.push_back({ .gameName = gameName, .path = file });
        }
        gameList[p.first] = games;
    }
}

std::string Proteus::Lookup(const std::string& console, const std::string& hash) {
    int size = -1;

    int left = 0;
    int right = std::size(NES_ROM_DB) - 1;

    while (left <= right) {
        int mid = (left + right) / 2;
        int cmp = std::strcmp(hash.c_str(), NES_ROM_DB[mid].md5);

        if (cmp == 0)
            return NES_ROM_DB[mid].name;
        
        if (cmp < 0)
            right = mid - 1;
        else
            left = mid + 1;
    }

    return "Unknown";
}

std::vector<ROM_DATA> Proteus::GetGameList(ConsoleID console) {
    return gameList[console];
}

void Proteus::LaunchGame(int index) {
    StartConsole();

    ROM_DATA game = gameList[state.selectedConsole][index];

    state.selectedGame = game.gameName;
    std::string path = game.path;

    std::string title = "PROTEUS: " + game.gameName;

    videoManager->InitGameTexture(title, station->SCREEN_WIDTH(), station->SCREEN_HEIGHT());

    if (station->loadCart(path)) {
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
            if (debug) {
                debugManager->SetDebugger(ConsoleID::NES, station);
                debugManager->CycleDebugViews();
            }
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