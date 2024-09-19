// SimpleVulkanEngine.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#ifndef NDEBUG
#define SHL_LOG_ALL
#else
#define SHL_LOG_WARN
#endif // !NDEBUG

#include "shl/logging.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"



int main()
{
	shl::logInfo("Version: ", PROJECT_VERSION);
	VkInstance vk_instance = nullptr;
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pEngineName = "test engine";
	app_info.apiVersion = VK_VERSION_1_0;
	app_info.pApplicationName = "test_app";
	app_info.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
	app_info.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
	VkInstanceCreateInfo c_info{};
	c_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	c_info.pApplicationInfo = &app_info;

	if (vkCreateInstance(&c_info, nullptr, &vk_instance) != VK_SUCCESS) {
		shl::logFatal("failed to create vulkan instance!");
	}
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(600, 400, PROJECT_NAME, nullptr, nullptr);

	while (!glfwWindowShouldClose(window)) 
		glfwPollEvents();

	glfwDestroyWindow(window);

	return 0;
}
