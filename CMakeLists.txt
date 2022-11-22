# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.10)
add_subdirectory("engine")

## Platformer example has one cpp file which can be found from: examples/platformer/main.cpp
#add_executable(platformer examples/platformer/main.cpp examples/platformer.h)
# Link platformer example with hungerland engine
#target_link_libraries(platformer hungerland)


## Platformer example has one cpp file which can be found from: examples/platformer/main.cpp
add_executable(CyberSpace examples/cs/main.cpp examples/platformer.h)
# Link platformer example with hungerland engine
target_link_libraries(CyberSpace hungerland)
