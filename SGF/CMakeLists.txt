cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SGF VERSION 0.0.1)

set(GLOB_RECU)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_library(SGF ${SRC_FILES})
target_include_directories(SGF PUBLIC "src/")
target_include_directories(SGF PUBLIC "include/")
target_compile_definitions(SGF PRIVATE SGF_ENABLE_VALIDATION)
add_subdirectory("lib/")
