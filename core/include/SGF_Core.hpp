#pragma once
#include "SGF_Types.hpp"

#ifndef SGF_MAX_FRAMES_IN_FLIGHT
#define SGF_MAX_FRAMES_IN_FLIGHT 2
#endif

namespace SGF {
    void PreInit();
    void Setup();
    void Cleanup();
}

#include "Logging/Logger.hpp"
#include "Logging/ErrorCodes.hpp"
#include "Timer.hpp"

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