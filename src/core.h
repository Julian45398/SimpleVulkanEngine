#pragma once

#ifndef NDEBUG
#define SHL_LOG_ALL
#else
#define SHL_LOG_WARN
#endif
#include <shl/logging.h>
#include <shl/Timer.h>

#include <volk.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef NDEBUG
#define VKL_ENABLE_VALIDATION
#define VKL_CHECK_SUCCESS
#define VKL_FAILED(X, ERROR_MSG) {shl::logFatal("VkResult: ", X, ' ', ERROR_MSG);}
#endif
#include <vkl.h>

#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))

#include <vector>

#include <stdint.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
