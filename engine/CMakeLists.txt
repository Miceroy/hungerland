# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.10)

##
## Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##
## External Dependencies:
option(TMXLITE_STATIC_LIB "" ON)
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
add_subdirectory("ext/glfw-3.3.6")
add_subdirectory("ext/glm-master")
add_subdirectory("ext/tmxlite-master/tmxlite")


##
## hungerland library:
# Define glad source files
set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad_gl.c")
file(GLOB_RECURSE ENGINE_INC_FILES "include/hungerland/*.h")
file(GLOB_RECURSE ENGINE_SRC_FILES "src/*.cpp")
file(GLOB_RECURSE ENGINE_INL_FILES "src/*.inl")
add_library(hungerland ${ENGINE_INC_FILES} ${ENGINE_SRC_FILES} ${ENGINE_INL_FILES} ${GLAD_GL})
target_link_libraries(hungerland PRIVATE glfw tmxlite PUBLIC glm)
if(WIN32)
	#target_compile_definitions(hungerland PUBLIC /wd4005)
	#add_definitions("/wd4005")
endif()
# Specify include directories
target_include_directories(hungerland PRIVATE "ext/stb-master")
target_include_directories(hungerland PRIVATE "ext/miniaudio")
target_include_directories(hungerland PRIVATE "${GLFW_SOURCE_DIR}/deps")
target_include_directories(hungerland PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/hungerland>
)


set_target_properties(hungerland PROPERTIES FOLDER "hungerland")
set_target_properties(tmxlite PROPERTIES FOLDER "hungerland")
set_target_properties(uninstall PROPERTIES FOLDER "hungerland")
set_target_properties(glfw PROPERTIES FOLDER "hungerland")
set_target_properties(update_mappings PROPERTIES FOLDER "hungerland")

