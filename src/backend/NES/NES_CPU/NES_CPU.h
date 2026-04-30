#pragma once

#include "../../../core/Helpers.h"
#include "../../../core/IDevice.h"
#include "../NES_BUS.h"

#include <memory>
#include <string>
#include <vector>

class NES_CPU : public IDevice<uint8_t, uint16_t> {
    friend class NES_DBG;
    public:
        bool pendingIRQ = false;
        bool pendingNMI = false;
        bool pendingRST = false;
        bool irqTrigger = false;
        bool nmiTrigger = false;
        bool delayInterrupt = false;

        NES_CPU(bool debug);
        ~NES_CPU();

        inline void connectBUS(std::shared_ptr<NES_BUS> b) { bus = b; }

        uint8_t read(uint16_t addr, bool readonly = false) override;
        void write(uint16_t addr, uint8_t data) override;

        void reset();
        void clock();

        inline void enableDebug() { debugEnabled = true; }

        std::shared_ptr<NES_BUS> bus = nullptr;

        bool debugEnabled = false;

        uint8_t a = 0;
        uint8_t x = 0;
        uint8_t y = 0;
        uint8_t sp = 0;

        uint64_t totalCycles = 0;
        uint8_t status = 0;
        uint8_t dStatus = 0;
        bool updateStatus = false;

        uint8_t fetched = 0x00;
        uint8_t opcode = 0x00;

        uint8_t cycles = 0;
        struct ADDR {
            uint8_t lo = 0x00;
            uint8_t hi = 0x00;

            ADDR() = default;
            ADDR(uint8_t l, uint8_t h) : lo(l), hi(h) {}
            ADDR(uint16_t a) {
                lo = a & 0xFF;
                hi = ((a >> 8) & 0xFF);
            }
            ADDR(const ADDR& other) {
                if (this != &other) {
                    lo = other.lo;
                    hi = other.hi;
                }
            }

            ADDR& operator=(uint16_t val) {
                lo = val & 0xFF;
                hi = ((val >> 8) & 0xFF);
                return *this;
            }
            ADDR& operator++() { // prefix
                if (lo == 0xFF) {
                    lo = 0x00;
                    if (hi == 0xFF) {
                        hi = 0x00;
                    } else hi++;
                } else lo++;
                return *this;
            }
            ADDR operator++(int) { // postfix
                ADDR temp = *this;
                ++(*this);
                return temp;
            }
            ADDR& operator--() {
                if (lo == 0x00) {
                    lo = 0xFF;
                    if (hi == 0x00) {
                        hi = 0xFF;
                    } else hi--;
                } else lo--;
                return *this;
            }
            ADDR operator--(int) {
                ADDR temp = *this;
                --(*this);
                return temp;
            }
            ADDR& operator+(uint16_t val) {
                add(val);
                return *this;
            }
            ADDR& operator+=(uint16_t val) {
                add(val);
                return *this;
            }
            bool operator==(const ADDR& other) {
                return value() == other.value();
            }
            bool operator==(const uint16_t& val) {
                return value() == val;
            }
            bool operator!=(const ADDR& other) {
                return value() != other.value();
            }
            bool operator!=(const uint16_t& val) {
                return value() != val;
            }
            bool operator>(const ADDR& other) {
                return value() > other.value();
            }
            bool operator>(const uint16_t& val) {
                return value() > val;
            }
            bool operator<(const ADDR& other) {
                return value() < other.value();
            }
            bool operator<(const uint16_t& val) {
                return value() < val;
            }
            bool operator>=(const ADDR& other) {
                return value() >= other.value();
            }
            bool operator>=(const uint16_t& val) {
                return value() >= val;
            }
            bool operator<=(const ADDR& other) {
                return value() <= other.value();
            }
            bool operator<=(const uint16_t& val) {
                return value() <= val;
            }
            operator uint16_t() {
                return value();
            }

            void add(uint16_t val) {
                uint8_t l = val & 0xFF;
                uint8_t h = (val >> 8) & 0xFF;
                if (0xFF - lo < l)
                    hi++;
                lo += l;
                hi += h;
            }
            void add_s(int8_t val) {
                if (((val >> 7) & 0x01) == 0) { // positive
                    if (val > (0xFF - lo)) hi++;
                    lo += val;
                } else { // negative
                    val = ~val;
                    val++;
                    if (val > lo) hi--;
                    lo -= val;
                }
            }
            void sub(uint16_t val) {
                uint8_t l = val & 0xFF;
                uint8_t h = (val >> 8) & 0xFF;
            }

            uint16_t value() const {
                return (((uint16_t)hi << 8) | lo);
            }
        } pc, absAddr, relAddr, indAddr;
        enum FLAGS {
            C = (1 << 0),
            Z = (1 << 1),
            I = (1 << 2),
            D = (1 << 3),
            B = (1 << 4),
            U = (1 << 5),
            V = (1 << 6),
            N = (1 << 7)
        };
        enum INSTRUCTION_TYPE : uint8_t {
            R,
            W,
            M,
            J,
            X
        };
        struct INST {
            std::string name;
            INSTRUCTION_TYPE type;
            uint8_t bytes = 0;

            void (NES_CPU::* address)(void) = nullptr;
            void (NES_CPU::* operate)(void) = nullptr;
        } *currInst = nullptr;
        uint8_t offset = 0x00;
        int8_t soffset = 0;
        bool paged = false;
        bool branch = false;
        void page(uint8_t val) {
            paged =  val > (0xFF - absAddr.lo);
            absAddr.lo += val;
        }
        void spage(uint8_t val) {
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
        std::vector<INST> lookup;

        void pollInterrupts();

        inline uint8_t getFlag(FLAGS f) const { return ((status & f) > 0) ? 1 : 0; }
        inline void setFlag(FLAGS f, bool v) {
            if (v)
                status |= f;
            else
                status &= ~f;
        }
        inline void setFlags(uint8_t val) {
            setFlag(C, (val & C) > 0);
            setFlag(Z, (val & Z) > 0);
            setFlag(I, (val & I) > 0);
            setFlag(D, (val & D) > 0);
            //setFlag(B, (val & B) > 0);
            setFlag(U, true);
            setFlag(V, (val & V) > 0);
            setFlag(N, (val & N) > 0);
        }
        inline void setZN(uint8_t val) {
            setFlag(Z, val == 0);
            setFlag(N, ((val >> 7) & 0x01) > 0);
        }

        #pragma region Addressing Modes
        void ABS();
        void ABX();
        void ABY();
        void ZP0();
        void ZPX();
        void ZPY();
        void ACC();
        void IMM();
        void IMP();
        void IND();
        void IZX();
        void IZY();
        void REL();
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
        void NOP(); void RST(); void IRQ(); void NMI();
        #pragma endregion

        #pragma region Unofficial Instructions
        void SLO(); void RLA(); void SRE(); void RRA();
        void SAX(); void LAX();
        void DCP(); void ISC();
        void SHA(); void SHS(); void SHY(); void SHX(); void LAS(); void ARR();
        void ANC(); void ASR(); void ANE(); void LXA(); void AXS();
        void JAM();
        #pragma endregion

        INST RST_INST = {"RST",X,0,nullptr,&NES_CPU::RST};
        INST NMI_INST = {"NMI",X,0,nullptr,&NES_CPU::NMI};
        INST IRQ_INST = {"IRQ",X,0,nullptr,&NES_CPU::IRQ};

        #pragma region Debugging
        bool debug;
        std::ofstream* traceStream = nullptr;

        enum INTERRUPT {
            RST_INTERRUPT,
            NMI_INTERRUPT,
            IRQ_INTERRUPT
        };

        std::string disassembleInst(uint16_t addr);
        std::string formatInst() const;
        std::string traceStack();
        std::string trace();
        std::string traceInterrupt(INTERRUPT i);
        #pragma endregion
};