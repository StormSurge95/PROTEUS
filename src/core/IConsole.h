#pragma once

#include <map>
#include <string>

class IConsole {
    public:
        virtual ~IConsole() = default;

        virtual void reset() = 0;
        virtual void clock() = 0;

        virtual bool loadCart(const std::string& path) = 0;

        virtual const uint32_t* getFrameBuffer() const = 0;

        virtual int SCREEN_WIDTH() const = 0;
        virtual int SCREEN_HEIGHT() const = 0;

        virtual void update(uint8_t player, bool* buttons) = 0;
};