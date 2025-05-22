#pragma once

#include "SGF_Core.hpp"

namespace SGF {
	class Swapchain {
		inline static VkSurfaceFormatKHR DEFAULT_SURFACE_FORMAT = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	public:
		uint32_t nextImage(VkSemaphore imageAvailable, VkFence fence) const;
		void presentImage(uint32_t imageIndex, const VkSemaphore* waitSemaphores, uint32_t waitCount) const;
		inline void presentImage(uint32_t imageIndex, VkSemaphore waitSemaphore) const { presentImage(imageIndex, &waitSemaphore, 1); }

		inline operator VkSwapchainKHR() const { return handle; }
		inline VkSwapchainKHR getHandle() const { return handle; }
		inline uint32_t getImageCount() const { return swapchainImageCount; }
		inline uint32_t getWidth() const { return width; }
		inline uint32_t getHeight() const { return height; }
		inline VkPresentModeKHR getPresentMode() const { return presentMode; }
		inline VkSurfaceFormatKHR getSurfaceFormat() const { return surfaceFormat; }
		inline VkFormat getImageFormat() const { return surfaceFormat.format; }
		inline VkColorSpaceKHR getColorSpace() const { return surfaceFormat.colorSpace; }
		const Device& getDevice() const;

		uint32_t loadImages(VkImage* pSwapchainImages);
		void update(uint32_t width, uint32_t height);
		void update(uint32_t width, uint32_t height, VkPresentModeKHR presentMode);
		void update(VkSurfaceKHR surface, uint32_t width, uint32_t height);
		void update(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentMode presentMode);

		Swapchain(const Device& device, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode);
		Swapchain(const Device& device, const Window& window, VkPresentModeKHR presentMode);
		~Swapchain();
	private:
		VkSwapchainKHR handle = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;
		VkSurfaceFormatKHR surfaceFormat = DEFAULT_SURFACE_FORMAT;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		uint32_t imageCount = 0;
		uint32_t width = 0;
		uint32_t height = 0;
	}
}