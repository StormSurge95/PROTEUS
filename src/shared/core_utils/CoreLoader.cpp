#include "./CoreLoader.h"

#ifdef _WIN32
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#undef max // prevent Windows.h macro from messing up our custom max function

void* CoreLoader::LoadLib(const string& filePath) {
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

bool CoreLoader::UnloadLib(void* handle) {
    if (!handle) return false;

    BOOL success = FreeLibrary(reinterpret_cast<HMODULE>(handle));

    if (!success) {
        DWORD error = ::GetLastError();
        char buffer[1024];
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buffer, sizeof(buffer), nullptr);
        lastError = string("FreeLibrary() failed: ") + buffer;
    } else
        lastError.clear();

    return success != 0;
}

void* CoreLoader::GetSym(void* handle, const string& symName) {
    if (!handle) {
        lastError = "Invalid library handle";
        return nullptr;
    }

    FARPROC sym = GetProcAddress(reinterpret_cast<HMODULE>(handle), symName.c_str());
    
    if (!sym) {
        lastError = "Symbol not found: " + symName;
        return nullptr;
    }

    lastError.clear();
    return reinterpret_cast<void*>(sym);
}

string CoreLoader::GetCoreExtension() { return ".dll"; }
string CoreLoader::GetLibraryPrefix() { return ""; }
#else
#include <dlfcn.h>
#include <cstring>

void* CoreLoader::LoadLib(const string& filePath) {
    void* handle = dlopen(filePath.c_str(), RTLD_LAZY | RTLD_LOCAL);

    if (!handle) {
        const char* error = dlerror();
        lastError = error ? string(error) : "Unknown dlopen error";
        return nullptr;
    }

    lastError.clear();
    return handle;
}

bool CoreLoader::UnloadLib(void* handle) {
    if (!handle) return false;

    int result = dlclose(handle);

    if (result != 0) {
        const char* error = dlerror();
        lastError = error ? string(error) : "Unknown dlclose error";
    } else
        lastError.clear();

    return result == 0;
}

void* CoreLoader::GetSym(void* handle, const string& symName) {
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
    string CoreLoader::GetCoreExtension() { return ".so"; }
    #else // __APPLE__
    string CoreLoader::GetCoreExtension() { return ".dylib"; }
    #endif
string CoreLoader::GetLibraryPrefix() { return ".lib"; }
#endif

bool CoreLoader::LoadCore(const string& filePath, LoadedCore& core) {
    // verify file exists
    if (!exists(filePath)) return false;

    // load lib
    void* handle = LoadLib(filePath);
    if (!handle) return false;

    // load function to get manifest
    GetManifestFunc getManifest = reinterpret_cast<GetManifestFunc>(
        GetSym(handle, "GetCoreManifest"));
    if (!getManifest) {
        UnloadLib(handle);
        return false;
    }

    // get manifest to validate compat
    const CoreManifest* manifest = getManifest();
    if (!manifest) {
        UnloadLib(handle);
        return false;
    }

    // validate contract versions
    if (manifest->iConsoleContractVersion != ICONSOLE_CONTRACT_VERSION) {
        UnloadLib(handle);
        return false;
    }

    // load core functions
    CreateCoreFunc createCore = reinterpret_cast<CreateCoreFunc>(
        GetSym(handle, "CreateCore"));
    DestroyCoreFunc destroyCore = reinterpret_cast<DestroyCoreFunc>(
        GetSym(handle, "DestroyCore"));
    if (!createCore || !destroyCore) {
        UnloadLib(handle);
        return false;
    }

    // load optional debugger functions
    CreateDebuggerFunc createDebugger = reinterpret_cast<CreateDebuggerFunc>(
        GetSym(handle, "CreateDebugger"));
    DestroyDebuggerFunc destroyDebugger = reinterpret_cast<DestroyDebuggerFunc>(
        GetSym(handle, "DestroyDebugger"));

    // debugger function must both exist or both be null
    if (bool(createDebugger) != bool(destroyDebugger)) {
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

bool CoreLoader::UnloadCore(LoadedCore& core) {
    if (!core.libHandle) return false;

    if (!UnloadLib(core.libHandle)) return false;

    core.libHandle = nullptr;
    return true;
}

string CoreLoader::GetExpectedFilename(const string& consoleName) {
    return GetLibraryPrefix() + consoleName + GetCoreExtension();
}