cmake_minimum_required(VERSION 3.9.4) # this version enables LTO
project(ctwenty48 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_COMPILER_VERSION 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE EQUAL "Debug")
	SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -ggdb3")
endif()

if(NOT BUILD_HEADLESS)
	set(BUILD_HEADLESS 0)
endif()
if(NOT BUILD_GUI)
	set(BUILD_GUI 0)
endif()
if(NOT BUILD_TEST)
	set(BUILD_TEST 0)
endif()



#CORE LIB
add_subdirectory(core)

#MAIN EXEC
if (BUILD_HEADLESS) 
	add_subdirectory(headless_app)
endif()

#GUI
if (BUILD_GUI)
	add_subdirectory(gui_app)
endif()

