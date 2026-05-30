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
find_package(SDL3 CONFIG QUIET)
if(NOT SDL3_FOUND)
	# fallback
	list(APPEND CMAKE_PREFIX_PATH "C:/SDL3" "C:/SDL3/cmake")
	find_package(SDL3 CONFIG REQUIRED)
endif()

# locate required OpenSSL headers and binaries
find_package(OpenSSL CONFIG QUIET)
if (NOT OpenSSL_FOUND)
	set(OPENSSL_NATIVE_DIR "${CMAKE_SOURCE_DIR}/Proteus/packages/openssl-native.3.5.5")
	set(OPENSSL_INCLUDE_DIR "${OPENSSL_NATIVE_DIR}/include")

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(OPENSSL_LIB_DIR "${OPENSSL_NATIVE_DIR}/lib/win-x64/native")
	else()
		set(OPENSSL_LIB_DIR "${OPENSSL_NATIVE_DIR}/lib/win-x86/native")
	endif()
	
	add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
	set_target_properties(OpenSSL::Crypto PROPERTIES
		IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIB}"
		INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
	)

	add_library(OpenSSL::SSL UNKNOWN IMPORTED)
	set_target_properties(OpenSSL::SSL PROPERTIES
		IMPORTED_LOCATION "${OPENSSL_SSL_LIB}"
		INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
		INTERFACE_LINK_LIBRARIES OpenSSL::Crypto
	)
endif()

# locate required nlohmann/json header (headonly-only version; no binaries necessary)
find_package(nlohmann_json CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
	include(FetchContent)
	FetchContent_Declare(
	nlohmann_json
	URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
	)
	FetchContent_MakeAvailable(nlohmann_json)
endif()

# Detailed target wiring lives under src/
add_subdirectory(Proteus/src)