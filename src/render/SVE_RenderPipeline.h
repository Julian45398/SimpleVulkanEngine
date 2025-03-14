#pragma once

#include "core.h"

class SveRenderPipelineBuilder {
	inline static constexpr uint32_t MAX_SHADER_STAGES = 3;
public:
	SveRenderPipelineBuilder(const char* vertexShaderFile, const char* fragmentShaderFile, VkPipelineLayout pipelineLayout, const VkPipelineVertexInputStateCreateInfo& vertexInputState); 	
	~SveRenderPipelineBuilder();
	VkPipeline build() const;

	inline SveRenderPipelineBuilder& setPrimitiveTopology(VkPrimitiveTopology topology) {
		inputAssembly.topology = topology;
		return *this;
	}
	inline SveRenderPipelineBuilder& setPolygonMode(VkPolygonMode polygonMode) {
		rasterizationState.polygonMode = polygonMode;
		return *this;
	}
	inline SveRenderPipelineBuilder& setDepthTest(VkBool32 depthTestEnable) {
		depthStencilState.depthTestEnable = depthTestEnable;
		return *this;
	}
	inline SveRenderPipelineBuilder& setDepthWrite(VkBool32 depthWriteEnable) {
		depthStencilState.depthWriteEnable = depthWriteEnable;
		return *this;
	}
	inline SveRenderPipelineBuilder& setDepthCompare(VkCompareOp depthCompare) {
		depthStencilState.depthCompareOp = depthCompare;
		return *this;
	}

	SveRenderPipelineBuilder& defaultMultisampleState();
	SveRenderPipelineBuilder& defaultDepthStencilState();
	SveRenderPipelineBuilder& defaultRasterizationState();
	SveRenderPipelineBuilder& defaultDynamicState();
	SveRenderPipelineBuilder& defaultInputAssemblyState();
	SveRenderPipelineBuilder& defaultColorBlend();
private:
	SveRenderPipelineBuilder& addShaderStage(const VkPipelineShaderStageCreateInfo& shaderStageInfo);

	VkGraphicsPipelineCreateInfo createInfo;
	VkPipelineShaderStageCreateInfo shaderStages[MAX_SHADER_STAGES];
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizationState;
	VkPipelineMultisampleStateCreateInfo multisampleState;
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	//VkPipelineTessellationStateCreateInfo tessellationState;
	VkPipelineColorBlendStateCreateInfo colorBlendState;
	VkPipelineDynamicStateCreateInfo dynamicState;
	std::vector<VkDynamicState> dynamicStates;
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendStates;
	std::vector<VkViewport> viewports;
	std::vector<VkRect2D> scissor;
	uint32_t shaderStageCount;
};

class SveRenderPipeline {
public:
	SveRenderPipeline(const char* vertexFileName, const char* fragmentFileName, const VkPipelineVertexInputStateCreateInfo& vertexInfo,
		uint32_t layoutCount, const VkDescriptorSetLayout* descriptorLayouts, uint32_t pushConstantCount, const VkPushConstantRange* ranges,
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT, VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	~SveRenderPipeline();

	inline void bindPipeline(VkCommandBuffer commands, uint32_t setCount, const VkDescriptorSet* sets) {
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineHandle);
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, setCount, sets, 0, nullptr);
	}
	inline const VkPipelineLayout getLayout() const { return pipelineLayout; }
	inline const VkPipeline getHandle() const { return pipelineHandle; }
private:
	VkPipelineLayout pipelineLayout;
	VkPipeline pipelineHandle;
};