cmake_minimum_required(VERSION 3.9.4)

set(CONAN_DEP_PATH ${PROJECT_SOURCE_DIR}/gui_app)
execute_process(COMMAND conan install ${CONAN_DEP_PATH}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gui_app
	)
include(${CMAKE_BINARY_DIR}/gui_app/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# providing x11 libraries is a pain so that is gonna be a dependency
# install debian package libx11-dev
# todo HOWTO incorporate that into conan?
find_package(X11 REQUIRED)
target_link_libraries(CONAN_PKG::nana INTERFACE
	${X11_LIBRARIES}
	${X11_Xft_LIB}
	)
find_package(Freetype)                # Freetype - todo test PRIVATE
if (FREETYPE_FOUND)
	find_package(Fontconfig REQUIRED)
	target_link_libraries(CONAN_PKG::nana INTERFACE
		${FREETYPE_LIBRARIES}
		${FONTCONFIG_LIBRARIES}
		)
endif(FREETYPE_FOUND)


add_executable(ctwenty48_gui 
	src/main.cc
	src/gui.cc
	src/widgets.cc
	src/gui_env.cc
	)

target_link_libraries(ctwenty48_gui PUBLIC
	ctwenty48_core
	CONAN_PKG::nana    
	)
target_include_directories(ctwenty48_gui PUBLIC
	${PROJECT_SOURCE_DIR}/gui_app/include
	)

