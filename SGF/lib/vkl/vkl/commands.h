#pragma once


#include "vkl_core.h"

namespace vkl {
	inline VkCommandPoolCreateInfo createCommandPoolInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {

		return {
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, pNext, flags, queueFamilyIndex
		};
	}
	inline VkCommandPool createCommandPool(VkDevice device, const VkCommandPoolCreateInfo& info) {
		VkCommandPool pool = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateCommandPool(device, &info, VKL_Callbacks, &pool), VKL_ERROR_COMMANDPOOL_CREATION_FAILED);
		return pool;
	}
	inline VkCommandPool createCommandPool(VkDevice device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		VkCommandPoolCreateInfo info = createCommandPoolInfo(queueFamilyIndex, flags, pNext);
		VkCommandPool pool = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateCommandPool(device, &info, VKL_Callbacks, &pool), VKL_ERROR_COMMANDPOOL_CREATION_FAILED);
		return pool;
	}
	inline void destroyCommandPool(VkDevice device, VkCommandPool commandPool) {
		vkDestroyCommandPool(device, commandPool, VKL_Callbacks);
	}
	inline void resetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags = VKL_FLAG_NONE) {
		VKL_CHECK(vkResetCommandPool(device, commandPool, flags), VKL_ERROR_COMMANDPOOL_RESET_FAILED);
	}
	inline VkCommandBufferAllocateInfo createCommandBufferAllocateInfo(VkCommandPool commandPool, uint32_t commandBufferCount, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, pNext, commandPool, level, commandBufferCount
		};
	}
	inline VkResult allocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo& info, VkCommandBuffer* commandBuffers) {
		return vkAllocateCommandBuffers(device, &info, commandBuffers);
	}
	inline VkCommandBuffer createCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, const void* pNext = nullptr) {
		VkCommandBufferAllocateInfo info = createCommandBufferAllocateInfo(commandPool, 1, level, pNext);
		VkCommandBuffer command_buffer = VK_NULL_HANDLE;
		VKL_CHECK(vkAllocateCommandBuffers(device, &info, &command_buffer), VKL_ERROR_COMMANDBUFFER_CREATION_FAILED);
		return command_buffer;
	}
	inline VkCommandBufferBeginInfo createCommandBufferBeginInfo(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo = nullptr, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, pNext, flags, pInheritanceInfo
		};
	}
	inline void beginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo& info) {
		VKL_CHECK(vkBeginCommandBuffer(commandBuffer, &info), VKL_ERROR_COMMANDBUFFER_BEGIN_FAILED);
	}
	inline void beginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo = nullptr, const void* pNext = nullptr) {
		VkCommandBufferBeginInfo info = createCommandBufferBeginInfo(flags, pInheritanceInfo, pNext);
		VKL_CHECK(vkBeginCommandBuffer(commandBuffer, &info), VKL_ERROR_COMMANDBUFFER_BEGIN_FAILED);
	}
	inline void endCommandBuffer(VkCommandBuffer commandBuffer) {
		VKL_CHECK(vkEndCommandBuffer(commandBuffer), VKL_ERROR_COMMANDBUFFER_END_FAILED);
	}
	inline VkCommandBufferInheritanceInfo createCommandBufferInheritanceInfo(VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE, VkBool32 occlusionQueryEnable = VK_FALSE, VkQueryControlFlags queryFlags = VKL_FLAG_NONE, VkQueryPipelineStatisticFlags pipelineStatistics = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, pNext, renderPass, subpass, framebuffer, occlusionQueryEnable, queryFlags, pipelineStatistics
		};
	}
	inline VkSubmitInfo createSubmitInfo(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers,
		uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores, const VkPipelineStageFlags* pWaitDstStageMask,
		uint32_t signalSemaphoreCount, const VkSemaphore* pSignalSemaphores, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			pNext,
			waitSemaphoreCount,
			pWaitSemaphores,
			pWaitDstStageMask,
			commandBufferCount,
			pCommandBuffers,
			signalSemaphoreCount,
			pSignalSemaphores
		};
	}
	inline void submitCommands(VkQueue queue, VkCommandBuffer commands, VkFence fence) {
		VkSubmitInfo info = createSubmitInfo(1, &commands, 0, nullptr, nullptr, 0, nullptr);
		VKL_CHECK(vkQueueSubmit(queue, 1, &info, fence), VKL_ERROR_QUEUE_SUBMIT_FAILED);
	}
	inline void submitCommands(VkQueue queue, VkCommandBuffer commands, VkSemaphore waitSemaphore, VkPipelineStageFlags waitStage, VkSemaphore signalSemaphore, VkFence fence) {
		VkSubmitInfo info = createSubmitInfo(1, &commands, 1, &waitSemaphore, &waitStage, 1, &signalSemaphore);
		VKL_CHECK(vkQueueSubmit(queue, 1, &info, fence), VKL_ERROR_QUEUE_SUBMIT_FAILED);
	}
	inline void submitCommands(VkQueue queue, VkCommandBuffer commands, uint32_t waitCount, const VkSemaphore* pWaitSemaphore, const VkPipelineStageFlags* pWaitStage,
		uint32_t signalCount, const VkSemaphore* pSignalSemaphore, VkFence fence) {
		VkSubmitInfo info = createSubmitInfo(1, &commands, waitCount, pWaitSemaphore, pWaitStage, signalCount, pSignalSemaphore);
		VKL_CHECK(vkQueueSubmit(queue, 1, &info, fence), VKL_ERROR_QUEUE_SUBMIT_FAILED);
	}
	inline void submitCommands(VkQueue queue, uint32_t commandBufferCount, const VkCommandBuffer* pCommands, uint32_t waitCount, const VkSemaphore* pWaitSemaphore, const VkPipelineStageFlags* pWaitStage,
		uint32_t signalCount, const VkSemaphore* pSignalSemaphore, VkFence fence) {
		VkSubmitInfo info = createSubmitInfo(commandBufferCount, pCommands, waitCount, pWaitSemaphore, pWaitStage, signalCount, pSignalSemaphore);
		VKL_CHECK(vkQueueSubmit(queue, 1, &info, fence), VKL_ERROR_QUEUE_SUBMIT_FAILED);
	}
	inline void submitCommands(VkQueue queue, const VkSubmitInfo& submitInfo, VkFence fence) {
		VKL_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence), VKL_ERROR_QUEUE_SUBMIT_FAILED);
	}
	inline void submitCommands(VkQueue queue, uint32_t commandBufferCount, const VkCommandBuffer* pCommands, VkFence fence) {
		VkSubmitInfo info = createSubmitInfo(commandBufferCount, pCommands, 0, nullptr, nullptr, 0, nullptr);
		VKL_CHECK(vkQueueSubmit(queue, 1, &info, fence), VKL_ERROR_QUEUE_SUBMIT_FAILED);
	}
}