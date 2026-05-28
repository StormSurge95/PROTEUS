#include "./PluginManager.h"
#include "../../shared/plugin_utils/PluginRegistry.h"

using namespace NS_Proteus;

bool PluginManager::Initialize() {
    // attempt discovery
    bool ok = PluginRegistry::DiscoverPlugins();

    // on failure, bubble up registry error
    if (!ok) {
        lastError = "Initialize() failure:\n\t" + PluginRegistry::GetLastError();
    } else {
        // discovery successful; mark initialized
        initialized = true;
    }

    return ok;
}

void PluginManager::Shutdown() {
    // best effort unload of all plugins
    PluginRegistry::UnloadAllPlugins();
    // manager no longer active
    initialized = false;
}

bool PluginManager::IsConsoleAvailable(ConsoleID id) {
    // manager must be initialized prior to availability checks
    if (!initialized) {
        lastError = "PluginManager is not initialized";
        return false;
    }

    // map console id to plugin id
    string pluginID = ToPluginID(id);

    // no mapping means unsupported console
    if (pluginID.empty()) return false;

    // discovered plugin means available console
    return PluginRegistry::IsPluginDiscovered(pluginID);
}

vector<ConsoleID> PluginManager::GetAvailableConsoles() {
    vector<ConsoleID> avail;

    // manager must be initialized prior to availability checks
    if (!initialized) {
        lastError = "PluginManager is not initialized";
        return avail;
    }

    // probe every known mapping and collect available consoles
    for (const auto& [id, str] : ConsolePluginMap) {
        if (IsConsoleAvailable(id)) avail.push_back(id);
    }

    return avail;
}

IConsole* PluginManager::CreateConsole(ConsoleID id) {
    // manager must be initialized prior to create
    if (!initialized) {
        lastError = "PluginManager is not initialized";
        return nullptr;
    }

    // map console id to plugin id
    string pluginID = ToPluginID(id);
    
    // no mapping means unsupported console
    if (pluginID.empty()) {
        lastError = "CreateConsole() failure: the requested console is currently unsupported";
        return nullptr;
    }

    // delegate creation to registry
    IConsole* c = PluginRegistry::CreateCore(pluginID);

    // on failure, bubble up registry error
    if (!c) {
        lastError = PluginRegistry::GetLastError();
    }
    return c;
}

bool PluginManager::DestroyConsole(IConsole* core) {
    // manager must be initialized prior to destroy
    if (!initialized) {
        lastError = "PluginManager is not initialized";
        return false;
    }

    // delegate destruction to registry
    bool b = PluginRegistry::DestroyCore(core);

    // on failure, bubble up registry error
    if (!b)
        lastError = PluginRegistry::GetLastError();

    return b;
}

IDebugger* PluginManager::CreateDebugger(ConsoleID id, IConsole* core) {
    // manager must be initialized prior to create
    if (!initialized) {
        lastError = "PluginManager is not initialized";
        return nullptr;
    }

    // map console id to plugin id
    string pluginID = ToPluginID(id);

    // no mapping means unsupported console
    if (pluginID.empty()) {
        lastError = "CreateDebugger() failure: the console of the requested debugger is currently unsupported";
        return nullptr;
    }

    // delegate creation to registry
    IDebugger* d = PluginRegistry::CreateDebugger(pluginID, core);

    // on failure, bubble up registry error
    if (!d)
        lastError = PluginRegistry::GetLastError();

    return d;
}

bool PluginManager::DestroyDebugger(IDebugger* dbg) {
    // manager must be initialized prior to destroy
    if (!initialized) {
        lastError = "PluginManager is not initialized";
        return false;
    }

    // delegate destruction to registry
    bool b = PluginRegistry::DestroyDebugger(dbg);

    // on failure, bubble up registry error
    if (!b)
        lastError = PluginRegistry::GetLastError();

    return b;
}

const PluginManifest* PluginManager::GetConsoleMetadata(ConsoleID id) {
    // manager must be initialized prior to query
    if (!initialized) {
        lastError = "PluginManager is not initialized";
        return nullptr;
    }

    // map console id to plugin id
    string pluginID = ToPluginID(id);

    // no mapping means unsupported console
    if (pluginID.empty()) {
        lastError = "GetConsoleMetadata() failure: the console of the requested metadata is currently unsupported";
        return nullptr;
    }

    // delegate query to registry
    const PluginManifest* p = PluginRegistry::GetManifest(pluginID);

    // on failure, bubble up registry error
    if (!p)
        lastError = PluginRegistry::GetLastError();

    return p;
}

string PluginManager::ToPluginID(ConsoleID id) {
    // find mapping for requested console id
    map<ConsoleID, string>::iterator it = ConsolePluginMap.find(id);

    // no mapping means unsupported console
    if (it == ConsolePluginMap.end()) return "";

    // return mapped plugin id
    return it->second;
}

ConsoleID PluginManager::ToConsoleID(const string& id) {
    // find mapping where plugin id matches input string
    map<ConsoleID, string>::iterator it = std::find_if(ConsolePluginMap.begin(),
        ConsolePluginMap.end(),
        [id](const pair<ConsoleID, string>& p) {
        return p.second == id;
    });

    // no mapping found
    if (it == ConsolePluginMap.end()) return ConsoleID::NONE;

    // return mapped console id
    return it->first;
}
