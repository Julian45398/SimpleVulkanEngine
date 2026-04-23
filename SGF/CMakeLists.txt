cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SimpleVulkanEngine C CXX)
message(STATUS "Cmake System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Cmake Host system: ${CMAKE_HOST_SYSTEM_NAME}")

# Add custom cmake/ folder to module path
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_library(SGF ${SRC_FILES})
target_include_directories(SGF PUBLIC "src/")
target_include_directories(SGF PUBLIC "include/")
target_compile_definitions(SGF PRIVATE SGF_ENABLE_VALIDATION)

add_subdirectory("lib/")

