cmake_minimum_required(VERSION 3.9.4) # this version enables LTO
project(ctwenty48 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_COMPILER_VERSION 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

execute_process(COMMAND conan install ${PROJECT_SOURCE_DIR}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CTest)
enable_testing()

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0) 

add_executable(ctwenty48 src/main.cc 
	src/types.cc
	)
target_link_libraries(ctwenty48 PUBLIC CONAN_PKG::llvm-openmp)
target_link_libraries(ctwenty48 PUBLIC CONAN_PKG::imgui)
target_link_libraries(ctwenty48 PUBLIC CONAN_PKG::darknet)
target_link_libraries(ctwenty48 PUBLIC CONAN_PKG::boost)
target_include_directories(ctwenty48 PUBLIC ${PROJECT_SOURCE_DIR}/include)

set_property(TARGET ctwenty48 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

