#pragma once

#include "engine_core.h"

struct TransferData {
	VkDeviceSize Size;
	VkDeviceSize DstOffset;
	const void* Data;
};

class StagingBuffer {
public:
	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory Memory = VK_NULL_HANDLE;
	uint8_t* MappedMemory = nullptr;
	VkDeviceSize MaxTransferSize = 0;
public:
	void allocate(VkDeviceSize maxTransferSize) {
		Buffer = vkl::createBuffer(Core.LogicalDevice, maxTransferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, 1, &Core.GraphicsIndex);
		auto req = vkl::getBufferMemoryRequirements(Core.LogicalDevice, Buffer);
		req.size = RenderCore::FRAMES_IN_FLIGHT * req.size;
		Memory = vkl::allocateMemory(Core.LogicalDevice, Core.PhysicalDevice, req, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vkBindBufferMemory(Core.LogicalDevice, Buffer, Memory, 0);
		MappedMemory = (uint8_t*)vkl::mapMemory(Core.LogicalDevice, Memory, VK_WHOLE_SIZE, 0);
		MaxTransferSize = maxTransferSize;
	}
	void free() {
		vkl::freeMemory(Core.LogicalDevice, Memory);
		vkl::destroyBuffer(Core.LogicalDevice, Buffer);
		MappedMemory = nullptr;
		MaxTransferSize = 0;
		Buffer = VK_NULL_HANDLE;
		Memory = VK_NULL_HANDLE;
	}
	void prepareForTransfer(size_t size, size_t offset, const void* data) {
		assert((offset + size) <= MaxTransferSize);
		memcpy(&MappedMemory[(offset + Core.FrameIndex * MaxTransferSize)], data, size);
	}
	void transferData(VkCommandBuffer commands, VkBuffer dstBuffer, size_t size, size_t offset, const void* data, VkPipelineStageFlags pipelineStage, VkAccessFlags bufferAccess, uint32_t queueFamilyIndex = Core.GraphicsIndex) {
		VkBufferMemoryBarrier barrier = vkl::createBufferMemoryBarrier(dstBuffer, VK_WHOLE_SIZE, 0, queueFamilyIndex, queueFamilyIndex, pipelineStage, VK_ACCESS_TRANSFER_WRITE_BIT);
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VKL_FLAG_NONE, 0, nullptr, 1, &barrier, 0, nullptr);
		VkBufferCopy region = {Core.FrameIndex * MaxTransferSize, offset, size};
		assert(size <= MaxTransferSize);
		memcpy(&MappedMemory[Core.FrameIndex * MaxTransferSize], data, size);
		vkCmdCopyBuffer(commands, Buffer, dstBuffer, 1, &region);
		barrier.dstAccessMask = barrier.srcAccessMask;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, pipelineStage, VKL_FLAG_NONE, 0, nullptr, 1, &barrier, 0, nullptr);
	}
	void transferDataUnsafe(VkCommandBuffer commands, VkBuffer dstBuffer, size_t dataSize, const void* data, size_t dstOffset = 0) {
		VkBufferCopy region = {Core.FrameIndex * MaxTransferSize, dstOffset, dataSize};
		assert(dataSize <= MaxTransferSize);
		memcpy(&MappedMemory[region.srcOffset], data, dataSize);
		vkCmdCopyBuffer(commands, Buffer, dstBuffer, 1, &region);
	}
	void transferDataUnsafe(VkCommandBuffer commands, VkBuffer dstBuffer, uint32_t transferCount, const TransferData* transferRegions, VkBufferCopy* pRegions) {
		size_t offset = Core.FrameIndex * MaxTransferSize;
		for (uint32_t i = 0; i < transferCount; ++i) {
			assert(offset + transferRegions[i].Size <= MaxTransferSize);
			memcpy(&MappedMemory[offset], transferRegions[i].Data, transferRegions[i].Size);
			offset += transferRegions[i].Size;
			pRegions[i].dstOffset = transferRegions[i].DstOffset;
			pRegions[i].size = transferRegions[i].Size;
			pRegions[i].srcOffset = offset;
		}
		vkCmdCopyBuffer(commands, Buffer, dstBuffer, transferCount, pRegions);
	}
	void transferData(VkCommandBuffer commands, VkBuffer dstBuffer, uint32_t transferCount, const TransferData* transferRegions, VkBufferCopy* pRegions, VkPipelineStageFlags pipelineStage, VkAccessFlags bufferAccess, uint32_t queueFamilyIndex = Core.GraphicsIndex) { 
		VkBufferMemoryBarrier barrier = vkl::createBufferMemoryBarrier(dstBuffer, VK_WHOLE_SIZE, 0, Core.GraphicsIndex, Core.GraphicsIndex, pipelineStage, VK_ACCESS_TRANSFER_WRITE_BIT);
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VKL_FLAG_NONE, 0, nullptr, 1, &barrier, 0, nullptr);
		size_t offset = Core.FrameIndex * MaxTransferSize;
		for (uint32_t i = 0; i < transferCount; ++i) {
			assert(offset + transferRegions[i].Size <= MaxTransferSize);
			memcpy(&MappedMemory[offset], transferRegions[i].Data, transferRegions[i].Size);
			offset += transferRegions[i].Size;
			pRegions[i].dstOffset = transferRegions[i].DstOffset;
			pRegions[i].size = transferRegions[i].Size;
			pRegions[i].srcOffset = offset;
		}
		vkCmdCopyBuffer(commands, Buffer, dstBuffer, transferCount, pRegions);
		barrier.dstAccessMask = barrier.srcAccessMask;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, pipelineStage, VKL_FLAG_NONE, 0, nullptr, 1, &barrier, 0, nullptr);
	}
	void transferData(VkCommandBuffer commands, VkBuffer dstBuffer, uint32_t transferCount, const TransferData* transferRegions, VkPipelineStageFlags pipelineStage, VkAccessFlags bufferAccess, uint32_t queueFamilyIndex = Core.GraphicsIndex) {
		std::vector<VkBufferCopy> regions(transferCount);
		transferData(commands, dstBuffer, transferCount, transferRegions, regions.data(), pipelineStage, bufferAccess, queueFamilyIndex);
	}
};
