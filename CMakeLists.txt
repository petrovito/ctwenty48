cmake_minimum_required(VERSION 3.9.4) # this version enables LTO
project(ctwenty48 VERSION 0.1.0)
enable_testing()

# CONAN
execute_process(COMMAND conan install ${PROJECT_SOURCE_DIR}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# other deps
include(FetchContent)
FetchContent_Declare(
  tiny-dnn
  GIT_REPOSITORY https://github.com/tiny-dnn/tiny-dnn.git
  GIT_TAG        v1.0.0a3
)

FetchContent_MakeAvailable(tiny-dnn)




set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_COMPILER_VERSION 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)



#CORE LIB
add_library(ctwenty48_core STATIC 
	src/types.cc
	src/search.cc
	src/cnn.cc
	)

#target_link_libraries(ctwenty48_core PUBLIC CONAN_PKG::llvm-openmp)
target_link_libraries(ctwenty48_core PUBLIC CONAN_PKG::imgui)
target_link_libraries(ctwenty48_core PUBLIC CONAN_PKG::boost)
target_include_directories(ctwenty48_core PUBLIC ${CONAN_INCLUDE_DIRS_GTEST})
target_include_directories(ctwenty48_core PUBLIC ${tiny-dnn_SOURCE_DIR})

target_include_directories(ctwenty48_core PUBLIC ${PROJECT_SOURCE_DIR}/include)


set_property(TARGET ctwenty48_core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)


#MAIN EXEC
add_executable(ctwenty48 src/main.cc )
target_link_libraries(ctwenty48 PRIVATE ctwenty48_core)


#TESTING
include(GoogleTest)
#utils
add_executable(unit_test 
	test/utils_test.cc
	test/search_test.cc
	)
target_link_libraries(unit_test CONAN_PKG::gtest ctwenty48_core)
gtest_discover_tests(unit_test)

