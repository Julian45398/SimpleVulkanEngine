#pragma once


#include "core.h"

inline GLFWwindow* g_Window = nullptr;
inline VkInstance g_Instance = VK_NULL_HANDLE;
inline VkSurfaceKHR g_Surface = VK_NULL_HANDLE;
inline VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
inline VkDevice g_Device = VK_NULL_HANDLE;
inline VkSwapchainKHR g_Swapchain = VK_NULL_HANDLE;
inline VkQueue g_GraphicsQueue = VK_NULL_HANDLE;
inline VkQueue g_PresentQueue = VK_NULL_HANDLE;
inline uint32_t g_GraphicsIndex = 0;
inline uint32_t g_PresentIndex = 0;
inline VkRenderPass g_RenderPass = VK_NULL_HANDLE;
inline VkImage g_DepthImage = VK_NULL_HANDLE;
inline VkImageView g_DepthImageView = VK_NULL_HANDLE;
inline VkDeviceMemory g_DepthMemory = VK_NULL_HANDLE;

struct FrameResources {
	VkImage image = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
};
inline std::vector<FrameResources> g_FrameResources;

void initWindow();
void onWindowResize();
void closeWindow();
