#include "AudioManager.h"

using namespace NS_Proteus;

AudioManager::AudioManager(Proteus* p, bool d) {
    proteus = p;
    debug = d;
}

AudioManager::~AudioManager() {
    Deinit();
}

void AudioManager::Init() {
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize SDL Audio! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    } else SDL_LogMessage(SDL_LOG_CATEGORY_AUDIO, SDL_LOG_PRIORITY_INFO, "SDL Input successfully initialized.\n");

    spec = {
        .format = SDL_AUDIO_F32,
        .channels = 1,
        .freq = 44100,
    };

    dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);

    if (dev == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to open audio device! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    stream = SDL_CreateAudioStream(&spec, &spec);

    if (!stream) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create audio stream! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (!SDL_BindAudioStream(dev, stream)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to bind audio stream to device! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if (!SDL_ResumeAudioDevice(dev)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to resume audio device! Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void AudioManager::Deinit() {
    if (dev != 0) {
        SDL_PauseAudioDevice(dev);

        if (stream != nullptr) {
            SDL_UnbindAudioStream(stream);
            SDL_DestroyAudioStream(stream);
            stream = nullptr;
        }

        SDL_CloseAudioDevice(dev);
        dev = 0;
    }
}

void AudioManager::Update(std::shared_ptr<IConsole>& station) {
    if (station == nullptr) return;
    std::vector<float> samples;
    station->collectAudio(samples);
    if (!samples.empty())
        SDL_PutAudioStreamData(stream, samples.data(), (int)samples.size() * sizeof(float));
}