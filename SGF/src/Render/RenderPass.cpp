#include "Render/RenderPass.hpp"
#include "Render/Device.hpp"


namespace SGF {

	RenderPass::Builder::Builder(const Device* device) : pDevice(device) {
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = FLAG_NONE;
		info.attachmentCount = 0;
		info.pAttachments = attachmentDescriptions;
		info.subpassCount = 0;
		info.pSubpasses = subpasses;
		info.dependencyCount = 0;
		info.pDependencies = dependencies;

		for (size_t i = 0; i < ARRAY_SIZE(dependencies); ++i) {
			dependencies[i] = {};
		}
		for (size_t i = 0; i < ARRAY_SIZE(attachmentDescriptions); ++i) {
			attachmentDescriptions[i] = {};
		}
		for (size_t i = 0; i < ARRAY_SIZE(colorAttachments); ++i) {
			colorAttachments[i] = {};
		}
		for (size_t i = 0; i < ARRAY_SIZE(inputAttachments); ++i) {
			inputAttachments[i] = {};
		}
		for (size_t i = 0; i < ARRAY_SIZE(preserveAttachments); ++i) {
			preserveAttachments[i] = {};
		}
		for (size_t i = 0; i < ARRAY_SIZE(resolveAttachments); ++i) {
			resolveAttachments[i] = {};
		}
		for (size_t i = 0; i < ARRAY_SIZE(depthAttachments); ++i) {
			depthAttachments[i] = {};
		}
		inputOffset = 0;
		colorOffset = 0;
		depthOffset = 0;
		resolveOffset = 0;
		preserveOffset = 0;

		subpasses[0] = {};
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	}
	RenderPass RenderPass::Builder::build() {
		createDependency(0, info.subpassCount);
		info.subpassCount++;
		return RenderPass(pDevice->CreateRenderPass(info));
	}
	RenderPass::Builder& RenderPass::Builder::addSubpass() {
		if (info.subpassCount != 0) {
			createDependency(info.subpassCount - 1, info.subpassCount);
		}
		info.subpassCount++;
		auto& s = subpasses[info.subpassCount];
		s.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		s.pInputAttachments = nullptr; //&inputAttachments[inputOffset];
		s.pColorAttachments = nullptr;// & colorAttachments[colorOffset];
		s.pDepthStencilAttachment = nullptr;// = &depthAttachments[depthOffset];
		s.pResolveAttachments = nullptr;//&resolveAttachments[resolveOffset];
		s.pPreserveAttachments = nullptr;//&preserveAttachments[preserveOffset];
		subpasses[0].colorAttachmentCount = 0;
		subpasses[0].inputAttachmentCount = 0;
		subpasses[0].preserveAttachmentCount = 0;
		return *this;
	}
	RenderPass::Builder& RenderPass::Builder::addDependency(uint32_t dependencySubpass, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkDependencyFlags dependencyFlags) {
		assert(info.dependencyCount < SGF_RENDER_PASS_MAX_SUBPASS_DEPENDENCIES);
		auto& dep = dependencies[info.dependencyCount];
		dep.srcSubpass = dependencySubpass;
		dep.dstSubpass = info.subpassCount;
		dep.dependencyFlags = dependencyFlags;
		dep.srcAccessMask = srcAccessMask;
		dep.dstAccessMask = dstAccessMask;
		dep.srcStageMask = srcStageMask;
		dep.dstStageMask = dstStageMask;
		info.dependencyCount++;
		return *this;
	}
	RenderPass::Builder& RenderPass::Builder::addInputAttachment(uint32_t attachment, VkImageLayout layout) {
		assert(inputOffset < SGF_RENDER_PASS_MAX_INPUT_ATTACHMENTS);
		if (subpasses[info.subpassCount].inputAttachmentCount == 0) {
			subpasses[info.subpassCount].pInputAttachments = &inputAttachments[inputOffset];
		}
		inputAttachments[inputOffset].attachment = attachment;
		inputAttachments[inputOffset].layout = layout;
		subpasses[info.subpassCount].inputAttachmentCount++;
		inputOffset++;
		return *this;
	}
	RenderPass::Builder& RenderPass::Builder::addColorAttachment(uint32_t attachment, VkImageLayout layout) {
		assert(colorOffset < SGF_RENDER_PASS_MAX_COLOR_ATTACHMENTS);
		if (subpasses[info.subpassCount].colorAttachmentCount == 0) {
			subpasses[info.subpassCount].pColorAttachments = &colorAttachments[colorOffset];
		}
		colorAttachments[colorOffset].attachment = attachment;
		colorAttachments[colorOffset].layout = layout;
		subpasses[info.subpassCount].colorAttachmentCount++;
		colorOffset++;
		return *this;
	}
	RenderPass::Builder& RenderPass::Builder::addDepthAttachment(uint32_t attachment, VkImageLayout layout) {
		assert(depthOffset < SGF_RENDER_PASS_MAX_DEPTH_ATTACHMENTS);
		subpasses[info.subpassCount].pDepthStencilAttachment = &depthAttachments[depthOffset];
		depthAttachments[depthOffset].attachment = attachment;
		depthAttachments[depthOffset].layout = layout;
		depthOffset++;
		return *this;
	}
	RenderPass::Builder& RenderPass::Builder::addResolveAttachment(uint32_t attachment, VkImageLayout layout) {
		assert(resolveOffset < SGF_RENDER_PASS_MAX_RESOLVE_ATTACHMENTS);
		if (subpasses[info.subpassCount].pResolveAttachments == nullptr) {
			subpasses[info.subpassCount].pResolveAttachments = &resolveAttachments[resolveOffset];
		}
		resolveAttachments[resolveOffset].attachment = attachment;
		resolveAttachments[resolveOffset].layout = layout;
		resolveOffset++;
		return *this;
	}
	RenderPass::Builder& RenderPass::Builder::addPreserveAttachent(uint32_t attachment) {
		assert(preserveOffset < SGF_RENDER_PASS_MAX_PRESERVE_ATTACHMENTS);
		if (subpasses[info.subpassCount].preserveAttachmentCount == 0) {
			subpasses[info.subpassCount].pPreserveAttachments = &preserveAttachments[preserveOffset];
		}
		preserveAttachments[preserveOffset] = attachment;
		subpasses[info.subpassCount].preserveAttachmentCount++;
		preserveOffset++;
		return *this;
	}
	RenderPass::Builder& RenderPass::Builder::addAttachmentDescription(VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, 
		VkSampleCountFlagBits samples, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp, VkAttachmentDescriptionFlags flags) {
		auto& ad = attachmentDescriptions[info.attachmentCount];
		ad.format = format;
		ad.initialLayout = initialLayout;
		ad.finalLayout = finalLayout;
		ad.loadOp = loadOp;
		ad.storeOp = storeOp;
		ad.stencilLoadOp = stencilLoadOp;
		ad.stencilStoreOp = stencilStoreOp;
		ad.samples = samples;
		ad.flags = flags;
		info.attachmentCount++;
		return *this;
	}
	void RenderPass::Builder::createDependency(uint32_t srcSubpass, uint32_t dstSubpass) {
		auto& dep = dependencies[info.dependencyCount];
		const auto& dst = subpasses[dstSubpass];
		const auto& src = subpasses[srcSubpass];
		dep.dependencyFlags = FLAG_NONE;
		dep.dstSubpass = dstSubpass;
		dep.srcSubpass = srcSubpass;
		dep.srcAccessMask = 0;
		dep.dstAccessMask = 0;
		dep.srcStageMask = 0;
		dep.dstStageMask = 0;
		for (uint32_t i = 0; i < dst.colorAttachmentCount; ++i) {
			const auto& dstColor = dst.pColorAttachments[dst.colorAttachmentCount];
			for (uint32_t j = 0; j < src.inputAttachmentCount; ++j) {
				const auto& srcInput = src.pInputAttachments[src.inputAttachmentCount];
				if (srcInput.attachment == dstColor.attachment) {
					dep.srcAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
					dep.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dep.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dep.srcStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
			}
			for (uint32_t j = 0; j < src.colorAttachmentCount; ++j) {
				const auto& srcColor = src.pColorAttachments[src.colorAttachmentCount];
				if (srcColor.attachment == dstColor.attachment) {
					dep.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dep.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dep.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dep.srcStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
			}
		}
		for (uint32_t i = 0; i < dst.inputAttachmentCount; ++i) {
			const auto& dstInput = dst.pInputAttachments[dst.colorAttachmentCount];
			for (uint32_t j = 0; j < src.colorAttachmentCount; ++j) {
				const auto& srcColor = src.pColorAttachments[src.colorAttachmentCount];
				if (srcColor.attachment == dstInput.attachment) {
					dep.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dep.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
					dep.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					dep.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				}
			}
			if (src.pDepthStencilAttachment != nullptr) {
				const auto& srcDepth = *src.pDepthStencilAttachment;
				if (srcDepth.attachment == dstInput.attachment) {
					dep.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					dep.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
					dep.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					dep.srcStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
			}
		}
	}
}