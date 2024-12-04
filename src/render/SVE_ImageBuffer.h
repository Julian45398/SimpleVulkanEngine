#pragma once

#include "SVE_Backend.h"


class SveImageBuffer {
private:
	VkDeviceMemory memory;
	VkImage imageHandle;
	VkImageView imageView;
	VkSampler sampler;
	uint32_t arraySize;
	uint32_t mipLevels;
	uint32_t width;
	uint32_t height;
	VkImageLayout layout;
	VkFormat format;
public:
	inline operator VkImage() { return imageHandle; }
	inline operator VkImageView() { return imageView; }
	inline uint32_t getWidth() { return width; }
	inline uint32_t getHeight() { return height; }
	inline uint32_t getArraySize() { return arraySize; }
	inline uint32_t getMipLevelCount() { return mipLevels; }
	inline VkImage getImage() { return imageHandle; }
	inline VkImageView getImageView() { return imageView; }
	inline VkSampler getSampler() { return sampler; }

	inline VkDescriptorImageInfo createImageInfo() {
		return { sampler, imageView, layout };
	}
	
	inline void allocate(uint32_t imageWidth, uint32_t imageHeight, uint32_t imageCount = 1, uint32_t mipLevelCount = 1, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB) {
		imageHandle = vkl::createImage(SVE::getDevice(), VK_IMAGE_TYPE_2D, imageFormat, { imageWidth, imageHeight, 1 }, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, mipLevelCount, imageCount, VK_SAMPLE_COUNT_1_BIT);
		memory = vkl::allocateForImage(SVE::getDevice(), SVE::getPhysicalDevice(), imageHandle, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		imageView = vkl::createImageView(SVE::getDevice(), imageHandle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0, imageCount, (imageCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY);
		sampler = vkl::createSampler(SVE::getDevice(), VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		arraySize = imageCount;
		mipLevels = mipLevelCount;
		width = imageWidth;
		height = imageHeight;
		layout = VK_IMAGE_LAYOUT_UNDEFINED;
		format = imageFormat;
	}
	inline void changeLayout(VkCommandBuffer commands, VkImageLayout newLayout) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = layout;
		barrier.newLayout = newLayout;
		barrier.image = imageHandle;
		barrier.srcQueueFamilyIndex = SVE::getGraphicsFamily();
		barrier.dstQueueFamilyIndex = SVE::getGraphicsFamily();
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, arraySize };
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
		layout = newLayout;
	}
	inline void uploadChanges(VkCommandBuffer commands, VkBuffer srcBuffer, uint32_t changeCount, const VkBufferImageCopy* pChanges, VkPipelineStageFlags pipelineStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkAccessFlags imageAccess = VK_ACCESS_SHADER_READ_BIT, VkImageLayout newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		shl::logDebug("uploaded image changes");
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = layout;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.image = imageHandle;
		barrier.srcQueueFamilyIndex = SVE::getGraphicsFamily();
		barrier.dstQueueFamilyIndex = SVE::getGraphicsFamily();
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, arraySize };
		vkCmdPipelineBarrier(commands, pipelineStage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
		vkCmdCopyBufferToImage(commands, srcBuffer, imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, changeCount, pChanges);
		barrier.srcAccessMask = barrier.dstAccessMask;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = barrier.newLayout;
		barrier.newLayout = newLayout;
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, pipelineStage, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
		layout = newLayout;
	}
	inline void free() {
		vkl::destroyImage(SVE::getDevice(), imageHandle);
		vkl::destroyImageView(SVE::getDevice(), imageView);
		vkl::freeMemory(SVE::getDevice(), memory);
		vkl::destroySampler(SVE::getDevice(), sampler);
		arraySize = 0;
		mipLevels = 0;
		width = 0;
		height = 0;
		layout = VK_IMAGE_LAYOUT_MAX_ENUM;
		format = VK_FORMAT_MAX_ENUM;
	}
	inline SveImageBuffer() {
	}
	inline SveImageBuffer(uint32_t imageWidth, uint32_t imageHeight, uint32_t imageCount = 1, uint32_t mipLevelCount = 1, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB) {
		allocate(imageWidth, imageHeight, imageCount, mipLevelCount, imageFormat);
	}
	inline ~SveImageBuffer() {
		free();
	}
};