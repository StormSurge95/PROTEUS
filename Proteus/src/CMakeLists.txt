# Proteus\src\CMakeLists.txt

# Frontend app (local)
add_subdirectory(frontend)

# Backend aggregation via sibling plugin projects
add_subdirectory(
	"${CMAKE_CURRENT_SOURCE_DIR}/../../ProteusNES/src"
	"${CMAKE_BINARY_DIR}/ProteusNES"
)
add_subdirectory(
	"${CMAKE_CURRENT_SOURCE_DIR}/../../ProteusGBA/src"
	"${CMAKE_BINARY_DIR}/ProteusGBA"
)
# TODO: add other subdirs as they become available

# Tests for validating application runtime
if(PROTEUS_BUILD_TESTS)
	add_subdirectory(tests)
endif()