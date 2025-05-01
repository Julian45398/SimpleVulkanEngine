#pragma once

#include <stdint.h>

#ifdef SGF_LOG_MSG
#define SGF_ERROR(CODE,NAME,MSG) inline const char SGF_ERROR_##NAME[] = MSG
#else
#define SGF_ERROR(CODE,NAME,MSG) inline const uint32_t SGF_ERROR_##NAME = CODE
#endif

namespace SGF {
    SGF_ERROR(1420, DEVICE_MEM_ALLOCATION, "failed to allocate device memory!");
    SGF_ERROR(1421, BIND_DEVICE_MEMORY, "failed to bind device memory!");
    SGF_ERROR(1422, UNSUPPORTED_MEMORY_TYPE, "unsupported memory type!");
    SGF_ERROR(1012, CREATE_BUFFER, "failed to create buffer");
    SGF_ERROR(1013, CREATE_IMAGE, "failed to create image");
    SGF_ERROR(1013, CREATE_IMAGE_VIEW, "failed to create image view");
    SGF_ERROR(1114, CREATE_RENDER_PASS, "failed to create render pass");
    SGF_ERROR(1015, CREATE_FRAMEBUFFER, "failed to create framebuffer");
    SGF_ERROR(1114, CREATE_LOGICAL_DEVICE, "failed to create logical device");
    SGF_ERROR(1115, FIND_PHYSICAL_DEVICE, "failed to create logical device");
}