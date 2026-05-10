#pragma once

#include "./FrontendPCH.h"
#include "./Proteus.h"

namespace NS_Proteus {
    class AudioManager {
        public:
            AudioManager(Proteus* proteus, bool debug = false);
            ~AudioManager();

            void Init();
            void Deinit();

            void Update(std::shared_ptr<IConsole>& station);
        private:
            Proteus* proteus;
            bool debug;

            SDL_AudioSpec spec = {};
            SDL_AudioDeviceID dev = 0;
            SDL_AudioStream* stream = nullptr;
    };
}