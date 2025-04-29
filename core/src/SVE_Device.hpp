#pragma once

#include "SVE_Core.hpp"

#define SVE_CHECK_VK(X, Y) do {if (X != VK_SUCCESS) {SVE::logFatal(Y);}} while(0)

#include "SVE_Errors.hpp"

const char ERR_WAIT_FENCE[] = "failed to wait for fences!";
const char ERR_CREATE_SEMAPHORE[] = "failed to create semaphores!";
const char ERR_CREATE_FENCE[] = "failed to create fences!";
const char ERR_CREATE_BUFFER[] = "failed to create buffer!";
const char ERR_CREATE_IMAGE[] = "failed to create image!";
const char ERR_CREATE_IMAGE_VIEW[] = "failed to create image view!";
const char ERR_CREATE_COMMAND_POOL[] = "failed to create command pool!";
const char ERR_CREATE_COMMAND_BUFFER[] = "failed to create command buffer!";

class SVE_Device {
public:
	SVE_Device(const char* deviceName);

	inline VkFence createFence() { 
		VkFenceCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.flags = 0;
		info.pNext = nullptr;
		VkFence fence;
		SVE_CHECK_VK(vkCreateFence(logical, &info, SVE::VulkanAllocationCallback, &fence, VK_TRUE, UINT64_MAX), ERR_CREATE_FENCE); 
		return fence;
	}
	inline VkFence createFenceSignaled() {
		VkFenceCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		info.pNext = nullptr;
		VkFence fence;
		SVE_CHECK_VK(vkCreateFence(logical, &info, SVE::VulkanAllocationCallback, &fence, VK_TRUE, UINT64_MAX), ERR_CREATE_FENCE);
		return fence;
	}
	inline void destroyFence(VkFence fence) { vkDestroyFence(logical, fence, SVE::VulkanAllocationCallback); }
	inline void waitFences(uint32_t fenceCount, const VkFence* pFences) { SVE_CHECK_VK(vkWaitForFences(logical, fenceCount, pFences, VK_TRUE, UINT64_MAX), ERR_WAIT_FENCE); }
	inline void waitFence(VkFence fence) { SVE_CHECK_VK(vkWaitForFences(logical, 1, &fence, VK_TRUE, UINT64_MAX), ERR_WAIT_FENCE); }
	inline VkSemaphore createSemaphore() {
		VkSemaphoreCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.flags = 0;
		info.pNext = nullptr;
		VkSemaphore semaphore;
		SVE_CHECK_VK(vkCreateSemaphore(logical, &info, SVE::VulkanAllocationCallback, &semaphore), ERR_CREATE_SEMAPHORE);
		return semaphore;
	}
	inline void destroySemaphore(VkSemaphore semaphore) { vkDestroySemaphore(logical, semaphore, SVE::VulkanAllocationCallback); }
private:
	friend SVE_Display;
	VkDevice logical = VK_NULL_HANDLE;
	VkPhysicalDevice physical = VK_NULL_HANDLE;
	uint32_t presentIndex = 0;
	uint32_t graphicsIndex = 0;
	uint32_t asyncTransferIndex = 0;
	uint32_t asyncComputeIndex = 0;
};
