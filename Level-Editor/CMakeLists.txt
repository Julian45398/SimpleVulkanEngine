cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Level-Editor VERSION 0.0.1)


file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(Level-Editor ${SRC_FILES})

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


#add libraries
#add_subdirectory(${LIB_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_MINOR="${PROJECT_VERSION_MINOR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION_PATCH="${PROJECT_VERSION_PATCH}")

target_sources(${PROJECT_NAME} PUBLIC ${PLATFORM_FILES})

add_subdirectory(lib)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})

target_include_directories(${PROJECT_NAME} PUBLIC "src/")
target_link_libraries(Level-Editor PRIVATE SGF)

include("cmake/compile_shaders.cmake")

