#pragma once

#include "core.h"

namespace sve {
	inline GLFWwindow* G_Window = nullptr;
	inline VkAllocationCallbacks G_Callbacks = {};
	inline VkInstance G_VkInstance = VK_NULL_HANDLE;
	inline VkSurfaceKHR G_VkSurface = VK_NULL_HANDLE;
	inline VkPhysicalDevice G_VkPhysicalDevice = VK_NULL_HANDLE;
	inline VkDevice G_VkDevice = VK_NULL_HANDLE;
	inline VkSwapchainKHR G_VkSwapchain = VK_NULL_HANDLE;
	inline VkQueue G_VkGraphicsQueue = VK_NULL_HANDLE;
	inline VkQueue G_VkPresentQueue = VK_NULL_HANDLE;
	inline uint32_t G_VkGraphicsIndex = VK_NULL_HANDLE;
	inline uint32_t G_VkPresentIndex = VK_NULL_HANDLE;

	void initVulkan();
	void terminateVulkan();

	inline void initCore() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		G_Window = glfwCreateWindow(400, 300, PROJECT_NAME, nullptr, nullptr);
		initVulkan();
	}

	inline void terminateCore() {
		terminateVulkan();
		glfwDestroyWindow(G_Window);
		glfwTerminate;
	}

}