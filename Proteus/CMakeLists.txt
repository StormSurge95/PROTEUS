cmake_minimum_required(VERSION 3.16)
project(Proteus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PROTEUS_BUILD_TESTS "Build Proteus test targets" OFF)
include(CTest)

# Temporary default until full install layout is added
if(NOT DEFINED INSTALL_PLUGIN_DIR)
	set(INSTALL_PLUGIN_DIR "plugins")
endif()

# locate required SDL3 package binaries
find_package(SDL3 CONFIG REQUIRED)

# locate required OpenSSL headers and binaries
set(OPENSSL_NATIVE_DIR "${CMAKE_SOURCE_DIR}/packages/openssl-native.3.5.5")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_NATIVE_DIR}/include")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(OPENSSL_LIB_DIR "${OPENSSL_NATIVE_DIR}/lib/win-x64/native")
else()
	set(OPENSSL_LIB_DIR "${OPENSSL_NATIVE_DIR}/lib/win-x86/native")
endif()
set(OPENSSL_CRYPTO_LIB "${OPENSSL_LIB_DIR}/libcrypto.lib")
set(OPENSSL_SSL_LIB "${OPENSSL_LIB_DIR}/libssl.lib")
if(NOT EXISTS "${OPENSSL_INCLUDE_DIR}/openssl/evp.h")# OR NOT EXISTS "${OPENSSL_INCLUDE_DIR}/openssl/md5.h")
	message(FATAL_ERROR "OpenSSL headers not found at ${OPENSSL_INCLUDE_DIR}")
endif()
if(NOT EXISTS "${OPENSSL_CRYPTO_LIB}")
	message(FATAL_ERROR "OpenSSL libcrypto not found at ${OPENSSL_CRYPTO_LIB}")
endif()

# locate required nlohmann/json header (headonly-only version; no binaries necessary)
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Detailed target wiring lives under src/
add_subdirectory(src)