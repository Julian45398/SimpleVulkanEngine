#pragma once

#include "vkl_core.h"

namespace vkl {
	inline VkSurfaceFormatKHR pickSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat) {
		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, nullptr);
		std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
		if (format_count != 0) {
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, surface_formats.data());
		}
		else {
			return { VK_FORMAT_MAX_ENUM, VK_COLOR_SPACE_MAX_ENUM_KHR };
		}
		for (const auto& available_format : surface_formats) {
			if (available_format.format == surfaceFormat.format && available_format.colorSpace == surfaceFormat.colorSpace)
			{
				return available_format;
			}
		}
		return surface_formats[0];
	}
	inline VkPresentModeKHR pickPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t presentModeCount, const VkPresentModeKHR* presentModes) {
		uint32_t available_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &available_mode_count, nullptr);
		std::vector<VkPresentModeKHR> available_modes(available_mode_count);
		if (available_mode_count != 0) {
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &available_mode_count, available_modes.data());
		}
		else {
			return VK_PRESENT_MODE_MAX_ENUM_KHR;
		}
		for (uint32_t i = 0; i < presentModeCount; ++i) {
			bool present_mode_found = false;
			for (const auto& available_present_mode : available_modes) {
				if (available_present_mode == presentModes[i]) {
					return available_present_mode;
				}
			}
		}
		return VK_PRESENT_MODE_MAX_ENUM_KHR;
	}
	inline VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, uint32_t candidate_count, const VkFormat* candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (uint32_t i = 0; i < candidate_count; i++) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return candidates[i];
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return candidates[i];
			}
		}
		return candidates[0];
	}
	inline std::vector<VkImage> getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain) {
		uint32_t count;
		vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
		std::vector<VkImage> images(count);
		vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());
		return images;
	}
	inline VkSwapchainKHR createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t presentQueueFamilyIndex, uint32_t graphicsQueueFamilyIndex,
		VkExtent2D extent, VkSurfaceFormatKHR targetFormat, VkPresentModeKHR presentMode, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE) {
		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.oldSwapchain = oldSwapchain;
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
		uint32_t image_count = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
			image_count = capabilities.maxImageCount;
		}
		VkSurfaceFormatKHR surface_format = pickSurfaceFormat(physicalDevice, surface, targetFormat);
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = image_count;
		createInfo.imageFormat = surface_format.format;
		createInfo.imageColorSpace = surface_format.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (presentQueueFamilyIndex != graphicsQueueFamilyIndex) {
			uint32_t queueFamilyIndices[] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateSwapchainKHR(device, &createInfo, VKL_Callbacks, &swapchain), VKL_ERROR_SWAPCHAIN_CREATION_FAILED);
		return swapchain;
	}
	inline void destroySwapchain(VkDevice device, VkSwapchainKHR swapchain) {
		vkDestroySwapchainKHR(device, swapchain, VKL_Callbacks);
	}
	inline uint32_t acquireNextImage(VkDevice device, VkSwapchainKHR swapchain, VkSemaphore imageAvailable, VkFence fence = VK_NULL_HANDLE) {
		uint32_t image_index;
		VKL_CHECK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, fence, &image_index), VKL_ERROR_IMAGE_ACQUIRING_FAILED);
		return image_index;
	}
	inline VkResult presentSwapchain(VkQueue queue, VkSwapchainKHR swapchain, uint32_t imageIndex, uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores, const void* pNext = nullptr) {
		VkResult result;
		VkPresentInfoKHR info{
			VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, pNext, waitSemaphoreCount, pWaitSemaphores, 1, &swapchain, &imageIndex, &result
		};
		vkQueuePresentKHR(queue, &info);
		return result;
	}
}