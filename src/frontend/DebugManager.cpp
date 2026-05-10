#include "./DebugManager.h"

#include "../backend/NES/NES.h"
#include "../backend/NES/Debugger.h"

using namespace NS_Proteus;

void DebugManager::SetDebugger(ConsoleID dbgType, std::shared_ptr<IConsole> station) {
    switch (dbgType) {
        default:
        case ConsoleID::NES:
            activeDebugger = make_shared<NES_NS::Debugger>(std::dynamic_pointer_cast<NES_NS::NES>(station));
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