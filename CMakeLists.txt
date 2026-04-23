cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#create executable
add_subdirectory("SGF/")

add_subdirectory("Level-Editor")
link_directories()
