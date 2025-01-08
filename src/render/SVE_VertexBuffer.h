#pragma once


#include "core.h"

#include "SVE_Backend.h"
#include "SVE_Model.h"


class SveVertexBuffer {
private:
	VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	uint32_t maxVertexCount = 0;
	uint32_t maxIndexCount = 0;
	VkDeviceSize indexOffset = 0;
public:
	inline void allocate(uint32_t maxVertices, uint32_t maxIndices) {
		maxVertexCount = maxVertices;
		maxIndexCount = maxIndices;
		vertexBuffer = vkl::createBuffer(SVE::getDevice(), sizeof(SveModelVertex) * maxVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, SVE::getGraphicsFamily());
		indexBuffer = vkl::createBuffer(SVE::getDevice(), sizeof(uint32_t) * maxIndices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, SVE::getGraphicsFamily());
		auto req = vkl::getBufferMemoryRequirements(SVE::getDevice(), vertexBuffer);
		auto index_req = vkl::getBufferMemoryRequirements(SVE::getDevice(), indexBuffer);
		indexOffset = req.size;
		uint32_t padding = static_cast<uint32_t>(req.alignment - (index_req.size % req.alignment));
		maxIndexCount += padding / sizeof(uint32_t);
		req.size += index_req.size + padding;
		req.memoryTypeBits |= index_req.memoryTypeBits;
		deviceMemory = vkl::allocateMemory(SVE::getDevice(), SVE::getPhysicalDevice(), req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkl::bindBufferMemory(SVE::getDevice(), vertexBuffer, deviceMemory, 0);
		vkl::bindBufferMemory(SVE::getDevice(), indexBuffer, deviceMemory, indexOffset);
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
	inline void bind(VkCommandBuffer commands) {
		vkCmdBindIndexBuffer(commands, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commands, 0, 1, &vertexBuffer, &offset);
	}
	inline void free() {
		vkl::destroyBuffer(SVE::getDevice(), vertexBuffer);
		vkl::destroyBuffer(SVE::getDevice(), indexBuffer);
		vkl::freeMemory(SVE::getDevice(), deviceMemory);
		deviceMemory = VK_NULL_HANDLE;
		vertexBuffer = VK_NULL_HANDLE;
		indexBuffer = VK_NULL_HANDLE;
		maxVertexCount = 0;
		maxIndexCount = 0;
		indexOffset = 0;
	}
};