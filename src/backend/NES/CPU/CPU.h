#pragma once

#include "../shared/NES_PCH.h"

namespace NES_NS {
    class CPU : public IDevice<u8, u16> {
            // Allow Debugger class to access all private members of the CPU class
            friend class Debugger;
        private:
            /// @brief 'magic' of instable opcode(s)
            bool magic = false;
            /// @brief CPU ram container
            array<u8, 2048> ram;
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
            u8 cpuBus = 0x00;

            /// @brief Helper variable for OAMDMA; refers to the page of WRAM to read OAM data from.
            u8 dmaPage = 0x00;
            /// @brief Helper variable for OAMDMA; refers to the OAM address to write the WRAM data to.
            u8 dmaAddr = 0x00;
            /// @brief Helper variable for OAMDMA; refers to the OAM data to be written after it is read from WRAM.
            u8 dmaData = 0x00;

            /// @brief Helper variable meant to be used for when the CPU is stalled via DMA dummy-read cycles.
            u16 lastReadAddr = 0x0000;

            /// @brief debug flag
            bool debugEnabled = false;
            /// @brief irq flag
            bool pendingIRQ = false;
            /// @brief nmi flag
            bool pendingNMI = false;
            /// @brief reset flag
            bool pendingRST = false;
            /// @brief interrupt delay flag
            bool delayInterrupt = false;

            deque<u16> prevInstAddrs = {};

            /// @brief Current program counter
            ADDR pc;
            /// @brief Current accumulator register
            u8 a = 0;
            /// @brief Current X register
            u8 x = 0;
            /// @brief Current Y register
            u8 y = 0;
            /// @brief Current Stack Pointer
            u8 sp = 0;
            /// @brief Current CPU status
            u8 status = 0;
            /// @brief delay CPU status; used for delayed status updates
            u8 dStatus = 0;
            /// @brief update flag; determines whether to update status using `dStatus`
            bool updateStatus = false;

            /// @brief data fetched during instruction sequence
            u8 fetched = 0x00;
            /// @brief opcode of current instruction sequence
            u8 opcode = 0x00;
            /// @brief operation information of current instruction sequence
            INST* currInst = nullptr;
            /// @brief helper variables for address operations
            ADDR absAddr, relAddr, indAddr;
            /// @brief helper variable for zero page and branch instructions
            u8 offset = 0x00;
            /// @brief flag to determine page boundary crossings
            bool paged = false;
            /// @brief flag to determine when to take branches
            bool branch = false;
            /**
             * @brief Helper function to determine page boundary crossings
             * @param val unsigned offset to use for setting flag
             */
            inline void page(u8 val) {
                /// if `val` is greater than the amount of space between `absAddr.lo` and `0xFF`
                /// then we are crossing to the next page
                paged = val > (0xFF - absAddr.lo);
                absAddr.lo += val;
            }
            /**
             * @brief Helper function to determine page boundary crossings
             * @param val signed offset to use for setting flag
             * @note `val` is passed in as an unsigned value and 2's complement logic is used to process it as signed.
             * @note the logic of this function is only applicable to `branch` instructions, and therefore effects pc
             */
            inline void spage(u8 val) {
                if (((val >> 7) & 0x01) == 0) {
                    /// for positive offsets, simply use the same logic  as `page()`
                    paged = val > (0xFF - pc.lo);
                    pc.lo += val;
                } else {
                    /// for negative offsets, we need to take the 2's complement to obtain the positive value
                    val = ~val; /// `not` the value
                    val++; /// add one

                    /// now, we can simply use logic the exact opposite of `page()`
                    /// if `val` is greater than `pc.lo`
                    /// then we are crossing to the previous page
                    paged = val > pc.lo;
                    pc.lo -= val;
                }
            }

            /// @brief flag to poll interrupts
            bool pollScheduled = false;
            /// @brief Poll the interrupt lines to determine if the next instruction should be an interrupt.
            void pollInterrupts();
            /// @brief Set flag to poll interrupts at beginning of next cycle
            inline void schedulePoll() { pollInterrupts(); }

            /**
             * @brief Helper function to obtain the value of a specific status flag
             * @param f The status flag to obtain.
             * @return `1` if flag is set; `0` otherwise
             */
            inline u8 getFlag(FLAGS f) const { return ((status & static_cast<u8>(f)) > 0) ? 1 : 0; }
            /**
             * @brief Helper function to set the value of a specific status flag
             * @param f The status flag to set.
             * @param v The value to set the flag to.
             * @details
             * The status flags are implemented as a set of enum values that are all powers of 2.
             * This means that we can simply cast them to a number and then use them with bitwise
             * operations to set their respective bits within our status register.
             */
            inline void setFlag(FLAGS f, bool v) {
                if (v)
                    status |= static_cast<u8>(f);
                else
                    status &= ~static_cast<u8>(f);
            }
            /**
             * @brief Helper function to set Z and N flags so we don't have to repeat the same code
             * @param val The value to use for setting the flags.
             */
            inline void setZN(u8 val) {
                /// `Z` flag is set only if the supplied value @b is @b equal @b to @b zero.
                setFlag(FLAGS::Z, val == 0);
                /// `N` flag is set when bit 7 of the supplied value is set 
                setFlag(FLAGS::N, ((val >> 7) & 0x01) > 0);
            }

            /// @brief instruction lookup table
            vector<INST> lookup;

            /// @brief helper function to halt cpu and handle side effects
            void halt();

            #pragma region Addressing Modes
            /// @brief Accumulator Instructions
            void ACC_A();
            /// @brief Immediate Instructions
            void IMM_A();
            /// @brief Implied Instructions
            void IMP_A();
            /// @brief Indirect Instructions (JMP only)
            void IND_J();
            /// @brief Relative Instructions (Branch only)
            void REL_B();
            /// @brief Absolute Instructions (Write, Read, RMW, and JMP)
            void ABS_W(); void ABS_R(); void ABS_M(); void ABS_J();
            /// @brief Indexed Absolute Instructions
            void ABX_W(); void ABX_R(); void ABX_M();
            void ABY_W(); void ABY_R(); void ABY_M();
            /// @brief Zero Page Instructions
            void ZP0_W(); void ZP0_R(); void ZP0_M();
            /// @brief Indexed Zero Page Instructions
            void ZPX_W(); void ZPX_R(); void ZPX_M();
            void ZPY_W(); void ZPY_R(); void ZPY_M();
            /// @brief Indexed Indirect Instructions
            void IZX_W(); void IZX_R(); void IZX_M();
            /// @brief Indirect Indexed Instructions
            void IZY_W(); void IZY_R(); void IZY_M();
            #pragma endregion

            #pragma region Official Instructions
            /// @brief Access Instructions
            void LDA(); void STA(); void LDX(); void STX(); void LDY(); void STY();
            /// @brief Transfer Instructions
            void TAX(); void TXA(); void TAY(); void TYA();
            /// @brief Arithmetic Instructions
            void ADC(); void SBC(); void INC(); void DEC(); void INX(); void DEX(); void INY(); void DEY();
            /// @brief Shift Instructions
            void ASL(); void LSR(); void ROL(); void ROR();
            /// @brief Bitwise Instructions
            void AND(); void ORA(); void EOR(); void BIT();
            /// @brief Compare Instructions; implemented as subtraction that only effects flags
            void CMP(); void CPX(); void CPY();
            /// @brief Branch Instructions
            void BCC(); void BCS(); void BEQ(); void BNE(); void BPL(); void BMI(); void BVC(); void BVS();
            /// @brief Jump Instructions
            void JMP(); void JSR(); void RTS(); void BRK(); void RTI();
            /// @brief Stack Instructions
            void PHA(); void PLA(); void PHP(); void PLP(); void TXS(); void TSX();
            /// @brief Flag Instructions
            void CLC(); void SEC(); void CLI(); void SEI(); void CLD(); void SED(); void CLV();
            /// @brief 'No Operation' Instruction
            void NOP();
            #pragma endregion

            #pragma region Interrupts
            void RST(); void IRQ(); void NMI();
            INST RST_INST = { "RST",0,nullptr,&CPU::RST };
            INST NMI_INST = { "NMI",0,nullptr,&CPU::NMI };
            INST IRQ_INST = { "IRQ",0,nullptr,&CPU::IRQ };
            #pragma endregion

            #pragma region Unofficial Instructions
            void SLO(); void RLA(); void SRE(); void RRA();
            void SAX(); void LAX();
            void DCP(); void ISC();
            void SHA(); void SHS(); void SHY(); void SHX(); void LAS(); void ARR();
            void ANC(); void ASR(); void ANE(); void LXA(); void AXS();
            void JAM();
            #pragma endregion
        public:
            /// @brief irq flag
            bool irqTrigger = false;
            /// @brief nmi flag
            bool nmiTrigger = false;
            /// @brief total cycles completed by CPU
            u64 totalCycles = 0;
            /// @brief cycle tracker for instruction operation
            u8 cycles = 0;

            /// @brief Delay flat for the DMA operations
            bool delayDMA = false;
            bool halted = false;
            /// @brief Flag for OAMDMA
            bool oamActive = false;
            /// @brief Flag for DMCDMA
            bool dmcActive = false;
            /// @brief Helper flag for running DMA with accurate cycle counts
            bool dmaDummy = true;

            /// @brief Explicit Constructor
            CPU();
            /// @brief Default Destructor
            ~CPU() = default;

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
             */
            void clockOAM();
            /**
             * @brief Clock function for DMCDMA.
             */
            void clockDMC();

            /// @brief 'power on' function
            void start();
            /// @brief 'reset' function
            void reset();
            /// @brief 'clock' function
            void clock();/// @brief WRAM of the console/cartridge.
    };
}