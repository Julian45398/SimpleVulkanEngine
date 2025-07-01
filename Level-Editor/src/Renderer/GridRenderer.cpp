#include "GridRenderer.hpp"

namespace SGF {
	const char GRID_VERTEX_SHADER_FILE[] = "shaders/grid.vert";
	const char GRID_FRAGMENT_SHADER_FILE[] = "shaders/grid.frag";
	//const VkPipelineVertexInputStateCreateInfo GRID_VERTEX_INPUT_INFO = vkl::createPipelineVertexInputStateInfo(0, nullptr, 0, nullptr);

	void GridRenderer::Init(VkRenderPass renderPass, uint32_t subpass, VkDescriptorSetLayout uniformLayout) {
		auto& device = SGF::Device::Get();
		pipelineLayout = device.CreatePipelineLayout(&uniformLayout, 1);
		pipeline = device.CreateGraphicsPipeline(pipelineLayout, renderPass, subpass)
			.VertexShader(GRID_VERTEX_SHADER_FILE).FragmentShader(GRID_FRAGMENT_SHADER_FILE)
			.DynamicState(VK_DYNAMIC_STATE_VIEWPORT).DynamicState(VK_DYNAMIC_STATE_SCISSOR).Depth(true, false, VK_COMPARE_OP_LESS).Build();
	}

	GridRenderer::~GridRenderer() {
		auto& device = SGF::Device::Get();
		device.Destroy(pipelineLayout, pipeline);
	}
	void GridRenderer::Draw(VkCommandBuffer commands, VkDescriptorSet uniformDescriptor, uint32_t width, uint32_t height) {
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		VkViewport viewport = { (float)0.0f, (float)0.0f, (float)width, (float)height, 0.0f, 1.0f };
		VkRect2D scissor = { {0, 0}, {width, height} };
		vkCmdSetViewport(commands, 0, 1, &viewport);
		vkCmdSetScissor(commands, 0, 1, &scissor);
		
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &uniformDescriptor, 0, nullptr);
		vkCmdDraw(commands, 6, 1, 0, 0);
	}
}