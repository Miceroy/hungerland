# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.10)

##
## Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##
## External Dependencies:
add_subdirectory("ext/mikroplot-main")
add_subdirectory("ext/glm-master")
add_subdirectory("ext/tmxlite-master/tmxlite")

##
## Include directories:
include_directories("include")
include_directories("ext/mikroplot-main/ext/glfw-3.3.6/deps")

## Platformer example
add_executable (platformer games/platformer/main.cpp ./include/engine/util.h include/engine/map.h src/shader.inl src/map.cpp)
target_link_libraries(platformer PUBLIC mikroplot glm tmxlite)
