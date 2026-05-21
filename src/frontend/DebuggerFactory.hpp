#pragma once

#include "./FrontendPCH.hpp"
#include "./Types.hpp"
#include "../backend/shared/IDebugger.hpp"
#include "../backend/shared/IConsole.hpp"
#include "../backend/Debuggers.hpp"

namespace NS_Proteus {
    namespace DebuggerFactory {
        inline static sptr<IDebugger> Create(ConsoleID console, sptr<IConsole> station) {
            switch (console) {
                case ConsoleID::NES:
                    return dynamic_pointer_cast<IDebugger>(make_shared<NES_NS::Debugger>(dynamic_pointer_cast<NES_NS::NES>(station)));
                default: return nullptr;
                    // TODO: add other console debuggers
            }
        }
    };
}