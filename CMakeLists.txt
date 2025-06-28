cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)

set(PROJECT_NAME SimpleVulkanEngine)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

project (${PROJECT_NAME} VERSION ${PROJECT_VERSION})

set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")
set(TEST_TARGET Tests)
include(CTest)

# add all source files from the src directory
option(BUILD_RUNTIME OFF)

if (BUILD_RUNTIME)
	message(STATUS "building runtime")
	set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/core/src" "${CMAKE_SOURCE_DIR}/core/include" "${CMAKE_SOURCE_DIR}/runtime/src")
	file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/core/src/*.cpp" "${CMAKE_SOURCE_DIR}/runtime/src/*.cpp")
else()
	message(STATUS "building editor")
	set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/core/src" "${CMAKE_SOURCE_DIR}/editor/src" "${CMAKE_SOURCE_DIR}/core/include")
	file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/core/src/*.cpp" "${CMAKE_SOURCE_DIR}/editor/src/*.cpp")
endif()

#create executable
add_executable(${PROJECT_NAME} ${SRC_FILES})
#add tests
add_subdirectory(${TEST_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_SINGLE_GPU)
#add defines depending on build mode 
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${PROJECT_NAME} PRIVATE SVE_LOG_ALL)
	target_compile_definitions(${PROJECT_NAME} PRIVATE VKL_ENABLE_VALIDATION)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_LOG_ALL)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_LOG_TERMINAL)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_ENABLE_VALIDATION)
	#target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_LOG_VULKAN_ALLOCATIONS)
	#target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_LOG_VULKAN_DEVICE_OBJECTS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_LOG_MSG)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	target_compile_definitions(${PROJECT_NAME} PRIVATE SVE_LOG_ALL)
	target_compile_definitions(${PROJECT_NAME} PRIVATE VKL_ENABLE_VALIDATION)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_LOG_ALL)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_LOG_TERMINAL)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_ENABLE_VALIDATION)
	target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_LOG_MSG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(${PROJECT_NAME} PRIVATE SVE_LOG_FATAL)
endif()

#define the platform specific options
if (WIN32)
	message(STATUS "building for windows")
	if (MSVC)
		target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
	endif()
	target_compile_definitions(${PROJECT_NAME} PUBLIC SGF_OS_WINDOWS)
elseif(UNIX AND NOT APPLE)
	target_compile_definitions(${PROJECT_NAME} PUBLIC SGF_OS_LINUX)
	option(SGF_USE_WAYLAND "defines usage of x11 wayland" FALSE)
	if (SGF_USE_WAYLAND)
		target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_USE_WAYLAND)
	else()
		target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_USE_X11)
	endif()
	message(STATUS "building for linux")
elseif(UNIX)
	message(STATUS "building for macOS")
	target_compile_definitions(${PROJECT_NAME} PRIVATE SGF_OS_APPLE)
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
#include("cmake/update_submodules.cmake")

