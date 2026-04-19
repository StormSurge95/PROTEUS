#pragma once

#include <iomanip>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>

inline std::string hex(uint32_t n, uint8_t d = 2) {
    std::stringstream ss;
    ss << std::uppercase << std::setfill('0') << std::setw(d) << std::hex << n;
    return ss.str();
}

struct PROCESSOR_STATE {
    uint16_t pc = 0;
    uint8_t sp = 0;
    uint8_t a = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t p = 0;
    std::vector<uint16_t> addresses;
    std::vector<uint8_t> bytes;

    PROCESSOR_STATE(uint16_t counter, uint8_t stack,
        uint8_t aReg, uint8_t xReg, uint8_t yReg,
        uint8_t status, std::vector<uint16_t> addr, std::vector<uint8_t> b) {
        pc = counter;
        sp = stack;
        a = aReg;
        x = xReg;
        y = yReg;
        p = status;
        for (int i = 0; i < addr.size(); i++) {
            addresses.push_back(addr[i]);
            bytes.push_back(b[i]);
        }
    }
    PROCESSOR_STATE(nlohmann::json data) {
        pc = data["pc"];
        sp = data["s"];
        a = data["a"];
        x = data["x"];
        y = data["y"];
        p = data["p"];
        for (auto& e : data["ram"]) {
            addresses.push_back(e[0]);
            bytes.push_back(e[1]);
        }
    }
    ~PROCESSOR_STATE() {
        addresses.clear();
        bytes.clear();
    }

    bool operator==(PROCESSOR_STATE& other) {
        if (pc != other.pc) return false;
        if (sp != other.sp) return false;
        if (a != other.a) return false;
        if (x != other.x) return false;
        if (y != other.y) return false;
        if (p != other.p) return false;
        if (addresses.size() != other.addresses.size()) return false;
        if (bytes.size() != other.bytes.size()) return false;
        std::vector<uint16_t>::iterator iA = addresses.begin();
        std::vector<uint16_t>::iterator oA = other.addresses.begin();
        for (; iA != addresses.end() && oA != other.addresses.end(); iA++, oA++) {
            if (*iA != *oA) return false;
        }
        std::vector<uint8_t>::iterator iB = bytes.begin();
        std::vector<uint8_t>::iterator oB = other.bytes.begin();
        for (; iB != bytes.end() && oB != other.bytes.end(); iB++, oB++) {
            if (*iB != *oB) return false;
        }
        return true;
    }

    bool operator!=(PROCESSOR_STATE& other) {
        return !(*this == other);
    }
};

struct CYCLE {
    uint16_t addr;
    uint8_t byte;
    std::string op;

    CYCLE(uint16_t a, uint8_t b, std::string o) : addr(a), byte(b), op(o) {}
};

struct SST {
    std::string name;
    std::vector<uint8_t> bytes;
    PROCESSOR_STATE iState;
    PROCESSOR_STATE fState;
    std::vector<CYCLE> cycles;

    SST(nlohmann::json data) : iState(data["initial"]), fState(data["final"]) {
        name = data["name"];
        std::stringstream ss(name);
        uint8_t temp;
        while (ss >> std::hex >> temp) {
            bytes.push_back(temp);
        }
        for (const auto& e : data["cycles"]) {
            cycles.push_back(CYCLE(e[0], e[1], e[2]));
        }
    }

    bool Check(PROCESSOR_STATE f) {
        return (fState == f);
    }
};

inline std::string getFlags(uint8_t status) {
    std::stringstream ss;
    ss << ((status & 0x80) > 0 ? "N" : "n");
    ss << ((status & 0x40) > 0 ? "V" : "v");
    ss << ((status & 0x20) > 0 ? "U" : "u");
    ss << ((status & 0x10) > 0 ? "B" : "b");
    ss << ((status & 0x08) > 0 ? "D" : "d");
    ss << ((status & 0x04) > 0 ? "I" : "i");
    ss << ((status & 0x02) > 0 ? "Z" : "z");
    ss << ((status & 0x01) > 0 ? "C" : "c");
    return ss.str();
}