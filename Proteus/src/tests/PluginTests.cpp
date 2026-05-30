#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../shared/plugin_utils/PluginRegistry.h"
#include "../shared/IConsole.h"

using std::string;
using std::runtime_error;
using std::exception;
using std::function;
using std::vector;
using std::pair;
using std::cout;

/// @brief Helper structure for keeping track of test results
struct TestResult {
    /// @brief The name of the test
    string name;
    /// @brief Whether the test passed or failed
    bool passed;
    /// @brief The error message of a failed test; or an empty string if test passed
    string message;
};

/**
 * @brief Assertion helper to determine pass/fail of various tests
 * @param cond The condition to assert (true = pass, false = fail)
 * @param msg The message to use in case of failure
 */
static void ExpectTrue(bool cond, const string& msg) {
    if (!cond) throw runtime_error(msg);
}

/**
 * @brief Helper function for running the various tests
 * @param name The test name
 * @param fn The test function
 * @return TestResult struct
 */
static TestResult RunTest(const string& name, const function<void()>& fn) {
    try {
        fn();
        return TestResult{ name, true, "" };
    } catch (const exception& e) {
        return TestResult{ name, false, e.what() };
    } catch (...) {
        return TestResult{ name, false, "Unknown exception" };
    }
}

/// @brief Asserts that PluginRegistry::DiscoverPlugins() is successful
static void RequireDiscovery() {
    ExpectTrue(PluginRegistry::DiscoverPlugins(), "DiscoverPlugins failed to find any plugins.");
}

/// @brief Asserts that attempting to load a plugin that doesn't exist will fail cleanly by not crashing
static void Test_LoadMissingPluginFails() {
    RequireDiscovery();
    const bool ok = PluginRegistry::LoadPlugin("does_not_exist");
    ExpectTrue(!ok, "LoadPlugin succeeded unexpectedly for invalid id.");

    string error = PluginRegistry::GetLastError();
    ExpectTrue(error.find("not discoverable") != string::npos, "LoadPlugin error message mismatch:\n\t" + error);
}

/// @brief Asserts that loading a NES plugin, creating/destroying a core, and then unloading the plugin are all successful.
static void Test_CoreAndDebuggerLifecycle() {
    RequireDiscovery();

    // Confirm that NES plugin successfully loads
    const string kId = "nes";
    ExpectTrue(PluginRegistry::LoadPlugin(kId), "Failed to load NES plugin.");
    ExpectTrue(PluginRegistry::IsPluginLoaded(kId), "NES plugin not marked loaded.");

    // Confirm that NES core is successfully created
    IConsole* core = PluginRegistry::CreateCore(kId);
    ExpectTrue(core != nullptr, "CreateCore returned null.");

    // CreateDebugger is allowed to return nullptr
    IDebugger* dbg = PluginRegistry::CreateDebugger(kId, core);
    // Confirm that DestroyDebugger is successful regardless of creation result
    ExpectTrue(PluginRegistry::DestroyDebugger(dbg), "DestroyDebugger failed.");

    // Confirm that NES core is successfully destroyed
    ExpectTrue(PluginRegistry::DestroyCore(core), "DestroyCore failed.");

    // Confirm that NES plugin is successfully unloaded
    ExpectTrue(PluginRegistry::UnloadPlugin(kId), "UnloadPlugin failed.");
    ExpectTrue(!PluginRegistry::IsPluginLoaded(kId), "NES plugin still marked loaded.");
}

/// @brief Asserts that loading the same plugin twice does not report as failure
static void Test_LoadSamePluginTwice() {
    RequireDiscovery();

    // Confirm that initial NES plugin is successful
    const string id = "nes";
    ExpectTrue(PluginRegistry::LoadPlugin(id), "Failed to load NES plugin.");
    ExpectTrue(PluginRegistry::IsPluginLoaded(id), "NES plugin not marked loaded.");

    // Confirm that loading the same plugin fails cleanly by reporting success
    ExpectTrue(PluginRegistry::LoadPlugin(id), "Loading a loaded plugin did not fail cleanly.");

    PluginRegistry::UnloadPlugin(id);
}

/// @brief Asserts that creating a core without first loading the plugin does not report as failure
static void Test_CreateCore_WithoutLoad() {
    RequireDiscovery();

    // Confirm that core creation is successful
    const string id = "nes";
    IConsole* core = PluginRegistry::CreateCore(id);
    ExpectTrue(core != nullptr, "CreateCore failed load the necessary plugin.");

    PluginRegistry::DestroyCore(core);
    PluginRegistry::UnloadPlugin(id);
}

/// @brief Asserts that attempting to destroy a null core (nullptr) fails cleanly by reporting success
static void Test_DestroyCore_Nullptr() {
    ExpectTrue(PluginRegistry::DestroyCore(nullptr), "DestroyCore did not fail cleanly.");
}

/// @brief Asserts that attempting to unload a plugin when there is an active core fails cleanly and does not successfully unload the plugin
static void Test_UnloadPlugin_ActiveCore() {
    RequireDiscovery();

    // Confirm that plugin load is successful
    const string id = "nes";
    ExpectTrue(PluginRegistry::LoadPlugin(id), "Failed to load NES plugin.");

    // Confirm that NES core is successfully created
    IConsole* core = PluginRegistry::CreateCore(id);
    ExpectTrue(core != nullptr, "CreateCore returned null.");

    // Confirm that plugin cannot be unloaded with an active core
    ExpectTrue(!PluginRegistry::UnloadPlugin(id), "UnloadPlugin should reject unload when an active core exists.");
    ExpectTrue(PluginRegistry::IsPluginLoaded(id), "Plugin should remain loaded after rejected unload.");
    
    string error = PluginRegistry::GetLastError();
    ExpectTrue(error.find("active core") != string::npos, "UnloadPlugin error message mismatch.");

    PluginRegistry::DestroyCore(core);
    PluginRegistry::UnloadPlugin(id);
}

/// @brief Asserts that a discovered plugin with a bad symbol set (e.g.: missing GetPluginManifest) cannot be loaded
static void Test_LoadPlugin_BadSymbolSet() {
    RequireDiscovery();

    ExpectTrue(PluginRegistry::IsPluginDiscovered("bad_symbols"), "Failed to discover bad plugin.");
    ExpectTrue(!PluginRegistry::LoadPlugin("bad_symbols"), "Successfully loaded bad plugin.");
    ExpectTrue(!PluginRegistry::IsPluginLoaded("bad_symbols"), "Bad plugin marked loaded.");

    string error = PluginRegistry::GetLastError();
    ExpectTrue(error.find("function missing") != string::npos, "LoadPlugin error message mismatch:\n\t" + error);
}

int main() {
    const vector<pair<string, function<void()>>> tests = {
        {"LoadMissingPluginFails", Test_LoadMissingPluginFails},
        {"CoreAndDebuggerLifecycle", Test_CoreAndDebuggerLifecycle},
        {"LoadSamePluginTwice", Test_LoadSamePluginTwice},
        {"CreateCore_WithoutLoad", Test_CreateCore_WithoutLoad},
        {"DestroyCore_Nullptr", Test_DestroyCore_Nullptr},
        {"UnloadPlugin_ActiveCore", Test_UnloadPlugin_ActiveCore},
        {"LoadPlugin_BadSymbolSet", Test_LoadPlugin_BadSymbolSet},
    };

    int failed = 0;
    for (const auto& t : tests) {
        const auto r = RunTest(t.first, t.second);
        if (r.passed) cout << "[PASS] " << r.name << "\n";
        else { ++failed; cout << "[FAIL] " << r.name << " :: " << r.message << "\n"; }
    }

    return failed == 0 ? 0 : 1;
}