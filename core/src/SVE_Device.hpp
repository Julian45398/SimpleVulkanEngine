#pragma once

#include "SVE_Core.hpp"

class SVE_Device {
public:
	SVE_Device(const char* deviceName, )
private:
	VkDevice logical = VK_NULL_HANDLE;
	VkPhysicalDevice physical = VK_NULL_HANDLE;
	uint32_t presentIndex = 0;
	uint32_t graphicsIndex = 0;
	uint32_t asyncTransferIndex = 0;
	uint32_t asyncComputeIndex = 0;
};
