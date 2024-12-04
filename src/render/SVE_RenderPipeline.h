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
public:
	SveRenderPipeline(const char* vertexFileName, const char* fragmentFileName, const VkPipelineVertexInputStateCreateInfo& vertexInfo, const VkBuffer* uniformBuffers, VkSampler imageSampler, VkImageView imageView);
	~SveRenderPipeline();
	void create(const VkBuffer* uniformBuffers, VkSampler imageSampler, VkImageView imageView);
	void destroy();
	inline void bindPipeline(VkCommandBuffer commands) {
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineHandle);
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[SVE::getImageIndex()], 0, nullptr);
	}
	void recreatePipeline();
private:
	void createGraphicPipeline();
};