#pragma once

struct SSTcycle {
    u16 addr;
    u8 data;
    string op;

    SSTcycle(json d) {
        addr = d[0];
        data = d[1];
        op = d[2];
    }
};

struct SSTstate {
    u16 pc;
    u8 s;
    u8 a;
    u8 x;
    u8 y;
    u8 p;
    vector<array<u16, 2>> ram;

    SSTstate(json data) {
        pc = data["pc"];
        s = data["s"];
        a = data["a"];
        x = data["x"];
        y = data["y"];
        p = data["p"];
        for (int i = 0; i < data["ram"].size(); i++) {
            ram.push_back({ data["ram"][i][0], data["ram"][i][1] });
        }
    }
};

struct SSTtest {
    string name;
    SSTstate initState;
    SSTstate finalState;
    vector<SSTcycle> cycles;

    SSTtest(json data) :
        name(data["name"]),
        initState(data["initial"]),
        finalState(data["final"]) {
        for (int i = 0; i < data["cycles"].size(); i++) {
            cycles.push_back(SSTcycle(data["cycles"][i]));
        }
    }
};