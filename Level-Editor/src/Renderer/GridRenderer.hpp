#pragma once

#include <SGF.hpp>

namespace SGF {
	class GridRenderer {
	public:
		void Init(VkRenderPass renderPass, uint32_t subpass, VkDescriptorSetLayout uniformLayout);
		inline GridRenderer(VkRenderPass renderPass, uint32_t subpass, VkDescriptorSetLayout uniformLayout) { Init(renderPass, subpass, uniformLayout); }
		inline GridRenderer() : pipeline(nullptr), pipelineLayout(nullptr) {}
		~GridRenderer();
		void Draw(VkCommandBuffer commands, VkDescriptorSet uniformSet, uint32_t width, uint32_t height);
	private:
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
	};
}
