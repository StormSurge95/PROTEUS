#pragma once

#include "./NES_PCH.h"
#include "./PPU.h"
#include "./Mappers/Mapper.h"

namespace NES_NS {
    class NES : public IConsole {
            friend class Debugger;
        public:
            sptr<CPU> cpu;
            u64 masterClock = 0x00;

            NES();
            ~NES() = default;

            void reset() override;
            void clock() override;

            bool loadCart(const string&) override;

            inline const u32* getFrameBuffer() const override { return ppu->getFrameBuffer(); }
            void collectAudio(vector<float>&) override;

            int SCREEN_WIDTH() const override { return 256; }
            int SCREEN_HEIGHT() const override { return 240; }

            void update(u8, bool*) override;

        private:
            sptr<BUS> bus;
            sptr<Gamepak> cart;
            sptr<PPU> ppu;
            sptr<APU> apu;
            sptr<Controller> player1;
            sptr<Controller> player2;

            void clockFrame();
            void clockCycleCPU();
            void clockCyclePPU();
    };
}