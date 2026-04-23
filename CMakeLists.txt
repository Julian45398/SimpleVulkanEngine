cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SimpleVulkanEngine C CXX)
message(STATUS "Cmake System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Cmake Host system: ${CMAKE_HOST_SYSTEM_NAME}")

#create executable
add_subdirectory("SGF/")

add_subdirectory("Level-Editor")
link_directories()
