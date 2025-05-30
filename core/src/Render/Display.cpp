#include "Render/Display.hpp"
#include "Render/Device.hpp"

namespace SGF {
	Display Instance;
	void Display::updateFramebuffers(VkSurfaceKHR surface, uint32_t w, uint32_t h) {
		destroyFramebuffers();
		swapchain.update(surface, w, h);
		createFramebuffers(w, h);
	}
	void Display::updateRenderPass(VkSurfaceKHR surface, uint32_t w, uint32_t h, const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount) {
		const auto& device = Device::Get();
		if (renderPass != VK_NULL_HANDLE) {
			device.destroy(renderPass);
			renderPass = VK_NULL_HANDLE;
		}
		renderPass = device.renderPass(pAttachments, attCount, pSubpasses, subpassCount, pDependencies, dependencyCount);
		if (attachmentData != nullptr) {
			freeAttachmentData();
		}
		if (!swapchain.isInitialized()) {
			swapchain.create(surface, w, h);
		}
		else {
			swapchain.update(surface, w, h);
		}
		allocateAttachmentData(w, h, pAttachments, attCount, pSubpasses, subpassCount);
	}

	void Display::create(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR mode, const VkAttachmentDescription* pAttachments, uint32_t attCount,
		const VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
		SGF::debug("creating swapchain object!");
		assert(attCount > 0);
		attachmentData = nullptr;
		attachmentCount = attCount-1;
		destroy();
		swapchain.create(surface, width, height, mode);
		if (pAttachments == nullptr) {
			assert(attCount == 0);
			assert(pSubpasses == 0);
			assert(subpassCount == 0);
			VkAttachmentDescription swapchainAttachment = { FLAG_NONE, swapchain.getImageFormat(), VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
			VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			VkSubpassDescription subpass = { FLAG_NONE, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorRef, nullptr, nullptr, 0, nullptr };
			createRenderPass(&swapchainAttachment, 1, &subpass, 1);
			allocateAttachmentData(width, height, &swapchainAttachment, 1, &subpass, 1);
		}
		else {
			createRenderPass(pAttachments, attCount, pSubpasses, subpassCount);
			allocateAttachmentData(width, height, pAttachments, attCount, pSubpasses, subpassCount);
		}
	}
	Display::Display(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR mode, const VkAttachmentDescription* pAttachments, uint32_t attCount, 
		const VkSubpassDescription* pSubpasses, uint32_t subpassCount) : swapchain(surface, width, height, mode) {
		create(surface, width, height, mode, pAttachments, attCount, pSubpasses, subpassCount);
	}

	void Display::destroy() {
		if (swapchain.isInitialized()) {
			swapchain.destroy();
		}
		if (renderPass != VK_NULL_HANDLE) {
			freeAttachmentData();
			auto& dev = Device::Get(); 
			dev.destroy(renderPass);
			renderPass = VK_NULL_HANDLE;
		}
	}
	Display::~Display() {
		if (renderPass != VK_NULL_HANDLE) {
			freeAttachmentData();
			auto& dev = Device::Get(); 
			dev.destroy(renderPass);
		}
	}

	void Display::createRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
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
		att[0].format = swapchain.getImageFormat();
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
		const auto& device = Device::Get();
		renderPass = device.renderPass(att.data(), att.size(), pSubpasses, subpassCount, dependencies.data(), dependencies.size());
		attachmentCount = attCount - 1;
	}
	VkImage* Display::getImagesMod() const {
		assert(attachmentData != nullptr);
		return (VkImage*)attachmentData;
	}
	VkImageView* Display::getImageViewsMod() const {
		assert(attachmentData != nullptr);
		return (VkImageView*)((char*)getImagesMod() + sizeof(VkImage) * getImageCount());
	}
	VkFramebuffer* Display::getFramebuffersMod() const {
		assert(attachmentData != nullptr);
		return (VkFramebuffer*)((char*)getImageViewsMod() + sizeof(VkImageView) * getImageCount());
	}
	VkImage* Display::getAttachmentImagesMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkImage*)((char*)getFramebuffersMod() + sizeof(VkFramebuffer) * getImageCount());
	}
	VkImageView* Display::getAttachmentImageViewsMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkImageView*)((char*)getAttachmentImagesMod() +  sizeof(VkImage) * getAttachmentCount());
	}
	VkDeviceMemory& Display::getAttachmentMemory() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return *(VkDeviceMemory*)((char*)getAttachmentImageViewsMod() + sizeof(VkImageView) * getAttachmentCount());
	}
	VkFormat* Display::getAttachmentFormatsMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkFormat*)((char*)getAttachmentImageViewsMod() + sizeof(VkImageView) * getAttachmentCount() + sizeof(VkDeviceMemory));
	}
	VkImageUsageFlags* Display::getAttachmentUsagesMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkImageUsageFlags*)((char*)getAttachmentFormatsMod() + sizeof(VkFormat) * getAttachmentCount());
	}
	VkSampleCountFlagBits* Display::getAttachmentSampleCountsMod() const {
		assert(attachmentData != nullptr);
		assert(attachmentCount != 0);
		return (VkSampleCountFlagBits*)((char*)getAttachmentUsagesMod() + sizeof(VkImageUsageFlags) * getAttachmentCount());
	}

	void Display::freeAttachmentData() {
		assert(attachmentData != nullptr);
		destroyFramebuffers();
		delete[] attachmentData;
		attachmentData = nullptr;
	}

	void Display::allocateAttachmentData(uint32_t width, uint32_t height, const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
		assert(attCount > 0);
		attachmentCount = attCount - 1;
		assert(attachmentData == nullptr);
		size_t allocSize = (sizeof(VkImage) + sizeof(VkFramebuffer) + sizeof(VkImageView)) * swapchain.getImageCount();
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
		createFramebuffers(width, height);
	}
	void Display::destroyFramebuffers() {
		SGF::debug("destroying framebuffers of swapchain!");
		auto& dev = Device::Get(); 
		assert(attachmentData != nullptr);
		auto imageViews = getSwapchainImageViews();
		auto framebuffers = getFramebuffers();
		for (uint32_t i = 0; i < swapchain.getImageCount(); ++i) {
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
	void Display::createFramebuffers(uint32_t width, uint32_t height) {
		SGF::debug("creating framebuffers of swapchain!");
		auto& dev = Device::Get();
		assert(attachmentData != nullptr);
		swapchain.loadImages(getImagesMod());
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
		for (uint32_t i = 0; i < swapchain.getImageCount(); ++i) {
			info.image = images[i];
			views[i] = dev.imageView(info);
			attachmentViews[0] = views[i];
			framebuffers[i] = dev.framebuffer(renderPass, attachmentViews.data(), attachmentViews.size(), width, height, 1);
		}
	}
}