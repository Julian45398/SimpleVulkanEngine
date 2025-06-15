#pragma once
#include "SGF_Types.hpp"

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

#define SGF_ENGINE_VERSION_VARIANT 0
#define SGF_ENGINE_VERSION_MAJOR 0
#define SGF_ENGINE_VERSION_MINOR 0
#define SGF_ENGINE_VERSION_PATCH 1

#define SGF_ENGINE_NAME "SGF"
#define SGF_ENGINE_VERSION VK_MAKE_API_VERSION(SGF_ENGINE_VERSION_VARIANT, SGF_ENGINE_VERSION_MAJOR, SGF_ENGINE_VERSION_MINOR, SGF_ENGINE_VERSION_PATCH)

#ifndef SGF_MAX_FRAMES_IN_FLIGHT
#define SGF_MAX_FRAMES_IN_FLIGHT 2
#endif

#ifndef SGF_APP_VERSION
#define SGF_APP_VERSION SGF_ENGINE_VERSION
#endif
#ifndef SGF_APP_NAME
#define SGF_APP_NAME SGF_ENGINE_NAME 
#endif

