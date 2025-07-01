cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)

set(PROJECT_NAME VKL)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

project (${PROJECT_NAME} VERSION ${PROJECT_VERSION})

# add all source files from the src directory
file(GLOB_RECURSE SRC_FILES "test/*.cpp" )
    
#create executable
add_executable (${PROJECT_NAME} ${SRC_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_MINOR="${PROJECT_VERSION_MINOR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_PATCH="${PROJECT_VERSION_PATCH}")
if (MSVC)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vulkanSDK/Include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vulkanSDK/Lib/vulkan-1.lib)
