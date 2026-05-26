#pragma once

#include "../FrontendPCH.h"
#include "../../shared/Types.h"
#include "../../shared/IDebugger.h"
#include "../../shared/IConsole.h"
#include "../../backend/Debuggers.h"

namespace NS_Proteus {
    namespace DebuggerFactory {
        inline static sptr<IDebugger> Create(ConsoleID console, sptr<IConsole> station) {
            switch (console) {
                case ConsoleID::NES:
                    return dynamic_pointer_cast<IDebugger>(make_shared<NS_NES::NesDebugger>(reinterpret_cast<NS_NES::NES*>(station.get())));
                default: return nullptr;
                    // TODO: add other console debuggers
            }
        }
    };
}