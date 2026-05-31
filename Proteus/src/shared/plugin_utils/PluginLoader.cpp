#include <PluginLoader.h>

#ifdef _WIN32
    #include <Windows.h>
    #include <Shlwapi.h>
    #pragma comment(lib, "shlwapi.lib") 

    // prevent Windows.h macro from messing up our custom max function
    #undef max

    void* PluginLoader::LoadLib(const string& filePath) {
        SetLastError(0);
        HMODULE handle = LoadLibraryA(filePath.c_str());

        if (!handle) {
            DWORD error = ::GetLastError();
            char buffer[1024];
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buffer, sizeof(buffer), nullptr);
            lastError = string("LoadLibrary() failed: ") + buffer;
            return nullptr;
        }

        lastError.clear();
        return reinterpret_cast<void*>(handle);
    }

    bool PluginLoader::UnloadLib(void* handle) {
        if (!handle) return false;

        BOOL success = FreeLibrary(reinterpret_cast<HMODULE>(handle));

        if (!success) {
            DWORD error = ::GetLastError();
            char buffer[1024];
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buffer, sizeof(buffer), nullptr);
            lastError = string("FreeLibrary() failed: ") + buffer;
        }

        return success != 0;
    }

    void* PluginLoader::GetSym(void* handle, const string& symName) {
        if (!handle) {
            lastError = "Invalid library handle";
            return nullptr;
        }

        FARPROC sym = GetProcAddress(reinterpret_cast<HMODULE>(handle), symName.c_str());
        
        if (!sym) {
            DWORD error = ::GetLastError();
            char buffer[1024];
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buffer, sizeof(buffer), nullptr);
            lastError = string("GetProcAddress() failed: ") + buffer;
            return nullptr;
        }

        lastError.clear();
        return reinterpret_cast<void*>(sym);
    }

    string PluginLoader::GetPluginExtension() { return ".dll"; }
    string PluginLoader::GetLibraryPrefix() { return ""; }
#else
    #include <dlfcn.h>
    #include <cstring>

    void* PluginLoader::LoadLib(const string& filePath) {
        void* handle = dlopen(filePath.c_str(), RTLD_LAZY | RTLD_LOCAL);

        if (!handle) {
            const char* error = dlerror();
            lastError = error ? string(error) : "Unknown dlopen error";
            return nullptr;
        }

        lastError.clear();
        return handle;
    }

    bool PluginLoader::UnloadLib(void* handle) {
        if (!handle) return false;

        int result = dlclose(handle);

        if (result != 0) {
            const char* error = dlerror();
            lastError = error ? string(error) : "Unknown dlclose error";
        }

        return result == 0;
    }

    void* PluginLoader::GetSym(void* handle, const string& symName) {
        if (!handle) {
            lastError = "Invalid lib handle";
            return nullptr;
        }

        dlerror();  // clear previous errors
        void* sym = dlsym(handle, symName.c_str());

        const char* error = dlerror();
        if (error) {
            lastError = string(error);
            return nullptr;
        }

        lastError.clear();
        return sym;
    }

        #ifdef __linux__
        string PluginLoader::GetPluginExtension() { return ".so"; }
        #else // __APPLE__
        string PluginLoader::GetPluginExtension() { return ".dylib"; }
        #endif
    string PluginLoader::GetLibraryPrefix() { return "lib"; }
#endif

bool PluginLoader::LoadPlugin(const string& filePath, LoadedPlugin& core) {
    // verify file exists
    if (!exists(filePath)) {
        lastError = filePath + ": Plugin Library file does not exist";
        return false;
    }

    // load lib
    void* handle = LoadLib(filePath);
    if (!handle) {
        lastError = string("LoadPlugin - ") + lastError + "\n" + filePath + ": Plugin Library failed to load";
        return false;
    }

    // load function to get manifest
    GetManifestFunc getManifest = reinterpret_cast<GetManifestFunc>(
        GetSym(handle, "GetPluginManifest"));
    if (!getManifest) {
        UnloadLib(handle);
        lastError = string("LoadPlugin - ") + lastError + "\n" + filePath + ": GetCoreManifest() function missing";
        return false;
    }

    // get manifest to validate compat
    const PluginManifest* manifest = getManifest();
    if (!manifest) {
        lastError = string("LoadPlugin - ") + filePath + ": Plugin Library GetCoreManifest() returns null";
        UnloadLib(handle);
        return false;
    }

    // validate contract versions
    if (manifest->iConsoleContractVersion != ICONSOLE_CONTRACT_VERSION) {
        lastError = string("LoadPlugin - ") + filePath + ": CoreManifest/Application iConsoleContractVersion mismatch";
        UnloadLib(handle);
        return false;
    }

    // load core functions
    CreateCoreFunc createCore = reinterpret_cast<CreateCoreFunc>(
        GetSym(handle, "CreateCore"));
    DestroyCoreFunc destroyCore = reinterpret_cast<DestroyCoreFunc>(
        GetSym(handle, "DestroyCore"));
    if (!createCore) {
        lastError = string("LoadPlugin - ") + lastError + "\n" + filePath + ": CreateCore() function missing";
        UnloadLib(handle);
        return false;
    }
    if (!destroyCore) {
        lastError = string("LoadPlugin - ") + lastError + "\n" + filePath + ": DestroyCore() function missing";
        UnloadLib(handle);
        return false;
    }

    // load optional debugger functions
    CreateDebuggerFunc createDebugger = reinterpret_cast<CreateDebuggerFunc>(
        GetSym(handle, "CreateDebugger"));
    DestroyDebuggerFunc destroyDebugger = reinterpret_cast<DestroyDebuggerFunc>(
        GetSym(handle, "DestroyDebugger"));

    // debugger functions must both exist or both be null
    if (bool(createDebugger) != bool(destroyDebugger)) {
        lastError = string("LoadPlugin - ") + lastError + "\n" + filePath + ": CreateDebugger() and DestroyDebugger() must either BOTH exist or BOTH be null";
        UnloadLib(handle);
        return false;
    }

    core.filePath = filePath;
    core.consoleName = manifest->consoleName;
    core.libHandle = handle;
    core.CreateCore = createCore;
    core.DestroyCore = destroyCore;
    core.CreateDebugger = createDebugger;
    core.DestroyDebugger = destroyDebugger;
    core.GetManifest = getManifest;
    core.manifest = *manifest;

    return true;
}

bool PluginLoader::UnloadPlugin(LoadedPlugin& core) {
    if (!core.libHandle) {
        lastError = "UnloadPlugin() failed: null library handle";
        return false;
    }

    if (!UnloadLib(core.libHandle)) {
        lastError += "\nUnloadPlugin() failure during platform call";
        return false;
    }

    core.libHandle = nullptr;
    return true;
}

string PluginLoader::GetExpectedFilename(const string& libName) {
    return GetLibraryPrefix() + libName + GetPluginExtension();
}