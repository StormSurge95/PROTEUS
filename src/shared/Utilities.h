#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)

#include "./SharedPCH.h"

/**
 * @brief Outputs a provided number `n` in hexadecimal notation with `d` digits.
 * 
 * @param n The number value to convert into a hex string.
 * @param d The minimum number of hexadecimal digits to display; will trim any leading zeros if possible.
 * @return std::string value containing the produced hexadecimal notation.
 */
static string hex(u64 n, u8 d = 2) {
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
static double max(const double& n1, const double& n2) {
    if (n1 > n2) return n1;
    else return n2;
}

static s32 FindMiddlestWhitespace(const std::string& s) {
    if (s.empty()) return -1;

    double mid = (s.length() - 1) / 2.0;
    s32 bestIdx = -1;
    double minDist = s.length();

    for (int i = 0; i < s.length(); i++) {
        if (s[i] == ' ') {
            double currDist = abs(i - mid);
            if (currDist < minDist) {
                minDist = currDist;
                bestIdx = i;
            }
        }
    }

    return bestIdx;
}

static string FormatDisplayName(const string& fullname, bool wrap = false) {
    size_t header = fullname.find(" (");
    string name = fullname.substr(0, header);

    size_t comma = name.rfind(", ");
    if (comma != string::npos) {
        string base = name.substr(0, comma);
        string suff = name.substr(comma + 2);

        if (suff == "The" || suff == "A" || suff == "An")
            name = suff.append(" ").append(base);
    }

    replace(name.begin(), name.end(), '_', ' ');

    if (wrap) {
        s32 middleSpace = FindMiddlestWhitespace(name);
        if (middleSpace != -1) name[middleSpace] = '\n';
    }

    // TODO: replace "<char>.<char>" with "<char> - <char>"

    return name;
}

#pragma warning(pop)