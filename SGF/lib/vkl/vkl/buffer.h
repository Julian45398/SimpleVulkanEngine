#pragma once

#include "vkl_core.h"

namespace vkl {
	inline VkBufferCreateInfo createBufferInfo(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* pQueueFamilyIndices, VkBufferCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, pNext, flags, size, usage, sharingMode, queueFamilyIndexCount, pQueueFamilyIndices 
		};
	}
	inline VkBuffer createBuffer(VkDevice device, const VkBufferCreateInfo& createInfo) {
		VkBuffer buffer = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateBuffer(device, &createInfo, VKL_Callbacks, &buffer), VKL_ERROR_BUFFER_CREATION_FAILED);
		return buffer;
	}
	inline VkBuffer createBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, const uint32_t* pQueueFamilyIndices, VkBufferCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		VkBufferCreateInfo info = createBufferInfo(size, usage, sharingMode, queueFamilyIndexCount, pQueueFamilyIndices, flags, pNext);
		VkBuffer buffer = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateBuffer(device, &info, VKL_Callbacks, &buffer), VKL_ERROR_BUFFER_CREATION_FAILED);
		return buffer;
	}
	inline VkBuffer createBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, uint32_t queueFamilyIndex, VkBufferCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		VkBufferCreateInfo info = createBufferInfo(size, usage, VK_SHARING_MODE_EXCLUSIVE, 1, &queueFamilyIndex, flags, pNext);
		VkBuffer buffer = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateBuffer(device, &info, VKL_Callbacks, &buffer), VKL_ERROR_BUFFER_CREATION_FAILED);
		return buffer;
	}
	inline void destroyBuffer(VkDevice device, VkBuffer buffer) {
		vkDestroyBuffer(device, buffer, VKL_Callbacks);
	}
	inline VkBufferMemoryBarrier createBufferMemoryBarrier(VkBuffer buffer, VkDeviceSize size, VkDeviceSize offset, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, pNext, srcAccessMask, dstAccessMask, srcQueueFamilyIndex, dstQueueFamilyIndex, buffer, offset, size
		};
	}
}