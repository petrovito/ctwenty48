cmake_minimum_required(VERSION 3.9.4) # this version enables LTO

# CONAN
set(CONAN_DEP_PATH ${PROJECT_SOURCE_DIR}/core)
execute_process(COMMAND conan install ${CONAN_DEP_PATH}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)



add_library(ctwenty48_core STATIC 
	src/types.cc
	src/search.cc
	src/mc_estimate.cc
	src/cnn.cc
	src/rollout_eval.cpp
	src/mcts.cc
	src/game_logger.cc
	src/selectors.cc
	src/game_play.cc
	src/ui.cc
	src/environment.cc
	)
target_link_libraries(ctwenty48_core PUBLIC 
    CONAN_PKG::boost
    CONAN_PKG::csv-parser
    CONAN_PKG::cppflow
	CONAN_PKG::spdlog
	CONAN_PKG::concurrentqueue
	CONAN_PKG::fmt
	CONAN_PKG::onetbb
	)
target_include_directories(ctwenty48_core PUBLIC 
	${CONAN_INCLUDE_DIRS_GTEST}
	${PROJECT_SOURCE_DIR}/core/include
	)
set_property(TARGET ctwenty48_core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)


#TESTING
if (BUILD_TEST)
	enable_testing()
	include(GoogleTest)
	add_executable(unit_test 
		test/utils_test.cc
		test/search_test.cc
		test/csv_test.cc
		test/selector_test.cc
		test/cnn_test.cc
		)
	target_link_libraries(unit_test CONAN_PKG::gtest ctwenty48_core)
	gtest_discover_tests(unit_test)
endif()

