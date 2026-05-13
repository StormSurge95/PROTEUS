#pragma once

#include "./NES_PCH.h"

namespace NES_NS {
    class BUS : IDevice<u8, u16> {
            // Allow Debugger class to access all private members of the BUS class
            friend class Debugger;
        public:
            /// @brief Flag for OAMDMA
            bool oamActive = false;
            /// @brief Flag for DMCDMA
            bool dmcActive = false;
            /// @brief Helper flag for running DMA with accurate cycle counts
            bool dmaDummy = true;

            /// @brief Explicit Constructor
            BUS();
            /// @brief Default Destructor
            ~BUS() = default;

            /**
             * @brief Data read request.
             * @param addr Address to be read.
             * @param readonly Flag to block side-effects.
             * @return The data read; or openbus if unmapped address
             */
            u8 read(u16 addr, bool readonly = false) override;
            /**
             * @brief Data write request.
             * @param addr Address to be written to.
             * @param data Data to be written.
             * @note If `addr` is an unmapped address, this function does nothing.
             */
            void write(u16 addr, u8 data) override;

            /**
             * @brief Connects the cpu to the bus.
             * @param c The cpu of the console.
             */
            void connectCPU(sptr<CPU>& c) { cpu = c; }
            /**
             * @brief Connects the ppu to the bus.
             * @param p The ppu of the console.
             */
            void connectPPU(sptr<PPU>& p) { ppu = p; }
            /**
             * @brief Connects the apu to the bus.
             * @param a The apu of the console.
             */
            void connectAPU(sptr<APU>& a) { apu = a; }
            /**
             * @brief Connects a Gamepak cartridge to the bus.
             * @param c The cartridge to connect.
             */
            void connectCART(sptr<Gamepak>& c) { cart = c; }
            /**
             * @brief Connects a controller to the bus.
             * @param c The controller to connect.
             * @param p The player this controller refers to.
             */
            void connectCONT(sptr<Controller>& c, u8 p);
            /**
             * @brief Clock function for OAMDMA.
             * @param c The master-cycle of the console.
             */
            void clockOAM(u64 c);
            /**
             * @brief Clock function for DMCDMA.
             * @param c The master-cycle of the console.
             */
            void clockDMC(u64 c);

        private:
            /// @brief WRAM of the console/cartridge.
            array<u8, 2048> ram;
            /// @brief The cpu of the console.
            wptr<CPU> cpu;
            /// @brief The ppu of the console.
            wptr<PPU> ppu;
            /// @brief The apu of the console.
            wptr<APU> apu;
            /// @brief The current cartridge that is connected.
            wptr<Gamepak> cart;
            /// @brief Player 1 controller
            wptr<Controller> player1;
            /// @brief Player 2 controller
            wptr<Controller> player2;

            /// @brief Current open-bus value to be updated/returned on read/write calls.
            u8 openBus = 0x00;
            
            /// @brief Helper variable for OAMDMA; refers to the page of WRAM to read OAM data from.
            u8 dmaPage = 0x00;
            /// @brief Helper variable for OAMDMA; refers to the OAM address to write the WRAM data to.
            u8 dmaAddr = 0x00;
            /// @brief Helper variable for OAMDMA; refers to the OAM data to be written after it is read from WRAM.
            u8 dmaData = 0x00;

            /// @brief Helper variable meant to be used for when the CPU is stalled via DMA dummy-read cycles.
            u16 lastReadAddr = 0x0000;
    };
}