#pragma once

#pragma warning(push)
#pragma warning(disable : 4244)

#include "./SharedPCH.h"
#include "./Types.h"

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
 * @brief Outputs a provided number `n` in binary notation
 * @note ONLY USE 8-BIT VALUES
 * @param n The number value to convert into a binary string
 * @return string value containing the produced binary notation
 */
static string bin(u8 n) {
    stringstream ss;
    ss << "0b";
    for (s8 i = 7; i >= 0; i--) {
        if (((n >> i) & 0x01) > 0) ss << "1";
        else ss << "0";
    }
    return ss.str();
}

#undef max

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

static int max(const int& n1, const int& n2) {
    if (n1 > n2) return n1;
    else return n2;
}

#undef min

static double min(const double& n1, const double& n2) {
    if (n1 < n2) return n1;
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

static string GetTimestamp() {
    #if __cpp_lib_chrono >= 201907L
        local_time t = std::chrono::current_zone()->to_local(system_clock::now());

        std::chrono::year_month_day ymd{ std::chrono::floor<std::chrono::days>(t) };
        int year = static_cast<int>(ymd.year());
        int month = static_cast<unsigned>(ymd.month());
        int day = static_cast<unsigned>(ymd.day());

        system_clock::duration r = t - std::chrono::floor<std::chrono::days>(t);
        std::chrono::hh_mm_ss<system_clock::duration> hms{ duration_cast<milliseconds>(r) };
        int hour = hms.hours().count();
        int minute = hms.minutes().count();
        int second = hms.seconds().count();
    #else // compatible with Apple Clang; cuz Apples really does suck at everything
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* local_tm = std::localtime(&now_time);

        int year = local_tm->tm_year;
        int month = local_tm->tm_mon;
        int day = local_tm->tm_mday;
        int hour = local_tm->tm_hour;
        int minute = local_tm->tm_min;
        int second = local_tm->tm_sec;
    #endif

    stringstream ss;
    if (month < 10) ss << '0';
    ss << month << '/';
    if (day < 10) ss << '0';
    ss << day << '/' << year << '@';
    if (hour < 10) ss << '0';
    ss << hour << ':';
    if (minute < 10) ss << '0';
    ss << minute << ':';
    if (second < 10) ss << '0';
    ss << second;
    return ss.str();
}

static string GetDuration(nanoseconds d) {
    stringstream ss;
    milliseconds ms = duration_cast<milliseconds, double, std::nano>(d);
    if (ms.count() > 1000) {
        hours h = duration_cast<hours>(d);
        d -= h;
        minutes m = duration_cast<minutes>(d);
        d -= m;
        seconds s = duration_cast<seconds>(d);

        if (h.count() < 10) ss << '0';
        ss << h.count() << ':';
        if (m.count() < 10) ss << '0';
        ss << m.count() << ':';
        if (s.count() < 10) ss << '0';
        ss << s.count();
    } else {
        ss << ms.count() << "ms";
    }
    return ss.str();
}

template<typename T, size_t capacity>
class RingBuffer {
    private:
        deque<T> data;
        T total = 0;
        size_t lastEntryPos = 0;
        size_t count = 0;
    public:
        RingBuffer() : data(capacity) {}

        void push(const T& val) {
            if (data.size() >= capacity) {
                total -= data.front();
                data.pop_front();
            }
            total += val;
            data.push_back(val);
        }

        double avg() { return (double)total / count; }
};

static ConsoleID GetIDFromName(string name) {
    for (const auto& [key, val] : ConsoleNamesShort) {
        if (strcmp(name.c_str(), val.c_str()) == 0)
            return key;
    }

    return ConsoleID::NONE;
}

static string GetNameFromID(ConsoleID id) {
    for (map<ConsoleID, string>::const_iterator it = ConsoleNamesShort.begin();
        it != ConsoleNamesShort.end(); it++) {
        if (it->first == id) return it->second;
    }

    return "";
}

static u32 LerpColor(u32 c1, u32 c2, float t) {
    // ABGR
    u8 r1 = c1 & 0xFF; u8 r2 = c2 & 0xFF;
    u8 g1 = (c1 >> 8) & 0xFF; u8 g2 = (c2 >> 8) & 0xFF;
    u8 b1 = (c1 >> 16) & 0xFF; u8 b2 = (c2 >> 16) & 0xFF;

    u32 r = u32((r2 - r1) * t + r1);
    u32 g = u32((g2 - g1) * t + g1);
    u32 b = u32((b2 - b1) * t + b1);

    return 0xFF000000 | (b << 16) | (g << 8) | r;
}

static u32 WithAlpha(u32 c, u32 a) {
    // ensure that we only use 8 bits for the alpha value
    a &= 0xFF;
    // clear existing alpha value and then set it to our new alpha
    return (c & 0x00FFFFFF) | (a << 24);
}

static u32 Brighten(u32 color, float factor) {
    if (factor < 1.0f) factor = 1 / factor;

    // ABGR
    u32 r = (color & 0xFF) * factor;
    color >>= 8;
    u32 g = (color & 0xFF) * factor;
    color >>= 8;
    u32 b = (color & 0xFF) * factor;
    color >>= 8;
    u32 a = color & 0xFF;

    u32 ret = a;
    ret <<= 8;
    ret |= b;
    ret <<= 8;
    ret |= g;
    ret <<= 8;
    ret |= r;
    
    return ret;
}

static u32 Dim(u32 color, float factor) {
    if (factor > 1.0f) factor = 1 / factor;

    // ABGR
    u32 r = (color & 0xFF) * factor;
    color >>= 8;
    u32 g = (color & 0xFF) * factor;
    color >>= 8;
    u32 b = (color & 0xFF) * factor;
    color >>= 8;
    u32 a = color & 0xFF;

    u32 ret = a;
    ret <<= 8;
    ret |= b;
    ret <<= 8;
    ret |= g;
    ret <<= 8;
    ret |= r;

    return ret;
}

template<class T>
static string FormatNum(T value) {
    stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}

#pragma warning(pop)