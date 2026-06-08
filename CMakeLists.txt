cmake_minimum_required(VERSION 3.16)
project(Proteus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PROTEUS_BUILD_TESTS "Build Proteus test targets" OFF)
option(PROTEUS_NES_BUILD_TESTS "Build ProteusNES test targets" OFF)
option(PROTEUS_GBA_BUILD_TESTS "Build ProteusGBA test targets" OFF)
include(CTest)

# Temporary default until full install layout is added
if(NOT DEFINED INSTALL_PLUGIN_DIR)
	set(INSTALL_PLUGIN_DIR "plugins")
endif()

# locate required SDL3 package binaries
find_package(SDL3 CONFIG REQUIRED)
message(STATUS "SDL3_DIR=${SDL3_DIR}")

# locate required OpenSSL headers and binaries
find_package(OpenSSL CONFIG REQUIRED)
message(STATUS "OpenSSL_DIR=${OpenSSL_DIR}")

# locate required nlohmann/json header (headonly-only version; no binaries necessary)
find_package(nlohmann_json CONFIG REQUIRED)
message(STATUS "nlohmann_json_DIR=${nlohmann_json_DIR}")

# Detailed target wiring lives under src/
add_subdirectory(Proteus/src)