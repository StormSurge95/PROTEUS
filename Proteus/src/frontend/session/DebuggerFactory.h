#pragma once

#include <FrontendPCH.h>
#include <IDebugger.h>
#include <IConsole.h>
#include <PluginManager.h>

namespace NS_Proteus {
    namespace DebuggerFactory {
        inline static sptr<IDebugger> Create(ConsoleID console, sptr<IConsole> station) {
            IDebugger* d = PluginManager::CreateDebugger(console, station.get());
            if (!d) return nullptr;
            return sptr<IDebugger>(d, [](IDebugger* p) {
                PluginManager::DestroyDebugger(p);
            });
        }
    };
}