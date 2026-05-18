#pragma once

#include "./FrontendPCH.h"
#include "./Types.h"
#include "../backend/shared/IDebugger.h"
#include "../backend/shared/IConsole.h"

namespace NS_Proteus {
    class DebugManager {
        private:
            sptr<IDebugger> activeDebugger = nullptr;
        public:
            DebugView currentView = DebugView::NONE;

            DebugManager() = default;
            ~DebugManager() = default;

            DebugManager(const DebugManager&) = delete;
            DebugManager& operator=(const DebugManager&) = delete;
            DebugManager(DebugManager&&) = delete;
            DebugManager& operator=(DebugManager&&) = delete;

            void SetDebugger(ConsoleID, sptr<IConsole>);
            sptr<IDebugger>& GetDebugger() { return activeDebugger; }
    };
}