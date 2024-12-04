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
		maxSize = stagingSize;
		buffer = vkl::createBuffer(SVE::getDevice(), stagingSize * SVE::FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, SVE::getGraphicsFamily());
		memory = vkl::allocateForBuffer(SVE::getDevice(), SVE::getPhysicalDevice(), buffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		mappedMemory = (uint8_t*)vkl::mapMemory(SVE::getDevice(), memory, VK_WHOLE_SIZE, 0);
		offset = 0;
	}
	inline VkDeviceSize addToTransfer(VkDeviceSize size, void* data) {
		assert(size + offset < maxSize);
		VkDeviceSize data_offset = SVE::getInFlightIndex() * maxSize + offset;
		memcpy(mappedMemory + data_offset, data, size);
		offset += size;
		return data_offset;
	}
	inline void clear() {
		offset = 0;
	}
	inline void free() {
		vkl::destroyBuffer(SVE::getDevice(), buffer);
		vkl::freeMemory(SVE::getDevice(), memory);
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