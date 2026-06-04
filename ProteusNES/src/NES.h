#pragma once

#include "./shared/NesPCH.h"
#include "./PPU/NesPPU.h"
#include "./PAK/Mappers/NesMapper.h"

namespace NS_NES {
    /**
     * @class NES NES.h "./NES.h"
     * @implements IConsole
     * @brief Master class for the emulation of the NES console.
     */
    class NES : public IConsole {
            /// @brief Allow Debugger class to access all private members of the NES class
            friend class NesDebugger;
        public:
            // master clock counter for the console
            u64 masterClock = 0x00;

            // explicit constructor
            NES();
            // default destructor
            ~NES() = default;

            void initSST(SingleStateTest::State s) override;
            void runSST() override;
            bool checkSST(SingleStateTest::State s, string& o) override;

            // console "init" function
            bool poweron() override;
            // console shutdown function
            bool shutdown() override;
            // console reset function
            // fun fact: up until starting the development of this project, I thought that the
            // reset buttons on consoles literally just power-cycled them.
            void reset() override;
            // clock function of the console; controls the clocking of the inner pieces
            void clock() override;
            // helper function for clocking an entire frame's-worth of cycles
            void clockFrame();
            // helper function for clocking a single cpu cycle
            void clockCycleCPU();
            // helper function for clocking a single ppu cycle
            void clockCyclePPU();
            // helper function for putting the other "clockXXX" functions together to clock a single master cycle
            void clockMaster();

            // loads a rom (i.e. gamepak) into the console.
            bool loadROM(const string&) override;

            // collects and returns a pointer to the array of pixel data produced by the PPU
            inline const u32* getFrameBuffer() const override { return ppu->getFrameBuffer(); }
            // populates a provided vector of floats with sound data produced by the APU
            void collectAudio(vector<float>&) override;

            // provides the original pixel width of the NES console screen
            const int SCREEN_WIDTH() const override { return 256; }
            // provides the original pixel height of the NES console screen
            const int SCREEN_HEIGHT() const override { return 240; }

            size_t buttonCount() const override { return 8; }

            // allows frontend to pass input data back to the backend
            void update(u8, const bool*) override;

            const CPU_STATE GetSnapshotCPU() const;

        private:
            sptr<CPU> cpu;              // shared pointer reference to the cpu
            sptr<Gamepak> cart;         // shared pointer reference to the cartridge/gamepak/rom
            sptr<PPU> ppu;              // shared pointer reference to the ppu
            sptr<APU> apu;              // shared pointer reference to the apu
            sptr<Controller> player1;   // shared pointer reference to p1 controller
            sptr<Controller> player2;   // shared pointer refernece to p2 contorller
            bool powered = false;
    };
}