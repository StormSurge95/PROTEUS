# Proteus\src\tests\CMakeLists.txt

if(NOT PROTEUS_BUILD_TESTS)
	return()
endif()

add_subdirectory(fixtures/bad_symbols)

add_executable(PluginTests
	PluginTests.cpp
	../shared/plugin_utils/PluginRegistry.cpp
	../shared/plugin_utils/PluginLoader.cpp
)
add_executable(PluginPerfTests
	PluginPerfTests.cpp
	../shared/plugin_utils/PluginRegistry.cpp
	../shared/plugin_utils/PluginLoader.cpp
)

target_include_directories(PluginTests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/../shared
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/plugin_utils
)
target_include_directories(PluginPerfTests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/../shared
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/plugin_utils
)

target_link_libraries(PluginTests PRIVATE
	SDL3::SDL3
	OpenSSL::SSL
	OpenSSL::Crypto
	nlohmann_json::nlohmann_json
)
target_link_libraries(PluginPerfTests PRIVATE
	SDL3::SDL3
	OpenSSL::SSL
	OpenSSL::Crypto
	nlohmann_json::nlohmann_json
)

set_target_properties(PluginTests PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/tests"
)
set_target_properties(PluginPerfTests PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/tests"
)

target_compile_features(PluginTests PRIVATE cxx_std_20)
target_compile_features(PluginPerfTests PRIVATE cxx_std_20)

add_test(NAME PluginTests COMMAND PluginTests)
add_test(NAME PluginPerfTests COMMAND PluginPerfTests)

set_tests_properties(PluginTests PROPERTIES
	WORKING_DIRECTORY "$<TARGET_FILE_DIR:Proteus>"
	LABELS "plugin"
)
set_tests_properties(PluginPerfTests PROPERTIES
	WORKING_DIRECTORY "$<TARGET_FILE_DIR:Proteus>"
	LABELS "timing"
)