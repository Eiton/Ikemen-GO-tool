SET(glfw_PATH "${PROJECT_SOURCE_DIR}/glfw")

file(GLOB glfw_INCLUDE
	"${glfw_PATH}/include/GLFW/*.h"
)
file(GLOB glfw_CPP
	"${glfw_PATH}/src/*.c"
)

add_library(glfw
	${glfw_CPP}
	${glfw_INCLUDE}
)

target_include_directories(glfw PUBLIC
	"${glfw_PATH}/src/"
	"${glfw_PATH}/include/"
	"${glfw_PATH}/deps/"
	config
)

target_compile_definitions(glfw PRIVATE _GLFW_WIN32 )