add_library(glad
	"${PROJECT_SOURCE_DIR}/glad/src/glad.c"
)

target_include_directories(glad PUBLIC
	"${PROJECT_SOURCE_DIR}/glad/include"
)

target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD  )
target_link_libraries(imgui PUBLIC glad Imm32 Dwmapi)