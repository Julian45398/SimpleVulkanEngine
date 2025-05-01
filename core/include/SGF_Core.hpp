#pragma once

namespace SGF {
    /**
     * @brief Initializes the SGF backend
     */
    void init();
    /**
     * @brief Terminates the SGF backend and frees allocated resources
     */
    void terminate();

    /**
     * @brief Tells wether the backend is initialized
     */
    bool isInitialized();
}

#include "SGF_Types.hpp"
#include "log/Logger.hpp"
#include "log/ErrorCodes.hpp"

#include <assert.h>
#include <algorithm>
#include <limits>
#include <vector>
#include <set>
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define BIT(X) (1<<X)
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))