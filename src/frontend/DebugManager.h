#pragma once

#include "./FrontendPCH.h"
#include "./AppState.h"
#include "../backend/shared/IDebugger.h"
#include "../backend/shared/IConsole.h"

enum class DebugView {
    NONE = -1,
    CPU,
    PPU,
    APU,
    TOTAL_VIEWS
};

class DebugManager {
    private:
        std::shared_ptr<IDebugger> activeDebugger = nullptr;
    public:
        DebugView currentView = DebugView::NONE;

        DebugManager() = default;

        void SetDebugger(CONSOLE_ID, std::shared_ptr<IConsole>);
        IDebugger* GetDebugger() const;

        void CycleDebugViews(bool = true);
};