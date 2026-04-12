#pragma once

#include <SDL3/SDL.h>

#include "./Proteus.h"

class AudioManager {
    public:
        AudioManager(Proteus* proteus, bool debug = false);
        ~AudioManager();

        void Init();
        void Deinit();

        void Update();
    private:
        Proteus* proteus;
        bool debug;

        SDL_AudioSpec spec = {};
        SDL_AudioDeviceID dev = 0;
        SDL_AudioStream* stream = nullptr;
};