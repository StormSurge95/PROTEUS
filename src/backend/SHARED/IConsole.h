#pragma once

#include "../BackendPCH.h"

class IConsole {
    public:
        virtual ~IConsole() = default;

        virtual void reset() = 0;
        virtual void clock() = 0;

        virtual bool loadROM(const string&) = 0;

        virtual const u32* getFrameBuffer() const = 0;
        virtual void collectAudio(vector<float>&) = 0;

        const virtual int SCREEN_WIDTH() const = 0;
        const virtual int SCREEN_HEIGHT() const = 0;

        virtual void update(u8, bool*) = 0;
};