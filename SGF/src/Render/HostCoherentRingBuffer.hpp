#pragma once

#include "SGF_Core.hpp"
#include "Device.hpp"

namespace SGF {
	template<size_t PAGE_COUNT>
	class HostCoherentRingBuffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
		void* mappedMemory;
		size_t pageSize;
		size_t currentIndex;
	public:
		inline HostCoherentRingBuffer(size_t size, VkBufferUsageFlags usage) {
			buffer = Device::Get().CreateBuffer(size * PAGE_COUNT, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			memory = Device::Get().AllocateMemory(buffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			mappedMemory = Device::Get().MapMemory(memory);
			pageSize = size;
			currentIndex = 0;
		}
		inline ~HostCoherentRingBuffer() {
			Device::Get().Destroy(buffer, memory);
		}
		inline void Write(const void* data, size_t dataSize = pageSize, size_t offset = 0) {
			assert(dataSize <= pageSize);
			offset = offset + currentIndex * pageSize;
			memcpy((char*)mappedMemory + offset, data, dataSize);
		}
		//inline void Write(const void* data) { Write(data, pageSize, 0); }
		inline void SetPageIndex(size_t index) { currentIndex = index % PAGE_COUNT; }
		inline void Resize(size_t allocSize, size_t alignment, VkBufferUsageFlags usage) {
			Device::Get().Destroy(buffer, memory);
			buffer = Device::Get().CreateBuffer(allocSize * PAGE_COUNT, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			memory = Device::Get().AllocateMemory(buffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			mappedMemory = Device::Get().MapMemory(memory);
			pageSize = allocSize;
		}
		inline void BindCurrentAsVertexBuffer(VkCommandBuffer commandBuffer, uint32_t binding) const { VkDeviceSize offset = currentIndex * pageSize; vkCmdBindVertexBuffers(commandBuffer, binding, 1, &buffer, &offset); }
		inline void BindCurrentAsIndexBuffer(VkCommandBuffer commandBuffer, VkIndexType indexType) const { vkCmdBindIndexBuffer(commandBuffer, buffer, currentIndex * pageSize, indexType); }
		inline void* GetCurrentPagePointer() const { return (char*)mappedMemory + currentIndex * pageSize; }
		inline void* GetPagePointer(size_t pageIndex) const {
			assert(pageIndex < PAGE_COUNT);
			return (char*)mappedMemory + pageIndex * pageSize;
		}
		inline void* GetMappedMemory() const { return mappedMemory; }
		inline VkDeviceMemory GetMemory() const { return memory; }
		inline VkBuffer GetBuffer() const { return buffer; }
		inline size_t GetCurrentBufferOffset() const { return currentIndex * pageSize; }
		inline size_t GetCurrentPageIndex() const { return currentIndex; }
		inline size_t GetPageSize() const { return pageSize; }
		inline void NextPage() { currentIndex = (currentIndex + 1) % PAGE_COUNT; }
	};
}