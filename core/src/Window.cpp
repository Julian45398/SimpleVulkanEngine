#include "SGF_Core.hpp"
#include "Window.hpp"
#include "Render/Device.hpp"
#include <assert.h>

namespace SGF {
    extern VkInstance VulkanInstance;
    extern VkAllocationCallbacks* VulkanAllocator;

#pragma region SWAPCHAIN_HELPER_FUNCTIONS

constexpr VkFormat POSSIBLE_DEPTH_FORMATS[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

    VkSurfaceFormatKHR pickSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat) {
        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
        if (format_count != 0) {
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, surface_formats.data());
        }
        else {
            SGF::fatal(ERROR_DEVICE_NO_SURFACE_SUPPORT);
        }
        for (const auto& available_format : surface_formats) {
            if (available_format.format == surfaceFormat.format && available_format.colorSpace == surfaceFormat.colorSpace)
            {
                return available_format;
            }
        }
        return surface_formats[0];
    }

#pragma endregion SWAPCHAIN_HELPER_FUNCTIONS

    void Window::onResize(WindowResizeEvent& event) {
        Window& win = event.getWindow();
        win.width = event.getWidth();
        win.height = event.getHeight();
        win.createSwapchain();
    }
    void Window::onClose(WindowCloseEvent& event) {
        Window& win = event.getWindow();
        win.close();
    }
    void Window::open(const char* name, uint32_t width, uint32_t height, WindowCreateFlags flags) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        if (flags & WINDOW_FLAG_RESIZABLE) {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        }
        if (flags & WINDOW_FLAG_MAXIMIZED) {
            glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        }
        GLFWmonitor* monitor = nullptr;
        if (flags & WINDOW_FLAG_FULLSCREEN) {
            monitor = glfwGetPrimaryMonitor();
        }
        window = glfwCreateWindow(width, height, name, monitor, nullptr);
        if (window == nullptr) {
            SGF::fatal(ERROR_CREATE_WINDOW);
        }
        glfwSetWindowUserPointer(window, this);
        if (glfwCreateWindowSurface(SGF::VulkanInstance, window, SGF::VulkanAllocator, &surface) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_SURFACE);
        }
        // Set GLFW callbacks
        glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            Window& win = *(Window*)glfwGetWindowUserPointer(window);
            WindowResizeEvent event(win, width, height);
            onResize(event);
            WindowEvents.dispatch(event);
		});

		glfwSetWindowCloseCallback(window, [](GLFWwindow* window)
		{
			Window& win = *(Window*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event(win);
			WindowEvents.dispatch(event);
            onClose(event);
		});

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			Window& win = *(Window*)glfwGetWindowUserPointer(window);
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(win, key, mods);
				WindowEvents.dispatch(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(win, key, mods);
				WindowEvents.dispatch(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyRepeatEvent event(win, key, mods);
				WindowEvents.dispatch(event);
				break;
			}
			}
		});

        glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);

                KeyTypedEvent event(win, codepoint);
                WindowEvents.dispatch(event);
            });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);

                switch (action)
                {
                case GLFW_PRESS:
                {
                    MousePressedEvent event(win, button);
                    WindowEvents.dispatch(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseReleasedEvent event(win, button);
                    WindowEvents.dispatch(event);
                    break;
                }
                }
            });

        glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);

                MouseScrollEvent event(win, xOffset, yOffset);
                LayerStack.dispatch(event);
            });
        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);

                MouseMovedEvent event(win, xPos, yPos);
                LayerStack.dispatch(event);
            });
    }
    void Window::close() {
        SGF::info("Destroying window: ", glfwGetWindowTitle(window));
        if (pDevice != nullptr) {
            destroySwapchain();
            //vkDestroyRenderPass(pDevice->logical, renderPass, SGF::VulkanAllocator);
        }
        vkDestroySurfaceKHR(SGF::VulkanInstance, surface, SGF::VulkanAllocator);
        glfwDestroyWindow(window);
        window = nullptr;
        surface = nullptr;
        pDevice = nullptr;
    }
    Window::Window(const char* name, uint32_t width, uint32_t height, WindowCreateFlags flags) {
        open(name, width, height, flags);
    }
    Window::~Window() {
        close();
    }
    void Window::destroySwapchain() {
        assert(pDevice != nullptr);
        //vkDestroyImage(pDevice->logical, depthImage, SGF::VulkanAllocator);
        //vkDestroyImageView(pDevice->logical, depthImageView, SGF::VulkanAllocator);
        //vkFreeMemory(pDevice->logical, depthMemory, SGF::VulkanAllocator);
        //for(uint32_t i = 0; i < imageCount; ++i) {
            //vkDestroyImageView(pDevice->logical, framebuffers[i].imageView, SGF::VulkanAllocator);
            //vkDestroyFramebuffer(pDevice->logical, framebuffers[i].framebuffer, SGF::VulkanAllocator);
            //vkDestroyFramebuffer(pDevice->logical, framebuffers[i].framebuffer, SGF::VulkanAllocator);
        //}
        //delete[] framebuffers;
        vkDestroySwapchainKHR(pDevice->logical, swapchain, SGF::VulkanAllocator);
    }

    void Window::enableVsync() {
        if (presentMode != VK_PRESENT_MODE_FIFO_KHR) {
            presentMode = VK_PRESENT_MODE_FIFO_KHR;
            createSwapchain();
        }
    }
    void Window::disableVsync() {
        // find present modes: 
        uint32_t presentCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice->physical, surface, &presentCount, nullptr);
        if (presentCount != 0 && presentMode != VK_PRESENT_MODE_MAILBOX_KHR) {
            std::vector<VkPresentModeKHR> available(presentCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice->physical, surface, &presentCount, available.data());
            for (const auto& mode : available) {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    presentMode = mode;
                    createSwapchain();
                    break;
                }
            }
        }
    }
    void Window::createSwapchain() {
        SGF::debug("creating swapchain");
        assert(pDevice != nullptr);
        assert(pDevice->presentCount != 0);
        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice->physical, surface, &capabilities);
        imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }
        width = std::min(capabilities.maxImageExtent.width, std::max(width, capabilities.minImageExtent.width));
        height = std::min(capabilities.maxImageExtent.height, std::max(height, capabilities.minImageExtent.height));
        info.oldSwapchain = swapchain;
        info.surface = surface;
        info.minImageCount = imageCount;
        info.imageFormat = surfaceFormat.format;
        info.imageColorSpace = surfaceFormat.colorSpace;
        info.imageExtent = { width, height };
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (pDevice->presentFamilyIndex != pDevice->graphicsFamilyIndex) {
            uint32_t queueFamilyIndices[] = { pDevice->graphicsFamilyIndex, pDevice->presentFamilyIndex };
            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        info.preTransform = capabilities.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = presentMode;
        info.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(pDevice->logical, &info, SGF::VulkanAllocator, &swapchain) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_SWAPCHAIN);
        }
    }
    void Window::bindDevice(Device& device) {
        SGF::info("device bound to window: ");
        if (pDevice != nullptr) {
            unbindDevice();
        }
        //WindowEvents.subscribe(Device::onWindowMinimize, &device);
        //Device* pDevice = &device;
        pDevice = &device;
        assert(swapchain == VK_NULL_HANDLE);
        surfaceFormat = pickSurfaceFormat(pDevice->physical, surface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR });

        //depthFormat = pDevice->getSupportedFormat(POSSIBLE_DEPTH_FORMATS, ARRAY_SIZE(POSSIBLE_DEPTH_FORMATS), VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        
        //createRenderPass();
        createSwapchain();
        //createResources();
    }

    void Window::onUpdate() {
        glfwPollEvents();
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(window);
    }
}