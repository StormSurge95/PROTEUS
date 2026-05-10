#pragma once

#include "./FrontendPCH.h"
#include "./Types.h"
#include "../backend/shared/IDebugger.h"
#include "../backend/shared/IConsole.h"

namespace NS_Proteus {
    class DebugManager {
        private:
            std::shared_ptr<IDebugger> activeDebugger = nullptr;
        public:
            DebugView currentView = DebugView::NONE;

            DebugManager() = default;

            void SetDebugger(ConsoleID, std::shared_ptr<IConsole>);
            IDebugger* GetDebugger() const;

            void CycleDebugViews(bool = true);
    };
}