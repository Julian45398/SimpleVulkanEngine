#include "SVE_GridRenderer.h"

#include "SVE_Backend.h"
#include "SVE_RenderPipeline.h"


const char GRID_VERTEX_SHADER_FILE[] = "shaders/grid.vert";
const char GRID_FRAGMENT_SHADER_FILE[] = "shaders/grid.frag";
const VkPipelineVertexInputStateCreateInfo GRID_VERTEX_INPUT_INFO = vkl::createPipelineVertexInputStateInfo(0, nullptr, 0, nullptr);

SveGridRenderer::SveGridRenderer(VkDescriptorSetLayout uniformLayout) {
	pipelineLayout = vkl::createPipelineLayout(SVE::getDevice(), 1, &uniformLayout);
	pipeline = SveRenderPipelineBuilder(GRID_VERTEX_SHADER_FILE, GRID_FRAGMENT_SHADER_FILE, pipelineLayout, GRID_VERTEX_INPUT_INFO).setDepthTest(VK_TRUE).build();
	//pipeline = SVE::createRenderPipeline(GRID_VERTEX_SHADER_FILE, GRID_FRAGMENT_SHADER_FILE, pipelineLayout, GRID_VERTEX_INPUT_INFO, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT);
}

SveGridRenderer::~SveGridRenderer() {
	SVE::destroyPipelineLayout(pipelineLayout);
	SVE::destroyPipeline(pipeline);
}
void SveGridRenderer::draw(VkCommandBuffer commands, VkDescriptorSet uniformDescriptor) {
	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkViewport viewport = { (float)SVE::getViewportOffsetX(), (float)SVE::getViewportOffsetY(), (float)SVE::getViewportWidth(), (float)SVE::getViewportHeight(), 0.0f, 1.0f };
	VkRect2D scissor = { {SVE::getViewportOffsetX(), SVE::getViewportOffsetY()}, {SVE::getViewportWidth(), SVE::getViewportHeight()} };
	vkCmdSetViewport(commands, 0, 1, &viewport);
	vkCmdSetScissor(commands, 0, 1, &scissor);
	
	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &uniformDescriptor, 0, nullptr);
	vkCmdDraw(commands, 6, 1, 0, 0);
}
