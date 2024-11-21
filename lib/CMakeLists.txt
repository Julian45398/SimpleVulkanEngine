cmake_minimum_required (VERSION 3.16)

message("including GLFW! ${LIB_DIR}")
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_VULKAN_STATIC ON)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_DIR "glfw")
add_subdirectory(${GLFW_DIR})



target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_include_directories(${PROJECT_NAME} PUBLIC "${GLFW_DIR}/include")

target_include_directories(${PROJECT_NAME} PUBLIC 
	"shl" 
	"glm"
	"stb"
	"imgui"
	"vkl"
	"tinygltf"
)

if (BUILD_TESTING)
	add_subdirectory("googletest")
	target_link_libraries(${TEST_TARGET} PUBLIC glfw)
	target_include_directories(${TEST_TARGET} PUBLIC "${GLFW_DIR}/include")

	target_include_directories(${TEST_TARGET} PUBLIC 
		"shl" 
		"glm"
		"stb"
		"imgui"
		"vkl"
		"tinygltf"
	)
endif()

#add header only libraries


if (WIN32)
	set(VULKAN_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkanSDK")
	target_link_libraries(${PROJECT_NAME} PUBLIC "${VULKAN_DIR}/Lib/vulkan-1.lib")
	target_include_directories(${PROJECT_NAME} PUBLIC "${VULKAN_DIR}/Include")
	if (BUILD_TESTING)
		target_link_libraries(${TEST_TARGET} PUBLIC "${VULKAN_DIR}/Lib/vulkan-1.lib")
		target_include_directories(${TEST_TARGET} PUBLIC "${VULKAN_DIR}/Include")
	endif()
else()
	message(FATAL_ERROR "only windows currently supported!")
endif()



