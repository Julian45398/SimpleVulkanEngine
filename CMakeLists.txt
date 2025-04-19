cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)

set(PROJECT_NAME SimpleVulkanEngine)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

project (${PROJECT_NAME} VERSION ${PROJECT_VERSION})

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")
set(TEST_TARGET Tests)
include(CTest)

# add all source files from the src directory
option(BUILD_RUNTIME OFF)

if (BUILD_RUNTIME)
	message(STATUS "building runtime")
	set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/core/src" "${CMAKE_SOURCE_DIR}/runtime/src")
	file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/core/src/*.cpp" "${CMAKE_SOURCE_DIR}/runtime/src/*.cpp")
else()
	message(STATUS "building editor")
	set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/core/src" "${CMAKE_SOURCE_DIR}/editor/src")
	file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/core/src/*.cpp" "${CMAKE_SOURCE_DIR}/editor/src/*.cpp")
endif()

#create executable
add_executable(${PROJECT_NAME} ${SRC_FILES})
#add tests
add_subdirectory(${TEST_DIR})

#add defines depending on build mode 
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
endif()

#define the platform specific options
if (WIN32)
	message(STATUS "building for windows")
	if (MSVC)
		target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
	endif()
	target_compile_definitions(${PROJECT_NAME} PRIVATE SVE_WINDOWS)
elseif(UNIX AND NOT APPLE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SVE_LINUX)
	option(SVE_USE_WAYLAND "defines usage of x11 wayland" TRUE)
	if (SVE_USE_WAYLAND)
		target_compile_definitions(${PROJECT_NAME} PRIVATE SVE_USE_WAYLAND)
	else()
		target_compile_definitions(${PROJECT_NAME} PRIVATE SVE_USE_X11)
	endif()
	message(STATUS "building for linux")
elseif(UNIX)
	message(STATUS "building for macOS")
	target_compile_definitions(${PROJECT_NAME} PRIVATE SVE_APPLEOS)
else()
	message(FATAL_ERROR "unknown OS")
endif()
#add libraries
add_subdirectory(${LIB_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_MINOR="${PROJECT_VERSION_MINOR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_PATCH="${PROJECT_VERSION_PATCH}")

target_sources(${PROJECT_NAME} PUBLIC ${PLATFORM_FILES})
target_sources(${TEST_TARGET} PUBLIC ${PLATFORM_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC ${SRC_DIR})
target_include_directories(${TEST_TARGET} PUBLIC ${SRC_DIR})

include("cmake/compile_shaders.cmake")
include("cmake/update_submodules.cmake")

