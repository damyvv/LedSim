cmake_minimum_required(VERSION 3.7)

set(SDL_SHARED OFF CACHE INTERNAL "")
set(SDL_ATOMIC OFF CACHE INTERNAL "")
set(SDL_AUDIO OFF CACHE INTERNAL "")
set(SDL_CPUINFO OFF CACHE INTERNAL "")
set(SDL_DLOPEN OFF CACHE INTERNAL "")
set(SDL_FILE OFF CACHE INTERNAL "")
set(SDL_FILESYSTEM OFF CACHE INTERNAL "")
set(SDL_RENDER OFF CACHE INTERNAL "")
set(VIDEO_OPENGLES OFF CACHE INTERNAL "")
set(VIDEO_VULKAN OFF CACHE INTERNAL "")

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
