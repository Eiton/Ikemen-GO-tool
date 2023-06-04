SET(lodepng_PATH "${PROJECT_SOURCE_DIR}/lodepng")

file(GLOB lodepng_CPP
	"${lodepng_PATH}/lodepng.cpp"
)

add_library(lodepng
	${lodepng_CPP}
)


target_include_directories(lodepng PUBLIC
	"${lodepng_PATH}"
)

target_compile_definitions(lodepng PRIVATE  )