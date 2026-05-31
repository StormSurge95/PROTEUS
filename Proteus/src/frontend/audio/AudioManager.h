#pragma once

#include <FrontendPCH.h>
#include <IManagerContexts.h>
#include <Logger.h>

namespace NS_Proteus {
    class Logger;

    class AudioManager {
        public:
            AudioManager(const IAudioContext* c) : ctx(c) {}
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

            SDL_AudioSpec spec = {};
            SDL_AudioDeviceID dev = 0;
            SDL_AudioStream* stream = nullptr;
    };
}