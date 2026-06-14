#pragma once

#include "../FrontendPCH.h"
#include "../../shared/IConsole.h"
#include "../plugin/PluginManager.h"

namespace NS_Proteus {
    namespace ConsoleFactory {
        inline static ConsoleHandle Create(ConsoleID console) {
            IConsole* c = PluginManager::CreateConsole(console);
            if (!c) {
                printf("%s", PluginManager::GetLastError().c_str());
                return nullptr;
            }
            return ConsoleHandle(c);
        }
    }
}