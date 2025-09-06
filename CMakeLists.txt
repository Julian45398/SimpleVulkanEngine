cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(PROJECT_VERSION_MAJOR 0)
#set(PROJECT_VERSION_MINOR 0)
#set(PROJECT_VERSION_PATCH 1)

set(PROJECT_NAME SimpleVulkanEngine)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

project(Build-System VERSION 0.0.1)

set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")

# add all source files from the src directory
#option(BUILD_RUNTIME OFF)

if (BUILD_RUNTIME)
	#message(STATUS "building runtime")
	#set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/core/src" "${CMAKE_SOURCE_DIR}/core/include" "${CMAKE_SOURCE_DIR}/runtime/src")
	#file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/core/src/*.cpp" "${CMAKE_SOURCE_DIR}/runtime/src/*.cpp")
else()
	#message(STATUS "building editor")
	#set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/core/src" "${CMAKE_SOURCE_DIR}/editor/src" "${CMAKE_SOURCE_DIR}/core/include")
	#file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/core/src/*.cpp" "${CMAKE_SOURCE_DIR}/editor/src/*.cpp")
endif()

#create executable
#add_executable(${PROJECT_NAME} ${SRC_FILES})
add_subdirectory("SGF/")

add_subdirectory("Level-Editor")
link_directories()

#include("cmake/update_submodules.cmake")