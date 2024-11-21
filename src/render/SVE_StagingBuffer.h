#pragma once


#include "engine_core.h"


class SveStagingBuffer {
private:
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkBuffer buffer = VK_NULL_HANDLE;
	uint8_t* mappedMemory = nullptr;
	VkDeviceSize maxSize = 0;
	VkDeviceSize offset = 0;
public:
	inline operator VkBuffer() {
		return buffer;
	}
	inline void allocate(VkDeviceSize stagingSize) {
		maxSize = stagingSize;
		buffer = vkl::createBuffer(Core.LogicalDevice, stagingSize * RenderCore::FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Core.GraphicsIndex);
		memory = vkl::allocateForBuffer(Core.LogicalDevice, Core.PhysicalDevice, buffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		vkBindBufferMemory(Core.LogicalDevice, buffer, memory, 0);
		mappedMemory = (uint8_t*)vkl::mapMemory(Core.LogicalDevice, memory, VK_WHOLE_SIZE, 0);
		offset = 0;
	}
	inline VkDeviceSize addToTransfer(VkDeviceSize size, void* data) {
		assert(size + offset < maxSize);
		VkDeviceSize data_offset = Core.FrameIndex * maxSize + offset;
		memcpy(mappedMemory + data_offset, data, size);
		offset += size;
		return data_offset;
	}
	inline void clear() {
		offset = 0;
	}
	inline void free() {
		vkl::destroyBuffer(Core.LogicalDevice, buffer);
		vkl::freeMemory(Core.LogicalDevice, memory);
		memory = VK_NULL_HANDLE;
		buffer = VK_NULL_HANDLE;
		mappedMemory = nullptr;
		maxSize = 0;
		offset = 0;
	}
};