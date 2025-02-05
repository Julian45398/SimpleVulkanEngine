#pragma once


#include "core.h"
#include "SVE_Backend.h"
#include "util.h"

class SveRenderPipeline {
private:
	inline static uint32_t windowResizeCallbackFunctionIndex = UINT32_MAX;
	const char* vertShaderFile;
	const char* fragShaderFile;
	const VkPipelineVertexInputStateCreateInfo& vertexInput;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipelineHandle;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL; 
	VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	uint32_t callbackListenerIndex;
public:
	SveRenderPipeline(const char* vertexFileName, const char* fragmentFileName, const VkPipelineVertexInputStateCreateInfo& vertexInfo, uint32_t layoutCount, const VkDescriptorSetLayout* descriptorLayouts,
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT, VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	~SveRenderPipeline();
	void create(uint32_t layoutCount, const VkDescriptorSetLayout* descriptorLayouts);
	void destroy();
	inline void bindPipeline(VkCommandBuffer commands) {
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineHandle);
		//vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[SVE::getImageIndex()], 0, nullptr);
		VkViewport viewport = { (float)SVE::getViewportOffsetX(), (float)SVE::getViewportOffsetY(), (float)SVE::getViewportWidth(), (float)SVE::getViewportHeight(), 0.0f, 1.0f};
		VkRect2D scissor = { {SVE::getViewportOffsetX(), SVE::getViewportOffsetY()}, {SVE::getViewportWidth(), SVE::getViewportHeight()}};
		vkCmdSetViewport(commands, 0, 1, &viewport);
		vkCmdSetScissor(commands, 0, 1, &scissor);
	}
	inline const VkPipelineLayout getLayout() const { return pipelineLayout; }
	inline const VkPipeline getHandle() const { return pipelineHandle; }
	void recreatePipeline();
	inline void setCullMode(VkCullModeFlags newCullMode) {
		cullMode = newCullMode;
		recreatePipeline();
	}
	inline void setPolygonMode(VkPolygonMode newPolygonMode) {
		polygonMode = newPolygonMode;
		recreatePipeline();
	}
private:
	void createGraphicPipeline();
};