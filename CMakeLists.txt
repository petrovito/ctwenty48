cmake_minimum_required(VERSION 3.9.4) # this version enables LTO
project(ctwenty48 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_COMPILER_VERSION 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)


enable_testing()

# CONAN
execute_process(COMMAND conan install ${PROJECT_SOURCE_DIR}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)


#CORE LIB
add_library(ctwenty48_core STATIC 
	src/types.cc
	)

target_link_libraries(ctwenty48_core PUBLIC CONAN_PKG::llvm-openmp)
target_link_libraries(ctwenty48_core PUBLIC CONAN_PKG::imgui)
target_link_libraries(ctwenty48_core PUBLIC CONAN_PKG::darknet)
target_link_libraries(ctwenty48_core PUBLIC CONAN_PKG::boost)
target_include_directories(ctwenty48_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(ctwenty48_core PUBLIC ${CONAN_INCLUDE_DIRS_GTEST})

set_property(TARGET ctwenty48_core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)


#MAIN EXEC
add_executable(ctwenty48 src/main.cc )
target_link_libraries(ctwenty48 PRIVATE ctwenty48_core)


#TESTING
include(GoogleTest)
#utils
add_executable(utils_test test/utils_test.cc)
target_link_libraries(utils_test CONAN_PKG::gtest ctwenty48_core)
gtest_discover_tests(utils_test)

