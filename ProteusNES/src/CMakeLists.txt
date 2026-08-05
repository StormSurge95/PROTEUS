cmake_minimum_required(VERSION 3.16)

project(ProteusNESCore VERSION 1.0.0 LANGUAGES CXX)

set(NES_CORE_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/NES.cpp						# NES console core
	${CMAKE_CURRENT_SOURCE_DIR}/NesController.cpp			# controller
	${CMAKE_CURRENT_SOURCE_DIR}/APU/NesAPU.cpp				# audio processing unit
	${CMAKE_CURRENT_SOURCE_DIR}/APU/NesDMC.cpp				# apu - delta modulation channel
	${CMAKE_CURRENT_SOURCE_DIR}/APU/NesNoise.cpp			# apu - noise channel
	${CMAKE_CURRENT_SOURCE_DIR}/APU/NesPulse.cpp			# apu - pulse channel
	${CMAKE_CURRENT_SOURCE_DIR}/APU/NesTriangle.cpp			# apu - triangle channel
	${CMAKE_CURRENT_SOURCE_DIR}/CPU/NesCPU.cpp				# central processing unit
	${CMAKE_CURRENT_SOURCE_DIR}/CPU/NesCPU_AddressModes.cpp	# cpu - addressing modes
	${CMAKE_CURRENT_SOURCE_DIR}/CPU/NesCPU_Instructions.cpp	# cpu - instructions
	${CMAKE_CURRENT_SOURCE_DIR}/PAK/NesGamepak.cpp			# gamepak cartridge
	${CMAKE_CURRENT_SOURCE_DIR}/PPU/NesPPU.cpp				# picture processing unit
	${CMAKE_CURRENT_SOURCE_DIR}/PPU/NesPPU_BG.cpp			# ppu - backgrounds
	${CMAKE_CURRENT_SOURCE_DIR}/PPU/NesPPU_Sprites.cpp		# ppu - sprites
	${CMAKE_CURRENT_SOURCE_DIR}/shared/NesDebugger.cpp		# debugger
)

# locate shared headers
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../Proteus/src")
	# building ProteusNES as sibling project
	set(PROTEUS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Proteus/src")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/src/shared/IConsole.h")
	# building ProteusNES as sub project
	set(PROTEUS_SRC_DIR "${CMAKE_SOURCE_DIR}/src")
else()
	message(FATAL_ERROR "Could not locate Proteus shared headers (expected Proteus/src).")
endif()

if(PROTEUS_NES_BUILD_TESTS)
	add_subdirectory(tests)
endif()

# Build as shared library (plugin)
add_library(ProteusNES SHARED
	# core contract info
	NesCoreExports.cpp
	### emulator implementation(s) ###
	${NES_CORE_SOURCES}
)

# Include directories
target_include_directories(ProteusNES PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${PROTEUS_SRC_DIR}
	${PROTEUS_SRC_DIR}/shared
	${PROTEUS_SRC_DIR}/shared/plugin_utils
)

target_link_libraries(ProteusNES PRIVATE
	SDL3::SDL3
	OpenSSL::SSL
	OpenSSL::Crypto
	nlohmann_json::nlohmann_json
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
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins"
)

# Install plugin
install(TARGETS ProteusNES DESTINATION ${INSTALL_PLUGIN_DIR})