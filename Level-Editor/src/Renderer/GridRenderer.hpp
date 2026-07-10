#pragma once

#include <SGF/Core/GPU.hpp>

namespace SGF {
	class GridRenderer {
	public:
		void Init(GPU::RenderPass renderPass, uint32_t subpass, GPU::DescriptorSetLayout uniformLayout);
		inline GridRenderer(GPU::RenderPass renderPass, uint32_t subpass, GPU::DescriptorSetLayout uniformLayout) { Init(renderPass, subpass, uniformLayout); }
		inline GridRenderer() : pipeline(nullptr), pipelineLayout(nullptr) {}
		~GridRenderer();
		void Draw(GPU::CommandList commands, GPU::DescriptorSet uniformSet, uint32_t width, uint32_t height);
	private:
		GPU::GraphicsPipeline pipeline;
		GPU::PipelineLayout pipelineLayout;
	};
}
