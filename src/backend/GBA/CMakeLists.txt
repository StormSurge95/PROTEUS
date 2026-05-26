cmake_minimum_required(VERSION 3.16)

project(ProteusGBACore VERSION 1.0.0 LANGUAGES CXX)

# Build as shared library (plugin)
add_library(ProteusGBA SHARED
	# Core emulation (TODO)
)

# Include directories
target_include_directories(ProteusGBA PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_SOURCE_DIR}/src
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
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Install plugin
install(TARGETS ProteusGBA DESTINATION ${INSTALL_PLUGIN_DIR})