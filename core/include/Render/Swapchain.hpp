#pragma once

#include "SGF_Core.hpp"

#include "Device.hpp"
namespace SGF {
	class Swapchain {
	public:
		inline static const VkSurfaceFormatKHR DEFAULT_SURFACE_FORMAT = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		static VkAttachmentDescription createAttachment(const Device& device, VkSurfaceKHR surface, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
	public:
		uint32_t nextImage(VkSemaphore imageAvailableSignal, VkFence fence) const;
		void presentImage(uint32_t imageIndex, const VkSemaphore* waitSemaphores, uint32_t waitCount) const;
		inline void presentImage(uint32_t imageIndex, VkSemaphore waitSemaphore) const { presentImage(imageIndex, &waitSemaphore, 1); }

		inline operator VkSwapchainKHR() const { return handle; }
		inline VkSwapchainKHR getHandle() const { return handle; }
		inline uint32_t getImageCount() const { return imageCount; }
		inline VkPresentModeKHR getPresentMode() const { return presentMode; }
		inline VkSurfaceFormatKHR getSurfaceFormat() const { return surfaceFormat; }
		inline VkFormat getImageFormat() const { return surfaceFormat.format; }
		inline VkColorSpaceKHR getColorSpace() const { return surfaceFormat.colorSpace; }

		uint32_t loadImages(VkImage* pSwapchainImages);
		void update(VkSurfaceKHR surface, uint32_t width, uint32_t height);
		inline void update(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR requestedMode) {
			presentMode = requestedMode;
			update(surface, width, height);
		}

		inline void requestSurfaceFormat(VkSurfaceFormatKHR requested) { surfaceFormat = requested; }
		inline void requestColorSpace(VkColorSpaceKHR requested) { surfaceFormat.colorSpace = requested; }
		inline void requestImageFormat(VkFormat requested) { surfaceFormat.format; }
		inline void requestPresentMode(VkPresentModeKHR requested) { presentMode = requested; }
		inline bool isInitialized() const { return handle != VK_NULL_HANDLE; }

		void create(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR);
		void destroy();

		inline Swapchain(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR mode) : handle(VK_NULL_HANDLE), presentQueue(Device::Get().presentQueue()) 
		{ update(surface, width, height, mode); }
		inline Swapchain() : handle(VK_NULL_HANDLE), presentQueue(VK_NULL_HANDLE), surfaceFormat(DEFAULT_SURFACE_FORMAT), presentMode(VK_PRESENT_MODE_FIFO_KHR), imageCount(0) {}
		inline ~Swapchain() { destroy(); }
	private:
		VkSwapchainKHR handle;
		VkQueue presentQueue;
		VkSurfaceFormatKHR surfaceFormat;
		VkPresentModeKHR presentMode;
		uint32_t imageCount;
	};
}