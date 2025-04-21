#pragma once

#include "SVE_Core.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class SVE_Display {
private:
	friend SVE_Device;
	friend void SVE::init();
	friend void SVE::terminate();
	uint32_t width = 0;
	uint32_t height = 0;
	GLFWwindow* window = nullptr;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkImage depthImage = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;
	VkDeviceMemory depthMemory = VK_NULL_HANDLE;
	SVE_Device* device = nullptr;
	struct Framebuffer {
		VkFramebuffer framebuffer;
		VkImageView imageView;
		VkImage image;
	};
	std::vector<Framebuffer> framebuffers;
public:
	/**
	* @brief creates a Fullscreen window
	*/
	SVE_Display(const char* name);
	/**
	* @brief creates a window with the given height and width
	*/
	SVE_Display(const char* name, uint32_t width, uint32_t height);
	inline uint32_t getWidth() { return width; }
	inline uint32_t getHeight() { return height; }
	uint32_t getImageCount();
	void setPresentMode(VkPresentModeKHR presentMode);
	void setFramebufferSize(uint32_t width, uint32_t height);
	void setFullscreen();
	void setWindowed(uint32_t width, uint32_t height);
	void toggleFullscreen();
	void resizeWindow(uint32_t width, uint32_t height);
	void bindDevice(const SVE_Device& device, VkPresentModeKHR presentMode);
};