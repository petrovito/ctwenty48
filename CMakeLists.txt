cmake_minimum_required(VERSION 3.9.4) # this version enables LTO
project(ctwenty48 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_COMPILER_VERSION 11)
set_property(TARGET example PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

execute_process(COMMAND conan install ${PROJECT_SOURCE_DIR}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CTest)
enable_testing()

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0) 

add_executable(ctwenty48 src/main.cpp 
	src/types.cc
	)
target_link_libraries(ctwenty48 PUBLIC CONAN_PKG::llvm-openmp)
target_link_libraries(ctwenty48 PUBLIC CONAN_PKG::imgui)
target_link_libraries(ctwenty48 PUBLIC CONAN_PKG::darknet)
target_include_directories(ctwenty48 PUBLIC ${GTKMM_INCLUDE_DIRS})
target_include_directories(ctwenty48 PUBLIC ${PROJECT_SOURCE_DIR}/include)

