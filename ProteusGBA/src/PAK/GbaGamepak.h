#pragma once

#include "../shared/GbaPCH.h"

namespace NS_GBA {
    class Gamepak : public IDevice<u16, u32> {
        public:
            Gamepak() = default;
            ~Gamepak() = default;

            u16 read(u32 addr, bool readonly = false) override;
            void write(u32 addr, u16 data) override;

            
        private:
            GbaHeader header = {};

            void readHeader();
    };
}