#include "./PluginRegistry.h"
#include "../Utilities.h"

#if defined(_WIN32)
#include <Windows.h>
#else
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#include <unistd.h>
#endif

bool PluginRegistry::DiscoverPlugins() {
    // initialize helper variables
    vector<string> paths = GetSearchPaths();
    vector<string> ids = GetKnownPluginIDs();

    // reset registry map 
    for (auto& [id, entry] : registry) {
        entry.discovered = false;
        entry.filePath = "";
    }

    // initialize count of discovered plugins
    u32 discoveredCount = 0;

    // search through each path for each id
    for (const string& id : ids) {
        string filename = PluginLoader::GetExpectedFilename(libBaseNames[id]);
        bool found = false;
        for (const string& dir : paths) {
            path candidate = path(dir) / filename;
            // if a plugin is found, add it to our map; regardless of compatibility
            if (exists(candidate)) {
                registry[id].id = id;
                registry[id].filePath = candidate.string();
                registry[id].discovered = true;
                found = true;
                discoveredCount++;
                break;
            }
        }
        // if no plugin is found, and no plugin was previously found, add new entry to registry map
        if (!found && !registry.contains(id))
            registry[id] = { .id = id, .discovered = false, .loaded = false };
    }

    // if no plugins found, update lastError to list all paths searched and all IDs searched for
    if (discoveredCount == 0) {
        stringstream ss;
        ss << "No core plugins discovered. Paths searched:\n";
        for (const string& s : paths)
            ss << "\t" << s << endl;
        ss << "IDs:\n";
        for (const string& id : ids)
            ss << "\t" << id << endl;
        lastError = ss.str();
        return false;
    }

    // on successful discovery, clear lastError and return success
    lastError = "";
    return true;
}

bool PluginRegistry::LoadPlugin(const string& id) {
    // if the requested plugin is not even discovered; then it cannot be loaded
    if (!IsPluginDiscovered(id)) {
        lastError = "LoadPlugin() failure: the requested library (" + id + ") was not discoverable";
        return false;
    }
    
    // plugin is discovered, and therefore exists within our map;
    // so accessing via the id is safe
    RegistryEntry& e = registry[id];

    // if it's already loaded, no further processing is necessary
    if (e.loaded) return true;

    // try to load the plugin; update lastError upon failure
    if (!PluginLoader::LoadPlugin(e.filePath, e.plugin)) {
        lastError = "LoadPlugin() failed: ";
        lastError += PluginLoader::GetLastError();
        return false;
    }

    // load successful; update entry, clear lastError, and return success
    lastError = "";
    e.loaded = true;
    return true;
}

void PluginRegistry::LoadAllPlugins() {
    // observer variable to determine success
    bool allLoaded = true;
    // container for any errors produced along the way
    vector<string> errors;
    for (auto& [id, entry] : registry) {
        // skip entries marked as undiscovered
        if (!entry.discovered) continue;
        // attempt to load each plugin; upon failure, add the most recent error to our container
        if (!LoadPlugin(id)) {
            allLoaded = false;
            errors.push_back("\t" + id + ": " + lastError + "\n");
        }
    }

    // if there were any load failures, update lastError to list them all
    if (!allLoaded) {
        stringstream ss;
        for (const string& s : errors) ss << s;
        lastError = "Some plugins failed to load:\n" + ss.str();
    }
}

bool PluginRegistry::UnloadPlugin(const string& id) {
    // If the plugin is not currently loaded; no further processing is necessary
    if (!IsPluginLoaded(id)) return true;

    // plugin is currentlyloaded, and therefore exists within our map;
    // so accessing via the id is safe
    RegistryEntry& e = registry[id];

    // try to unload the plugin; update lastError upon failure
    if (!PluginLoader::UnloadPlugin(e.plugin)) {
        lastError = "UnloadPlugin() failed: ";
        lastError += PluginLoader::GetLastError();
        return false;
    }

    // unload successful; update entry, clear lastError, and return success
    lastError = "";
    e.loaded = false;
    return true;
}

void PluginRegistry::UnloadAllPlugins() {
    // observer variable to determine success
    bool allUnloaded = true;
    // container for any errors produced along the way
    vector<string> errors;
    for (auto& [id, entry] : registry) {
        // attempt to unload each plugin; upon failure, add the most recent error to our container
        if (!UnloadPlugin(id)) {
            allUnloaded = false;
            errors.push_back("\t" + id + ": " + lastError + "\n");
        }
    }

    // if there were any unload failures, update lastError to list them all
    if (!allUnloaded) {
        stringstream ss;
        for (const string& s : errors) ss << s;
        lastError = "Some plugins failed to unload:\n" + ss.str();
    }
}

IConsole* PluginRegistry::CreateCore(const string& id) {
    // if the plugin is not discovered, record error and return null
    if (!IsPluginDiscovered(id)) {
        lastError = "CreateCore() failure: the requested id (" + id + ") does not currently refer to a valid library";
        return nullptr;
    }

    // if plugin is not already loaded, attempt to load it
    if (!IsPluginLoaded(id)) {
        // if plugin fails to load, record error and return null
        if (!LoadPlugin(id)) {
            lastError = "CreateCore() failure:\n\t" + lastError;
            return nullptr;
        }
    }

    // plugin is loaded; attempt core creation
    IConsole* c = registry[id].plugin.CreateCore();

    // if creation fails; record error and return null
    if (!c) {
        lastError = "CreateCore() failure: internal failure within registry->plugin->CreateCore()";
        return nullptr;
    }

    // core creation successful; clear lastError and return pointer
    lastError = "";
    consoleCreators[c] = id;
    return c;
}

bool PluginRegistry::DestroyCore(IConsole* core) {
    // if the core is already null; then we don't need to do anything
    if (!core) return true;

    auto it = consoleCreators.find(core);
    if (it == consoleCreators.end()) {
        lastError = "DestroyCore() failure: unknown core pointer (not owned by registry)";
        return false;
    }

    const string id = it->second;

    // if the plugin is not loaded, or not discovered, then something has gone wrong during runtime and/or implementation
    if (!IsPluginLoaded(id)) {
        lastError = "DestroyCore() failure: Attempt to destroy core reference via unloaded library";
        return false;
    }

    // remove map entry to prevent stale pointer
    consoleCreators.erase(core);
    // use obtained id to destroy core
    registry[id].plugin.DestroyCore(core);
    // clear last error
    lastError = "";
    // return success
    return true;
}

IDebugger* PluginRegistry::CreateDebugger(const string& id, IConsole* core) {
    if (!core) {
        lastError = "CreateDebugger() failure: the provided core instance was null; a valid core is required to create a debugger";
        return nullptr;
    }

    // if plugin undiscovered, record error and return null
    if (!IsPluginDiscovered(id)) {
        lastError = "CreateDebugger() failure: the requested id (" + id + ") does not currently refer to a valid library";
        return nullptr;
    }

    // if plugin is not already loaded, attempt to load it
    if (!IsPluginLoaded(id)) {
        // if plugin load fails, record error and return null
        if (!LoadPlugin(id)) {
            lastError = "CreateDebugger() failure:\n\t" + lastError;
            return nullptr;
        }
    }

    // plugin is loaded; attempt to get debugger creation function
    PluginLoader::CreateDebuggerFunc f = registry[id].plugin.CreateDebugger;
    if (!f) {
        lastError = "CreateDebugger() failure: IDebugger not currently implemented by provided library core";
        return nullptr;
    }

    // plugin is loaded and debugger creation function exists; attempt to create debugger
    IDebugger* d = f(core);

    // if creation fails, record error and return null
    if (!d) {
        lastError = "CreateDebugger() failure: internal failure within registry->plugin->CreateDebugger()";
        return nullptr;
    }

    // debugger creation successful; clear lastError and return pointer
    lastError = "";
    debuggerCreators[d] = id;
    return d;
}

bool PluginRegistry::DestroyDebugger(IDebugger* dbg) {
    // if the debugger is already null; then we don't need to do anything
    if (!dbg) return true;

    auto it = debuggerCreators.find(dbg);
    if (it == debuggerCreators.end()) {
        lastError = "DestroyDebugger() failure: unknown debugger pointer (not owned by registry)";
        return false;
    }

    const string id = it->second;

    // if the plugin is not loaded or not discovered, then something has gone wrong during runtime and/or implementation
    if (!IsPluginLoaded(id)) {
        lastError = "DestroyDebugger() failure: Attempt to destroy debugger reference via unloaded library";
        return false;
    }

    // plugin is loaded; verify destroy function exists
    PluginLoader::DestroyDebuggerFunc f = registry[id].plugin.DestroyDebugger;
    if (!f) {
        lastError = "CreateDebugger() failure: IDebugger not currently implemented by provided library core";
        lastError += "\n...how did you manage to get this far..?";
        return false;
    }

    // function exists; proceed with destruction
    // -----------------------------------------
    // remove map entry to prevent stale pointer
    debuggerCreators.erase(dbg);
    // use obtained function to destroy debugger
    f(dbg);
    // clear last error
    lastError = "";
    // return success
    return true;
}

bool PluginRegistry::IsPluginDiscovered(const string& id) {
    // can't be discovered if it's not even in the registry
    if (!registry.contains(id)) return false;
    // use registry value as return value
    return registry[id].discovered;
}

bool PluginRegistry::IsPluginLoaded(const string& id) {
    // can't be loaded if it's not even discovered
    if (!IsPluginDiscovered(id)) {
        lastError = "No discovered core for `" + id + "` exists";
        return false;
    }
    // use registry value as return value
    return registry[id].loaded;
}

const PluginManifest* PluginRegistry::GetManifest(const string& id) {
    // if the plugin is not discovered, record error and return null
    if (!IsPluginDiscovered(id)) {
        lastError = "GetManifest() failure: the requested id (" + id + ") does not currently refer to a valid library";
        return nullptr;
    }

    // if plugin is not already loaded, attemtp to load it
    if (!IsPluginLoaded(id)) {
        // if plugin fails to load, record error and return null
        if (!LoadPlugin(id)) {
            lastError = "GetManifest() failure:\n\t" + lastError;
            return nullptr;
        }
    }

    // plugin is loaded; attempt to get manifest
    const PluginManifest* m = registry[id].plugin.GetManifest();

    // if get fails; record error and return null
    if (!m) {
        lastError = "GetManifest() failure: internal failure within registry->plugin->GetManifest()";
        return nullptr;
    }

    // get of manifest successful; clear lastError and return pointer
    lastError = "";
    return m;
}

vector<string> PluginRegistry::GetDiscoveredPlugins() {
    vector<string> d;
    
    // no need to use member function here as only IDs contained within the registry are tested
    for (const auto& [id, entry] : registry)
        if (entry.discovered) d.push_back(id);

    return d;
}

vector<string> PluginRegistry::GetLoadedPlugins() {
    vector<string> l;

    for (const auto& [id, entry] : registry)
        if (entry.loaded) l.push_back(id);

    return l;
}

/**
 * @brief Use OS-specific methods to get the system path of the executable
 * @return std::filesystem::path object containing the executable filepath
 */
static path GetExecutablePath() {
    #if defined(_WIN32)
        char buffer[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        return (len > 0) ? path(buffer) : path();
    #elif defined(__APPLE__)
        u32 size = 0;
        _NSGetExecutablePath(nullptr, &size);
        string buf(size, '\0');
        if (_NSGetExecutablePath(buf.data(), &size) == 0)
            return weakly_canonical(path(buf.c_str()));
        return {};
    #else
        char buffer[4096] = {};
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            return path(buffer);
        }
        return {};
    #endif
}

/**
 * @brief Adds a path to a vector of valid paths if it is a valid existing directory
 * @param [in,out] out Reference to the vector of valid directory paths
 * @param [in,out] seen Reference to a set of seen paths
 * @param [in] p The path to be tested and possibly added to `out`
 */
static void AddIfDir(vector<string>& out, unordered_set<string>& seen, path p) {
    // initialize var to hold any possible error code(s) produced
    std::error_code ec;

    // make sure `p` is not an empty path
    if (p.empty()) return;

    // normalize the path variable
    path norm = weakly_canonical(p, ec);
    if (ec) norm = p.lexically_normal();

    // make sure the produced normalized path exists and is a directory
    if (!exists(norm, ec) || !is_directory(norm, ec)) return;

    // add the verified path to `out` only if we haven't already
    string s = norm.string();
    if (seen.insert(s).second) out.push_back(s);
}

vector<string> PluginRegistry::GetSearchPaths() {
    // initialize return variable
    vector<string> paths;
    // initialize helper variables
    unordered_set<string> seen;

    // try to get the current env directory variable from PATH
    string envPluginDir;
    #ifdef _WIN32
    char* raw = nullptr;
    size_t len = 0;
    if (_dupenv_s(&raw, &len, "PROTEUS_PLUGIN_DIR") == 0 && raw != nullptr)
        envPluginDir = raw;
    free(raw);
    #else
    if (const char* env = getenv("PROTEUS_PLUGIN_DIR"); env && *env)
        envPluginDir = env;
    #endif
    // ensure that the value obtained from PATH actually exists before trying to add it
    if (!envPluginDir.empty())
        AddIfDir(paths, seen, path(envPluginDir));

    // get the current path of the executable
    path exePath = GetExecutablePath();
    if (!exePath.empty()) // not sure how it could be empty if the executable is running; but test just in case
        AddIfDir(paths, seen, exePath.parent_path() / "plugins");

    #if defined(_DEBUG)
    const char* cfg = "Debug";
    #else
    const char* cfg = "Release";
    #endif

    std::error_code ec;
    path cwd = current_path(ec);
    if (!ec && !cwd.empty()) {
        AddIfDir(paths, seen, cwd / "plugins");
        AddIfDir(paths, seen, cwd / "plugins" / cfg);
    }

    return paths;
}

vector<string> PluginRegistry::GetKnownPluginIDs() {
    vector<string> v;

    for (const auto& [key, val] : libBaseNames) {
        v.push_back(key);
    }

    return v;
}

PluginRegistry::RegistryEntry* PluginRegistry::FindEntryMutable(const string& id) {
    // if registry does not contain the key, record error and return null
    if (!registry.contains(id)) {
        lastError = "FindEntryMutable() failure: no valid registry entry found for `" + id + "`";
        return nullptr;
    } else {
        // otherwise, return a reference to the requested entry
        return &registry[id];
    }
}

const PluginRegistry::RegistryEntry* PluginRegistry::FindEntry(const string& id) {
    // no need to duplicate code; simply perform implicit conversion from non-const to const
    return FindEntryMutable(id);
}