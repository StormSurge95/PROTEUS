#include "./Proteus.h"

#include "../../resources/NES_DB.h"
#include "../backend/NES/NES_CORE.h"

#include "./AudioManager.h"
#include "./InputManager.h"
#include "./VideoManager.h"

#include <algorithm>
#include <fstream>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <SDL3/SDL.h>

Proteus::Proteus(bool d) {
    debug = d;
    if (debug) SDL_SetLogPriorities(SDL_LOG_PRIORITY_DEBUG);

    videoManager = std::make_shared<VideoManager>(this, debug);
    inputManager = std::make_shared<InputManager>(this, debug);
    audioManager = std::make_shared<AudioManager>(this, debug);
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
        videoManager->Render();
        audioManager->Update();
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

void Proteus::ProcessEvents() {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                    quit = true;
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
    const std::filesystem::path base = "C:/ROMS/";
    if (!std::filesystem::exists(base)) std::filesystem::create_directory(base);

    for (const std::pair<std::string, std::string>& pair : CONSOLES) {
        if (pair.first != "NES") continue;
        std::filesystem::path path = base.string() + pair.first + "/";
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
            continue;
        }
        std::vector<ROM> games = {};
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            // get text data
            std::string file = entry.path().string();
            std::string filename = file.substr(file.find_last_of('/') + 1);
            
            // get hash
            std::string hash = MD5(file);

            std::string gameName = Lookup(pair.first, hash);
            if (gameName == "Unknown") gameName = filename.substr(0, filename.length() - 4);
            games.push_back({ .gameName = gameName, .path = file });
        }

        std::pair<std::string, std::vector<ROM>> p(pair.first, games);
        gameList.insert(p);
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
    state.currentView = GAME_VIEW;

    StartConsole();

    ROM game = gameList[GetConsoleFromID(state.selectedConsole)][index];

    state.selectedGame = game.gameName;
    std::string path = game.path;

    std::string title = "PROTEUS: " + game.gameName;

    videoManager->InitGameTexture(title, station->SCREEN_WIDTH(), station->SCREEN_HEIGHT());

    station->loadCart(path);
}

void Proteus::StartConsole() {
    switch (state.selectedConsole) {
        case CONSOLE_ID::NES:
            station = std::make_shared<NES_CORE>(debug);
            return;
        default:
            exit(EXIT_FAILURE);
    }
}

void Proteus::ShutDownConsole(bool shutdownApp) {
    if (shutdownApp)
        exit(EXIT_SUCCESS);

    station.reset();
    SetState(GAME_LIST, state.selectedConsole);
}

const uint32_t* Proteus::GetFrameBuffer() {
    if (station.get() != nullptr) {
        return station->getFrameBuffer();
    }
    return nullptr;
}