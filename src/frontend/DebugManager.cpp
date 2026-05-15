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
        // TODO: add other console debuggers
    }
}