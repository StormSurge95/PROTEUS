#pragma once

#include "../FrontendPCH.h"
#include "../../shared/IConsole.h"
#include "../../backend/Consoles.h"

namespace NS_Proteus {
    namespace ConsoleFactory {
        inline static sptr<IConsole> Create(ConsoleID console) {
            switch (console) {
                case ConsoleID::NES:
                    return make_shared<NS_NES::NES>();
                default:
                    return nullptr;
            }
        }
    }
}