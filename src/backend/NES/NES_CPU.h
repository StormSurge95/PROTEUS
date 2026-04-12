#pragma once

#include "../../core/Helpers.h"
#include "../../core/IDevice.h"
#include "NES_BUS.h"

#include <memory>
#include <string>
#include <vector>

class NES_CPU : public IDevice<uint8_t, uint16_t> {
    public:
        bool doIRQ = false;
        bool doNMI = false;
        bool irqTrigger = false;
        bool nmiTrigger = false;
        bool delayIRQ = false;
        bool delayNMI = false;
        bool doRST = false;

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
                if (val < 0 && val > lo)
                    hi--;
                else if (val > 0 && (val > (0xFF - lo)))
                    hi++;
                lo += val;
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
        struct INST {
            std::string name;

            void (NES_CPU::* operate)(void) = nullptr;

            uint8_t cycles = 0;
            uint8_t bytes = 0;
        } *currInst = nullptr;
        uint8_t offset = 0x00;
        int8_t soffset = 0;
        bool paged = false;
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
        inline void setZN(uint8_t val) {
            setFlag(Z, val == 0);
            setFlag(N, ((val >> 7) & 0x01) > 0);
        }
        
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
        void NOP(); void XXX(); void RST(); void IRQ(); void NMI();

        void ADChelper();
        void SBChelper();
        void branch(bool take);
        void shift(char dir, bool rotate);
        void CMPop(uint8_t val);

        INST RST_INST = { "RESET", &NES_CPU::RST, 7, 0 };
        INST NMI_INST = { "NMI", &NES_CPU::NMI, 7, 0 };
        INST IRQ_INST = { "IRQ", &NES_CPU::IRQ, 7, 0 };

    #pragma region Debugging
        bool debug;
        std::ofstream* traceStream = nullptr;

        std::string disassembleInst(uint16_t addr);
        std::string formatInst();
        std::string traceStack();
        std::string trace();
    #pragma endregion
};