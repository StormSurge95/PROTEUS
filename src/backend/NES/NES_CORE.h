#pragma once

#include "../../core/IConsole.h"
#include "./NES_BUS.h"
#include "./NES_CART.h"
#include "./NES_CONT.h"
#include "./NES_CPU.h"
#include "./NES_PPU.h"

#include <memory>

class NES_CORE : public IConsole {
    public:
        std::shared_ptr<NES_CPU> cpu;
        uint64_t masterClock = 0x00;
            
        NES_CORE(bool debug = false);
        ~NES_CORE() = default;

        void reset() override;
        void clock() override;

        bool loadCart(const std::string& path) override;

        inline const uint32_t* getFrameBuffer() const override { return ppu->getFrameBuffer(); }

        int SCREEN_WIDTH() const override { return 256; }
        int SCREEN_HEIGHT() const override { return 240; }

        void update(uint8_t player, bool* buttons) override;

    private:
        std::shared_ptr<NES_BUS> bus;
        std::shared_ptr<NES_CART> cart;
        std::shared_ptr<NES_PPU> ppu;
        std::shared_ptr<NES_CONT> player1;
        std::shared_ptr<NES_CONT> player2;
        static const int AUDIO_BATCH_SIZE = 1024;
};