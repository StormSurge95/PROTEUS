#pragma once

#include "./NesPCH.h"

struct NesEventSink {
    virtual ~NesEventSink() = default;

    virtual void OnPpuRegisterRead(u16 addr, u8 value) = 0;
    virtual void OnPpuRegisterWrite(u16 addr, u8 value) = 0;

    virtual void OnApuRegisterRead(u16 addr, u8 value) = 0;
    virtual void OnApuRegisterWrite(u16 addr, u8 value) = 0;

    virtual void OnMapperRegisterRead(string details, u16 addr, u8 value) = 0;
    virtual void OnMapperRegisterWrite(string details, u16 addr, u8 value) = 0;

    virtual void OnControllerRead(string details, u16 addr, u8 value) = 0;
    virtual void OnControllerWrite(string details, u16 addr, u8 value) = 0;

    virtual void OnDmcDmaRead(string details, u16 addr, u8 value) = 0;
    virtual void OnOamDmaRead(u16 addr, u8 value) = 0;
    virtual void OnOamDmaStart(u8 value) = 0;
    
    virtual void OnInterrupt(string type, string details) = 0;
    virtual void OnSpriteZeroHit() = 0;
    virtual void OnMarkedBreakpoint(string details) = 0;
    virtual void OnFrameComplete() = 0;
};