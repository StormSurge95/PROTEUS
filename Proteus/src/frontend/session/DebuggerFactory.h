#pragma once

#include "../FrontendPCH.h"
#include "../../shared/Types.h"
#include "../../shared/IDebugger.h"
#include "../../shared/IConsole.h"
#include "../plugin/PluginManager.h"

namespace NS_Proteus {
    namespace DebuggerFactory {
        inline static DebuggerHandle Create(ConsoleID console, IConsole* station) {
            IDebugger* d = PluginManager::CreateDebugger(console, station);
            if (!d) return nullptr;
            return DebuggerHandle(d);
        }
    };
}