# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.10)
project(hungerland)
add_subdirectory("engine")

##
## Use C++20
option(HUNGERLAND_BUILD_EXAMPLES "Build hungerland examples" ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${HUNGERLAND_BUILD_EXAMPLES})
	## Platformer example has one cpp file which can be found from: examples/platformer/main.cpp
	add_executable(PlatformerExample examples/platformer/main.cpp examples/platformer.h)
	# Link platformer example with hungerland engine
	target_link_libraries(PlatformerExample hungerland)
	# Copy assets command
	add_custom_command(TARGET PlatformerExample POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${PROJECT_SOURCE_DIR}/examples/platformer/assets
		${PROJECT_BINARY_DIR}/assets
		COMMENT "Copying PlatformerExample asset files to binary directory")
endif()
