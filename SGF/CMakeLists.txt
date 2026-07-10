cmake_minimum_required(VERSION 3.16)

project(SGF VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add custom cmake/ folder to module path
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_library(SGF ${SRC_FILES})
target_include_directories(SGF PUBLIC "include/")
target_include_directories(SGF PRIVATE "src/")
target_compile_definitions(SGF PRIVATE SGF_ENABLE_VALIDATION)
target_compile_definitions(SGF PRIVATE SGF_ENGINE_NAME="${PROJECT_NAME}")
target_compile_definitions(SGF PRIVATE SGF_VERSION="${PROJECT_VERSION}")
target_compile_definitions(SGF PRIVATE SGF_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(SGF PRIVATE SGF_VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(SGF PRIVATE SGF_VERSION_PATCH=${PROJECT_VERSION_PATCH})
add_subdirectory("lib/")

