#pragma once


#include "engine_core.h"
#include "util.h"

class SveRenderPipeline {
	VkPipelineLayout pipelineLayout;
	VkPipeline pipelineHandle;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	inline void create(const char* vertexFileName, const char* fragmentFileName, const VkPipelineVertexInputStateCreateInfo& vertexInfo) {
		pipelineLayout = vkl::createPipelineLayout(Core.LogicalDevice, 1, &descriptorSetLayout);
		pipelineHandle = util::createGraphicPipeline(vertexFileName, fragmentFileName, pipelineLayout, vertexInfo);
	}
};