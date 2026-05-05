#pragma once

#include "NES_PCH.h"

namespace NES_NS {
    class Controller : public IDeviceIO<u8> {
        friend class Debugger;
        public:
            bool strobe = false;
            u8 cursor = 0x00;
            wptr<Controller> other;

            Controller(u8 p) { player = p; }
            
            u8 onRead() override;
            void onWrite(u8) override;

            inline void update(BUTTONS btn, bool isPressed) { buttons[static_cast<int>(btn)] = isPressed; }
        private:
            array<bool, 8> buttons = { false };
            u8 player = 0x00;
    };
}