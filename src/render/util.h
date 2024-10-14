#pragma once

#include "engine_core.h"




namespace util {
	inline VkImageView createImageView(VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t layerCount = 1, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D) {
		return vkl::createImageView(g_Device, image, viewType, format, { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY }, { imageAspect, mipLevel, levelCount, arrayLayer, layerCount });
	}
	inline VkImageView createImageView(VkImage image, VkFormat format, const VkImageSubresourceRange& subresourceRange, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D) {
		return vkl::createImageView(g_Device, image, viewType, format, { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY }, subresourceRange);
	}
	inline VkImage createImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, VkImageCreateFlags flags = VKL_FLAG_NONE) {
		return vkl::createImage(g_Device, VK_IMAGE_TYPE_2D, format, { width, height, 1 }, 1, 1, VK_SAMPLE_COUNT_1_BIT, tiling, usage, flags);
	}
}