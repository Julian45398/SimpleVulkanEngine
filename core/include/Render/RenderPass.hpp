#pragma once

#include "SGF_Core.hpp"

#ifndef SGF_RENDER_PASS_MAX_SUBPASSES
#define SGF_RENDER_PASS_MAX_SUBPASSES 8
#endif
#ifndef SGF_RENDER_PASS_MAX_ATTACHMENT_DESCRIPTIONS
#define SGF_RENDER_PASS_MAX_ATTACHMENT_DESCRIPTIONS 8
#endif
#ifndef SGF_RENDER_PASS_MAX_SUBPASS_DEPENDENCIES
#define SGF_RENDER_PASS_MAX_SUBPASS_DEPENDENCIES 8
#endif
#ifndef SGF_RENDER_PASS_MAX_INPUT_ATTACHMENTS
#define SGF_RENDER_PASS_MAX_INPUT_ATTACHMENTS 16
#endif
#ifndef SGF_RENDER_PASS_MAX_COLOR_ATTACHMENTS
#define SGF_RENDER_PASS_MAX_COLOR_ATTACHMENTS 16
#endif
#ifndef SGF_RENDER_PASS_MAX_DEPTH_ATTACHMENTS
#define SGF_RENDER_PASS_MAX_DEPTH_ATTACHMENTS SGF_RENDER_PASS_MAX_SUBPASSES
#endif
#ifndef SGF_RENDER_PASS_MAX_RESOLVE_ATTACHMENTS
#define SGF_RENDER_PASS_MAX_RESOLVE_ATTACHMENTS 8
#endif
#ifndef SGF_RENDER_PASS_MAX_PRESERVE_ATTACHMENTS
#define SGF_RENDER_PASS_MAX_PRESERVE_ATTACHMENTS 4
#endif

#include "Device.hpp"

namespace SGF {
	namespace Vk {
		inline VkAttachmentDescription CreateAttachmentDescription(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE, VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE) {
			return { FLAG_NONE, format, sampleCount, loadOp, storeOp, stencilLoadOp, stencilStoreOp, initialLayout, finalLayout };
		}
		inline VkSubpassDescription CreateSubpassDescription(const VkAttachmentReference* pColorAttachments = nullptr, uint32_t colorAttachmentCount = 0, const VkAttachmentReference* pResolveAttachments = nullptr, const VkAttachmentReference* pDepthAttachment = nullptr,
			const VkAttachmentReference* pInputAttachments = nullptr, uint32_t inputAttachmentCount = 0, const uint32_t* pPreserveAttachments = nullptr, uint32_t preserveCount = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) {
			return { FLAG_NONE, bindPoint, inputAttachmentCount, pInputAttachments, colorAttachmentCount, pColorAttachments, pResolveAttachments, pDepthAttachment, preserveCount, pPreserveAttachments };
		}
		inline VkAttachmentReference CreateAttachmentReference(uint32_t index, VkImageLayout layout) {
			return { index, layout };
		}
		inline VkAttachmentReference CreateColorAttachmentReference(uint32_t index) {
			return { index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		}
		inline VkAttachmentReference CreateDepthStencilAttachmentReference(uint32_t index) {
			return { index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		}
		inline VkAttachmentDescription CreateDepthAttachment(VkFormat format = VK_FORMAT_D16_UNORM, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE) {
			return CreateAttachmentDescription(format, sampleCount, initialLayout, finalLayout, loadOp, storeOp, loadOp, storeOp);
		}
	}

	class AttachmentDefinition {
	public:
		void addSwapchainAttachment(VkSurfaceKHR surface, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR) {
			//auto& device = getDevice();
			swapchainAttachmentIndex = (uint32_t)descriptions.size();
			VkAttachmentDescription desc = { 0, /*device.getSwapchainFormat(surface)*/VK_FORMAT_R8G8B8A8_SRGB, VK_SAMPLE_COUNT_1_BIT, loadOp, VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
			descriptions.emplace_back(desc);
		}
		inline void addColorAttachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout initialLayout, VkImageLayout finalLayout) {
			addAttachment(format, sampleCount, loadOp, storeOp,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, initialLayout, finalLayout);
		}
		inline void addDepthAttachment(VkFormat depthFormat, VkSampleCountFlagBits sampleCount, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp) {
			addAttachment(depthFormat, sampleCount, loadOp, storeOp, stencilLoadOp, stencilStoreOp, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}
		inline void addAttachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkAttachmentLoadOp stencilLoad, VkAttachmentStoreOp stencilStore, VkImageLayout initialLayout, VkImageLayout finalLayout) {
			VkAttachmentDescription desc = { 0, format, sampleCount, loadOp, storeOp,
				stencilLoad, stencilStore, initialLayout, finalLayout };
			descriptions.emplace_back(desc);
		}
	private:
		std::vector<VkAttachmentDescription> descriptions;
		std::vector<VkImageUsageFlags> imageUsages;
		uint32_t swapchainAttachmentIndex;
	};
	class RenderPass {
	public:
		inline operator VkRenderPass() { return handle; }
		inline VkRenderPass getHandle() { return handle; }
		class Builder {
		public:
			RenderPass build();
			Builder& addSubpass();
			Builder& addDependency(uint32_t dependencySubpass, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkDependencyFlags dependencyFlags = FLAG_NONE);
			Builder& addInputAttachment(uint32_t attachment, VkImageLayout layout);
			Builder& addColorAttachment(uint32_t attachment, VkImageLayout layout);
			Builder& addDepthAttachment(uint32_t attachment, VkImageLayout layout);
			Builder& addResolveAttachment(uint32_t attachment, VkImageLayout layout);
			Builder& addPreserveAttachent(uint32_t attachment);
			Builder& addAttachmentDescription(VkFormat format, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, 
				VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, VkAttachmentDescriptionFlags flags = FLAG_NONE);
		private:
			void createDependency(uint32_t srcSubpass, uint32_t dstSubpass);
			Builder(const Device* device);
			VkRenderPassCreateInfo info;
			VkSubpassDescription subpasses[SGF_RENDER_PASS_MAX_SUBPASSES];
			VkAttachmentDescription attachmentDescriptions[SGF_RENDER_PASS_MAX_ATTACHMENT_DESCRIPTIONS];
			VkSubpassDependency dependencies[SGF_RENDER_PASS_MAX_SUBPASS_DEPENDENCIES];
			VkAttachmentReference inputAttachments[SGF_RENDER_PASS_MAX_INPUT_ATTACHMENTS];
			VkAttachmentReference colorAttachments[SGF_RENDER_PASS_MAX_COLOR_ATTACHMENTS];
			VkAttachmentReference depthAttachments[SGF_RENDER_PASS_MAX_DEPTH_ATTACHMENTS];
			VkAttachmentReference resolveAttachments[SGF_RENDER_PASS_MAX_RESOLVE_ATTACHMENTS];
			uint32_t preserveAttachments[SGF_RENDER_PASS_MAX_PRESERVE_ATTACHMENTS];
			uint32_t inputOffset;
			uint32_t colorOffset;
			uint32_t depthOffset;
			uint32_t resolveOffset;
			uint32_t preserveOffset;
			friend Device;
			const Device* pDevice;
		};
	private:
		inline RenderPass(VkRenderPass renderPass) : handle(renderPass) {}
		friend Device;
		VkRenderPass handle;
	};
}