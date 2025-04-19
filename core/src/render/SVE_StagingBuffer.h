#pragma once


#include "core.h"
#include "SVE_Backend.h"


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
		assert(memory == VK_NULL_HANDLE);
		assert(buffer == VK_NULL_HANDLE);
		assert(mappedMemory == nullptr);
		assert(maxSize == 0);
		assert(offset == 0);
		maxSize = stagingSize;
		buffer = SVE::createBuffer(stagingSize * SVE::FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, SVE::getGraphicsFamily());
		memory = SVE::allocateForBuffer(buffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		mappedMemory = (uint8_t*)vkl::mapMemory(SVE::getDevice(), memory, VK_WHOLE_SIZE, 0);
		offset = 0;
	}
	inline SveStagingBuffer() {}
	inline SveStagingBuffer(VkDeviceSize stagingSize) {
		allocate(stagingSize);
	}
	inline VkDeviceSize addToTransfer(VkDeviceSize size, void* data) {
		assert(size + offset < maxSize);
		VkDeviceSize data_offset = SVE::getInFlightIndex() * maxSize + offset;
		memcpy(mappedMemory + data_offset, data, size);
		offset += size;
		return data_offset;
	}
	inline void transferToBuffer(VkCommandBuffer commands, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize dstOffset, void* data) {
		VkBufferCopy region;
		region.srcOffset = addToTransfer(size, data);
		region.size = size;
		region.dstOffset = dstOffset;
		vkCmdCopyBuffer(commands, buffer, dstBuffer, 1, &region);
	}

	inline void clear() {
		offset = 0;
	}
	inline void free() {
		SVE::destroyBuffer(buffer);
		SVE::freeMemory(memory);
		memory = VK_NULL_HANDLE;
		buffer = VK_NULL_HANDLE;
		mappedMemory = nullptr;
		maxSize = 0;
		offset = 0;
	}
	inline ~SveStagingBuffer() {
		free();
	}
};