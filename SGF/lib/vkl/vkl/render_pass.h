#pragma once

#include "vkl_core.h"

namespace vkl {
	inline VkAttachmentReference createAttachmentReference(uint32_t attachment, VkImageLayout layout) {
		return { attachment, layout };
	}
	inline VkAttachmentDescription createAttachmentDescription(VkFormat format, VkSampleCountFlagBits samples, 
		VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, 
		VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp, 
		VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentDescriptionFlags flags = VKL_FLAG_NONE) {
		return {
			flags, format, samples,
			loadOp, storeOp,
			stencilLoadOp, stencilStoreOp,
			initialLayout, finalLayout
		};
	}
	inline VkSubpassDescription createSubpassDescription(VkPipelineBindPoint pipelineBindPoint, 
		uint32_t inputAttachmentCount, const VkAttachmentReference* pInputAttachments,
		uint32_t colorAttachmentCount, const VkAttachmentReference* pColorAttachments, const VkAttachmentReference* pResolveAttachments,
		const VkAttachmentReference& pDepthStencilAttachment,
		uint32_t preserveAttachmentCount, const uint32_t* pPreserveAttachments, VkSubpassDescriptionFlags flags = VKL_FLAG_NONE) {
		return {
			flags, pipelineBindPoint,
			inputAttachmentCount, pInputAttachments,
			colorAttachmentCount, pColorAttachments, pResolveAttachments,
			&pDepthStencilAttachment,
			preserveAttachmentCount, pPreserveAttachments
		};
	}
	inline VkSubpassDependency createSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass,
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
		VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
		VkDependencyFlags dependencyFlags = VKL_FLAG_NONE) {
		return {
			srcSubpass, dstSubpass,
			srcStageMask, dstStageMask,
			srcAccessMask, dstAccessMask,
			dependencyFlags
		};
	}
	inline VkRenderPassCreateInfo createRenderPassInfo(uint32_t attachmentCount, const VkAttachmentDescription* pAttachments, uint32_t subpassCount, const VkSubpassDescription* pSubpasses, uint32_t dependencyCount, const VkSubpassDependency* pDependencies,
		VkRenderPassCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			pNext, flags, attachmentCount, pAttachments, subpassCount, pSubpasses, dependencyCount, pDependencies
		};
	}
	inline VkRenderPass createRenderPass(VkDevice device, const VkRenderPassCreateInfo& createInfo) {
		VkRenderPass render_pass = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateRenderPass(device, &createInfo, VKL_Callbacks, &render_pass), VKL_ERROR_RENDER_PASS_CREATION_FAILED);
		return render_pass;
	}
	inline VkRenderPass createRenderPass(VkDevice device, uint32_t attachmentCount, const VkAttachmentDescription* pAttachments, uint32_t subpassCount, const VkSubpassDescription* pSubpasses, uint32_t dependencyCount, const VkSubpassDependency* pDependencies,
		VkRenderPassCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		VkRenderPassCreateInfo info = createRenderPassInfo(attachmentCount, pAttachments, subpassCount, pSubpasses, dependencyCount, pDependencies, flags, pNext);
		VkRenderPass render_pass = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateRenderPass(device, &info, VKL_Callbacks, &render_pass), VKL_ERROR_RENDER_PASS_CREATION_FAILED);
		return render_pass;
	}
	inline void destroyRenderPass(VkDevice device, VkRenderPass renderPass) {
		vkDestroyRenderPass(device, renderPass, VKL_Callbacks);
	}
	inline VkFramebufferCreateInfo createFramebufferInfo(VkRenderPass renderPass, uint32_t attachmentCount, const VkImageView* pAttachments, uint32_t width, uint32_t height, uint32_t layers,
		VkFramebufferCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			pNext, flags, renderPass, attachmentCount, pAttachments, width, height, layers
		};
	}
	inline VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass renderPass, uint32_t attachmentCount, const VkImageView* pAttachments, uint32_t width, uint32_t height, uint32_t layers,
		VkFramebufferCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		VkFramebufferCreateInfo info = createFramebufferInfo(renderPass, attachmentCount, pAttachments, width, height, layers, flags, pNext);
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateFramebuffer(device, &info, VKL_Callbacks, &framebuffer), VKL_ERROR_FRAMEBUFFER_CREATION_FAILED);
		return framebuffer;
	}
	inline VkFramebuffer createFramebuffer(VkDevice device, const VkFramebufferCreateInfo& info) {
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VKL_CHECK(vkCreateFramebuffer(device, &info, VKL_Callbacks, &framebuffer), VKL_ERROR_FRAMEBUFFER_CREATION_FAILED);
		return framebuffer;
	}
	inline void destroyFramebuffer(VkDevice device, VkFramebuffer framebuffer) {
		vkDestroyFramebuffer(device, framebuffer, VKL_Callbacks);
	}
}