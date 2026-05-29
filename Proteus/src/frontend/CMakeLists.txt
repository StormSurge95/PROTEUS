set(PROTEUS_FRONTEND_SOURCES
	app/Proteus.cpp
	audio/AudioManager.cpp
	input/Gamepad.cpp
	input/InputManager.cpp
	logging/Logger.cpp
	plugin/PluginManager.cpp
	rom_library/RomLibrary.cpp
	session/ConsoleSession.cpp
	video/VideoManager.cpp

	# ImGui
	ImGui/imgui.cpp
	ImGui/imgui_draw.cpp
	ImGui/imgui_tables.cpp
	ImGui/imgui_widgets.cpp
	ImGui/imgui_impl_sdl3.cpp
	ImGui/imgui_impl_sdlrenderer3.cpp

	# shared plugin utils currently built in app target
	../shared/plugin_utils/PluginLoader.cpp
	../shared/plugin_utils/PluginRegistry.cpp

	../main.cpp
)

add_executable(Proteus ${PROTEUS_FRONTEND_SOURCES})

target_include_directories(Proteus PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/../shared
	${CMAKE_CURRENT_SOURCE_DIR}/../shared/plugin_utils
	${CMAKE_CURRENT_SOURCE_DIR}/ImGui
	${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(Proteus PRIVATE
	SDL3::SDL3
	${OPENSSL_SSL_LIB}
	${OPENSSL_CRYPTO_LIB}
	nlohmann_json::nlohmann_json
)

set_target_properties(Proteus PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)

add_custom_command(TARGET Proteus POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"C:/SDL3/lib/x64/SDL3.dll"
		"$<TARGET_FILE_DIR:Proteus>/SDL3.dll"
)