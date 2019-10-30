cmake_minimum_required(VERSION 3.7)

project(LedSim VERSION 1.0.0 LANGUAGES C)

set(CMAKE_PREFIX_PATH 
	${CMAKE_PREFIX_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL2-2.0.10
)

find_package(SDL2 REQUIRED)

add_executable(LedSim src/main.c src/ledsim.c)

target_include_directories(LedSim
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/inc
		${SDL2_INCLUDE_DIRS}
)

target_link_libraries(LedSim ${SDL2_LIBRARIES} opengl32)
