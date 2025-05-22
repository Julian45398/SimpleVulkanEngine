#include "Render/Swapchain.hpp"
#include "Render/Device.hpp"
#include "Window.hpp"

#ifndef SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT
#define SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT 1000000000
#endif

namespace SGF {
	uint32_t Swapchain::nextImage(VkSemaphore imageAvailable, VkFence fence) const {
		auto& device = getDevice();
		uint32_t index;
		if (vkAcquireNextImageKHR(device.logical, handle, UINT32_MAX, imageAvailable, fence, &index) != VK_NULL_HANDLE) {
			fatal(ERROR_ACQUIRE_NEXT_IMAGE);
		}
		return index;
	}
	void Swapchain::presentImage(uint32_t imageIndex, const VkSemaphore* pWaitSemaphores, uint32_t waitCount) const {
		VkPresentInfoKHR info;
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;
		info.swapchainCount = 1;
		info.pImageIndices = &imageIndex;
		info.pSwapchains = &handle;
		info.pWaitSemaphores = pWaitSemaphores;
		info.waitSemaphoreCount = waitCount;
		info.pResults = nullptr;
		if (vkQueuePresentKHR(presentQueue, &info) != VK_SUCCESS) {
			fatal(ERROR_PRESENT_IMAGE);
		}
	}
	const Device& Swapchain::getDevice() const {
		return SGF::getDevice();
	}

	uint32_t loadImages(VkImage* pSwapchainImages) {
		auto& device = getDevice();
		uint32_t count;
		device.getSwapchainImages(handle, &count, pSwapchainImages);
		return count;
	}
	void update(uint32_t width, uint32_t height);
	void update(uint32_t width, uint32_t height, VkPresentModeKHR presentMode);
	void update(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void update(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentMode presentMode);

	Swapchain(const Device& device, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode);
	Swapchain(const Device& device, const Window& window, VkPresentModeKHR presentMode);
	~Swapchain();
}