cmake_minimum_required(VERSION 3.16)

project(ProteusNESCore VERSION 1.0.0 LANGUAGES CXX)

# Build as shared library (plugin)
add_library(ProteusNES SHARED
	# Core emulation (TODO)
	NES.cpp
	APU/NesAPU.cpp
	APU/NesDMC.cpp
	APU/NesNoise.cpp
	APU/NesPulse.cpp
	APU/NesTriangle.cpp
	CPU/NesCPU.cpp
	CPU/NesCPU_AddressModes.cpp
	CPU/NesCPU_Instructions.cpp
)

# Include directories
target_include_directories(ProteusNES PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_SOURCE_DIR}/src
)

# Compiler settings
set_target_properties(ProteusNES PROPERTIES
	CXX_STANDARD 17
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

# Set output directory for cores
set_target_properties(ProteusNES PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Install plugin
install(TARGETS ProteusNES DESTINATION ${INSTALL_PLUGIN_DIR})