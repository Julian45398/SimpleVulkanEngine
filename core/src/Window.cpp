#include "SGF_Core.hpp"
#include "Window.hpp"
#include "Render/Device.hpp"
#include "Layers/LayerStack.hpp"
#include "Events/InputEvents.hpp"
#include "Render/CommandList.hpp"
#include "Render/RenderPass.hpp"

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

#ifndef SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT
#define SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT 1000000000
#endif

namespace SGF {
    extern VkInstance VulkanInstance;
    extern VkAllocationCallbacks* VulkanAllocator;

    void Input::PollEvents() {
        glfwPollEvents();
    }
    void Input::WaitEvents() {
        glfwWaitEvents();
    }
    glm::dvec2 Input::GetCursorPos() {
        static glm::dvec2 pos(0, 0);
        GLFWwindow* win = (GLFWwindow*)Window::GetNativeFocused();
        if (win != nullptr) {
            glfwGetCursorPos(win, &pos.x, &pos.y);
        }
        return pos;
    }
    bool Input::IsMouseButtonPressed(Mousecode button) {
        GLFWwindow* win = (GLFWwindow*)Window::GetNativeFocused();
        return win != nullptr && glfwGetMouseButton(win, button) == GLFW_PRESS;
    }
    bool Input::IsKeyPressed(Keycode key) {
        GLFWwindow* win = (GLFWwindow*)Window::GetNativeFocused();
        return win != nullptr && glfwGetKey(win, key) == GLFW_PRESS;
    }
    void WindowHandle::open(const char* title, uint32_t width, uint32_t height, WindowCreateFlags flags) {
        assert(nativeHandle == nullptr);
        
        GLFWmonitor* monitor = nullptr;
        if (flags & WINDOW_FLAG_FULLSCREEN) {
            monitor = glfwGetPrimaryMonitor();
            const auto mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
        }
        
        nativeHandle = glfwCreateWindow(width, height, title, monitor, nullptr);
        if (nativeHandle == nullptr) {
            SGF::fatal(ERROR_CREATE_WINDOW);
        }
        if (flags & WINDOW_FLAG_RESIZABLE) {
            glfwSetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_RESIZABLE, GLFW_TRUE);
        } 
        if (flags & WINDOW_FLAG_BORDERLESS) {
            glfwSetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_DECORATED, GLFW_FALSE);
        }
        glfwSetWindowCloseCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window) {
            SGF::info("Window should close: ", glfwGetWindowTitle(window));
        });
    }
    void WindowHandle::close() {
        
        glfwDestroyWindow((GLFWwindow*)nativeHandle);
        nativeHandle = nullptr;
    }
    bool WindowHandle::shouldClose() const {
        return glfwWindowShouldClose((GLFWwindow*)nativeHandle);
    }
    uint32_t WindowHandle::getWidth() const {
        int width;
        glfwGetWindowSize((GLFWwindow*)nativeHandle, &width, nullptr);
        return width;
    }
    uint32_t WindowHandle::getHeight() const {
        int height;
        glfwGetWindowSize((GLFWwindow*)nativeHandle, nullptr, &height);
        return height;
    }
    VkExtent2D WindowHandle::getSize() const {
        static_assert(sizeof(int) == sizeof(uint32_t));
        VkExtent2D size;
        glfwGetWindowSize((GLFWwindow*)nativeHandle, (int*)&size.width, (int*)&size.height);
        return size;
    }
    bool WindowHandle::isKeyPressed(Keycode key) const {
        return glfwGetKey((GLFWwindow*)nativeHandle, key) == GLFW_PRESS;
    }
    bool WindowHandle::isMouseButtonPressed(Mousecode button) const {
        return glfwGetMouseButton((GLFWwindow*)nativeHandle, button) == GLFW_PRESS;
    }
    glm::dvec2 WindowHandle::getCursorPos() const  {
        glm::dvec2 pos;
        glfwGetCursorPos((GLFWwindow*)nativeHandle, &pos.x, &pos.y);
        return pos;
    }
    void WindowHandle::captureCursor() const {
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    void WindowHandle::freeCursor() const {
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    bool WindowHandle::isFullscreen() const {
        return glfwGetWindowMonitor((GLFWwindow*)nativeHandle) != nullptr;
    }
    bool WindowHandle::isMinimized() const  {
        auto size = getSize();
        return size.width == 0 || size.height == 0;
    }
    void WindowHandle::setUserPointer(void* pUser) const {
        glfwSetWindowUserPointer((GLFWwindow*)nativeHandle, pUser);
    }
    void WindowHandle::setTitle(const char* title) const {
        glfwSetWindowTitle((GLFWwindow*)nativeHandle, title);
    }
    

    const char* WindowHandle::getTitle() const {
        return glfwGetWindowTitle((GLFWwindow*)nativeHandle);
    }

    void WindowHandle::setCursorPos(const glm::dvec2& pos) const {
        glfwSetCursorPos((GLFWwindow*)nativeHandle, pos.x, pos.y);
    }
    void WindowHandle::setFullscreen() const {
        auto m = glfwGetPrimaryMonitor();
        auto mode = glfwGetVideoMode(m);
        glfwSetWindowMonitor((GLFWwindow*)nativeHandle, m, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    void WindowHandle::setWindowed(uint32_t width, uint32_t height) const {
        glfwSetWindowMonitor((GLFWwindow*)nativeHandle, nullptr, (int)(width / 2), (int)(height / 2), (int)width, (int)height, GLFW_DONT_CARE);
    }
    void WindowHandle::resize(uint32_t width, uint32_t height) const {
        glfwSetWindowSize((GLFWwindow*)nativeHandle, (int)width, (int)height);
    }
    void WindowHandle::minimize() const {
        glfwIconifyWindow((GLFWwindow*)nativeHandle);
    }

    constexpr VkFormat POSSIBLE_STENCIL_FORMATS[] = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT };

    VkAttachmentDescription Window::createSwapchainAttachment(VkAttachmentLoadOp loadOp) {
		VkAttachmentDescription att;
		att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		att.samples = VK_SAMPLE_COUNT_1_BIT;
		att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		att.loadOp = loadOp;
		att.format = Get().surfaceFormat.format;
		att.flags = 0;
		att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		return att;
	}

    Window Window::s_MainWindow;
    WindowSettings Window::s_WindowSettings = {
        .title = SGF_APP_NAME,
        .createFlags = 0,
        .width = 600,
        .height = 400
    };

    VkExtent2D Window::getMonitorSize(uint32_t index) {
        int count;
        auto monitors = glfwGetMonitors(&count);
        assert(index < (uint32_t)count);
        auto vidmode = glfwGetVideoMode(monitors[index]);
        VkExtent2D extent;
        extent.width = vidmode->width;
        extent.height = vidmode->height;
        return extent;
    }
    VkExtent2D Window::getMaxMonitorSize() {
        VkExtent2D extent{};
        int count;
        auto monitors = glfwGetMonitors(&count);
        if (monitors == nullptr) {
            return {0, 0};
        }
        for (uint32_t i = 0; i < count; ++i) {
            auto vidmode = glfwGetVideoMode(monitors[i]);
            extent.width = std::max(extent.width, (uint32_t)vidmode->width);
            extent.height = std::max(extent.height, (uint32_t)vidmode->height);
        }
        return extent;
    }
    uint32_t Window::getMonitorCount() {
        int count;
        glfwGetMonitors(&count);
        return (uint32_t)count;
    }

    void Window::nextFrame(VkSemaphore imageAvailableSignal, VkFence fence) {
		assert(swapchain != VK_NULL_HANDLE);
		auto& device = Device::Get();
        VkResult res = VK_ERROR_OUT_OF_DATE_KHR;
		res = vkAcquireNextImageKHR(device, swapchain, SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT, imageAvailableSignal, fence, &imageIndex);
		while (res == VK_ERROR_OUT_OF_DATE_KHR) {
			debug("swapchain out of date!");
            int w = 0, h = 0;
            glfwGetFramebufferSize((GLFWwindow*)windowHandle.getHandle(), &w, &h);
            while (w == 0 || h == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize((GLFWwindow*)windowHandle.getHandle(), &w, &h);
            }
            width = w;
            height = h;
            updateFramebuffers();
		    res = vkAcquireNextImageKHR(device, swapchain, SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT, imageAvailableSignal, fence, &imageIndex);
		}
        if (res == VK_SUBOPTIMAL_KHR) {
            debug("swapchain image suboptimal");
        } else if (res != VK_SUCCESS) {
			fatal(ERROR_ACQUIRE_NEXT_IMAGE);
		}
    }
	void Window::presentFrame(const VkSemaphore* pWaitSemaphores, uint32_t waitCount) {
		assert(swapchain != VK_NULL_HANDLE);
		
		VkPresentInfoKHR info;
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;
		info.swapchainCount = 1;
		info.pImageIndices = &imageIndex;
		info.pSwapchains = &swapchain;
		info.pWaitSemaphores = pWaitSemaphores;
		info.waitSemaphoreCount = waitCount;
		info.pResults = nullptr;
		VkResult result = vkQueuePresentKHR(presentQueue, &info);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
			SGF::info("swapchain out of date!");
            int w = 0, h = 0;
            glfwGetFramebufferSize((GLFWwindow*)windowHandle.getHandle(), &w, &h);
            while (w == 0 || h == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize((GLFWwindow*)windowHandle.getHandle(), &w, &h);
            }
            width = w;
            height = h;
            updateFramebuffers();
		}
		else if (result != VK_SUCCESS) {
			fatal(ERROR_PRESENT_IMAGE);
		}
	}

    void Window::open(const char* name, uint32_t newWidth, uint32_t newHeight, WindowCreateFlags flags, VkSampleCountFlagBits multisampleCount) {
        if (isOpen()) {
            close();
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        windowHandle.open(name, newWidth, newHeight, flags);
        
        windowHandle.setUserPointer(this);

        if (glfwCreateWindowSurface(SGF::VulkanInstance, (GLFWwindow*)windowHandle.getHandle(), SGF::VulkanAllocator, &surface) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_SURFACE);
        }

        auto& device = Device::Get();
        assert(device.isCreated());
        if (!device.checkSurfaceSupport(surface)) {
            SGF::fatal("device is missing surface support!");
        }
        presentQueue = device.presentQueue();
        if (!(flags & WINDOW_FLAG_VSYNC)) {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
        updateSwapchain();

        // Set GLFW callbacks
        glfwSetFramebufferSizeCallback((GLFWwindow*)windowHandle.getHandle(), [](GLFWwindow* window, int width, int height) {
            WindowHandle& windowHandle = *(WindowHandle*)&window;
            SGF::info("framebuffersizecallback ....");
            

            if (width == 0 || height == 0) {
                SGF::info("window is minimized!");
                WindowMinimizeEvent event(windowHandle, true);
				EventManager::dispatch(event);
                do {
                    glfwGetFramebufferSize(window, &width, &height);
                    glfwWaitEvents();
                    SGF::info("polled events finished!");
                } while (width == 0 || height == 0);
                WindowMinimizeEvent maxEvent(windowHandle, false);
                EventManager::dispatch(maxEvent);
                SGF::info("window is maximized again!");
            }
			auto pWindow = (Window*)glfwGetWindowUserPointer(window);
            if (pWindow != nullptr) {
                Window& win = *pWindow;
                Device::Get().waitIdle();
                win.resizeFramebuffers(width, height);
            }
            WindowResizeEvent event(windowHandle, width, height);
            EventManager::dispatch(event);
		});

		glfwSetWindowCloseCallback((GLFWwindow*)windowHandle.getHandle(), [](GLFWwindow* window)
		{
			WindowHandle& win = *(WindowHandle*)&window;
            WindowCloseEvent event(win);
            EventManager::dispatch(event);
		});

		glfwSetKeyCallback((GLFWwindow*)windowHandle.getHandle(), [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowHandle& win = *(WindowHandle*)&window;
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(win, key, mods);
                LayerStack::OnEvent(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(win, key, mods);
                LayerStack::OnEvent(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyRepeatEvent event(win, key, mods);
                LayerStack::OnEvent(event);
				break;
			}
			}
		});

        glfwSetCharCallback((GLFWwindow*)windowHandle.getHandle(), [](GLFWwindow* window, unsigned int codepoint)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            KeyTypedEvent event(win, codepoint);
            LayerStack::OnEvent(event);
            //WindowEvents.dispatch(event);
        });

        glfwSetMouseButtonCallback((GLFWwindow*)windowHandle.getHandle(), [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            switch (action)
            {
            case GLFW_PRESS:
            {
                MousePressedEvent event(win, button);
                LayerStack::OnEvent(event);
                //WindowEvents.dispatch(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseReleasedEvent event(win, button);
                LayerStack::OnEvent(event);
                break;
            }
            }
        });
        glfwSetWindowFocusCallback((GLFWwindow*)windowHandle.getHandle(), [](GLFWwindow* window, int focus) 
        {
            WindowHandle* win = (WindowHandle*)&window;
            if (win == nullptr) {
                return;
                SGF::warn("window user pointer is null!");
            }
            if (focus == GLFW_TRUE) {
                SGF::info("Window: ", win->getTitle(), " is now focused!");
                s_NativeFocused = window;
            } else if (focus == GLFW_FALSE) {
                SGF::info("Window: ", win->getTitle(), " lost focus");
                if (s_NativeFocused == window) {
                    s_NativeFocused == nullptr;
                }
            }
        });

        glfwSetScrollCallback((GLFWwindow*)windowHandle.getHandle(), [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowHandle& win = *(WindowHandle*)window;

            MouseScrollEvent event(win, xOffset, yOffset);
            LayerStack::OnEvent(event);
        });
        glfwSetCursorPosCallback((GLFWwindow*)windowHandle.getHandle(), [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowHandle& win = *(WindowHandle*)window;

            MouseMovedEvent event(win, xPos, yPos);
            LayerStack::OnEvent(event);
        });

        if (flags & WINDOW_FLAG_CUSTOM_RENDER_PASS) return;
        std::vector<VkAttachmentDescription> attachments;
        attachments.reserve(3);
        std::vector<VkClearValue> clearValues;
        clearValues.reserve(3);

        VkSubpassDescription subpass; 
        subpass.flags = FLAG_NONE;
        subpass.colorAttachmentCount = 1;
        subpass.inputAttachmentCount = 0;
        subpass.preserveAttachmentCount = 0;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pPreserveAttachments = nullptr;
        subpass.pResolveAttachments = nullptr;
        subpass.pDepthStencilAttachment = nullptr;

        VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        subpass.pColorAttachments = &colorRef;
        VkAttachmentReference depthRef = { 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        VkAttachmentReference resolveRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        if (WINDOW_FLAG_NO_COLOR_CLEAR) {
            loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        
        attachments.push_back(createSwapchainAttachment(loadOp));
        clearValues.push_back(createColorClearValue(0.f, 0.f, 0.f, 0.f));
        VkFormat depthFormat = VK_FORMAT_D16_UNORM;
        if (WINDOW_FLAG_STENCIL_ATTACHMENT & flags) {
            depthFormat = device.getSupportedFormat(POSSIBLE_STENCIL_FORMATS, ARRAY_SIZE(POSSIBLE_STENCIL_FORMATS), VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }
        if (flags & (WINDOW_FLAG_STENCIL_ATTACHMENT | WINDOW_FLAG_DEPTH_ATTACHMENT)) {
			auto depthAttachment = createDepthAttachment(depthFormat, multisampleCount);
            attachments.push_back(depthAttachment);
            depthRef.attachment = 1;
            subpass.pDepthStencilAttachment = &depthRef;
            clearValues.push_back(createDepthClearValue(1.f, 0));
        }
        if (multisampleCount != VK_SAMPLE_COUNT_1_BIT) {
            subpass.pResolveAttachments = &resolveRef;
            attachments.push_back(createAttachmentDescription(surfaceFormat.format, multisampleCount, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, loadOp));
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorRef.attachment = depthRef.attachment + 1;
            clearValues.push_back(createColorClearValue(0.f, 2.f, 2.f, 0.f));
        }
        VkSubpassDependency dependency = { VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 
            0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };
        setRenderPass(attachments.data(), clearValues.data(), (uint32_t)clearValues.size(), &subpass, 1, &dependency, 1);
    }
    void Window::close() {
        if (isOpen()) {
            Device::Get().waitIdle();
            freeAttachmentData();
            Device::Get().destroy(swapchain, renderPass);
            vkDestroySurfaceKHR(SGF::VulkanInstance, surface, SGF::VulkanAllocator);
            windowHandle.close();
            surface = nullptr;
        }
    }

    const char* Window::getName() const {
        return windowHandle.getTitle();
    }

    bool Window::isFullscreen() const {
        return windowHandle.isFullscreen();
    }
    bool Window::isMinimized() const {
        return (width == 0 && height == 0);
    }
    void Window::setFullscreen() {
        windowHandle.setFullscreen();
    }
    void Window::setWindowed(uint32_t newWidth, uint32_t newHeight) {
        width = newWidth;
        height = newHeight;
        if (windowHandle.isFullscreen()) {
            windowHandle.setWindowed(newWidth, newHeight);
        }
        else {
            windowHandle.resize(newWidth, newHeight);
        }
    }
    void Window::minimize() {
        width = 0;
        height = 0;
        windowHandle.minimize();
    }
    bool Window::shouldClose() const {
        return windowHandle.shouldClose();
    }
    
    bool Window::isKeyPressed(Keycode key) const {
        return windowHandle.isKeyPressed(key);
    }
    bool Window::isMousePressed(Mousecode button) const {
        return windowHandle.isMouseButtonPressed(button);
    }
    glm::dvec2 Window::getCursorPos() const {
        return windowHandle.getCursorPos();
    }

    std::string Window::openFileDialog(const FileFilter& filter) const {
		return openFileDialog(1, &filter);
	}
	std::string Window::openFileDialog(uint32_t filterCount, const FileFilter* pFilters) const {
		NFD_Init();

		nfdu8char_t* outPath;
		nfdopendialogu8args_t args = { 0 };

		std::string filepath;
        if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)windowHandle.getHandle(), &args.parentWindow)) {
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
		if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)windowHandle.getHandle(), &args.parentWindow)) {
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

    void Window::freeAttachmentData() {
		assert(attachmentData != nullptr);
		destroyFramebuffers();
        free(attachmentData);
        info("freed memory!");
		attachmentData = nullptr;
	}
    void Window::setRenderPass(const VkAttachmentDescription* pAttachments, const VkClearValue* pClearValues, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount) {
        auto& device = Device::Get();
        if (attachmentData != nullptr) {
            freeAttachmentData();
        }
        if (renderPass != nullptr) {
            device.destroy(renderPass);
        }
        renderPass = device.renderPass(pAttachments, attCount, pSubpasses, subpassCount, pDependencies, dependencyCount);
        allocateAttachmentData(pAttachments, pClearValues, attCount, pSubpasses, subpassCount);
    }

    void Window::updateSwapchain() {
		imageCount = 0;
		const auto& device = Device::Get();
		presentMode = device.pickPresentMode(surface, presentMode);
		surfaceFormat = device.pickSurfaceFormat(surface, surfaceFormat);
        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical, surface, &capabilities);
        imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }
        width = std::min(capabilities.maxImageExtent.width, std::max(width, capabilities.minImageExtent.width));
        height = std::min(capabilities.maxImageExtent.height, std::max(height, capabilities.minImageExtent.height));
		VkSwapchainKHR old = swapchain;
        info.oldSwapchain = old;
        info.surface = surface;
        info.minImageCount = imageCount;
        info.imageFormat = surfaceFormat.format;
        info.imageColorSpace = surfaceFormat.colorSpace;
        info.imageExtent = { width, height };
        info.imageArrayLayers = 1;
        if (device.presentFamilyIndex != device.graphicsFamilyIndex) {
            uint32_t queueFamilyIndices[] = { device.graphicsFamilyIndex, device.presentFamilyIndex };
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
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		swapchain = device.swapchain(info);
		device.getSwapchainImages(swapchain, &imageCount, nullptr);
		if (old != VK_NULL_HANDLE) {
			device.destroy(old);
		}
		assert(imageCount != 0);
	}

	void Window::allocateAttachmentData(const VkAttachmentDescription* pAttachments, const VkClearValue* pClearValues, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
		assert(attCount > 0);
		attachmentCount = attCount - 1;
        assert(swapchain != VK_NULL_HANDLE);
        assert(imageCount != 0);
		assert(attachmentData == nullptr);
		size_t allocSize = (sizeof(VkImage) + sizeof(VkFramebuffer) + sizeof(VkImageView)) * imageCount + sizeof(pClearValues[0]) * attCount;
		if (attachmentCount != 0) {
			allocSize += (sizeof(VkImage) + sizeof(VkImageView) + sizeof(VkFormat) + sizeof(VkImageUsageFlags) + sizeof(VkSampleCountFlags)) * attachmentCount + sizeof(VkDeviceMemory);
		}
		attachmentData = (char*)malloc(allocSize);
        if (attachmentData == nullptr) {
            fatal("failed to allocate attachment data!");
        }

		memcpy(getClearValuesMod(), pClearValues, sizeof(pClearValues[0]) * attCount);
		if (attachmentCount != 0) {
			auto formats = getAttachmentFormatsMod();
			auto usages = getAttachmentUsagesMod();
			auto samples = getAttachmentSampleCountsMod();
			for (uint32_t i = 1; i < attCount; ++i) {
				formats[i-1] = pAttachments[i].format;
				VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
				samples[i-1] = pAttachments[i].samples;
				for (uint32_t k = 0; k < subpassCount; ++k) {
					auto& subpass = pSubpasses[k];
					for (uint32_t l = 0; l < subpass.colorAttachmentCount; ++l) {
						if (subpass.pColorAttachments[l].attachment == i) {
							usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
						}
					}
					for (uint32_t l = 0; l < subpass.inputAttachmentCount; ++l) {
						if (subpass.pInputAttachments[l].attachment == i) {
							usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
						}
					}
					if (subpass.pDepthStencilAttachment != nullptr) {
						if (subpass.pDepthStencilAttachment[0].attachment == i) {
							usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
						}
					}
				}
				usages[i - 1] = usage;
			}
		}
		createFramebuffers();
	}
	void Window::createFramebuffers() {
		SGF::debug("creating framebuffers of swapchain!");
		auto& dev = Device::Get();
		assert(attachmentData != nullptr);
        {
            uint32_t count = imageCount;
		    dev.getSwapchainImages(swapchain, &count, getImagesMod());
            if (count != imageCount) {
                fatal(ERROR_CREATE_SWAPCHAIN);
            }
        }
		VkImageView* views = getImageViewsMod();
		auto images = getSwapchainImages();
		VkImageViewCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = FLAG_NONE;
		info.format = getImageFormat();
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.levelCount = 1;
		std::vector<VkImageView> attachmentViews(attachmentCount + 1);
		if (attachmentCount != 0) {
			const VkFormat* attFormats = getAttachmentFormats();
			const VkImageUsageFlags* attUsages = getAttachmentUsages();
			const VkSampleCountFlagBits* attSamples = getAttachmentSampleCounts();
			VkImage* attImages = getAttachmentImagesMod();
			VkImageView* attImageViews = getAttachmentImageViewsMod();
			for (uint32_t i = 0; i < attachmentCount; ++i) {
				attImages[i] = dev.image2D(width, height, attFormats[i], attUsages[i], attSamples[i]);
			}
			auto& memory = getAttachmentMemory();
			memory = dev.allocate(attImages, attachmentCount);
			for (uint32_t i = 0; i < attachmentCount; ++i) {
				VkImageUsageFlags usage = attUsages[i];
				if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
					info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
					if ((attFormats[i] == VK_FORMAT_D16_UNORM_S8_UINT || attFormats[i] == VK_FORMAT_D24_UNORM_S8_UINT || attFormats[i] == VK_FORMAT_D32_SFLOAT_S8_UINT)) {
						info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
					}
				}
				else {
					info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				}
				info.format = attFormats[i];
				info.image = attImages[i];
				attImageViews[i] = dev.imageView(info);
			}
			
			for (size_t i = 1; i < attachmentViews.size(); ++i) {
				attachmentViews[i] = attImageViews[i-1];
			}
		}
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		auto framebuffers = getFramebuffersMod();
		info.format = getImageFormat();
		for (uint32_t i = 0; i < imageCount; ++i) {
			info.image = images[i];
			views[i] = dev.imageView(info);
			attachmentViews[0] = views[i];
			framebuffers[i] = dev.framebuffer(renderPass, attachmentViews.data(), attachmentViews.size(), width, height, 1);
		}
	}
    void Window::destroyFramebuffers() {
		SGF::debug("destroying framebuffers of swapchain!");
		auto& dev = Device::Get(); 
		assert(attachmentData != nullptr);
		auto imageViews = getSwapchainImageViews();
		auto framebuffers = getFramebuffers();
		for (uint32_t i = 0; i < imageCount; ++i) {
			dev.destroy(imageViews[i], framebuffers[i]);
		}
		for (uint32_t i = 0; i < attachmentCount; ++i) {
			auto attachmentImages = getAttachmentImages();
			auto attachmentImageViews = getAttachmentImageViews();
			dev.destroy(attachmentImages[i], attachmentImageViews[i]);
		}
		if (attachmentCount != 0) {
			dev.destroy(getAttachmentMemory());
		}
	}
    void Window::updateFramebuffers() {
        assert(attachmentData != nullptr);
        Device::Get().waitIdle();
        destroyFramebuffers();
        updateSwapchain();
        createFramebuffers();
    }
}