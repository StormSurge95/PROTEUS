cmake_minimum_required(VERSION 3.16)

project(ProteusGBACore VERSION 1.0.0 LANGUAGES CXX)

# Build as shared library (plugin)
add_library(ProteusGBA SHARED
	# Core contract info TODO

	### emulator implementation(s) ###

	# core TODO

	# APU TODO

	# CPU TODO

	# PAK
	PAK/GbaGamepak.cpp

	# PPU TODO

	# shared TODO
)

# locate shared headers
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../Proteus/src")
	# building ProteusGBA as sibling project
	set(PROTEUS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Proteus/src")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/src/shared/IConsole.h")
	#building ProteusGBA as sub project
	set(PROTEUS_SRC_DIR "${CMAKE_SOURCE_DIR}/src")
else()
	message(FATAL_ERROR "Could not locate Proteus shared headers (expected Proteus/src).")
endif()

# Include directories
target_include_directories(ProteusGBA PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${PROTEUS_SRC_DIR}
	${PROTEUS_SRC_DIR}/shared
	${PROTEUS_SRC_DIR}/shared/plugin_utils
)

target_link_libraries(ProteusGBA PRIVATE
	SDL3::SDL3
	OpenSSL::SSL
	OpenSSL::Crypto
	nlohmann_json::nlohmann_json
)

# Compiler settings
set_target_properties(ProteusGBA PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

# Platform-specific settings
if(WIN32)
	set_target_properties(ProteusGBA PROPERTIES
		PREFIX ""
		SUFFIX ".dll"
	)
elseif(APPLE)
	set_target_properties(ProteusGBA PROPERTIES
		PREFIX "lib"
		SUFFIX ".dylib"
		MACOSX_RPATH ON
	)
else()
	set_target_properties(ProteusGBA PROPERTIES
		PREFIX "lib"
		SUFFIX ".so"
	)
endif()

# Set output directory for cores
set_target_properties(ProteusGBA PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins"
)

# Install plugin
install(TARGETS ProteusGBA DESTINATION ${INSTALL_PLUGIN_DIR})