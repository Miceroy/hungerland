# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.10)
project(hungerland)
add_subdirectory("engine")

##
## Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Platformer example has one cpp file which can be found from: examples/platformer/main.cpp
#add_executable(platformer examples/platformer/main.cpp examples/platformer.h)
# Link platformer example with hungerland engine
#target_link_libraries(platformer hungerland)


## Platformer example has one cpp file which can be found from: examples/platformer/main.cpp
add_executable(PlatformerExample examples/platformer/main.cpp examples/platformer.h)
# Link platformer example with hungerland engine
target_link_libraries(PlatformerExample hungerland)
