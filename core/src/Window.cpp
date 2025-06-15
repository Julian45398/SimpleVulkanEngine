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
            glfwGetFramebufferSize((GLFWwindow*)window, &w, &h);
            while (w == 0 || h == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize((GLFWwindow*)window, &w, &h);
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
            glfwGetFramebufferSize((GLFWwindow*)window, &w, &h);
            while (w == 0 || h == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize((GLFWwindow*)window, &w, &h);
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
        if (flags & WINDOW_FLAG_RESIZABLE) {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
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
        glfwSetFramebufferSizeCallback((GLFWwindow*)window, [](GLFWwindow* window, int width, int height) {
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
            Device::Get().waitIdle();
            win.resizeFramebuffers(width, height);
            WindowResizeEvent event(win, width, height);
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
                } else if (focus == GLFW_FALSE) {
                    SGF::info("Window: ", win->getName(), " lost focus");
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
            glfwDestroyWindow((GLFWwindow*)window);
            window = nullptr;
            surface = nullptr;
        }
    }

    const char* Window::getName() const {
        return glfwGetWindowTitle((GLFWwindow*)window);
    }

    bool Window::isFullscreen() const {
        return glfwGetWindowMonitor((GLFWwindow*)window) != nullptr;
    }
    bool Window::isMinimized() {
        return (width == 0 && height == 0);
    }
    void Window::setFullscreen() {
        if (glfwGetWindowMonitor((GLFWwindow*)window) != nullptr) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const auto mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor((GLFWwindow*)window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
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
    void Window::onUpdate() {
        glfwPollEvents();
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

    
    VkImage* Window::getImagesMod() const {
		assert(attachmentData != nullptr);
		return (VkImage*)attachmentData;
	}
	VkImageView* Window::getImageViewsMod() const {
		assert(attachmentData != nullptr);
		return (VkImageView*)((char*)getImagesMod() + sizeof(VkImage) * getImageCount());
	}
	VkFramebuffer* Window::getFramebuffersMod() const {
		assert(attachmentData != nullptr);
		return (VkFramebuffer*)((char*)getImageViewsMod() + sizeof(VkImageView) * getImageCount());
	}
	VkImage* Window::getAttachmentImagesMod() const {
		assert(attachmentData != nullptr);
		return (VkImage*)((char*)getFramebuffersMod() + sizeof(VkFramebuffer) * getImageCount());
	}
	VkImageView* Window::getAttachmentImageViewsMod() const {
		assert(attachmentData != nullptr);
		return (VkImageView*)((char*)getAttachmentImagesMod() +  sizeof(VkImage) * getAttachmentCount());
	}
	VkDeviceMemory& Window::getAttachmentMemory() const {
		assert(attachmentData != nullptr);
		return *(VkDeviceMemory*)((char*)getAttachmentImageViewsMod() + sizeof(VkImageView) * getAttachmentCount());
	}
	VkFormat* Window::getAttachmentFormatsMod() const {
		assert(attachmentData != nullptr);
		return (VkFormat*)((char*)getAttachmentImageViewsMod() + sizeof(VkImageView) * getAttachmentCount() + sizeof(VkDeviceMemory));
	}
	VkImageUsageFlags* Window::getAttachmentUsagesMod() const {
		assert(attachmentData != nullptr);
		return (VkImageUsageFlags*)((char*)getAttachmentFormatsMod() + sizeof(VkFormat) * getAttachmentCount());
	}
	VkSampleCountFlagBits* Window::getAttachmentSampleCountsMod() const {
		assert(attachmentData != nullptr);
		return (VkSampleCountFlagBits*)((char*)getAttachmentUsagesMod() + sizeof(VkImageUsageFlags) * getAttachmentCount());
	}

	VkClearValue* Window::getClearValuesMod() const {
		assert(attachmentData != nullptr);
		return (VkClearValue*)((char*)getAttachmentSampleCountsMod() + sizeof(VkSampleCountFlagBits) * getAttachmentCount());
	}
    void Window::freeAttachmentData() {
		assert(attachmentData != nullptr);
		destroyFramebuffers();
		delete[] attachmentData;
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
		assert(attachmentData == nullptr);
		size_t allocSize = (sizeof(VkImage) + sizeof(VkFramebuffer) + sizeof(VkImageView)) * imageCount + sizeof(VkClearValue) * attCount;
		if (attachmentCount != 0) {
			allocSize += (sizeof(VkImage) + sizeof(VkImageView) + sizeof(VkFormat) + sizeof(VkImageUsageFlags) + sizeof(VkSampleCountFlags)) * attachmentCount + sizeof(VkDeviceMemory);
		}
		attachmentData = new char[allocSize];

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