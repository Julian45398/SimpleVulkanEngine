#pragma once

#include "vkl_core.h"

namespace vkl {
	inline VkFence createFence(VkDevice device, VkFenceCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		VkFence fence;
		VkFenceCreateInfo info {
			VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, 
			pNext, flags
		};
		VKL_CHECK(vkCreateFence(device, &info, VKL_Callbacks, &fence), VKL_ERROR_FENCE_CREATION_FAILED);
		return fence;
	}
	inline void waitForFence(VkDevice device, VkFence fence, uint64_t timeout = UINT64_MAX) {
		VKL_CHECK(vkWaitForFences(device, 1, &fence, VK_TRUE, timeout), VKL_ERROR_FENCE_WAIT_FAILED);
	}
	inline void waitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll = VK_TRUE, uint64_t timeout = UINT64_MAX) {
		VKL_CHECK(vkWaitForFences(device, fenceCount, pFences, waitAll, timeout), VKL_ERROR_FENCE_WAIT_FAILED);
	}
	inline void resetFence(VkDevice device, VkFence fence) {
		VKL_CHECK(vkResetFences(device, 1, &fence), VKL_ERROR_FENCE_RESET_FAILED);
	}
	inline void destroyFence(VkDevice device, VkFence fence) {
		vkDestroyFence(device, fence, VKL_Callbacks);
	}
	inline VkSemaphore createSemaphore(VkDevice device, VkSemaphoreCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		VkSemaphore semaphore;
		VkSemaphoreCreateInfo info {
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 
			pNext, flags
		};
		VKL_CHECK(vkCreateSemaphore(device, &info, VKL_Callbacks, &semaphore), VKL_ERROR_SEMAPHORE_CREATION_FAILED);
		return semaphore;
	}
	inline void destroySemaphore(VkDevice device, VkSemaphore semaphore) {
		vkDestroySemaphore(device, semaphore, VKL_Callbacks);
	}
}