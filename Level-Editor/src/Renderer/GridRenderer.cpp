#include "GridRenderer.hpp"

namespace SGF {
	const char GRID_VERTEX_SHADER_FILE[] = "shaders/grid.vert";
	const char GRID_FRAGMENT_SHADER_FILE[] = "shaders/grid.frag";
	//const VkPipelineVertexInputStateCreateInfo GRID_VERTEX_INPUT_INFO = vkl::createPipelineVertexInputStateInfo(0, nullptr, 0, nullptr);

	void GridRenderer::Init(GPU::RenderPass renderPass, uint32_t subpass, GPU::DescriptorSetLayout uniformLayout) {
		pipelineLayout = GPU::CreatePipelineLayout(&uniformLayout, 1);
		pipeline = GPU::CreateGraphicsPipeline(pipelineLayout, renderPass, subpass)
			.VertexShader(GRID_VERTEX_SHADER_FILE).FragmentShader(GRID_FRAGMENT_SHADER_FILE).AddColorBlendAttachment(false, GPU::ColorComponent::NONE)
			.DynamicState(GPU::DynamicState::VIEWPORT).DynamicState(GPU::DynamicState::SCISSOR).Depth(true, false, GPU::CompareOp::LESS).Build();
	}

	GridRenderer::~GridRenderer() {
		GPU::Destroy(pipelineLayout, pipeline);
	}
	void GridRenderer::Draw(GPU::CommandList commands, GPU::DescriptorSet uniformDescriptor, uint32_t width, uint32_t height) {
		commands.BindPipeline(pipeline);
		commands.SetViewport((float)width, (float)height, 0, 0, 0.0f, 1.0f);
		commands.SetScissor(width, height, 0, 0);
		commands.BindDescriptorSet(GPU::PipelineType::GRAPHICS , pipelineLayout ,0, uniformDescriptor);
		commands.Draw(6);
	}
}