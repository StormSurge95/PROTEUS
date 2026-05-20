#pragma once

#include "./FrontendPCH.h"
#include "./Types.h"
#include "../backend/shared/IConsole.h"
#include "../backend/Consoles.h"

namespace NS_Proteus {
    namespace ConsoleFactory {
        inline static sptr<IConsole> Create(ConsoleID console) {
            switch (console) {
                case ConsoleID::NES:
                    return make_shared<NES_NS::NES>();
                default:
                    return nullptr;
            }
        }
    }
}