#pragma once

#include "./NES_PCH.h"
#include "./BUS.h"

namespace NES_NS {
    class CPU : public IDevice<u8, u16> {
        friend class Debugger;
        private:
            bool debugEnabled = false;
            bool pendingIRQ = false;
            bool pendingNMI = false;
            bool pendingRST = false;
            bool delayInterrupt = false;
            sptr<BUS> bus = nullptr;

            ADDR pc;
            u8 a = 0;
            u8 x = 0;
            u8 y = 0;
            u8 sp = 0;
            u8 status = 0;
            u8 dStatus = 0;
            bool updateStatus = false;

            u8 fetched = 0x00;
            u8 opcode = 0x00;
            INST* currInst = nullptr;
            ADDR absAddr, relAddr, indAddr;
            u8 offset = 0x00;
            s8 soffset = 0;
            bool paged = false;
            bool branch = false;
            inline void page(u8 val) {
                paged = val > (0xFF - absAddr.lo);
                absAddr.lo += val;
            }
            inline void spage(u8 val) {
                if (((val >> 7) & 0x01) == 0) { // positive offset
                    paged = val > (0xFF - pc.lo);
                    pc.lo += val;
                } else { // negative offset
                    val = ~val;
                    val++;
                    paged = val > pc.lo;
                    pc.lo -= val;
                }
            }

            void pollInterrupts();

            inline u8 getFlag(FLAGS f) const { return ((status & static_cast<u8>(f)) > 0) ? 1 : 0; }
            inline void setFlag(FLAGS f, bool v) {
                if (v)
                    status |= static_cast<u8>(f);
                else
                    status &= ~static_cast<u8>(f);
            }
            inline void setFlags(u8 val) {
                setFlag(FLAGS::C, (val & static_cast<u8>(FLAGS::C)) > 0);
                setFlag(FLAGS::Z, (val & static_cast<u8>(FLAGS::Z)) > 0);
                setFlag(FLAGS::I, (val & static_cast<u8>(FLAGS::I)) > 0);
                setFlag(FLAGS::D, (val & static_cast<u8>(FLAGS::D)) > 0);
                //setFlag(B, (val & B) > 0);
                setFlag(FLAGS::U, true);
                setFlag(FLAGS::V, (val & static_cast<u8>(FLAGS::V)) > 0);
                setFlag(FLAGS::N, (val & static_cast<u8>(FLAGS::N)) > 0);
            }
            inline void setZN(u8 val) {
                setFlag(FLAGS::Z, val == 0);
                setFlag(FLAGS::N, ((val >> 7) & 0x01) > 0);
            }
            inline string getFlags() {
                stringstream ss;
                ss << to_string(getFlag(FLAGS::N)) << " " <<
                    to_string(getFlag(FLAGS::V)) << " " <<
                    to_string(getFlag(FLAGS::U)) << " " <<
                    to_string(getFlag(FLAGS::B)) << " " <<
                    to_string(getFlag(FLAGS::D)) << " " <<
                    to_string(getFlag(FLAGS::I)) << " " <<
                    to_string(getFlag(FLAGS::Z)) << " " <<
                    to_string(getFlag(FLAGS::C));
                return ss.str();
            }

            vector<INST> lookup;

            #pragma region Addressing Modes
            void ACC_A();
            void IMM_A();
            void IMP_A();
            void IND_J();
            void REL_B();
            void ABS_W(); void ABS_R(); void ABS_M(); void ABS_J();
            void ABX_W(); void ABX_R(); void ABX_M();
            void ABY_W(); void ABY_R(); void ABY_M();
            void ZP0_W(); void ZP0_R(); void ZP0_M();
            void ZPX_W(); void ZPX_R(); void ZPX_M();
            void ZPY_W(); void ZPY_R(); void ZPY_M();
            void IZX_W(); void IZX_R(); void IZX_M();
            void IZY_W(); void IZY_R(); void IZY_M();
            #pragma endregion

            #pragma region Official Instructions
            void LDA(); void STA(); void LDX(); void STX(); void LDY(); void STY();
            void TAX(); void TXA(); void TAY(); void TYA();
            void ADC(); void SBC(); void INC(); void DEC(); void INX(); void DEX(); void INY(); void DEY();
            void ASL(); void LSR(); void ROL(); void ROR();
            void AND(); void ORA(); void EOR(); void BIT();
            void CMP(); void CPX(); void CPY();
            void BCC(); void BCS(); void BEQ(); void BNE(); void BPL(); void BMI(); void BVC(); void BVS();
            void JMP(); void JSR(); void RTS(); void BRK(); void RTI();
            void PHA(); void PLA(); void PHP(); void PLP(); void TXS(); void TSX();
            void CLC(); void SEC(); void CLI(); void SEI(); void CLD(); void SED(); void CLV();
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

            #pragma region Debugging
            string disassembleInst(u16 addr);
            #pragma endregion
        public:
            bool irqTrigger = false;
            bool nmiTrigger = false;
            u64 totalCycles = 0;
            u8 cycles = 0;

            CPU();
            ~CPU() = default;

            inline void connectBUS(sptr<BUS> b) { bus = b; }

            u8 read(u16 addr, bool readonly = false) override;
            void write(u16 addr, u8 data) override;

            void start();
            void reset();
            void clock();
    };
}