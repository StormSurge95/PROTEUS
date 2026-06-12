#include <list>

#include "./frontend/app/Proteus.h"
#include "./frontend/app/Headless.h"

using namespace NS_Proteus;

struct LaunchArgs {
    bool headless = false;
    ConsoleID coreID = ConsoleID::NONE;
    path romPath = path();
    string inputSpec = "";
};

struct ParseResult {
    bool ok = true;
    LaunchArgs args = {};
    std::list<string> errors = {};
    std::list<string> warnings = {};
};

ParseResult ParseArgs(int argc, char** argv) {
    ParseResult result;

    std::list<string> unknowns;

    for (int i = 1; i < argc; i++) {
        string tok = argv[i];

        if (tok == "-headless") {
            result.args.headless = true;
            continue;
        }

        if (tok == "-core") {
            if (result.args.coreID != ConsoleID::NONE) {
                result.ok = false;
                result.errors.push_back("Only one core can be defined");
                continue;
            }
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                result.ok = false;
                result.errors.push_back("'-core' arg must be followed by the name of a core");
                continue;
            }
            string val = argv[i + 1];
            ConsoleID id = GetIDFromName(val);
            if (id == ConsoleID::NONE) {
                result.ok = false;
                result.errors.push_back("Provided core name \"" + val + "\" is invalid");
                continue;
            }
            result.args.coreID = id;
            i += 1;
            continue;
        }

        if (tok == "-rom") {
            if (!result.args.romPath.empty()) {
                result.ok = false;
                result.errors.push_back("Only one rom path can be defined");
                continue;
            }
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                result.ok = false;
                result.errors.push_back("'-rom' arg must be followed by a file path");
                continue;
            }
            string val = argv[i + 1];
            if (!exists(val) || !is_regular_file(val)) {
                result.ok = false;
                result.errors.push_back("Provided rom path \"" + val + "\" does not point to a valid file");
                continue;
            }
            result.args.romPath = val;
            i += 1;
            continue;
        }

        if (tok == "-input") {
            if (!result.args.inputSpec.empty()) {
                result.ok = false;
                result.errors.push_back("Only one input string can be defined");
                continue;
            }
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                result.ok = false;
                result.errors.push_back("'-input' arg must be followed by an input string");
                continue;
            }
            result.args.inputSpec = argv[i + 1];
            i += 1;
            continue;
        }

        unknowns.push_back(tok);
    }

    if (result.args.headless) {
        if (result.args.coreID == ConsoleID::NONE || result.args.romPath.empty()) {
            result.ok = false;
            result.errors.push_back("'-core' and '-rom' are both required when attempting to run in headless mode");
        }
    } else {
        if (result.args.coreID != ConsoleID::NONE) {
            result.warnings.push_back("application not running in headless mode; '-core' arg ignored");
        }
        if (!result.args.romPath.empty()) {
            result.warnings.push_back("application not running in headless mode; '-rom' arg ignored");
        }
    }

    if (!unknowns.empty()) {
        string msg = to_string(unknowns.size()) + " unknown arguments passed to application: ";
        for (const string& s : unknowns) msg += s + " ";
        result.warnings.push_back(msg);
    }

    return result;
}

// TODO: switch to WINDOWS mode instead of CONSOLE
int main(int argc, char** args) {
    ParseResult result = ParseArgs(argc, args);

    if (!result.ok) {
        string msg = "Failed to run Proteus.exe:\n";
        for (const string& s : result.errors) {
            msg += s;
            msg += "\n";
        }
        throw std::invalid_argument(msg);
    }

    if (!result.warnings.empty()) {
        for (const string& s : result.warnings) {
            std::cerr << "Warning: " << s << std::endl;
        }
    }
    
    if (!result.args.headless) {
        Proteus* proteus = new Proteus();
        proteus->Init();
        proteus->Run();
        delete proteus;
    } else {
        Headless::Run(result.args.coreID, result.args.romPath, result.args.inputSpec);
    }

    return 0;
}