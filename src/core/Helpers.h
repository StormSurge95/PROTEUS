#pragma once

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

inline std::string hex(uint32_t n, uint8_t d = 2) {
    std::stringstream ss;
    ss << std::uppercase << std::setfill('0') << std::setw(d) << std::hex << n;
    return ss.str();
}

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