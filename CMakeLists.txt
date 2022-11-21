# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.10)
add_subdirectory("engine")

## Platformer example
add_executable(platformer examples/platformer/main.cpp)
target_link_libraries(platformer hungerland)
