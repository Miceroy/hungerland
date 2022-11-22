# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.10)

##
## Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##
## External Dependencies:
option(MIKROPLOT_BUILD_EXAMPLES "Build MikRoPlot examples" OFF )
add_subdirectory("ext/mikroplot-main")
add_subdirectory("ext/glm-master")
add_subdirectory("ext/tmxlite-master/tmxlite")

##
## Include directories:
file(GLOB_RECURSE ENGINE_INC_FILES "include/hungerland/*.h")
file(GLOB_RECURSE ENGINE_SRC_FILES "src/*.cpp")
file(GLOB_RECURSE ENGINE_INL_FILES "src/*.inl")
add_library(hungerland ${ENGINE_INC_FILES} ${ENGINE_SRC_FILES} ${ENGINE_INL_FILES})
target_link_libraries(hungerland PUBLIC mikroplot glm tmxlite)
target_include_directories(hungerland PUBLIC "include")
target_include_directories(hungerland PUBLIC "ext/mikroplot-main/ext/glfw-3.3.6/deps")
target_include_directories(hungerland PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/hungerland>
)


