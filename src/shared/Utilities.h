#pragma once

#include "SharedPCH.h"

/**
 * @brief Outputs a provided number `n` in hexadecimal notation with `d` digits.
 * 
 * @param n The number value to convert into a hex string.
 * @param d The minimum number of hexadecimal digits to display; will trim any leading zeros if possible.
 * @return std::string value containing the produced hexadecimal notation.
 */
static string hex(u32 n, u8 d = 2) {
    stringstream ss;
    ss << std::uppercase << std::setfill('0') << std::setw(d) << std::hex << n;
    return ss.str();
}