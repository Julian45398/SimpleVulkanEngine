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

namespace SVE {
	void init();
	void terminate();
}
