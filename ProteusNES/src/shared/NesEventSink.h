#pragma once

#include "./NesPCH.h"

namespace NS_NES {
    enum class INTERRUPT_EVENT {
        NMI_REQ, NMI_ACK,
        IRQ_REQ_DMC, IRQ_REQ_APU, IRQ_REQ_MAP, IRQ_ACK
    };

    static const array<const char*, 6> INTERRUPT_DETAILS {
        "requested", "acknowledged",
        "requested via DMC", "requested via APU", "requested via Mapper", "acknowledged"
    };

    static const array<const char*, 8> PPU_REGS = {
        "PPUCTRL", "PPUMASK", "PPUSTATUS", "OAMADDR", "OAMDATA", "PPUSCROLL", "PPUADDR", "PPUDATA"
    };

    static const array<const char*, 24> APU_REGS = {
        "APU - Pulse 1 - control",
        "APU - Pulse 1 - sweep",
        "APU - Pulse 1 - timer low",
        "APU - Pulse 1 - lcl/timer high",
        "APU - Pulse 2 - control",
        "APU - Pulse 2 - sweep",
        "APU - Pulse 2 - timer low",
        "APU - Pulse 2 - lcl/timer high",
        "APU - Triangle - control",
        "",
        "APU - Triangle - timer low",
        "APU - Triangle - lcl/timer high",
        "APU - Noise - control",
        "",
        "APU - Noise - mode/period",
        "APU - Noise - lcl",
        "APU - DMC - irq/loop/frequency",
        "APU - DMC - load counter",
        "APU - DMC - sample address",
        "APU - DMC - sample length",
        "",
        "APU - control/status",
        "",
        "APU - frame counter"
    };

    struct NesEventSink {
        virtual ~NesEventSink() = default;

        virtual void OnPpuRegisterRead(u16 addr, u8 value) = 0;
        virtual void OnPpuRegisterWrite(u16 addr, u8 value) = 0;

        virtual void OnApuRegisterRead(u16 addr, u8 value) = 0;
        virtual void OnApuRegisterWrite(u16 addr, u8 value) = 0;

        virtual void OnMapperRegisterRead(string details, u16 addr, u8 value) = 0;
        virtual void OnMapperRegisterWrite(string details, u16 addr, u8 value) = 0;

        virtual void OnControllerRead(u8 player, u16 addr, u8 value) = 0;
        virtual void OnControllerWrite(u8 player, u16 addr, u8 value) = 0;

        virtual void OnDmcDmaRead(string details, u16 addr, u8 value) = 0;
        virtual void OnOamDmaRead(u16 addr, u8 value) = 0;
        virtual void OnOamDmaStart(u8 value) = 0;

        virtual void OnInterrupt(INTERRUPT_EVENT type) = 0;
        virtual void OnSpriteZeroHit() = 0;
        virtual void OnMarkedBreakpoint(string details) = 0;
        virtual void OnFrameComplete() = 0;
    };
}