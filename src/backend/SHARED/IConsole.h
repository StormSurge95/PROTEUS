#pragma once

#include "./BackendPCH.h"
/**
 * @interface IConsole IConsole.h "backend/SHARED/IConsole.h"
 * @brief "Manager" interface class implemented by each of the various consoles.
 * @details
 * Includes various functions required for interaction with the frontend.
 */
class IConsole {
    public:
        /// @brief default virtual deconstructor
        virtual ~IConsole() = default;
        /// @brief required for frontend to reset consoles/games
        virtual void reset() = 0;
        /// @brief required for frontend to progress emulation
        virtual void clock() = 0;
        /// @brief required for frontend to start games
        virtual bool loadROM(const string&) = 0;
        /// @brief required for frontend to retrieve picture data for rendering
        virtual const u32* getFrameBuffer() const = 0;
        /// @brief required for frontend to retrieve audio data for playback
        virtual void collectAudio(vector<float>&) = 0;
        /// @brief required for frontend to retrieve the base frame width
        const virtual int SCREEN_WIDTH() const = 0;
        /// @brief required for frontend to retrieve the base frame height
        const virtual int SCREEN_HEIGHT() const = 0;
        /// @brief required for frontend to pass necessary inputs to the console
        virtual void update(u8, bool*) = 0;
};