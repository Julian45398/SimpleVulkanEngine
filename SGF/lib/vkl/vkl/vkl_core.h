#pragma once

#include <vulkan/vulkan.h>

#ifndef VKL_ENGINE_NAME
#define VKL_ENGINE_NAME "vkl engine"
#endif
#ifndef VKL_ENGINE_VERSION
#define VKL_ENGINE_VERSION VK_MAKE_API_VERSION(0, 0, 0, 1)
#endif
#ifndef VKL_APPLICATION_NAME
#define VKL_APPLICATION_NAME "vkl app"
#endif
#ifndef VKL_APPLICATION_VERSION
#define VKL_APPLICATION_VERSION VK_MAKE_API_VERSION(0, 0, 0, 1)
#endif

#include <vector>

#ifdef VKL_CHECK_SUCCESS
#ifndef VKL_FAILED
#error it is required to define: VKL_FAILED(VK_RETURN, ERROR_MSG) to handle failures
#define VKL_FAILED(VK_RETURN, ERROR_MSG)
#endif
#define VKL_CHECK(X, ERROR_MSG) do {VkResult res = X; if (res != VK_SUCCESS) { VKL_FAILED(res, ERROR_MSG); } } while (0)
#else
#define VKL_CHECK(X, ERROR_MSG) do { X; } while (0)
#define VKL_FAILED(VK_RETURN, ERROR_MSG)
#endif

#define VKL_FLAG_NONE 0


namespace vkl {
	inline const char* VKL_VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

	inline const VkAllocationCallbacks* VKL_Callbacks = nullptr;
	inline void setAllocationCallbacks(const VkAllocationCallbacks* pCallbacks) {
		VKL_Callbacks = pCallbacks;
	}
}

#include "vkl_error_codes.h"

