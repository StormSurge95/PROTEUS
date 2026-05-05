#pragma once

#include <iomanip>
#include <fstream>
#include <cstdint>
#include <array>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

using std::chrono::steady_clock;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::milli;

using std::this_thread::sleep_for;

using std::vector;
using std::array;

using std::ifstream;
using std::ios;
using std::stringstream;
using std::string;
using std::endl;
using std::to_string;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

template <class T>
using uptr = std::unique_ptr<T>;
using std::make_unique;

template <class T>
using sptr = std::shared_ptr<T>;
using std::make_shared;

template <class T>
using wptr = std::weak_ptr<T>;