#include "SGF_Core.hpp"
#include "Window.hpp"
#include "Render/Device.hpp"
#include "Layers/LayerStack.hpp"
#include "Events/InputEvents.hpp"

#ifdef SGF_WINDOWS 
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(SGF_LINUX)
#ifdef SGF_USE_X11
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(SGF_USE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#elif defined(SGF_APPLE)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <nfd.h>
#include <nfd_glfw3.h>

#include "Events/Event.hpp"

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
    void Window::open(const char* name, uint32_t newWidth, uint32_t newHeight, WindowCreateFlags flags) {
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
			if (newWidth == 0 && newHeight == 0) {
                const auto mode = glfwGetVideoMode(monitor);
                newWidth = mode->width;
                newHeight = mode->height;
			}
        }
        width = newWidth;
        height = newHeight;
        
        window = glfwCreateWindow(width, height, name, monitor, nullptr);
        if (window == nullptr) {
            SGF::fatal(ERROR_CREATE_WINDOW);
        }
        glfwSetWindowUserPointer((GLFWwindow*)window, this);
        if (glfwCreateWindowSurface(SGF::VulkanInstance, (GLFWwindow*)window, SGF::VulkanAllocator, &surface) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_SURFACE);
        }
        // Set GLFW callbacks
        glfwSetWindowSizeCallback((GLFWwindow*)window, [](GLFWwindow* window, int width, int height) {
            Window& win = *(Window*)glfwGetWindowUserPointer(window);
            if (width == 0 || height == 0) {
                SGF::info("window is minimized!");
                WindowMinimizeEvent event(win, true);
				EventManager::dispatch(event);
                do {
                    glfwGetFramebufferSize(window, &width, &height);
                    glfwWaitEvents();
                    SGF::info("polled events finished!");
                } while (width == 0 || height == 0);
                WindowMinimizeEvent maxEvent(win, false);
                EventManager::dispatch(maxEvent);
                SGF::info("window is maximized again!");
            }
            WindowResizeEvent event(win, width, height);
            onResize(event);
            EventManager::dispatch(event);
		});

		glfwSetWindowCloseCallback((GLFWwindow*)window, [](GLFWwindow* window)
		{
			Window& win = *(Window*)glfwGetWindowUserPointer(window);
            WindowCloseEvent event(win);
            EventManager::dispatch(event);
		});

		glfwSetKeyCallback((GLFWwindow*)window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			Window& win = *(Window*)glfwGetWindowUserPointer(window);
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(win, key, mods);
				//WindowEvents.dispatch(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(win, key, mods);
				//WindowEvents.dispatch(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyRepeatEvent event(win, key, mods);
				//WindowEvents.dispatch(event);
				break;
			}
			}
		});

        glfwSetCharCallback((GLFWwindow*)window, [](GLFWwindow* window, unsigned int codepoint)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);

                KeyTypedEvent event(win, codepoint);
                //WindowEvents.dispatch(event);
            });

        glfwSetMouseButtonCallback((GLFWwindow*)window, [](GLFWwindow* window, int button, int action, int mods)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);

                switch (action)
                {
                case GLFW_PRESS:
                {
                    MousePressedEvent event(win, button);
                    //WindowEvents.dispatch(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseReleasedEvent event(win, button);
                    //WindowEvents.dispatch(event);
                    break;
                }
                }
            });
        glfwSetWindowFocusCallback((GLFWwindow*)window, [](GLFWwindow* window, int focus) 
            {
                Window* win = (Window*)glfwGetWindowUserPointer(window);
                if (focus == GLFW_TRUE) {
                    SGF::info("Window: ", win->getName(), " is now focused!");
                    focusedWindow = win;
                } else if (focus == GLFW_FALSE && win == focusedWindow) {
                    SGF::info("Window: ", win->getName(), " lost focus");
                    focusedWindow = nullptr;
                }
            });

        glfwSetScrollCallback((GLFWwindow*)window, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);

                MouseScrollEvent event(win, xOffset, yOffset);
                //LayerStack.dispatch(event);
            });
        glfwSetCursorPosCallback((GLFWwindow*)window, [](GLFWwindow* window, double xPos, double yPos)
            {
                Window& win = *(Window*)glfwGetWindowUserPointer(window);

                MouseMovedEvent event(win, xPos, yPos);
                //LayerStack.dispatch(event);
            });
    }
    void Window::close() {
        if (pDevice != nullptr) {
            pDevice->destroy(swapchain);
        }
        vkDestroySurfaceKHR(SGF::VulkanInstance, surface, SGF::VulkanAllocator);
        glfwDestroyWindow((GLFWwindow*)window);
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

    const char* Window::getName() const {
        return glfwGetWindowTitle((GLFWwindow*)window);
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
    void Window::onDeviceDestroy(const DeviceDestroyEvent& event, Window* window) {
        assert(window != nullptr);
        assert(window->pDevice != nullptr);
        if (event.getDevice() == window->pDevice) {
            window->unbindDevice();
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
        EventManager::addListener(onDeviceDestroy, this);
        //createResources();
    }

    void Window::unbindDevice() {
        assert(pDevice != nullptr);
        pDevice->destroy(swapchain);
        pDevice = nullptr;
        EventManager::removeListener(onDeviceDestroy, this);
    }

    void Window::onUpdate() {
        glfwPollEvents();
    }
    void Window::nextImage(VkSemaphore signalSemaphore, VkFence fence) {
        if (vkAcquireNextImageKHR(pDevice->logical, swapchain, 2000000000, signalSemaphore, fence, &imageIndex) != VK_SUCCESS) {
            fatal(ERROR_ACQUIRE_NEXT_IMAGE);
        }
    }
    void Window::presentImage(VkSemaphore waitSemaphore, VkFence fence) const {
        VkPresentInfoKHR info;
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.pNext = nullptr;
        info.pImageIndices = &imageIndex;
        info.swapchainCount = 1;
        info.pSwapchains = &swapchain;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &waitSemaphore;
        if (vkQueuePresentKHR(presentQueue, &info) != VK_SUCCESS) {
            fatal(ERROR_PRESENT_IMAGE);
        }
    }
    bool Window::isFullscreen() const {
        return glfwGetWindowMonitor((GLFWwindow*)window) != nullptr;
    }
    bool Window::isMinimized() {
        return (width == 0 && height == 0);
    }
    void Window::enableVsync() {
        if (presentMode != VK_PRESENT_MODE_FIFO_KHR) {
            presentMode = VK_PRESENT_MODE_FIFO_KHR;
            createSwapchain();
        }
    }
    void Window::disableVsync() {
        // find present modes: 
        if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
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
    }
    void Window::setFullscreen() {
        if (glfwGetWindowMonitor((GLFWwindow*)window) != nullptr) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const auto mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor((GLFWwindow*)window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
        }
    }
    void Window::setFullscreenKeepResolution() {
        if (glfwGetWindowMonitor((GLFWwindow*)window) != nullptr) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            glfwSetWindowMonitor((GLFWwindow*)window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
        }
    }
    void Window::setWindowed(uint32_t newWidth, uint32_t newHeight) {
        width = newWidth;
        height = newHeight;
        if (glfwGetWindowMonitor((GLFWwindow*)window) != nullptr) {
            glfwSetWindowMonitor((GLFWwindow*)window, nullptr, 0, 0, newWidth, newHeight, GLFW_DONT_CARE);
        }
        else {
            glfwSetWindowSize((GLFWwindow*)window, newWidth, newHeight);
        }
    }
    void Window::minimize() {
        width = 0;
        height = 0;
        glfwSetWindowSize((GLFWwindow*)window, width, height);
    }
    bool Window::shouldClose() const {
        return glfwWindowShouldClose((GLFWwindow*)window);
    }
    
    bool Window::isKeyPressed(Keycode key) const {
        return glfwGetKey((GLFWwindow*)window, (int)key) == GLFW_PRESS;
    }
    bool Window::isMousePressed(Mousecode button) const {
        return glfwGetMouseButton((GLFWwindow*)window, (int)button) == GLFW_PRESS;
    }
    glm::dvec2 Window::getCursorPos() const {
        glm::dvec2 pos;
        glfwGetCursorPos((GLFWwindow*)window, &pos.x, &pos.y);
        return pos;
    }

    std::string Window::openFileDialog(const FileFilter& filter) const {
		return openFileDialog(1, &filter);
	}
	std::string Window::openFileDialog(uint32_t filterCount, const FileFilter* pFilters) const {
		NFD_Init();

		nfdu8char_t* outPath;
		nfdopendialogu8args_t args = { 0 };

		std::string filepath;
        if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)window, &args.parentWindow)) {
            SGF::error(ERROR_OPEN_FILE_DIALOG);
        }
		args.filterList = (const nfdu8filteritem_t*)(pFilters);
		args.filterCount = filterCount;
		nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

		if (result == NFD_OKAY) {
			filepath = outPath;
			SGF::info("Picked file: ", filepath);
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL) {
			SGF::info("User pressed cancel.");
		}
		else {
			SGF::error(NFD_GetError());
		}
		NFD_Quit();

		return filepath;
	}
	std::string Window::saveFileDialog(const FileFilter& filter) const
	{
		return saveFileDialog(1, &filter);
	}
	std::string Window::saveFileDialog(uint32_t filterCount, const FileFilter* pFilters) const
	{
		NFD_Init();

		nfdu8char_t* outPath;
		nfdsavedialogu8args_t args = {};
		if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)window, &args.parentWindow)) {
			SGF::error(ERROR_OPEN_FILE_DIALOG);
		}
		args.filterList = (const nfdu8filteritem_t*)(pFilters);
		args.filterCount = filterCount;
		nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);

		std::string filepath;
		if (result == NFD_OKAY)
		{
			filepath = outPath;
			SGF::info("User picked savefile: ", filepath);
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL)
		{
			SGF::info("User pressed cancel.");
		}
		else
		{
			SGF::error(NFD_GetError());
		}
		NFD_Quit();

		return filepath;
	}
}