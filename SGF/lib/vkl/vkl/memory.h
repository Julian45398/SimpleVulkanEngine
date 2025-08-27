#pragma once

#include "vkl_core.h"

namespace vkl {
	inline VkMemoryRequirements getBufferMemoryRequirements(VkDevice device, VkBuffer buffer) {
		assert(device != VK_NULL_HANDLE);
		assert(buffer != VK_NULL_HANDLE);
		VkMemoryRequirements req;
		vkGetBufferMemoryRequirements(device, buffer, &req);
		return req;
	}
	inline VkMemoryRequirements getImageMemoryRequirements(VkDevice device, VkImage image) {
		assert(device != VK_NULL_HANDLE);
		assert(image != VK_NULL_HANDLE);
		VkMemoryRequirements req;
		vkGetImageMemoryRequirements(device, image, &req);
		return req;
	}
	inline VkPhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice) {
		assert(physicalDevice != VK_NULL_HANDLE);
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &mem_properties);
		return mem_properties;
	}
	inline uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		assert(physicalDevice != VK_NULL_HANDLE);
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &mem_properties);

		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		return UINT32_MAX;
	}
	inline VkMemoryAllocateInfo createMemoryAllocateInfo(VkDeviceSize allocationSize, uint32_t memoryTypeIndex, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, pNext, allocationSize, memoryTypeIndex
		};
	}
	inline VkDeviceMemory allocateMemory(VkDevice device, const VkMemoryAllocateInfo& info) {
		VkDeviceMemory memory;
		VKL_CHECK(vkAllocateMemory(device, &info, VKL_Callbacks, &memory), VKL_ERROR_MEMORY_ALLOCATION_FAILED);
		return memory;
	}
	inline VkDeviceMemory allocateMemory(VkDevice device, VkDeviceSize allocationSize, uint32_t memoryTypeIndex, const void* pNext = nullptr) {
		VkMemoryAllocateInfo info = createMemoryAllocateInfo(allocationSize, memoryTypeIndex, pNext);
		VkDeviceMemory memory;
		VKL_CHECK(vkAllocateMemory(device, &info, VKL_Callbacks, &memory), VKL_ERROR_MEMORY_ALLOCATION_FAILED);
		return memory;
	}
	inline VkDeviceMemory allocateMemory(VkDevice device, VkPhysicalDevice physicalDevice, const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags properties) {
		return allocateMemory(device, memoryRequirements.size, findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties));
	}
	inline void bindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset = 0) {
		VKL_CHECK(vkBindBufferMemory(device, buffer, memory, offset), VKL_ERROR_BIND_BUFFER_FAILED);
	}
	inline void bindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize offset = 0) {
		VKL_CHECK(vkBindImageMemory(device, image, memory, offset), VKL_ERROR_BIND_IMAGE_FAILED);
	}
	inline VkDeviceMemory allocateForBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBuffer buffer, VkMemoryPropertyFlags properties) {
		VkDeviceMemory mem = allocateMemory(device, physicalDevice, vkl::getBufferMemoryRequirements(device, buffer), properties);
		bindBufferMemory(device, buffer, mem);
		return mem;
	}
	inline VkDeviceMemory allocateForImage(VkDevice device, VkPhysicalDevice physicalDevice, VkImage image, VkMemoryPropertyFlags properties) {
		VkDeviceMemory mem = allocateMemory(device, physicalDevice, getImageMemoryRequirements(device, image), properties);
		bindImageMemory(device, image, mem, 0);
		return mem;
	}
	inline VkDeviceMemory allocateAndBind(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t imageCount, const VkImage* images, VkMemoryPropertyFlags properties) {
		VkMemoryRequirements mem_req{};
		std::vector<VkDeviceSize> offsets(imageCount);
		for (uint32_t i = 0; i < imageCount; ++i) {
			VkMemoryRequirements req = getImageMemoryRequirements(device, images[i]);
			if (mem_req.alignment < req.alignment) {
				mem_req.alignment = req.alignment;
			} 
			VkDeviceSize t = mem_req.size % req.alignment;
			if (t != 0) {
				offsets[i] = mem_req.size + req.alignment - t;
			}
			else {
				offsets[i] = mem_req.size;
			}
			mem_req.size = offsets[i] + req.size;
			mem_req.memoryTypeBits |= req.memoryTypeBits;
		}
		VkDeviceSize t = mem_req.size % mem_req.alignment;
		if (t != 0) {
			mem_req.size += mem_req.alignment - t;
		}
		VkDeviceMemory memory = allocateMemory(device, physicalDevice, mem_req, properties);
		for (uint32_t i = 0; i < imageCount; ++i) {
			vkBindImageMemory(device, images[i], memory, offsets[i]);
		}
		return memory;
	}
	inline VkDeviceMemory allocateAndBind(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t bufferCount, const VkBuffer* buffers, VkMemoryPropertyFlags properties) {
		VkMemoryRequirements mem_req{};
		std::vector<VkDeviceSize> offsets(bufferCount);
		for (uint32_t i = 0; i < bufferCount; ++i) {
			VkMemoryRequirements req = getBufferMemoryRequirements(device, buffers[i]);
			if (mem_req.alignment < req.alignment) {
				mem_req.alignment = req.alignment;
			} 
			VkDeviceSize t = mem_req.size % req.alignment;
			if (t != 0) {
				offsets[i] = mem_req.size + req.alignment - t;
			}
			else {
				offsets[i] = mem_req.size;
			}
			mem_req.size = offsets[i] + req.size;
			mem_req.memoryTypeBits |= req.memoryTypeBits;
		}
		VkDeviceSize t = mem_req.size % mem_req.alignment;
		if (t != 0) {
			mem_req.size += mem_req.alignment - t;
		}
		VkDeviceMemory memory = allocateMemory(device, physicalDevice, mem_req, properties);
		for (uint32_t i = 0; i < bufferCount; ++i) {
			vkBindBufferMemory(device, buffers[i], memory, offsets[i]);
		}
		return memory;
	}
	inline VkDeviceMemory allocateAndBind(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t bufferCount, const VkBuffer* buffers, uint32_t imageCount, const VkImage* images, VkMemoryPropertyFlags properties) {
		VkMemoryRequirements mem_req{};
		std::vector<VkDeviceSize> offsets(bufferCount + imageCount);
		for (uint32_t i = 0; i < imageCount; ++i) {
			VkMemoryRequirements req = getImageMemoryRequirements(device, images[i]);
			if (mem_req.alignment < req.alignment) {
				mem_req.alignment = req.alignment;
			} 
			VkDeviceSize t = mem_req.size % req.alignment;
			if (t != 0) {
				offsets[i] = mem_req.size + req.alignment - t;
			}
			else {
				offsets[i] = mem_req.size;
			}
			mem_req.size = offsets[i] + req.size;
			mem_req.memoryTypeBits |= req.memoryTypeBits;
		}
		for (uint32_t i = 0; i < bufferCount; ++i) {
			VkMemoryRequirements req = getBufferMemoryRequirements(device, buffers[i]);
			if (mem_req.alignment < req.alignment) {
				mem_req.alignment = req.alignment;
			} 
			VkDeviceSize t = mem_req.size % req.alignment;
			if (t != 0) {
				offsets[imageCount + i] = mem_req.size + req.alignment - t;
			}
			else {
				offsets[imageCount + i] = mem_req.size;
			}
			mem_req.size = offsets[imageCount + i] + req.size;
			mem_req.memoryTypeBits |= req.memoryTypeBits;
		}
		VkDeviceSize t = mem_req.size % mem_req.alignment;
		if (t != 0) {
			mem_req.size += mem_req.alignment - t;
		}
		VkDeviceMemory memory = allocateMemory(device, physicalDevice, mem_req, properties);
		for (uint32_t i = 0; i < imageCount; ++i) {
			vkBindImageMemory(device, images[i], memory, offsets[i]);
		}
		for (uint32_t i = 0; i < bufferCount; ++i) {
			vkBindBufferMemory(device, buffers[i], memory, offsets[i + imageCount]);
		}
		return memory;
	}
	inline void freeMemory(VkDevice device, VkDeviceMemory memory) {
		vkFreeMemory(device, memory, VKL_Callbacks);
	}
	inline void* mapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize size, VkDeviceSize offset, VkMemoryMapFlags flags = VKL_FLAG_NONE) {
		void* mapped_memory;
		VKL_CHECK(vkMapMemory(device, memory, offset, size, flags, &mapped_memory), VKL_ERROR_MEMORY_MAPPING_FAILED);
		return mapped_memory;
	}
}