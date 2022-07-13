cmake_minimum_required(VERSION 3.0.0)
project(ctwenty48 VERSION 0.1.0)


include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

include(CTest)
enable_testing()

add_executable(ctwenty48 src/main.cpp)

include(CPack)
