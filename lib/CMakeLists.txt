cmake_minimum_required (VERSION 3.16)

#setting up glfw:
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_VULKAN_STATIC ON)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glfw")
add_subdirectory(${GLFW_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_include_directories(${PROJECT_NAME} PUBLIC "${GLFW_DIR}/include")


#setting up native file dialog:
set(NFD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/nativefiledialog-extended")
add_subdirectory("${NFD_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE nfd)
target_include_directories(${PROJECT_NAME} PUBLIC "${NFD_DIR}/src/include")

#setting up imgui:
set(IMGUI "imgui/")
target_sources(${PROJECT_NAME} PUBLIC "${IMGUI}imgui.cpp" "${IMGUI}imgui_tables.cpp" "${IMGUI}imgui_draw.cpp" 
	"${IMGUI}imgui_widgets.cpp" "${IMGUI}backends/imgui_impl_glfw.cpp" "${IMGUI}backends/imgui_impl_vulkan.cpp" )

target_include_directories(${PROJECT_NAME} PUBLIC "${IMGUI}" "${IMGUI}backends/")
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_IMGUI)
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GLFW)

#volk:
target_sources(${PROJECT_NAME} PUBLIC "volk/volk.c")
target_compile_definitions(${PROJECT_NAME} PUBLIC VK_NO_PROTOTYPES IMGUI_IMPL_VULKAN_USE_VOLK)


#adding header libraries:
target_include_directories(${PROJECT_NAME} PUBLIC 
	"shl" 
	"glm"
	"stb"
	"imgui"
	"vkl"
	"tinygltf"
	"volk"
	"Vulkan-Headers/include"
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
		"volk"
		"Vulkan-Headers/include"
	)
endif()
