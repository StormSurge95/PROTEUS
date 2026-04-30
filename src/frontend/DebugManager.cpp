#include "./DebugManager.h"

#include "../backend/NES/NES_DBG/NES_DBG.h"

void DebugManager::SetDebugger(CONSOLE_ID dbgType, std::shared_ptr<IConsole> station) {
    switch (dbgType) {
        default:
        case NES:
            activeDebugger = std::make_shared<NES_DBG>(std::dynamic_pointer_cast<NES_CORE>(station));
            return;
    }
}

IDebugger* DebugManager::GetDebugger() const {
    return activeDebugger.get();
}

void DebugManager::CycleDebugViews(bool active) {
    if (!active)
        currentView = DebugView::NONE;
    else if (currentView == DebugView::NONE)
        currentView = DebugView::CPU;
    else
        currentView = static_cast<DebugView>(
            (static_cast<int>(currentView) + 1) % static_cast<int>(DebugView::TOTAL_VIEWS)
        );
}