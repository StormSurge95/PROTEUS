#pragma once

//#define TEST_SST

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include "./json.h"
#include <map>
#include <memory>
#include <SDL3/SDL.h>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// algorithm
using std::begin;
using std::end;
using std::find;

// array
using std::array;

// chrono
using std::chrono::steady_clock;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::system_clock;
using file_time = std::chrono::file_time<std::chrono::file_clock::duration>;
using std::chrono::local_time;
using std::chrono::time_point;
using std::chrono::nanoseconds;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;
using std::milli;
using std::chrono::duration_cast;

// cstdint
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

// deque
using std::deque;

// filesystem
using std::filesystem::path;
using std::filesystem::directory_iterator;
using std::filesystem::directory_entry;
using std::filesystem::exists;
using std::filesystem::create_directory;
using std::filesystem::is_regular_file;
using std::filesystem::absolute;

// fstream & iomanip
using std::ifstream;
using std::ofstream;
using std::ios;
using std::endl;

// json
using nlohmann::json;

// map
using std::unordered_map;
using std::map;
using std::pair;

// memory
template <class T>
using uptr = std::unique_ptr<T>;
using std::make_unique;
template <class T>
using sptr = std::shared_ptr<T>;
using std::make_shared;
template <class T>
using wptr = std::weak_ptr<T>;
using std::dynamic_pointer_cast;
using std::static_pointer_cast;

// set
using std::set;

// sstream & string
using std::stringstream;
using std::string;
using std::to_string;
using std::format;
using std::transform;
using std::tolower;

// thread
using std::this_thread::sleep_for;

// vector
using std::vector;