

#ifndef NDEBUG
#define SHL_LOG_ALL
#else
#define SHL_LOG_WARN
#endif
#include <shl/logging.h>
#include <shl/Timer.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#ifndef NDEBUG
#define VKL_ENABLE_VALIDATION
#endif
#include <vkl.h>



#define VK_FLAG_NONE 0
#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))

#include <vector>

#include <stdint.h>