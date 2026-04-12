#pragma once

#include <iomanip>
#include <sstream>
#include <string>

inline std::string hex(uint32_t n, uint8_t d = 2) {
    std::stringstream ss;
    ss << std::uppercase << std::setfill('0') << std::setw(d) << std::hex << n;
    return ss.str();
}