#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>
#include <filesystem>

// we define this here so that including NES.h after will create the right SST environment
#define TEST_SST

#include <NES.h>

using std::string;
using std::runtime_error;
using std::exception;
using std::function;
using std::vector;
using std::pair;
using std::cout;
using nlohmann::json;

using namespace std::filesystem;
using namespace NS_NES;

struct TestResult {
    string name;
    bool passed;
    string message;
};

static void ExpectTrue(bool cond, const string& msg) {
    if (!cond) throw runtime_error(msg);
}

struct CONFIG {
    u8 opcode;
    string sstRoot;
};

CONFIG ParseArgs(int argc, char* argv[]) {
    // args will be passed as: <opcode> <rootPath>
    char* opcodeArg = argv[1];
    char* rootArg = argv[2];

    u8 opcode = std::stoi(opcodeArg, nullptr, 16);
    return {
        (u8)std::stoi(opcodeArg, nullptr, 16),
        string(rootArg)
    };
}

int main(int argc, char* argv[]) {
    CONFIG cfg = ParseArgs(argc, argv);

    NES* nes = new NES();

    ifstream f(path(format("{}{}.json", cfg.sstRoot.c_str(), hex(cfg.opcode, 2).c_str())).make_preferred());
    json data = json::parse(f);
    f.close();

    vector<SingleStateTest> SST;
    for (u32 i = 0; i < data.size(); i++)
        SST.push_back(SingleStateTest(data[i]));

    for (const SingleStateTest& test : SST) {
        nes->initSST(test.initState);
        nes->runSST();
        string result;
        if (!nes->checkSST(test.finalState, result)) return 1;
    }

    return 0;
}