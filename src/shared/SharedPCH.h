#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <SDL3/SDL.h>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// array
using std::array;

// chrono
using std::chrono::steady_clock;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::system_clock;
using std::milli;

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
using std::filesystem::exists;
using std::filesystem::create_directory;

// fstream & iomanip
using std::ifstream;
using std::ofstream;
using std::ios;
using std::endl;

// map
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

// sstream & string
using std::stringstream;
using std::string;
using std::to_string;
using std::format;

// thread
using std::this_thread::sleep_for;

// vector
using std::vector;