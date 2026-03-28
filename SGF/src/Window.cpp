#include "SGF_Core.hpp"
#include "Window.hpp"
#include "Render/Device.hpp"
#include "Layers/LayerStack.hpp"
#include "Render/CommandList.hpp"
#include "Render/RenderPass.hpp"
#include "Filesystem/File.hpp"

#ifdef SGF_OS_WINDOWS 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(SGF_OS_LINUX)
#ifdef SGF_USE_X11
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(SGF_USE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#elif defined(SGF_OS_APPLE)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <nfd.h>
#include <nfd_glfw3.h>

#include "Events/Event.hpp"
#include <algorithm>

#ifndef SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT
#define SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT 1000000000
#endif

namespace SGF {
    extern VkInstance g_VulkanInstance;
    extern VkAllocationCallbacks* g_VulkanAllocator;

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
			Log::Debug("swapchain out of date!");
            int w = 0, h = 0;
            glfwGetFramebufferSize((GLFWwindow*)GetHandle(), &w, &h);
            while (w == 0 || h == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize((GLFWwindow*)GetHandle(), &w, &h);
            }
            width = w;
            height = h;
            UpdateFramebuffers();
		    res = vkAcquireNextImageKHR(device, swapchain, SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT, imageAvailableSignal, fence, &imageIndex);
		}
        if (res == VK_SUBOPTIMAL_KHR) {
            Log::Debug("swapchain image suboptimal");
        } else if (res != VK_SUCCESS) {
			Log::Fatal(ERROR_ACQUIRE_NEXT_IMAGE);
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
			SGF::Log::Info("swapchain out of date!");
            int w = 0;
            int h = 0;
            glfwGetFramebufferSize((GLFWwindow*)GetHandle(), &w, &h);
            while (w == 0 || h == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize((GLFWwindow*)GetHandle(), &w, &h);
            }
            width = w;
            height = h;
            UpdateFramebuffers();
		}
		else if (result != VK_SUCCESS) {
			SGF::Log::Fatal(ERROR_PRESENT_IMAGE);
		}
	}

    void Window::Open(const char* name, uint32_t newWidth, uint32_t newHeight, WindowCreateFlags flags, VkSampleCountFlagBits multisampleCount) {
        if (IsOpen()) {
            Close();
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        WindowHandle::Open(name, newWidth, newHeight, flags);
        
        SetUserPointer(this);

        if (glfwCreateWindowSurface(SGF::g_VulkanInstance, (GLFWwindow*)GetHandle(), SGF::g_VulkanAllocator, &surface) != VK_SUCCESS) {
            SGF::Log::Fatal(ERROR_CREATE_SURFACE);
        }

        auto& device = Device::Get();
        assert(device.IsCreated());
        if (!device.CheckSurfaceSupport(surface)) {
            SGF::Log::Fatal("device is missing surface support!");
        }
        presentQueue = device.GetPresentQueue();
        if (!(flags & WINDOW_FLAG_VSYNC)) {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
        UpdateSwapchain();

        // Set GLFW callbacks
        glfwSetFramebufferSizeCallback((GLFWwindow*)GetHandle(), [](GLFWwindow* window, int width, int height) {
            WindowHandle& windowHandle = *(WindowHandle*)&window;
            SGF::Log::Debug("framebuffersizecallback ....");
            

            if (width == 0 || height == 0) {
                SGF::Log::Debug("window is minimized!");
                WindowMinimizeEvent event(windowHandle, true);
				EventManager::Dispatch(event);
                do {
                    glfwGetFramebufferSize(window, &width, &height);
                    glfwWaitEvents();
                    SGF::Log::Debug("polled events finished!");
                } while (width == 0 || height == 0);
                WindowMinimizeEvent maxEvent(windowHandle, false);
                EventManager::Dispatch(maxEvent);
                SGF::Log::Debug("window is maximized again!");
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
            vkDestroySurfaceKHR(SGF::g_VulkanInstance, surface, SGF::g_VulkanAllocator);
            WindowHandle::Close();
            surface = nullptr;
        }
    }

    bool Window::IsMinimized() const {
        assert((WindowHandle::IsMinimized() && width == 0 && height == 0) || (!WindowHandle::IsMinimized() && (width != 0 || height != 0)));
        return (width == 0 && height == 0);
    }
    void Window::SetWindowed(uint32_t newWidth, uint32_t newHeight) {
        width = newWidth;
        height = newHeight;
        if (WindowHandle::IsFullscreen()) {
            WindowHandle::SetWindowed(newWidth, newHeight);
        }
        else {
            WindowHandle::Resize(newWidth, newHeight);
        }
    }
    void Window::Minimize() {
        width = 0;
        height = 0;
        WindowHandle::Minimize();
    }
    void Window::FreeAttachmentData() {
		assert(attachmentData != nullptr);
		DestroyFramebuffers();
        free(attachmentData);
        SGF::Log::Debug("freed memory!");
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
            SGF::Log::Fatal("failed to allocate attachment data!");
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
		SGF::Log::Debug("creating framebuffers of swapchain!");
		auto& dev = Device::Get();
		assert(attachmentData != nullptr);
        {
            uint32_t count = imageCount;
		    dev.GetSwapchainImages(swapchain, &count, GetImagesMod());
            if (count != imageCount) {
                SGF::Log::Fatal(ERROR_CREATE_SWAPCHAIN);
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
		SGF::Log::Debug("destroying framebuffers of swapchain!");
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