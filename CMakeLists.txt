cmake_minimum_required(VERSION 3.16)
project(Proteus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Temporary default until full install layout is added
if(NOT DEFINED INSTALL_PLUGIN_DIR)
	set(INSTALL_PLUGIN_DIR "plugins")
endif()

add_subdirectory(src/backend/NES)