cmake_minimum_required(VERSION 3.13...3.19)
project(ikemen_tool VERSION 0.1 LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")


set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )
set (EXE "Ikemen_Tool" )

include(imgui)
include(glfw)
include(glad)
include(lodepng)
#include(IconFontCppHeaders)

add_executable(${EXE} WIN32
	src/main.cpp
	src/fileDialog.cpp
	src/combobox.cpp
	src/char.cpp
	src/render.cpp
	src/utility.cpp)
	
target_include_directories(${EXE} PRIVATE ".")
target_include_directories(${EXE} PRIVATE "./IconFontCppHeaders/")
target_compile_definitions(${EXE} PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)

find_package(OpenGL REQUIRED)
target_link_libraries(${EXE} PRIVATE glad imgui lodepng winmm glfw OpenGL::GL)

if(MINGW)
	message(status "With -municode")
	target_link_options(${EXE} PRIVATE -municode)
endif()
