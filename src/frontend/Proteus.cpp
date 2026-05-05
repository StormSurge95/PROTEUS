#include "./Proteus.h"

#include "FrontendPCH.h"

#include "../../resources/NES_DB.h"
#include "../backend/NES/NES.h"

#include "./AudioManager.h"
#include "./InputManager.h"
#include "./VideoManager.h"
#include "./DebugManager.h"

#include <algorithm>
#include <fstream>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <SDL3/SDL.h>

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
    videoManager->LoadCaches();

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
        ProcessInputs();
        if (state.currentView == GAME_VIEW) {
            station->clock();
        }
        if (state.currentView != GAME_VIEW || ROMactive) videoManager->Render();
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
            debugManager->SetDebugger(CONSOLE_ID::NES, station);
            debugManager->CycleDebugViews();
        } else debugManager->CycleDebugViews(false);
    }
}

void Proteus::ProcessEvents() {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                    quit = true;
                else if (event.key.key == SDLK_F8)
                    ToggleDebug();
                else if (event.key.key == SDLK_F5) {
                    // TODO: backstep/rewind rom
                } else if (event.key.key == SDLK_F6) {
                    // TODO: pause/resume rom
                } else if (event.key.key == SDLK_F7) {
                    // TODO: fowardstep/fastforward rom
                } else if (event.key.key == SDLK_TAB) {
                    debugManager->CycleDebugViews();
                }
                break;
            case SDL_EVENT_QUIT:
                quit = true;
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
                // TODO: Open overlay menu instead of immediate shutdown
                if (event.gbutton.button == SDL_GAMEPAD_BUTTON_GUIDE && state.currentView == GAME_VIEW)
                    ShutDownConsole(false);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                videoManager->OnMouseMove(event.motion.x, event.motion.y);
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                videoManager->OnMouseScroll(event.wheel.integer_y);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (static_cast<MouseButton>(event.button.button) == MouseButton::LEFT)
                    videoManager->OnSelect();
                else if (static_cast<MouseButton>(event.button.button) == MouseButton::RIGHT)
                    videoManager->OnCancel();
                // TODO: Open overlay menu instead of immediate shutdown
                else if (static_cast<MouseButton>(event.button.button) == MouseButton::MIDDLE && state.currentView == GAME_VIEW)
                    ShutDownConsole(false);
                break;
        }
    }
}

void Proteus::ProcessInputs() {
    switch (GetState().currentView) {
        case CONSOLE_SELECT:
        case GAME_LIST:
            {
                Inputs* i = inputManager->ReadInputs(0, true);
                videoManager->OnInput(i);
            }
            break;
        case GAME_VIEW:
            inputManager->TranslateInputs(station, state.selectedConsole);
            break;
        default:
            SDL_Log("Seriously? What the hell?");
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

    for (const pair<string, string>& p : CONSOLES) {
        if (p.first != "NES") continue;
        path path = base.string() + p.first + "/";
        if (!exists(path)) {
            create_directory(path);
            continue;
        }
        vector<ROM> games = {};
        for (const auto& entry : directory_iterator(path)) {
            // get text data
            string file = entry.path().string();
            string filename = file.substr(file.find_last_of('/') + 1);
            
            // get hash
            string hash = MD5(file);

            string gameName = Lookup(p.first, hash);
            if (gameName == "Unknown") gameName = filename.substr(0, filename.length() - 4);
            games.push_back({ .gameName = gameName, .path = file });
        }

        pair<string, vector<ROM>> p2(p.first, games);
        gameList.insert(p2);
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

std::vector<ROM> Proteus::GetGameList(const std::string& console) {
    return gameList[console];
}

void Proteus::LaunchGame(int index) {
    StartConsole();

    ROM game = gameList[GetConsoleFromID(state.selectedConsole)][index];

    state.selectedGame = game.gameName;
    std::string path = game.path;

    std::string title = "PROTEUS: " + game.gameName;

    videoManager->InitGameTexture(title, station->SCREEN_WIDTH(), station->SCREEN_HEIGHT());

    if (station->loadCart(path)) {
        state.currentView = GAME_VIEW;
        ROMactive = true;
    } else {
        ROMactive = false;
    }
}

void Proteus::StartConsole() {
    switch (state.selectedConsole) {
        case CONSOLE_ID::NES:
            station = std::make_shared<NES_NS::NES>();
            if (debug) {
                debugManager->SetDebugger(CONSOLE_ID::NES, station);
                debugManager->CycleDebugViews();
            }
            return;
        default:
            exit(EXIT_FAILURE);
    }
}

void Proteus::ShutDownConsole(bool shutdownApp) {
    if (shutdownApp)
        exit(EXIT_SUCCESS);

    station.reset();
    ROMactive = false;
    SetState(GAME_LIST, state.selectedConsole);
}

const u32* Proteus::GetFrameBuffer() {
    if (station.get() != nullptr) {
        return station->getFrameBuffer();
    }
    return nullptr;
}

std::string Proteus::GetDebugInfoCPU() {
    if (!ROMactive) return "No ROM active...\n";
    return debugManager->GetDebugger()->GetStateCPU();
}

std::string Proteus::GetDebugInfoRAM() {
    if (!ROMactive) return "No ROM active...\n";
    return debugManager->GetDebugger()->GetStateRAM();
}

std::vector<u32> Proteus::GetDebugPaletteColors() {
    if (!ROMactive) return {};
    return debugManager->GetDebugger()->GetPaletteColors();
}

std::vector<u32> Proteus::GetDebugPatternTable(int index) {
    if (!ROMactive) return {};
    return debugManager->GetDebugger()->GetPatternTable(index);
}