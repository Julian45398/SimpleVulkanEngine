#include "Render/Swapchain.hpp"
#include "Render/Device.hpp"
#include "Window.hpp"

#ifndef SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT
#define SGF_SWAPCHAIN_NEXT_IMAGE_TIMEOUT 1000000000
#endif

namespace SGF {
	
	extern VkAllocationCallbacks* VulkanAllocator;
	void Swapchain::nextImage(VkSemaphore imageAvailable, VkFence fence) {
		const auto& device = getDevice();
		if (vkAcquireNextImageKHR(device.logical, handle, 1000000000, imageAvailable, fence, &currentImageIndex) != VK_SUCCESS) {
			fatal(ERROR_ACQUIRE_NEXT_IMAGE);
		}
	}
	void Swapchain::presentImage(const VkSemaphore* pWaitSemaphores, uint32_t count) const {
		VkPresentInfoKHR info;
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;
		info.swapchainCount = 1;
		info.pImageIndices = &currentImageIndex;
		info.pSwapchains = &handle;
		info.waitSemaphoreCount = count;
		info.pWaitSemaphores = pWaitSemaphores;
		info.pResults = nullptr;
		if (vkQueuePresentKHR(presentQueue, &info) != VK_SUCCESS) {
			fatal(ERROR_PRESENT_IMAGE);
		}
	}
	void Swapchain::presentImage(VkSemaphore waitSemaphore) const {
		VkPresentInfoKHR info;
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;
		info.swapchainCount = 1;
		info.pImageIndices = &currentImageIndex;
		info.pSwapchains = &handle;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &waitSemaphore;
		info.pResults = nullptr;
		if (vkQueuePresentKHR(presentQueue, &info) != VK_SUCCESS) {
			fatal(ERROR_PRESENT_IMAGE);
		}
	}
	void Swapchain::updateSwapchain(const Window& window) {
		updateSwapchain(window, window.getWidth(), window.getHeight());
	}
	void Swapchain::updateSwapchain(const Window& window, uint32_t w, uint32_t h) {
		destroyFramebuffers();
		createSwapchain(window, w, h);
		createFramebuffers();
	}
	void Swapchain::updateRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
		const auto& device = getDevice();
		device.waitIdle();
		if (renderPass != VK_NULL_HANDLE) {
			device.destroy(renderPass);
			renderPass = VK_NULL_HANDLE;
		}
		createRenderPass(pAttachments, attCount, pSubpasses, subpassCount);
		if (attachmentData != nullptr) {
			freeAttachmentData();
		}
		allocateAttachmentData(pAttachments, attCount, pSubpasses, subpassCount);
		//createFramebuffers();
	}
	Swapchain::Swapchain(const Device& device, const Window& window, VkPresentModeKHR mode, const VkAttachmentDescription* pAttachments, uint32_t attCount, 
		const VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
		SGF::debug("creating swapchain object!");
		presentMode = mode;
		presentQueue = device.presentQueue();
		surfaceFormat = DEFAULT_SURFACE_FORMAT;
		attachmentData = nullptr;
		handle = VK_NULL_HANDLE;
		createSwapchain(window, window.getWidth(), window.getHeight());
		if (pAttachments == nullptr) {
			assert(attCount == 0);
			assert(pSubpasses == 0);
			assert(subpassCount == 0);
			VkAttachmentDescription swapchainAttachment = { FLAG_NONE, surfaceFormat.format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
			VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			VkSubpassDescription subpass = { FLAG_NONE, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorRef, nullptr, nullptr, 0, nullptr };
			createRenderPass(&swapchainAttachment, 1, &subpass, 1);
			allocateAttachmentData(&swapchainAttachment, 1, &subpass, 1);
		}
		else {
			createRenderPass(pAttachments, attCount, pSubpasses, subpassCount);
			allocateAttachmentData(pAttachments, attCount, pSubpasses, subpassCount);
		}
		//updateSwapchain(window);
		//createFramebuffers();
	}
	void Swapchain::destroy() {
		if (!isInitialized()) {
			return;
		}
		SGF::debug("swapchain destroy called!");
		freeAttachmentData();
		auto& dev = getDevice(); 
		dev.destroy(handle, renderPass);
		handle = VK_NULL_HANDLE;
		renderPass = VK_NULL_HANDLE;
		height = 0;
		width = 0;
		swapchainImageCount = 0;
		currentImageIndex = 0;
		presentQueue = VK_NULL_HANDLE;
	}
	void Swapchain::createSwapchain(const Window& window, uint32_t w, uint32_t h) {
		currentImageIndex = 0;
		swapchainImageCount = 0;
		const auto& device = getDevice();
		presentMode = device.pickPresentMode(window.getSurface(), presentMode);
		surfaceFormat = device.pickSurfaceFormat(window.getSurface(), surfaceFormat);
        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical, window.getSurface(), &capabilities);
        swapchainImageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && swapchainImageCount > capabilities.maxImageCount) {
            swapchainImageCount = capabilities.maxImageCount;
        }
        width = std::min(capabilities.maxImageExtent.width, std::max(w, capabilities.minImageExtent.width));
        height = std::min(capabilities.maxImageExtent.height, std::max(h, capabilities.minImageExtent.height));
		if (width != w || height != h) {
			SGF::warn("swapchain doesnt have requested dimensions! dimensions: { ", width, ", ", height, " }, requested: { ", w, ", ", h, " }");
		}
        info.oldSwapchain = handle;
        info.surface = window.getSurface();
        info.minImageCount = swapchainImageCount;
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

		handle = device.swapchain(info);
		vkGetSwapchainImagesKHR(device.logical, handle, &swapchainImageCount, nullptr);
		assert(swapchainImageCount != 0);
	}
	void Swapchain::createRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
		assert(attCount > 0);
		std::vector<VkSubpassDependency> dependencies(subpassCount);
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstAccessMask = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstStageMask = 0;
		std::vector<VkAttachmentDescription> att(attCount);
		for (size_t i = 0; i < attCount; ++i) {
			att[i] = pAttachments[i];
		}
		att[0].format = surfaceFormat.format;
		att[0].samples = VK_SAMPLE_COUNT_1_BIT;
			
		if (pSubpasses[0].colorAttachmentCount != 0) {
			dependencies[0].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		if (pSubpasses[0].pDepthStencilAttachment != nullptr) {
			dependencies[0].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			dependencies[0].dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		if (pSubpasses[0].inputAttachmentCount != 0) {
			dependencies[0].dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			dependencies[0].dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].srcStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		for (uint32_t i = 1; i < subpassCount; ++i) {
			dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies[i].srcSubpass = i - 1;
			dependencies[i].dstSubpass = i;
			dependencies[i].srcAccessMask = 0;
			dependencies[i].dstAccessMask = 0;
			dependencies[i].srcStageMask = 0;
			dependencies[i].dstStageMask = 0;
			for (uint32_t j = 0; j < pSubpasses[i].colorAttachmentCount; ++j) {
				const auto& dstAtt = pSubpasses[i].pColorAttachments[j];
				for (uint32_t k = 0; k < pSubpasses[i - 1].colorAttachmentCount; ++k) {
					const auto& srcAtt = pSubpasses[i - 1].pColorAttachments[k];
					if (srcAtt.attachment == dstAtt.attachment) {
						dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						dependencies[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					}
					if (pSubpasses[i - 1].pResolveAttachments != nullptr) {
						const auto& srcAttRes = pSubpasses[i - 1].pResolveAttachments[k];
						if (srcAttRes.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						}
					}
				}
				for (uint32_t k = 0; k < pSubpasses[i - 1].inputAttachmentCount; ++k) {
					const auto& srcAtt = pSubpasses[i - 1].pInputAttachments[k];
					if (srcAtt.attachment == dstAtt.attachment) {
						dependencies[i].srcAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
						dependencies[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					}
				}
				if (pSubpasses[i - 1].pDepthStencilAttachment != nullptr) {
					const auto& srcAtt = pSubpasses[i - 1].pDepthStencilAttachment[0];
					if (srcAtt.attachment == dstAtt.attachment) {
						dependencies[i].srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						dependencies[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
						dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					}
				}
			}
			for (uint32_t j = 0; j < pSubpasses[i].inputAttachmentCount; ++j) {
				const auto& dstAtt = pSubpasses[i].pInputAttachments[j];
				for (uint32_t k = 0; k < pSubpasses[i - 1].colorAttachmentCount; ++k) {
					const auto& srcAtt = pSubpasses[i - 1].pColorAttachments[k];
					if (srcAtt.attachment == dstAtt.attachment) {
						dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						dependencies[i].dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
						dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					}
					if (pSubpasses[i - 1].pResolveAttachments != nullptr) {
						const auto& srcAttRes = pSubpasses[i - 1].pResolveAttachments[k];
						if (srcAttRes.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						}
					}
				}
				if (pSubpasses[i - 1].pDepthStencilAttachment != nullptr) {
					const auto& srcAtt = pSubpasses[i - 1].pDepthStencilAttachment[0];
					if (srcAtt.attachment == dstAtt.attachment) {
						dependencies[i].srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						dependencies[i].dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
						dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
						dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					}
				}
			}
			if (pSubpasses[i].pDepthStencilAttachment != nullptr) {
				const auto& dstAtt = pSubpasses[i].pDepthStencilAttachment[0];
				for (uint32_t k = 0; k < pSubpasses[i - 1].colorAttachmentCount; ++k) {
					const auto& srcAtt = pSubpasses[i - 1].pColorAttachments[k];
					if (srcAtt.attachment == dstAtt.attachment) {
						dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						dependencies[i].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
						dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					}
					if (pSubpasses[i - 1].pResolveAttachments != nullptr) {
						const auto& srcAttRes = pSubpasses[i - 1].pResolveAttachments[k];
						if (srcAttRes.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						}
					}
				}
				if (pSubpasses[i - 1].pDepthStencilAttachment != nullptr) {
					const auto& srcAtt = pSubpasses[i - 1].pDepthStencilAttachment[0];
					if (srcAtt.attachment == dstAtt.attachment) {
						dependencies[i].srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						dependencies[i].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
						dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					}
				}
			}
		}
		const auto& device = getDevice();
		renderPass = device.renderPass(att.data(), att.size(), pSubpasses, subpassCount, dependencies.data(), dependencies.size());
		attachmentCount = attCount - 1;
	}
	VkImage* Swapchain::getImagesMod() const {
		assert(attachmentData != nullptr);
		return (VkImage*)attachmentData;
	}
	VkImageView* Swapchain::getImageViewsMod() const {
		assert(attachmentData != nullptr);
		return (VkImageView*)((char*)getImagesMod() + sizeof(VkImage) * getImageCount());
	}
	VkFramebuffer* Swapchain::getFramebuffersMod() const {
		assert(attachmentData != nullptr);
		return (VkFramebuffer*)((char*)getImageViewsMod() + sizeof(VkImageView) * getImageCount());
	}
	VkImage* Swapchain::getAttachmentImagesMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkImage*)((char*)getFramebuffersMod() + sizeof(VkFramebuffer) * getImageCount());
	}
	VkImageView* Swapchain::getAttachmentImageViewsMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkImageView*)((char*)getAttachmentImagesMod() +  sizeof(VkImage) * getAttachmentCount());
	}
	VkDeviceMemory Swapchain::getAttachmentMemory() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return *(VkDeviceMemory*)((char*)getAttachmentImageViewsMod() + sizeof(VkImageView) * getAttachmentCount());
	}
	VkFormat* Swapchain::getAttachmentFormatsMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkFormat*)((char*)getAttachmentImageViewsMod() + sizeof(VkImageView) * getAttachmentCount() + sizeof(VkDeviceMemory));
	}
	VkImageUsageFlags* Swapchain::getAttachmentUsagesMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkImageUsageFlags*)((char*)getAttachmentFormatsMod() + sizeof(VkFormat) * getAttachmentCount());
	}
	VkSampleCountFlagBits* Swapchain::getAttachmentSampleCountsMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkSampleCountFlagBits*)((char*)getAttachmentUsagesMod() + sizeof(VkImageUsageFlags) * getAttachmentCount());
	}

	void Swapchain::freeAttachmentData() {
		assert(handle != VK_NULL_HANDLE);
		assert(attachmentData != nullptr);
		destroyFramebuffers();
		delete[] attachmentData;
		attachmentData = nullptr;
	}

	void Swapchain::allocateAttachmentData(const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
		assert(attCount == attachmentCount + 1);
		assert(attachmentData == nullptr);
		size_t allocSize = (sizeof(VkImage) + sizeof(VkFramebuffer) + sizeof(VkImageView)) * swapchainImageCount;
		if (attachmentCount != 0) {
			allocSize += (sizeof(VkImage) + sizeof(VkImageView) + sizeof(VkFormat) + sizeof(VkImageUsageFlags) + sizeof(VkSampleCountFlags)) * attachmentCount + sizeof(VkDeviceMemory);
		}
		attachmentData = new char[allocSize];
		if (attachmentCount != 0) {
			auto formats = getAttachmentFormatsMod();
			auto usages = getAttachmentUsagesMod();
			auto samples = getAttachmentSampleCountsMod();
			for (uint32_t i = 1; i < attCount; ++i) {
				formats[i-1] = pAttachments[i].format;
				usages[i-1] = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
				samples[i-1] = pAttachments[i].samples;
				for (uint32_t k = 0; k < subpassCount; ++k) {
					auto& subpass = pSubpasses[k];
					for (uint32_t l = 0; l < subpass.colorAttachmentCount; ++l) {
						if (subpass.pColorAttachments[l].attachment == i) {
							usages[i-1] |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
						}
					}
					for (uint32_t l = 0; l < subpass.inputAttachmentCount; ++l) {
						if (subpass.pInputAttachments[l].attachment == i) {
							usages[i-1] |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
						}
					}
					if (subpass.pDepthStencilAttachment != nullptr) {
						if (subpass.pDepthStencilAttachment[0].attachment == i) {
							usages[i-1] |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
						}
					}
				}
			}
		}
		createFramebuffers();
	}
	void Swapchain::destroyFramebuffers() {
		SGF::debug("destroying framebuffers of swapchain!");
		auto& dev = getDevice(); 
		assert(attachmentData != nullptr);
		assert(swapchainImageCount != 0);
		auto imageViews = getImageViews();
		auto framebuffers = getFramebuffers();
		for (uint32_t i = 0; i < swapchainImageCount; ++i) {
			dev.destroy(imageViews[i], framebuffers[i]);
		}
		for (uint32_t i = 0; i < attachmentCount; ++i) {
			auto attachmentImages = getAttachmentImages();
			auto attachmentImageViews = getAttachmentImageViews();
			dev.destroy(attachmentImages[i], attachmentImageViews[i]);
		}
	}
	void Swapchain::createFramebuffers() {
		SGF::debug("creating framebuffers of swapchain!");
		auto& dev = getDevice();
		assert(attachmentData != nullptr);
		assert(swapchainImageCount != 0);
		if (vkGetSwapchainImagesKHR(dev.logical, handle, &swapchainImageCount, getImagesMod()) != VK_SUCCESS) {
			SGF::fatal(ERROR_GET_SWAPCHAIN_IMAGES);
		}
		assert(swapchainImageCount != 0);
		VkImageView* views = getImageViewsMod();
		auto images = getImages();
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
				attImages[i] = dev.image2D(width, height, attFormats[i], attUsages[i], 1, attSamples[i]);
				if (attUsages[i] | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
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
		for (uint32_t i = 0; i < swapchainImageCount; ++i) {
			info.image = images[i];
			views[i] = dev.imageView(info);
			attachmentViews[0] = views[i];
			framebuffers[i] = dev.framebuffer(renderPass, attachmentViews.data(), attachmentViews.size(), width, height, 1);
		}
	}
}