#pragma once
/*

#include "core.h"

#include "SVE_Backend.h"

class SveDeviceMemory {
private:
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize memoryCapacity = 0;

public:
	inline SveDeviceMemory(const VkBuffer* pBuffers, uint32_t bufferCount, const VkImage* pImages, uint32_t imageCount) {
	}
	inline SveDeviceMemory(const VkMemoryRequirements& memReq) {
		memory = SVE::allocateMemory(memReq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		memoryCapacity = memReq.size;
	}
	inline ~SveDeviceMemory() { SVE::freeMemory(memory); }
	inline operator VkDeviceMemory() { return memory; }
};

template<typename T>
class SveVertexBuffer {
private:
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceSize memorySize = VK_NULL_HANDLE;
	uint32_t maxVertexCapacity = 0;
	uint32_t maxIndexCapacity = 0;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
public:
	inline operator VkBuffer() { return vertexBuffer; }
	inline VkDeviceSize getMemorySize() const { return memorySize; }
	inline uint32_t getVertexCount() const { return vertexCount; }
	inline uint32_t getIndexCount() const { return indexCount; }
	inline void allocate(uint32_t maxVertices, uint32_t maxIndices) {
		assert(vertexBuffer == VK_NULL_HANDLE);
		assert(memorySize == 0);
		assert(maxVertexCapacity == 0);
		assert(maxIndexCapacity == 0);
		assert(vertexCount == 0);
		assert(indexCount == 0);
		maxVertexCount = maxVertexCapacity;
		maxIndexCount = maxIndexCapacity;
		memorySize = sizeof(T) * maxVertices + sizeof(uint32_t) * maxIndices;
		vertexBuffer = SVE::createBuffer(memorySize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, SVE::getGraphicsFamily());
		auto req = SVE::getBufferMemoryRequirements(vertexBuffer);
		uint32_t padding = req - memorySize;
		maxIndexCount += padding / sizeof(uint32_t);
		memorySize = req.size;
		//deviceMemory = SVE::allocateMemory(req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}
	inline VkMemoryRequirements getMemoryRequirements() const {
		return SVE::getBufferMemoryRequirements(vertexBuffer);
	}
	inline void bindMemory(const VkDeviceMemory memory, VkDeviceSize memoryOffset) {
		SVE::bindBufferMemory(vertexBuffer, memory, memoryOffset);
		SVE::bindBufferMemory(indexBuffer, memory, sizeof(T) * maxVertexCapacity + memoryOffset);
	}
	inline SveVertexBuffer(uint32_t maxVertices, uint32_t maxIndices, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
		allocate(maxVertices, maxIndices);
	}
	
	inline void uploadVertexData(VkCommandBuffer commands, VkBuffer srcBuffer, uint32_t vertexRegionCount, const VkBufferCopy* pVertexRegions, uint32_t indexRegionCount, const VkBufferCopy* pIndexRegions) {
		assert(vertexRegionCount != 0 || indexRegionCount != 0);
		VkBufferMemoryBarrier barriers[] = {
			vkl::createBufferMemoryBarrier(vertexBuffer, VK_WHOLE_SIZE, 0, SVE::getGraphicsFamily(), SVE::getGraphicsFamily(), VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT),
			vkl::createBufferMemoryBarrier(indexBuffer, VK_WHOLE_SIZE, 0, SVE::getGraphicsFamily(), SVE::getGraphicsFamily(), VK_ACCESS_INDEX_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT)
		};
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VKL_FLAG_NONE, 0, nullptr, ARRAY_SIZE(barriers), barriers, 0, nullptr);

		if (vertexRegionCount != 0) {
			vkCmdCopyBuffer(commands, srcBuffer, vertexBuffer, vertexRegionCount, pVertexRegions);
		}
		if (indexRegionCount != 0) {
			vkCmdCopyBuffer(commands, srcBuffer, indexBuffer, indexRegionCount, pIndexRegions);
		}

		barriers[0].dstAccessMask = barriers[0].srcAccessMask;
		barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barriers[1].dstAccessMask = barriers[1].srcAccessMask;
		barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VKL_FLAG_NONE, 0, nullptr, ARRAY_SIZE(barriers), barriers, 0, nullptr);
	}
	inline void lazyUploadVertexData(uint32_t vertexCount, uint32_t vertexOffset, const T* vertexData, uint32_t indexCount, uint32_t indexOffset, const uint32_t* indexData) {
		VkCommandPool command_pool = vkl::createCommandPool(SVE::getDevice(), SVE::getGraphicsFamily(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		VkCommandBuffer commands = vkl::createCommandBuffer(SVE::getDevice(), command_pool);
		vkl::beginCommandBuffer(commands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VkDeviceSize total_size = vertexCount * sizeof(T) + indexCount * sizeof(uint32_t);
		VkBuffer staging_buffer = SVE::createBuffer(total_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		shl::logFatal("TODO: finish function lazyUploadVertexData for vertex buffer!");
		//uploadVertexData(commands, )
	}
	inline void bind(VkCommandBuffer commands) {
		vkCmdBindIndexBuffer(commands, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commands, 0, 1, &vertexBuffer, &offset);
	}
	inline void free() {
		vkl::destroyBuffer(SVE::getDevice(), vertexBuffer);
		//vkl::destroyBuffer(SVE::getDevice(), indexBuffer);
		//vkl::freeMemory(SVE::getDevice(), deviceMemory);
		deviceMemory = VK_NULL_HANDLE;
		vertexBuffer = VK_NULL_HANDLE;
		indexBuffer = VK_NULL_HANDLE;
		maxVertexCapacity = 0;
		maxIndexCapacity = 0;
		//indexOffset = 0;
	}
};
*/