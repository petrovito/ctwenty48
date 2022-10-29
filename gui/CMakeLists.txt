cmake_minimum_required(VERSION 3.9.4)

add_executable(ctwenty48_gui 
	src/main.cc
	src/gui.cc
	)

target_link_libraries(ctwenty48_gui PUBLIC
	ctwenty48_core
	CONAN_PKG::nana    
	)
target_include_directories(ctwenty48_gui PUBLIC
	${PROJECT_SOURCE_DIR}/gui/include
	)

