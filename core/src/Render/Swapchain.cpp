#include "Render/Swapchain.hpp"
#include "Render/Device.hpp"
#include "Window.hpp"

#ifndef SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT
#define SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT 1000000000
#endif

namespace SGF {
	VkAttachmentDescription Swapchain::createAttachment(const Device& device, VkSurfaceKHR surface, VkAttachmentLoadOp loadOp) {
		VkAttachmentDescription att;
		att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		att.samples = VK_SAMPLE_COUNT_1_BIT;
		att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		att.loadOp = loadOp;
		att.format = device.pickSurfaceFormat(surface, DEFAULT_SURFACE_FORMAT).format;
		att.flags = 0;
		att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		return att;
	}
	uint32_t Swapchain::nextImage(VkSemaphore imageAvailable, VkFence fence) const {
		assert(handle != VK_NULL_HANDLE);
		auto& device = getDevice();
		uint32_t index;
		if (vkAcquireNextImageKHR(device, handle, SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT, imageAvailable, fence, &index) != VK_SUCCESS) {
			fatal(ERROR_ACQUIRE_NEXT_IMAGE);
		}
		return index;
	}
	void Swapchain::presentImage(uint32_t imageIndex, const VkSemaphore* pWaitSemaphores, uint32_t waitCount) const {
		assert(handle != VK_NULL_HANDLE);
		
		VkPresentInfoKHR info;
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;
		info.swapchainCount = 1;
		info.pImageIndices = &imageIndex;
		info.pSwapchains = &handle;
		info.pWaitSemaphores = pWaitSemaphores;
		info.waitSemaphoreCount = waitCount;
		info.pResults = nullptr;
		VkResult result = vkQueuePresentKHR(presentQueue, &info);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
			warn("swapchain out of date!");
		}
		else if (result != VK_SUCCESS) {
			fatal(ERROR_PRESENT_IMAGE);
		}
	}
	uint32_t Swapchain::loadImages(VkImage* pSwapchainImages) {
		assert(handle != VK_NULL_HANDLE);
		auto& device = getDevice();
		uint32_t count;
		device.getSwapchainImages(handle, &count, pSwapchainImages);
		return count;
	}
	void Swapchain::update(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
		imageCount = 0;
		const auto& device = getDevice();
		presentMode = device.pickPresentMode(surface, presentMode);
		surfaceFormat = device.pickSurfaceFormat(surface, surfaceFormat);
        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical, surface, &capabilities);
        imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }
        width = std::min(capabilities.maxImageExtent.width, std::max(width, capabilities.minImageExtent.width));
        height = std::min(capabilities.maxImageExtent.height, std::max(height, capabilities.minImageExtent.height));
		VkSwapchainKHR old = handle;
        info.oldSwapchain = old;
        info.surface = surface;
        info.minImageCount = imageCount;
        info.imageFormat = surfaceFormat.format;
        info.imageColorSpace = surfaceFormat.colorSpace;
        info.imageExtent = { width, height };
        info.imageArrayLayers = 1;
        if (device.presentFamilyIndex != device.graphicsFamilyIndex) {
            uint32_t queueFamilyIndices[] = { device.graphicsFamilyIndex, device.presentFamilyIndex };
            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        info.preTransform = capabilities.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = presentMode;
        info.clipped = VK_TRUE;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		handle = device.swapchain(info);
		vkGetSwapchainImagesKHR(device.logical, handle, &imageCount, nullptr);
		if (old != VK_NULL_HANDLE) {
			device.destroy(old);
		}
		assert(imageCount != 0);
	}
	void Swapchain::destroy() {
		auto& device = Device::Get();
		if (isInitialized()) {
			device.destroy(handle);
			handle = VK_NULL_HANDLE;
		}
	}
	void Swapchain::create(VkSurfaceKHR s, uint32_t w, uint32_t h, VkPresentModeKHR mode) {
		destroy();
		presentQueue = getDevice().presentQueue();
		update(s, w, h, mode);
	}
}