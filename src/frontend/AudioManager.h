#pragma once

#include "./FrontendPCH.h"
#include "./Proteus.h"

namespace NS_Proteus {
    class AudioManager {
        public:
            AudioManager(const IAudioContext* ctx, bool debug = false);
            ~AudioManager();

            AudioManager(const AudioManager&) = delete;
            AudioManager& operator=(const AudioManager&) = delete;
            AudioManager(AudioManager&&) = delete;
            AudioManager& operator=(AudioManager&&) = delete;

            void Init();
            void Deinit();

            void Update(const sptr<IConsole>& station);
        private:
            const IAudioContext* ctx;
            bool debug;

            SDL_AudioSpec spec = {};
            SDL_AudioDeviceID dev = 0;
            SDL_AudioStream* stream = nullptr;
    };
}