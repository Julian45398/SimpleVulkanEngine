#pragma once

#include "engine_core.h"

struct ModelVertex {
	alignas(16) glm::vec3 Position;
	uint32_t ImageIndex;
	glm::vec2 UVCoord;
	uint32_t AnimationIndex;
	float Intensity;
};

class Model {
public:
	std::vector<ModelVertex> Vertices;
	std::vector<uint32_t> Indices;
	std::vector<uint8_t> PixelData;
	uint32_t ImageHeight;
	uint32_t ImageWidth;
public:
	Model(const char* filename);
private:



};

class ModelVertexBuffer {
private:
	static const uint32_t MAX_CHANGES = 10;
	VkDeviceMemory Memory = VK_NULL_HANDLE;
	VkDeviceMemory StagingMemory = VK_NULL_HANDLE;
	VkBuffer VertexBuffer = VK_NULL_HANDLE;
	VkBuffer IndexBuffer = VK_NULL_HANDLE;
	VkBuffer StagingBuffer = VK_NULL_HANDLE;
	uint8_t* MappedMemory = nullptr;
	uint32_t MaxVertexCount = 0;
	uint32_t MaxIndexCount = 0;
	size_t MaxTransferSize = 0;
	size_t TransferOffset = 0;
	VkBufferCopy IndexRegions[MAX_CHANGES] = {};
	VkBufferCopy VertexRegions[MAX_CHANGES] = {};
	uint32_t VertexRegionCount = 0;
	uint32_t IndexRegionCount = 0;
	uint32_t IndexCount = 0;
#ifndef NDEBUG
	uint32_t FrameUploadedIndex = 0;
#endif
	//std::vector<ModelVertex> Vertices;
	//std::vector<uint32_t> Indices;
	//struct ChangeRegion {
		//uint32_t Size = 0;
		//uint32_t Offset = 0;
	//};
public:
	void bindAndDraw(VkCommandBuffer commands) {
#ifndef NDEBUG
		FrameUploadedIndex = 0;
#endif
		assert(IndexRegionCount == 0);
		assert(VertexRegionCount == 0);
		assert(IndexCount % 3 == 0);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commands, 0, 1, &VertexBuffer, &offset);
		vkCmdBindIndexBuffer(commands, IndexBuffer, offset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commands, IndexCount, 1, 0, 0, 0);
	}
	void allocate(uint32_t maxVertexCount, uint32_t maxIndexCount, size_t maxTransferSize) {
		MaxVertexCount = maxVertexCount;
		MaxIndexCount = maxIndexCount;
		VertexBuffer = vkl::createBuffer(Core.LogicalDevice, sizeof(ModelVertex) * MaxVertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Core.GraphicsIndex);
		IndexBuffer = vkl::createBuffer(Core.LogicalDevice, sizeof(uint32_t) * MaxIndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Core.GraphicsIndex);
		StagingBuffer = vkl::createBuffer(Core.LogicalDevice, maxTransferSize * RenderCore::FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Core.GraphicsIndex);
		StagingMemory = vkl::allocateForBuffer(Core.LogicalDevice, Core.PhysicalDevice, StagingBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		vkBindBufferMemory(Core.LogicalDevice, StagingBuffer, StagingMemory, 0);
		MappedMemory = (uint8_t*)vkl::mapMemory(Core.LogicalDevice, StagingMemory, VK_WHOLE_SIZE, 0);
		VkBuffer buffers[] = {
			VertexBuffer, IndexBuffer
		};
		Memory = vkl::allocateAndBind(Core.LogicalDevice, Core.PhysicalDevice, ARRAY_SIZE(buffers), buffers, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		MaxTransferSize = maxTransferSize;
	}
	void stageChanges(uint32_t indexCount, uint32_t indexOffset, const uint32_t* indices) {
		shl::logInfo("preparing index changes: ", indexCount, " with offset: ", indexOffset);
#ifndef NDEBUG
		assert(FrameUploadedIndex == 0);
#endif 
		assert(indexCount + indexOffset < MaxIndexCount);
		assert(indexOffset <= IndexCount);
		{
			uint32_t count = indexOffset + indexCount;
			if (IndexCount < count) {
				IndexCount = count;
			}
		}
		assert(TransferOffset + indexCount * sizeof(uint32_t) < MaxTransferSize);
		assert(IndexRegionCount < MAX_CHANGES);
		//size_t copy_size = vertexCount * sizeof(ModelVertex);
		auto& region = IndexRegions[IndexRegionCount];
		region.size = indexCount * sizeof(uint32_t);
		region.dstOffset = indexOffset * sizeof(uint32_t);
		region.srcOffset = TransferOffset + MaxTransferSize * Core.FrameIndex;
		memcpy(MappedMemory + region.srcOffset, indices, region.size);
		TransferOffset += region.size;
		IndexRegionCount++;
	}
	void stageChanges(uint32_t vertexCount, uint32_t vertexOffset, const ModelVertex* vertices) {
		shl::logInfo("preparing vertex changes: ", vertexCount, " with offset: ", vertexOffset);
#ifndef NDEBUG
		assert(FrameUploadedIndex == 0);
#endif 
		assert(TransferOffset + vertexCount * sizeof(ModelVertex) < MaxTransferSize);
		assert(vertexCount + vertexOffset < MaxVertexCount);
		assert(VertexRegionCount < MAX_CHANGES);
		//size_t copy_size = vertexCount * sizeof(ModelVertex);
		auto& region = VertexRegions[VertexRegionCount];
		region.size = vertexCount * sizeof(ModelVertex);
		region.dstOffset = vertexOffset * sizeof(ModelVertex);
		region.srcOffset = TransferOffset + MaxTransferSize * Core.FrameIndex;
		memcpy(MappedMemory + region.srcOffset, vertices, region.size);
		TransferOffset += region.size;
		VertexRegionCount++;
	}
	void stageChanges(uint32_t vertexCount, uint32_t vertexOffset, const ModelVertex* vertices, uint32_t indexCount, uint32_t indexOffset, const uint32_t* indices) {
		stageChanges(vertexCount, vertexOffset, vertices);
		stageChanges(indexCount, indexOffset, indices);
	}
	void upload(VkCommandBuffer commands) {
		assert(VertexRegionCount != 0 && IndexRegionCount != 0);
		shl::logInfo("uploading ", IndexRegionCount, " index and ", VertexRegionCount, " vertex changes");
		VkBufferMemoryBarrier barriers[] = {
			vkl::createBufferMemoryBarrier(VertexBuffer, VK_WHOLE_SIZE, 0, Core.GraphicsIndex, Core.GraphicsIndex, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT),
			vkl::createBufferMemoryBarrier(IndexBuffer, VK_WHOLE_SIZE, 0, Core.GraphicsIndex, Core.GraphicsIndex, VK_ACCESS_INDEX_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT)
		};
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VKL_FLAG_NONE, 0, nullptr, ARRAY_SIZE(barriers), barriers, 0, nullptr);

		if (VertexRegionCount != 0) {
			vkCmdCopyBuffer(commands, StagingBuffer, VertexBuffer, VertexRegionCount, VertexRegions);
		}
		if (IndexRegionCount != 0) {
			vkCmdCopyBuffer(commands, StagingBuffer, IndexBuffer, IndexRegionCount, IndexRegions);
		}

		barriers[0].dstAccessMask = barriers[0].srcAccessMask;
		barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barriers[1].dstAccessMask = barriers[1].srcAccessMask;
		barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VKL_FLAG_NONE, 0, nullptr, ARRAY_SIZE(barriers), barriers, 0, nullptr);
		VertexRegionCount = 0;
		IndexRegionCount = 0;

#ifndef NDEBUG
		FrameUploadedIndex = UINT32_MAX;
#endif 
	}

	void free() {
		vkl::destroyBuffer(Core.LogicalDevice, IndexBuffer);
		vkl::destroyBuffer(Core.LogicalDevice, VertexBuffer);
		vkl::freeMemory(Core.LogicalDevice, Memory);
		vkl::destroyBuffer(Core.LogicalDevice, StagingBuffer);
		vkl::freeMemory(Core.LogicalDevice, StagingMemory);
		Memory = VK_NULL_HANDLE;
		VertexBuffer = VK_NULL_HANDLE;
		IndexBuffer = VK_NULL_HANDLE;
		StagingBuffer = VK_NULL_HANDLE;
		StagingMemory = VK_NULL_HANDLE;
		MappedMemory = nullptr;
		MaxVertexCount = 0;
		MaxIndexCount = 0;
		TransferOffset = 0;
		IndexCount = 0;
		VertexRegionCount = 0;
		IndexRegionCount = 0;
		for (uint32_t i = 0; i < MAX_CHANGES; ++i) {
			IndexRegions[i].dstOffset = 0;
			IndexRegions[i].size = 0;
			IndexRegions[i].srcOffset = 0;
			VertexRegions[i].dstOffset = 0;
			VertexRegions[i].size = 0;
			VertexRegions[i].srcOffset = 0;
		}
#ifndef NDEBUG
		FrameUploadedIndex = 0;
#endif
	}
};