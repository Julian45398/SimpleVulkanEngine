#include "SGF_Core.hpp"
#include "Window.hpp"
#include "Render/Device.hpp"
#include "Layers/LayerStack.hpp"
#include "Render/CommandList.hpp"
#include "Render/RenderPass.hpp"
#include "Filesystem/File.hpp"

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
        static glm::dvec2 pos(0,0);
        assert(HasFocus());
        pos = s_FocusedWindow.GetCursorPos();
        return pos;
    }
    void Input::SetCursorPos(double xpos, double ypos) {
        assert(HasFocus());
        s_FocusedWindow.SetCursorPos(xpos, ypos);
    }
    void Input::SetCursorPos(const glm::dvec2& pos) {
        assert(HasFocus());
        s_FocusedWindow.SetCursorPos(pos);
    }
    void Input::CaptureCursor() {
        assert(HasFocus());
        s_FocusedWindow.CaptureCursor();
    }
    void Input::HideCursor() {
        assert(HasFocus());
        s_FocusedWindow.HideCursor();
    }
    void Input::RestrictCursor() {
        assert(HasFocus());
        s_FocusedWindow.RestrictCursor();
    }
    void Input::FreeCursor() {
        assert(HasFocus());
        s_FocusedWindow.FreeCursor();
    }
    void Input::SetCursor(const Cursor& cursor) {
        assert(HasFocus());
        s_FocusedWindow.SetCursor(cursor);
    }


    bool Input::IsMouseButtonPressed(Mousecode button) {
        return HasFocus() && GetFocusedWindow().IsMouseButtonPressed(button);
    }
    bool Input::IsKeyPressed(Keycode key) {
        return HasFocus() && GetFocusedWindow().IsKeyPressed(key);
    }
    bool Input::HasFocus() {
        return s_FocusedWindow.IsOpen();
    }

    Cursor::Cursor(const char* textureFile) {
        GLFWimage image;
        auto data = LoadTextureFile(textureFile, (uint32_t*)&image.width, (uint32_t*)&image.height);
        image.pixels = data.data();
        handle = glfwCreateCursor(&image, 0, 0);
    }
    Cursor::~Cursor() { glfwDestroyCursor((GLFWcursor*)handle); }
    void WindowHandle::Open(const char* title, uint32_t width, uint32_t height, WindowCreateFlags flags) {
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
        if (glfwRawMouseMotionSupported()) {
            SGF::info("raw mouse motion supported!");
            glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        if (flags & WINDOW_FLAG_RESIZABLE) {
            glfwSetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_RESIZABLE, GLFW_TRUE);
        } 
        if (flags & WINDOW_FLAG_BORDERLESS) {
            glfwSetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_DECORATED, GLFW_FALSE);
        }
		glfwSetKeyCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowHandle& win = *(WindowHandle*)&window;
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(win, key, mods);
                LayerStack::Get().OnEvent(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(win, key, mods);
                LayerStack::Get().OnEvent(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyRepeatEvent event(win, key, mods);
                LayerStack::Get().OnEvent(event);
				break;
			}
			}
		});

        glfwSetCharCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, unsigned int codepoint)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            KeyTypedEvent event(win, codepoint);
            LayerStack::Get().OnEvent(event);
            //WindowEvents.dispatch(event);
        });

        glfwSetMouseButtonCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            switch (action)
            {
            case GLFW_PRESS:
            {
                MousePressedEvent event(win, button);
                LayerStack::Get().OnEvent(event);
                //WindowEvents.dispatch(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseReleasedEvent event(win, button);
                LayerStack::Get().OnEvent(event);
                break;
            }
            }
        });
        glfwSetWindowFocusCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int focus) 
        {
            assert(window != nullptr);
            WindowHandle& win = *(WindowHandle*)&window;
            if (focus == GLFW_TRUE) {
                SGF::info("Window: ", win.GetTitle(), " is now focused!");
                Input::s_FocusedWindow.SetHandle(window);
            } else if (focus == GLFW_FALSE) {
                SGF::info("Window: ", win.GetTitle(), " lost focus");
                if (Input::s_FocusedWindow.GetHandle() == window) {
                    Input::s_FocusedWindow.SetHandle(nullptr);
                }
            }
        });

        glfwSetScrollCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            MouseScrollEvent event(win, xOffset, yOffset);
            LayerStack::Get().OnEvent(event);
        });
        glfwSetCursorPosCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            MouseMovedEvent event(win, xPos, yPos);
            LayerStack::Get().OnEvent(event);
        });
        glfwSetWindowIconifyCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int iconified) {
            WindowHandle& win = *(WindowHandle*)&window;
            WindowIconifyEvent event(win, iconified);
            LayerStack::Get().OnEvent(event);
        });
        glfwSetWindowSizeCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int width, int height) {
            WindowHandle& win = *(WindowHandle*)&window;
            auto size = win.GetSize();
            WindowResizeEvent event(win, size.width, size.height);
            LayerStack::Get().OnEvent(event);
        });
    }
    void WindowHandle::Close() {
        WindowCloseEvent event(*this);
        EventManager::Dispatch(event);
        glfwDestroyWindow((GLFWwindow*)nativeHandle);
        nativeHandle = nullptr;
    }
    bool WindowHandle::ShouldClose() const {
        return glfwWindowShouldClose((GLFWwindow*)nativeHandle);
    }
    uint32_t WindowHandle::GetWidth() const {
        int width;
        glfwGetFramebufferSize((GLFWwindow*)nativeHandle, &width, nullptr);
        return width;
    }
    uint32_t WindowHandle::GetHeight() const {
        int height;
        glfwGetFramebufferSize((GLFWwindow*)nativeHandle, nullptr, &height);
        return height;
    }
    VkExtent2D WindowHandle::GetSize() const {
        static_assert(sizeof(int) == sizeof(uint32_t));
        VkExtent2D size;
        glfwGetFramebufferSize((GLFWwindow*)nativeHandle, (int*)&size.width, (int*)&size.height);
        return size;
    }
    bool WindowHandle::IsKeyPressed(Keycode key) const {
        return glfwGetKey((GLFWwindow*)nativeHandle, key) == GLFW_PRESS;
    }
    bool WindowHandle::IsMouseButtonPressed(Mousecode button) const {
        return glfwGetMouseButton((GLFWwindow*)nativeHandle, button) == GLFW_PRESS;
    }
    glm::dvec2 WindowHandle::GetCursorPos() const  {
        glm::dvec2 pos;
        glfwGetCursorPos((GLFWwindow*)nativeHandle, &pos.x, &pos.y);
        return pos;
    }
    void WindowHandle::SetCursorPos(double xpos, double ypos) const {
        glfwSetCursorPos((GLFWwindow*) nativeHandle, xpos, ypos);
    }
    void WindowHandle::SetCursorPos(const glm::dvec2& pos) const {
        glfwSetCursorPos((GLFWwindow*) nativeHandle, pos.x, pos.y);
    }
    void WindowHandle::CaptureCursor() const {
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    void WindowHandle::HideCursor() const {
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    void WindowHandle::RestrictCursor() const {
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
    }
    void WindowHandle::FreeCursor() const {
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    void WindowHandle::SetCursor(const Cursor& cursor) const {
        glfwSetCursor((GLFWwindow*)nativeHandle, (GLFWcursor*)cursor.handle);
    }
    bool WindowHandle::IsFullscreen() const {
        return glfwGetWindowMonitor((GLFWwindow*)nativeHandle) != nullptr;
    }
    bool WindowHandle::IsMinimized() const  {
        auto size = GetSize();
        return size.width == 0 || size.height == 0;
    }
    void WindowHandle::SetUserPointer(void* pUser) const {
        glfwSetWindowUserPointer((GLFWwindow*)nativeHandle, pUser);
    }
    void WindowHandle::SetTitle(const char* title) const {
        glfwSetWindowTitle((GLFWwindow*)nativeHandle, title);
    }
    const char* WindowHandle::GetTitle() const {
        return glfwGetWindowTitle((GLFWwindow*)nativeHandle);
    }

    void WindowHandle::SetFullscreen() const {
        auto m = glfwGetPrimaryMonitor();
        auto mode = glfwGetVideoMode(m);
        glfwSetWindowMonitor((GLFWwindow*)nativeHandle, m, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    void WindowHandle::SetWindowed(uint32_t width, uint32_t height) const {
        glfwSetWindowMonitor((GLFWwindow*)nativeHandle, nullptr, (int)(width / 2), (int)(height / 2), (int)width, (int)height, GLFW_DONT_CARE);
    }
    void WindowHandle::Resize(uint32_t width, uint32_t height) const {
        glfwSetWindowSize((GLFWwindow*)nativeHandle, (int)width, (int)height);
    }
    void WindowHandle::Minimize() const {
        glfwIconifyWindow((GLFWwindow*)nativeHandle);
    }

    constexpr VkFormat POSSIBLE_STENCIL_FORMATS[] = { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT };

    VkAttachmentDescription Window::CreateSwapchainAttachment(VkAttachmentLoadOp loadOp) {
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

    VkExtent2D Window::GetMonitorSize(uint32_t index) {
        int count;
        auto monitors = glfwGetMonitors(&count);
        assert(index < (uint32_t)count);
        auto vidmode = glfwGetVideoMode(monitors[index]);
        VkExtent2D extent;
        extent.width = vidmode->width;
        extent.height = vidmode->height;
        return extent;
    }
    VkExtent2D Window::GetMaxMonitorSize() {
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
    uint32_t Window::GetMonitorCount() {
        int count;
        glfwGetMonitors(&count);
        return (uint32_t)count;
    }

    void Window::NextFrame(VkSemaphore imageAvailableSignal, VkFence fence) {
		assert(swapchain != VK_NULL_HANDLE);
		auto& device = Device::Get();
        VkResult res = VK_ERROR_OUT_OF_DATE_KHR;
		res = vkAcquireNextImageKHR(device, swapchain, SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT, imageAvailableSignal, fence, &imageIndex);
		while (res == VK_ERROR_OUT_OF_DATE_KHR) {
			debug("swapchain out of date!");
            int w = 0, h = 0;
            glfwGetFramebufferSize((GLFWwindow*)windowHandle.GetHandle(), &w, &h);
            while (w == 0 || h == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize((GLFWwindow*)windowHandle.GetHandle(), &w, &h);
            }
            width = w;
            height = h;
            UpdateFramebuffers();
		    res = vkAcquireNextImageKHR(device, swapchain, SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT, imageAvailableSignal, fence, &imageIndex);
		}
        if (res == VK_SUBOPTIMAL_KHR) {
            debug("swapchain image suboptimal");
        } else if (res != VK_SUCCESS) {
			fatal(ERROR_ACQUIRE_NEXT_IMAGE);
		}
    }
	void Window::PresentFrame(const VkSemaphore* pWaitSemaphores, uint32_t waitCount) {
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
            glfwGetFramebufferSize((GLFWwindow*)windowHandle.GetHandle(), &w, &h);
            while (w == 0 || h == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize((GLFWwindow*)windowHandle.GetHandle(), &w, &h);
            }
            width = w;
            height = h;
            UpdateFramebuffers();
		}
		else if (result != VK_SUCCESS) {
			fatal(ERROR_PRESENT_IMAGE);
		}
	}

    void Window::Open(const char* name, uint32_t newWidth, uint32_t newHeight, WindowCreateFlags flags, VkSampleCountFlagBits multisampleCount) {
        if (IsOpen()) {
            Close();
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        windowHandle.Open(name, newWidth, newHeight, flags);
        
        windowHandle.SetUserPointer(this);

        if (glfwCreateWindowSurface(SGF::VulkanInstance, (GLFWwindow*)windowHandle.GetHandle(), SGF::VulkanAllocator, &surface) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_SURFACE);
        }

        auto& device = Device::Get();
        assert(device.IsCreated());
        if (!device.CheckSurfaceSupport(surface)) {
            SGF::fatal("device is missing surface support!");
        }
        presentQueue = device.GetPresentQueue();
        if (!(flags & WINDOW_FLAG_VSYNC)) {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
        UpdateSwapchain();

        // Set GLFW callbacks
        glfwSetFramebufferSizeCallback((GLFWwindow*)windowHandle.GetHandle(), [](GLFWwindow* window, int width, int height) {
            WindowHandle& windowHandle = *(WindowHandle*)&window;
            SGF::info("framebuffersizecallback ....");
            

            if (width == 0 || height == 0) {
                SGF::info("window is minimized!");
                WindowMinimizeEvent event(windowHandle, true);
				EventManager::Dispatch(event);
                do {
                    glfwGetFramebufferSize(window, &width, &height);
                    glfwWaitEvents();
                    SGF::info("polled events finished!");
                } while (width == 0 || height == 0);
                WindowMinimizeEvent maxEvent(windowHandle, false);
                EventManager::Dispatch(maxEvent);
                SGF::info("window is maximized again!");
            }
			auto pWindow = (Window*)glfwGetWindowUserPointer(window);
            if (pWindow != nullptr) {
                Window& win = *pWindow;
                Device::Get().WaitIdle();
                win.ResizeFramebuffers(width, height);
            }
            WindowResizeEvent event(windowHandle, width, height);
            EventManager::Dispatch(event);
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
        
        attachments.push_back(CreateSwapchainAttachment(loadOp));
        clearValues.push_back(Vk::CreateColorClearValue(0.f, 0.f, 0.f, 0.f));
        VkFormat depthFormat = VK_FORMAT_D16_UNORM;
        if (WINDOW_FLAG_STENCIL_ATTACHMENT & flags) {
            depthFormat = device.GetSupportedFormat(POSSIBLE_STENCIL_FORMATS, ARRAY_SIZE(POSSIBLE_STENCIL_FORMATS), VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }
        if (flags & (WINDOW_FLAG_STENCIL_ATTACHMENT | WINDOW_FLAG_DEPTH_ATTACHMENT)) {
			auto depthAttachment = Vk::CreateDepthAttachment(depthFormat, multisampleCount);
            attachments.push_back(depthAttachment);
            depthRef.attachment = 1;
            subpass.pDepthStencilAttachment = &depthRef;
            clearValues.push_back(Vk::CreateDepthClearValue(1.f, 0));
        }
        if (multisampleCount != VK_SAMPLE_COUNT_1_BIT) {
            subpass.pResolveAttachments = &resolveRef;
            attachments.push_back(Vk::CreateAttachmentDescription(surfaceFormat.format, multisampleCount, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, loadOp));
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorRef.attachment = depthRef.attachment + 1;
            clearValues.push_back(Vk::CreateColorClearValue(0.f, 2.f, 2.f, 0.f));
        }
        VkSubpassDependency dependency = { VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 
            0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };
        SetRenderPass(attachments.data(), clearValues.data(), (uint32_t)clearValues.size(), &subpass, 1, &dependency, 1);
    }
    void Window::Close() {
        if (IsOpen()) {
            Device::Get().WaitIdle();
            FreeAttachmentData();
            Device::Get().Destroy(swapchain, renderPass);
            vkDestroySurfaceKHR(SGF::VulkanInstance, surface, SGF::VulkanAllocator);
            windowHandle.Close();
            surface = nullptr;
        }
    }

    const char* Window::GetName() const {
        return windowHandle.GetTitle();
    }

    bool Window::IsFullscreen() const {
        return windowHandle.IsFullscreen();
    }
    bool Window::IsMinimized() const {
        return (width == 0 && height == 0);
    }
    void Window::SetFullscreen() {
        windowHandle.SetFullscreen();
    }
    void Window::SetWindowed(uint32_t newWidth, uint32_t newHeight) {
        width = newWidth;
        height = newHeight;
        if (windowHandle.IsFullscreen()) {
            windowHandle.SetWindowed(newWidth, newHeight);
        }
        else {
            windowHandle.Resize(newWidth, newHeight);
        }
    }
    void Window::Minimize() {
        width = 0;
        height = 0;
        windowHandle.Minimize();
    }
    bool Window::ShouldClose() const {
        return windowHandle.ShouldClose();
    }
    
    bool Window::IsKeyPressed(Keycode key) const {
        return windowHandle.IsKeyPressed(key);
    }
    bool Window::IsMousePressed(Mousecode button) const {
        return windowHandle.IsMouseButtonPressed(button);
    }
    glm::dvec2 Window::GetCursorPos() const {
        return windowHandle.GetCursorPos();
    }

    std::string Window::OpenFileDialog(const FileFilter& filter) const {
		return OpenFileDialog(1, &filter);
	}
	std::string Window::OpenFileDialog(uint32_t filterCount, const FileFilter* pFilters) const {
		NFD_Init();

		nfdu8char_t* outPath;
		nfdopendialogu8args_t args = { 0 };

		std::string filepath;
        if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)windowHandle.GetHandle(), &args.parentWindow)) {
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
	std::string Window::SaveFileDialog(const FileFilter& filter) const
	{
		return SaveFileDialog(1, &filter);
	}
	std::string Window::SaveFileDialog(uint32_t filterCount, const FileFilter* pFilters) const
	{
		NFD_Init();

		nfdu8char_t* outPath;
		nfdsavedialogu8args_t args = {};
		if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)windowHandle.GetHandle(), &args.parentWindow)) {
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

    void Window::FreeAttachmentData() {
		assert(attachmentData != nullptr);
		DestroyFramebuffers();
        free(attachmentData);
        info("freed memory!");
		attachmentData = nullptr;
	}
    void Window::SetRenderPass(const VkAttachmentDescription* pAttachments, const VkClearValue* pClearValues, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount) {
        auto& device = Device::Get();
        if (attachmentData != nullptr) {
            FreeAttachmentData();
        }
        if (renderPass != nullptr) {
            device.Destroy(renderPass);
        }
        renderPass = device.CreateRenderPass(pAttachments, attCount, pSubpasses, subpassCount, pDependencies, dependencyCount);
        AllocateAttachmentData(pAttachments, pClearValues, attCount, pSubpasses, subpassCount);
    }

    void Window::UpdateSwapchain() {
		imageCount = 0;
		const auto& device = Device::Get();
		presentMode = device.PickPresentMode(surface, presentMode);
		surfaceFormat = device.PickSurfaceFormat(surface, surfaceFormat);
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

		swapchain = device.CreateSwapchain(info);
		device.GetSwapchainImages(swapchain, &imageCount, nullptr);
		if (old != VK_NULL_HANDLE) {
			device.Destroy(old);
		}
		assert(imageCount != 0);
	}

	void Window::AllocateAttachmentData(const VkAttachmentDescription* pAttachments, const VkClearValue* pClearValues, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
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

		memcpy(GetClearValuesMod(), pClearValues, sizeof(pClearValues[0]) * attCount);
		if (attachmentCount != 0) {
			auto formats = GetAttachmentFormatsMod();
			auto usages = GetAttachmentUsagesMod();
			auto samples = GetAttachmentSampleCountsMod();
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
		CreateFramebuffers();
	}
	void Window::CreateFramebuffers() {
		SGF::debug("creating framebuffers of swapchain!");
		auto& dev = Device::Get();
		assert(attachmentData != nullptr);
        {
            uint32_t count = imageCount;
		    dev.GetSwapchainImages(swapchain, &count, GetImagesMod());
            if (count != imageCount) {
                fatal(ERROR_CREATE_SWAPCHAIN);
            }
        }
		VkImageView* views = GetImageViewsMod();
		auto images = GetSwapchainImages();
		VkImageViewCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = FLAG_NONE;
		info.format = GetImageFormat();
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
			const VkFormat* attFormats = GetAttachmentFormats();
			const VkImageUsageFlags* attUsages = GetAttachmentUsages();
			const VkSampleCountFlagBits* attSamples = GetAttachmentSampleCounts();
			VkImage* attImages = GetAttachmentImagesMod();
			VkImageView* attImageViews = GetAttachmentImageViewsMod();
			for (uint32_t i = 0; i < attachmentCount; ++i) {
				attImages[i] = dev.CreateImage2D(width, height, attFormats[i], attUsages[i], attSamples[i]);
			}
			auto& memory = GetAttachmentMemory();
			memory = dev.AllocateMemory(attImages, attachmentCount);
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
				attImageViews[i] = dev.CreateImageView(info);
			}
			
			for (size_t i = 1; i < attachmentViews.size(); ++i) {
				attachmentViews[i] = attImageViews[i-1];
			}
		}
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		auto framebuffers = GetFramebuffersMod();
		info.format = GetImageFormat();
		for (uint32_t i = 0; i < imageCount; ++i) {
			info.image = images[i];
			views[i] = dev.CreateImageView(info);
			attachmentViews[0] = views[i];
			framebuffers[i] = dev.CreateFramebuffer(renderPass, attachmentViews.data(), attachmentViews.size(), width, height, 1);
		}
	}
    void Window::DestroyFramebuffers() {
		SGF::debug("destroying framebuffers of swapchain!");
		auto& dev = Device::Get(); 
		assert(attachmentData != nullptr);
		auto imageViews = GetSwapchainImageViews();
		auto framebuffers = GetFramebuffers();
		for (uint32_t i = 0; i < imageCount; ++i) {
			dev.Destroy(imageViews[i], framebuffers[i]);
		}
		for (uint32_t i = 0; i < attachmentCount; ++i) {
			auto attachmentImages = GetAttachmentImages();
			auto attachmentImageViews = GetAttachmentImageViews();
			dev.Destroy(attachmentImages[i], attachmentImageViews[i]);
		}
		if (attachmentCount != 0) {
			dev.Destroy(GetAttachmentMemory());
		}
	}
    void Window::UpdateFramebuffers() {
        assert(attachmentData != nullptr);
        Device::Get().WaitIdle();
        DestroyFramebuffers();
        UpdateSwapchain();
        CreateFramebuffers();
    }
}