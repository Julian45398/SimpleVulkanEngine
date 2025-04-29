#pragma once

#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))

#include <stdint.h>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include "vk_types.h"

#include "util/logging.hpp"
#include "util/SVE_Timer.hpp"

#ifndef SVE_FRAMES_IN_FLIGHT
#define SVE_FRAMES_IN_FLIGHT 2
#endif

namespace SVE {
	/*
	inline const uint32_t FRAMES_IN_FLIGHT = SVE_FRAMES_IN_FLIGHT;
	inline VkAllocationCallbacks* VulkanAllocationCallback = nullptr;
	inline SVE_Display Display;
	inline SVE_Device GPU;
	inline SVE_Logger Log;
	inline SVE_Allocator Mem;
	inline SVE_Event Evt;
*/
	void init();
	void terminate();
}
