#pragma once

#include "SstTypes.h"

/**
 * @interface IConsole
 * @brief Primary bridge interface between frontend and emulator cores
 * @details This interface is the CONTRACT that all console emulation
 *          cores MUST implement. Frontend code ONLY interacts with
 *          console cores through this interface.
 * @important Changes to this interface are BREAKING CHANGES for all
 *            existing cores. Version carefully.
 */
class IConsole {
    public:
        /// @brief default virtual destructor
        virtual ~IConsole() = default;

        /**
         * @brief Initialize the console
         * @return true if successful
         */
        virtual bool initialize() { return true; }

        /**
         * @brief Shutdown the console and release resources
         * @return true if successful
         */
        virtual bool shutdown() { return true; }

        /**
         * @brief Load a ROM file into the console
         * @param romPath Full path to ROM file
         * @return true if loaded successfully
         */
        virtual bool loadROM(const string&) = 0;

        /// @brief Reset the console to initial state
        virtual void reset() = 0;

        /// @brief required for frontend to progress emulation
        virtual void clock() = 0;

        /**
         * @brief Get the current framebuffer
         * @return Non-ownding pointer managed by the core;
         *      valid until the next `clock()` call or `shutdown()`.
         *      Never free this pointer in frontend code.
         */
        virtual const u32* getFrameBuffer() const = 0;

        /// @brief Get native screen width in pixels
        const virtual int SCREEN_WIDTH() const = 0;

        /// @brief Get native screen height in pixels
        const virtual int SCREEN_HEIGHT() const = 0;

        /**
         * @brief Collect audio samples from the console
         * @param outSamples Vector to fill with float audio data (-1.0 to 1.0)
         */
        virtual void collectAudio(vector<float>&) = 0;

        /**
         * @brief Number of button states expected by `update(...)` for this console
         */
        virtual size_t buttonCount() const = 0;

        /**
         * @brief Send input state to the console
         * @param playerIndex Player index (0-3 for multiplayer)
         * @param buttonStates Pointer to at least `buttonCount()` booleans.
         *          Read-only for the duration of this call; must not be null
         */
        virtual void update(u8, const bool*) = 0;

        /**
         * @brief Initialize console for Single State Test
         * @param state The initial/starting state required by the test
         */
        virtual void initSST(SingleStateTest::State) = 0;

        /// @brief Execute the currently initialized test
        virtual void runSST() = 0;

        /**
         * @brief Compare the final state of the system to the expected state
         * @param state The expected final state to compare to.
         * @param message Reference variable for error messages; or "SUCCESS" if no error occurred.
         * @return true if final state of system is equivalent to expected final state.
         */
        virtual bool checkSST(SingleStateTest::State, string&) = 0;
};

// Plugin contract version - increment on breaking changes
#define ICONSOLE_CONTRACT_VERSION 1