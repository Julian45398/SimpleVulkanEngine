#pragma once

#include <volk.h>
#define GLFW_INCLUDE_VULKAN
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <GLFW/glfw3.h>

#include "SGF_Types.hpp"

namespace SGF {
    class Window {
    public:
        /**
         * @brief Creates a window with the specified name, width, and height.
         * 
         * Creates a window without binding it to a GPU. 
         * When the window is created in fullscreen mode the width and the height specify 
         * the target resolution.
         * 
         * @param name - the name of the window
         * @param width - window width - or when fullscreen the resolution
         * @param height - window height - or when fullscreen the resolution
         * @param fullscreen - controls whether the window should be created in fullscreen mode
         */
        Window(const char* name, uint32_t width, uint32_t height, bool fullscreen);
        ~Window();
        Window(Window&& other) noexcept;
        Window(const Window& other) = delete;
        Window(Window& other) = delete;
        
        void open(const char* name, uint32_t width, uint32_t height, bool fullscreen);
        void close();
        void onUpdate();
        bool shouldClose();
        inline uint32_t getWidth() { return width; }
        inline uint32_t getHeight() { return height; }
        void bindDevice(Device& device);
        /**
         * @brief Sets the resolution of the window - when the window is not in fullscreen mode the window also gets resized
         */
        void setResolution(uint32_t width, uint32_t height);
        void nextImage(VkFence fence);
        void presentImage(VkSemaphore waitSemaphore, VkFence fence);
        bool isFullscreen();
        bool isMinimized();
        void enableVsync();
        void disableVsync();
        void setFullscreen();
        void setFullscreenKeepResolution();
        void setWindowed();
        void minimize();
        void toggleFullscreen();
    private:
        friend Device;
        static void onWindowResize(GLFWwindow* window, int width, int height);
        void destroySwapchain();
        void createSwapchain();
        //void createRenderPass();
        //void createResources();
        uint32_t width = 0;
        uint32_t height = 0;
        GLFWwindow* window = nullptr;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        Device* pDevice = nullptr;
        //VkRenderPass renderPass = VK_NULL_HANDLE;
        //VkDeviceMemory depthMemory = VK_NULL_HANDLE;
        //VkImage depthImage = VK_NULL_HANDLE;
        //VkImageView depthImageView = VK_NULL_HANDLE;
        //struct Framebuffer {
            //VkImage colorImage;
            //VkImageView imageView;
            //VkFramebuffer framebuffer;
        //};
        uint32_t imageCount = 0;
        uint32_t imageIndex = 0;
        VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_MAX_ENUM, VK_COLOR_SPACE_MAX_ENUM_KHR};
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        //VkFormat depthFormat = VK_FORMAT_MAX_ENUM;
        //VkSampleCountFlagBits multiSampleState = VK_SAMPLE_COUNT_1_BIT;
        //Framebuffer* framebuffers = nullptr;
        VkQueue presentQueue = VK_NULL_HANDLE;
    };
}