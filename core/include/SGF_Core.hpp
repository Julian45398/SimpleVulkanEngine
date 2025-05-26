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
#include "Logging/Logger.hpp"
#include "Logging/ErrorCodes.hpp"

#include <assert.h>
#include <algorithm>
#include <limits>
#include <vector>
#include <set>
#include <stack>
#include <string>
#include <type_traits>

#include <volk.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define SGF_SINGLE_GRAPHICS_DEVICE
