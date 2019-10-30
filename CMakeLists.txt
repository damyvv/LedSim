cmake_minimum_required(VERSION 3.7)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL2-2.0.10)

project(LedSim VERSION 1.0.0 LANGUAGES C)

add_executable(LedSim src/main.c src/ledsim.c)

target_include_directories(LedSim
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/inc
		${SDL2_INCLUDE_DIRS}
)

if (WIN32)
	set(OPENGL_LIB opengl32)
else()
	set(OPENGL_LIB GL)
endif()

target_link_libraries(LedSim ${OPENGL_LIB} SDL2-static SDL2main)
