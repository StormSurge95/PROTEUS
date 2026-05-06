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

/**
 * @brief Returns the higher of two numbers
 * @param n1 
 * @param n2 
 * @return 
 */
static u64 max(const u64& n1, const u64& n2) {
    if (n1 > n2) return n1;
    else return n2;
}