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
target_sources(${PROJECT_NAME} PUBLIC "${IMGUI}imgui.cpp" "${IMGUI}imgui_tables.cpp" "${IMGUI}imgui_draw.cpp"  "${IMGUI}imgui_demo.cpp" "${IMGUI}imgui_widgets.cpp")
#target_sources(${PROJECT_NAME} PUBLIC "${IMGUI}backends/imgui_impl_glfw.cpp" "${IMGUI}backends/imgui_impl_vulkan.cpp" )

target_include_directories(${PROJECT_NAME} PUBLIC "${IMGUI}")
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_IMGUI)
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GLFW)

#volk:
target_sources(${PROJECT_NAME} PUBLIC "volk/volk.c")
target_compile_definitions(${PROJECT_NAME} PUBLIC VK_NO_PROTOTYPES IMGUI_IMPL_VULKAN_USE_VOLK)

#define the platform specific options
if (WIN32)
	message(STATUS "building for windows")
	if (MSVC)
		target_compile_definitions(${PROJECT_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
	endif()
	target_compile_definitions(${PROJECT_NAME} PUBLIC SGF_OS_WINDOWS)
elseif(UNIX AND NOT APPLE)
	target_compile_definitions(${PROJECT_NAME} PUBLIC SGF_OS_LINUX)
	option(SGF_USE_WAYLAND "defines usage of x11 wayland" FALSE)
	if (SGF_USE_WAYLAND)
		target_compile_definitions(${PROJECT_NAME} PUBLIC SGF_USE_WAYLAND)
	else()
		target_compile_definitions(${PROJECT_NAME} PUBLIC SGF_USE_X11)
	endif()
	message(STATUS "building for linux")
elseif(UNIX)
	message(STATUS "building for macOS")
	target_compile_definitions(${PROJECT_NAME} PUBLIC SGF_OS_APPLE)
else()
	message(FATAL_ERROR "unknown OS")
endif()

#VulkanMemoryAllocator:
#add_subdirectory("VulkanMemoryAllocator")
#target_link_libraries(${PROJECTT_NAME} VulkanMemoryAllocator)


set(HEADER_DIRS PUBLIC
	"shl" 
	"glm"
	"stb"
	"imgui"
	"vkl"
	"tinygltf"
	"volk"
	"Vulkan-Headers/include"
)
#adding header libraries:
target_include_directories(${PROJECT_NAME} PUBLIC ${HEADER_DIRS})