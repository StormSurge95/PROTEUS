#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>
#include <filesystem>

#include "NES.h"

using std::cout;
using std::stoi;
using std::filesystem::path;
using namespace NS_NES;

/**
 * THIS TEST ASSUMES THAT ARGUMENTS ARE PASSED IN THE FORMAT
 * `SingleStateTests <opcode> <root>` where:
 *  - <opcode> is a two-digit hex string
 *  - <root> is a valid path to the folder containing each of the SST json files 
 */
int main(int argc, char* argv[]) {
    // collect arguments
    string opcode;
    string root;

    try {
        opcode = string(argv[1]);
        root = string(argv[2]);
    } catch (...) {
        return 5;
    }

    // create NES object
    NES* nes = new NES();

    // construct SST file path
    path p = root;
    p /= opcode;
    p.replace_extension(".json");

    // collect SST data for the provided opcode
    ifstream f(p);
    if (!f.is_open()) {
        cout << "INVALID ARGUMENTS! PATH (" << p.string().c_str() << ") DOES NOT EXIST OR CANNOT BE OPENED!";
        delete nes; // memory cleanup
        return 2;
    }
    json data = json::parse(f);
    f.close();

    // convert the json data so that it is usable by teh NES object
    vector<SingleStateTest> tests;
    try {
        for (size_t i = 0; i < data.size(); i++)
            tests.push_back(SingleStateTest(data[i]));
    } catch (...) {
        return 3;
    }

    // run each test within the NES
    for (const SingleStateTest& test : tests) {
        nes->initSST(test.initState);
        try {
            nes->runSST();
        } catch (...) {
            return 4;
        }
        string s;
        // if the result doesn't match what is expected, the opcode fails
        if (!nes->checkSST(test.finalState, s)) {
            // output the produced message so that we can figure out the cause of failure
            cout << "SST Failed!\n" << s;
            delete nes; // memory cleanup
            return 1;
        }
    }

    delete nes; // memory cleanup

    return 0;
}