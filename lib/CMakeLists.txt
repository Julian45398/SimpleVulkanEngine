cmake_minimum_required (VERSION 3.16)

#setting up glfw:
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_VULKAN_STATIC ON)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glfw")
add_subdirectory(${GLFW_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_include_directories(${PROJECT_NAME} PUBLIC "${GLFW_DIR}/include")

#setting up imgui:
set(IMGUI "imgui/")
target_sources(${PROJECT_NAME} PUBLIC "${IMGUI}imgui.cpp" "${IMGUI}imgui_tables.cpp" "${IMGUI}imgui_draw.cpp" 
	"${IMGUI}imgui_widgets.cpp" "${IMGUI}backends/imgui_impl_glfw.cpp" "${IMGUI}backends/imgui_impl_vulkan.cpp" )

target_include_directories(${PROJECT_NAME} PUBLIC "${IMGUI}" "${IMGUI}backends/")
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_IMGUI)
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_GLFW)

#adding header libraries:
target_include_directories(${PROJECT_NAME} PUBLIC 
	"shl" 
	"glm"
	"stb"
	"imgui"
	"vkl"
	"tinygltf"
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
		"Vulkan-Headers/include"
	)
endif()

#add header only libraries

#searching for vulkan library:
if(WIN32)
	set(VK_LIB_NAME "vulkan-1.lib")
	set(PATH_LIST $ENV{PATH})
else()
	set(VK_LIB_NAME "libvulkan.so")
	string(REPLACE ":" ";" PATH_LIST $ENV{PATH})
endif()

set(VULKAN_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/vulkan")
set(VULKAN_LIBRARY "${VULKAN_DIR}/Lib/${VK_LIB_NAME}")
if (NOT EXISTS "${VULKAN_LIBRARY}")
	message(STATUS "failed to find library in local dir: ${VULKAN_DIR}")
	message(STATUS "searching path variables...")
	set(FILE_FOUND FALSE)
	foreach(PATH_DIR $ENV{PATH})
		set(CURRENT_FILE "${PATH_DIR}/${VK_LIB_NAME}")
		set(OTHER_FILE "${PATH_DIR}/Lib/${VK_LIB_NAME}")
		message(STATUS "searching path: ${PATH_DIR}")
		if(EXISTS "${CURRENT_FILE}" OR EXISTS )
			set(FILE_FOUND TRUE)
			set(VULKAN_LIBRARY "${CURRENT_FILE}")
			break()
		elseif(EXISTS "${OTHER_FILE}")
			set(FILE_FOUND TRUE)
			set(VULKAN_LIBRARY "${OTHER_FILE}")
			break()
		endif()
	endforeach()
	if (NOT FILE_FOUND)
		message(FATAL_ERROR "failed to find vulkan library: ${VK_LIB_NAME} in system path")
	else()
		message(STATUS "vulkan library found: ${VULKAN_LIBRARY}")
	endif()
else()
	message(STATUS "vulkan library found locally: ${VULKAN_LIBRARY}")
endif()
#link vulkan library
target_link_libraries(${PROJECT_NAME} PUBLIC ${VULKAN_LIBRARY})
if (BUILD_TESTING)
	target_link_libraries(${TEST_TARGET} PUBLIC ${VULKAN_LIBRARY})
endif()
