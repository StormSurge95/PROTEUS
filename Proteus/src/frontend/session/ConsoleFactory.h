#pragma once

#include "../FrontendPCH.h"
#include "../../shared/IConsole.h"
#include "../plugin/PluginManager.h"

namespace NS_Proteus {
    namespace ConsoleFactory {
        inline static sptr<IConsole> Create(ConsoleID console) {
            IConsole* c = PluginManager::CreateConsole(console);
            if (!c) {
                printf(PluginManager::GetLastError().c_str());
                return nullptr;
            }
            return sptr<IConsole>(c, [](IConsole* p) {
                PluginManager::DestroyConsole(p);
            });
        }
    }
}