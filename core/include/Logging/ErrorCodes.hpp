#pragma once

#include <stdint.h>

#ifdef SGF_LOG_MSG
#define SGF_ERROR(CODE,NAME,MSG) inline const char ERROR_##NAME[] = MSG
#else
#define SGF_ERROR(CODE,NAME,MSG) inline const uint32_t ERROR_##NAME = CODE
#endif

namespace SGF {
    SGF_ERROR(1420, DEVICE_MEM_ALLOCATION, "failed to allocate device memory!");
    SGF_ERROR(1421, BIND_DEVICE_MEMORY, "failed to bind device memory");
    SGF_ERROR(1422, UNSUPPORTED_MEMORY_TYPE, "unsupported memory type");
    SGF_ERROR(1422, DEVICE_WAIT_IDLE, "failed waiting for device idling");
    SGF_ERROR(1422, WAIT_FENCE, "failed waiting for fence");
    SGF_ERROR(1422, RESET_FENCE, "failed resetting fences");
    SGF_ERROR(1422, QUEUE_SUBMIT, "failed to submit commands");
    SGF_ERROR(1422, VULKAN_IMGUI, "vulkan error for imgui operation");

    SGF_ERROR(1012, CREATE_BUFFER, "failed to create buffer");
    SGF_ERROR(1013, CREATE_IMAGE, "failed to create image");
    SGF_ERROR(1013, CREATE_IMAGE_VIEW, "failed to create image view");
    SGF_ERROR(1013, CREATE_SAMPLER, "failed to create image sampler");

    SGF_ERROR(1100, CREATE_RENDER_PASS, "failed to create render pass");
    SGF_ERROR(1101, CREATE_FRAMEBUFFER, "failed to create framebuffer");
    SGF_ERROR(1102, CREATE_SHADER_MODULE, "failed to create shader module");
    SGF_ERROR(1103, CREATE_PIPELINE_LAYOUT, "failed to create pipeline layout");
    SGF_ERROR(1104, CREATE_RENDER_PIPELINE, "failed to create render pipeline");
    SGF_ERROR(1105, CREATE_COMPUTE_PIPELINE, "failed to create compute pipeline");
    SGF_ERROR(1106, CREATE_DESCRIPTOR_POOL, "failed to create descriptor pool");
    SGF_ERROR(1107, CREATE_DESCRIPTOR_LAYOUT, "failed to create descriptor layout");
    SGF_ERROR(1108, CREATE_COMMAND_POOL, "failed to create command pool");
    SGF_ERROR(1108, ALLOCATE_COMMAND_BUFFERS, "failed to allocate command buffers");
    SGF_ERROR(1108, ALLOCATE_DESCRIPTOR_SETS, "failed to allocate descriptor sets");
    SGF_ERROR(1109, CREATE_FENCE, "failed to create fence");
    SGF_ERROR(1110, CREATE_SEMAPHORE, "failed to create semaphore");

    SGF_ERROR(1006, CREATE_SWAPCHAIN, "failed to create swapchain");
    SGF_ERROR(1006, ACQUIRE_NEXT_IMAGE, "failed to acquire next swapchain image");
    SGF_ERROR(1006, PRESENT_IMAGE, "failed to present swapchain image");
    SGF_ERROR(1422, GET_SWAPCHAIN_IMAGES, "unsupported memory type!");

    SGF_ERROR(1005, CREATE_LOGICAL_DEVICE, "failed to create logical device");
    SGF_ERROR(1004, FIND_PHYSICAL_DEVICE, "failed to suitable graphics device");

    SGF_ERROR(1003, CREATE_SURFACE, "failed to create window surface");
    SGF_ERROR(1013, DEVICE_NO_SURFACE_SUPPORT, "device is missing surface support");
    SGF_ERROR(1002, CREATE_WINDOW, "failed to create window");
    SGF_ERROR(1081, OPEN_FILE_DIALOG, "failed to open file dialog");
    SGF_ERROR(1001, CREATE_VULKAN_INSTANCE, "failed to create vulkan instance");
}