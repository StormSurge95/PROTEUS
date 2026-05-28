cmake_minimum_required(VERSION 3.16)

project(ProteusNESCore VERSION 1.0.0 LANGUAGES CXX)

# Build as shared library (plugin)
add_library(ProteusNES SHARED
	# core contract info
	NesCoreExports.cpp

	### emulator implementation(s) ###

	# core
	NES.cpp
	NesController.cpp

	# APU
	APU/NesAPU.cpp
	APU/NesDMC.cpp
	APU/NesNoise.cpp
	APU/NesPulse.cpp
	APU/NesTriangle.cpp

	# CPU
	CPU/NesCPU.cpp
	CPU/NesCPU_AddressModes.cpp
	CPU/NesCPU_Instructions.cpp

	# PAK
	PAK/NesGamepak.cpp

	# PPU
	PPU/NesPPU.cpp
	PPU/NesPPU_BG.cpp
	PPU/NesPPU_Sprites.cpp

	# shared
	shared/NesDebugger.cpp
)

# Include directories
target_include_directories(ProteusNES PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_SOURCE_DIR}/src
)

# Compiler settings
set_target_properties(ProteusNES PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

# Platform-specific settings
if(WIN32)
	set_target_properties(ProteusNES PROPERTIES
		PREFIX ""
		SUFFIX ".dll"
	)
elseif(APPLE)
	set_target_properties(ProteusNES PROPERTIES
		PREFIX "lib"
		SUFFIX ".dylib"
		MACOSX_RPATH ON
	)
else()
	set_target_properties(ProteusNES PROPERTIES
		PREFIX "lib"
		SUFFIX ".so"
	)
endif()

# Set output directory for plugin binaries
set_target_properties(ProteusNES PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"

    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/plugins/Debug"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/plugins/Release"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/plugins/RelWithDebInfo"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/plugins/MinSizeRel"

    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/plugins/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/plugins/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/plugins/RelWithDebInfo"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/plugins/MinSizeRel"
)

# Install plugin
install(TARGETS ProteusNES DESTINATION ${INSTALL_PLUGIN_DIR})