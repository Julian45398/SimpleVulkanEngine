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
#ifndef SGF_RENDER_PASS_MAX_ATTACHMENT_REFERENCES
#define SGF_RENDER_PASS_MAX_ATTACHMENT_REFERENCES 8
#endif

namespace SGF {
	class RenderPass {
	public:
		inline operator VkRenderPass() { return handle; }
		inline VkRenderPass getHandle() { return handle; }
		class Builder {
		public:
			RenderPass build();

		private:
			VkRenderPassCreateInfo info;
			VkSubpassDescription subpasses[SGF_RENDER_PASS_MAX_SUBPASSES];
			VkAttachmentDescription attachmentDescriptions[SGF_RENDER_PASS_MAX_ATTACHMENT_DESCRIPTIONS];
			VkSubpassDependency dependencies[SGF_RENDER_PASS_MAX_SUBPASS_DEPENDENCIES];
			VkAttachmentReference references[SGF_RENDER_PASS_MAX_ATTACHMENT_REFERENCES];
		};
	private:
		friend Device;
		VkRenderPass handle;
	};
}