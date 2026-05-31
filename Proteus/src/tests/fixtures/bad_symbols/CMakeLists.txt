add_library(BadSymbolsPlugin SHARED BadSymbolsPlugin.cpp)

target_link_libraries(BadSymbolsPlugin PRIVATE
	SDL3::SDL3
	OpenSSL::SSL
	OpenSSL::Crypto
	nlohmann_json::nlohmann_json
)

set_target_properties(BadSymbolsPlugin PROPERTIES
	OUTPUT_NAME "BadSymbolsPlugin"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins"
)

# Platform-specific settings
if(WIN32)
	set_target_properties(BadSymbolsPlugin PROPERTIES
		PREFIX ""
		SUFFIX ".dll"
	)
elseif(APPLE)
	set_target_properties(BadSymbolsPlugin PROPERTIES
		PREFIX "lib"
		SUFFIX ".dylib"
		MACOSX_RPATH ON
	)
else()
	set_target_properties(BadSymbolsPlugin PROPERTIES
		PREFIX "lib"
		SUFFIX ".so"
	)
endif()