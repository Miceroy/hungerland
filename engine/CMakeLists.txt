# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.10)

##
## Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##
## External Dependencies:
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
add_subdirectory("./ext/glfw-3.3.6")
#option(MIKROPLOT_BUILD_EXAMPLES "Build MikRoPlot examples" OFF )
#add_subdirectory("ext/mikroplot-main")
add_subdirectory("ext/glm-master")
add_subdirectory("ext/tmxlite-master/tmxlite")


##
## hungerland library:
# Define glad source files
set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h" "${GLFW_SOURCE_DIR}/deps/glad_gl.c")
file(GLOB_RECURSE ENGINE_INC_FILES "include/hungerland/*.h")
file(GLOB_RECURSE ENGINE_SRC_FILES "src/*.cpp")
file(GLOB_RECURSE ENGINE_INL_FILES "src/*.inl")
add_library(hungerland ${ENGINE_INC_FILES} ${ENGINE_SRC_FILES} ${ENGINE_INL_FILES} ${GLAD_GL})
target_link_libraries(hungerland PUBLIC glfw glm tmxlite)
# Specify include directories
include_directories("include")
target_include_directories(hungerland PUBLIC "ext/glfw-3.3.6/include")
target_include_directories(hungerland PUBLIC "ext/stb-master")
target_include_directories(hungerland PUBLIC "ext/miniaudio")
target_include_directories(hungerland PUBLIC "${GLFW_SOURCE_DIR}/deps")

target_include_directories(hungerland PUBLIC "include")
target_include_directories(hungerland PUBLIC "ext/mikroplot-main/ext/glfw-3.3.6/deps")
target_include_directories(hungerland PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/hungerland>
)


