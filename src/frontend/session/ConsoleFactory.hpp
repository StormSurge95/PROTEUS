#pragma once

#include "../FrontendPCH.hpp"
#include "../Types.hpp"
#include "../../backend/shared/IConsole.hpp"
#include "../../backend/Consoles.hpp"

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