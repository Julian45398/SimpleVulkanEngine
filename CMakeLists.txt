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
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp" )
    
#include("cmake/update_submodules.cmake")

#create executable
add_executable (${PROJECT_NAME} ${SRC_FILES})
#add tests
add_subdirectory(${TEST_DIR})

option(RUN_IN_IDE "defines if run in IDE" OFF)
if (RUN_IN_IDE)
endif()

	target_compile_definitions(${PROJECT_NAME} PRIVATE RUN_IN_IDE)
#define the platform specific options
if (WIN32)
	message("building for windows")
	set(INCLUDE_GLFW ON)
	set(PLATFORM_DIR "platform/windows/")
	set(IMGUI "lib/imgui/")
	target_sources(${PROJECT_NAME} PUBLIC "${IMGUI}imgui.cpp" "${IMGUI}imgui_tables.cpp" "${IMGUI}imgui_draw.cpp" 
		"${IMGUI}imgui_widgets.cpp" "${IMGUI}backends/imgui_impl_glfw.cpp" "${IMGUI}backends/imgui_impl_vulkan.cpp" )

	target_include_directories(${PROJECT_NAME} PUBLIC "${IMGUI}" "${IMGUI}backends/")
	target_compile_definitions(${PROJECT_NAME} PRIVATE USE_IMGUI)
	target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GLFW)

	if (MSVC)
		target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
	endif()
else()
	message(FATAL_ERROR "only windows currently supported!")
endif()
#add libraries
add_subdirectory(${LIB_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_MINOR="${PROJECT_VERSION_MINOR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_PATCH="${PROJECT_VERSION_PATCH}")

file(GLOB_RECURSE PLATFORM_FILES "${PLATFORM_DIR}/*.cpp")

target_sources(${PROJECT_NAME} PUBLIC ${PLATFORM_FILES})
target_sources(${TEST_TARGET} PUBLIC ${PLATFORM_FILES})


target_include_directories(${PROJECT_NAME} PUBLIC ${PLATFORM_DIR} ${SRC_DIR})
target_include_directories(${TEST_TARGET} PUBLIC ${PLATFORM_DIR} ${SRC_DIR})

include("cmake/compile_shaders.cmake")
include("cmake/update_submodules.cmake")

