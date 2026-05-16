#pragma once

#include "./shared/NES_PCH.h"

namespace NES_NS {
    class Controller : public IDeviceIO<u8> {
        friend class Debugger;
        public:
            /// @brief strobe flag
            bool strobe = false;
            /// @brief refers to which button is currently being polled
            u8 cursor = 0x00;
            /// @brief refers to the other controller
            wptr<Controller> other;

            /**
             * @brief Explicit Constructor
             * @param p The player id to set this controller to (i.e. 1 or 2)
             */
            Controller(u8 p) { player = p; }
            
            /**
             * @brief Data read request for Controller(s).
             * @return Whether the button referred to by `cursor` is pressed.
             */
            u8 onRead() override;
            /**
             * @brief Write request for Controller(s).
             * @param d The data to be written.
             */
            void onWrite(u8 d) override;

            /**
             * @brief Helper function to update the current state of controller buttons.
             * @param btn The button to be updated
             * @param isPressed Whether or not the button is currently pressed.
             */
            inline void update(BUTTONS btn, bool isPressed) { buttons[static_cast<int>(btn)] = isPressed; }
        private:
            /// @brief The set of pollable buttons on a NES controller
            array<bool, 8> buttons = { false };
            /// @brief the player ID
            u8 player = 0x00;
    };
}