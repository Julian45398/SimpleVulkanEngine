#pragma once

#include "SGF_Core.hpp"

#ifndef SGF_PIPELINE_MAX_DYNAMIC_STATES 
#define SGF_PIPELINE_MAX_DYNAMIC_STATES 32
#endif
#ifndef SGF_PIPELINE_MAX_PIPELINE_STAGES 
#define SGF_PIPELINE_MAX_PIPELINE_STAGES 4
#endif

namespace SGF {
	class GraphicsPipelineBuilder {
		inline static const VkPipelineVertexInputStateCreateInfo VERTEX_INPUT_NONE = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, FLAG_NONE, 0, nullptr, 0, nullptr };
	public:
		VkPipeline build();
		inline GraphicsPipelineBuilder& layout(VkPipelineLayout layout) { info.layout = layout; return *this; }
		inline GraphicsPipelineBuilder& layout(VkRenderPass renderPass, uint32_t subpass = 0) { info.renderPass = renderPass; info.subpass = subpass; return *this; }
		GraphicsPipelineBuilder& geometryShader(const char* filename);
		GraphicsPipelineBuilder& tesselationControlShader(const char* filename);
		GraphicsPipelineBuilder& tesselationEvaluationShader(const char* filename);
		GraphicsPipelineBuilder& vertexShader(const char* filename);
		GraphicsPipelineBuilder& fragmentShader(const char* filename);
		inline GraphicsPipelineBuilder& vertexInput(const VkPipelineVertexInputStateCreateInfo& inputState) { info.pVertexInputState = &inputState; return *this; }
		inline GraphicsPipelineBuilder& polygonMode(VkPolygonMode mode) { rasterizationState.polygonMode = mode; return *this; }
		inline GraphicsPipelineBuilder& topology(VkPrimitiveTopology topology) { inputAssemblyState.topology = topology; return *this; }
		inline GraphicsPipelineBuilder& viewport(float width, float height, float xOffset = 0.f, float yOffset = 0.f, float minDepth = 0.0f, float maxDepth = 1.0f) { stViewport = { xOffset, yOffset, width, height, minDepth, maxDepth };  return *this; }
		inline GraphicsPipelineBuilder& scissor(uint32_t width, uint32_t height, int32_t xOffset = 0, int32_t yOffset = 0) { stScissor = { {xOffset, yOffset}, {width, height} };  return *this; }
		inline GraphicsPipelineBuilder& depth(bool test, bool write, VkCompareOp op = VK_COMPARE_OP_LESS) {depthStencilState.depthWriteEnable = (VkBool32)write; depthStencilState.depthTestEnable = (VkBool32)test; depthStencilState.depthCompareOp = op; return*this; }
		inline GraphicsPipelineBuilder& dynamicState(VkDynamicState state) { dynamicStates[dynamicStateInfo.dynamicStateCount] = state; dynamicStateInfo.dynamicStateCount++; return *this; }
		inline GraphicsPipelineBuilder& sampleCount(VkSampleCountFlagBits sampleCount) { multisampleState.rasterizationSamples = sampleCount; return *this; }
		~GraphicsPipelineBuilder();
	private:
		GraphicsPipelineBuilder(const Device* device, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass);
		void addShaderStage(const char* filename, VkShaderStageFlagBits stage);
		friend Device;
		VkGraphicsPipelineCreateInfo info;
		//VkPipelineVertexInputStateCreateInfo vertexInputState;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		VkPipelineTessellationStateCreateInfo tessellationState;
		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineRasterizationStateCreateInfo rasterizationState;
		VkPipelineMultisampleStateCreateInfo multisampleState;
		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		VkPipelineColorBlendStateCreateInfo colorBlendState;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
		VkPipelineShaderStageCreateInfo pipelineStages[SGF_PIPELINE_MAX_PIPELINE_STAGES];
		VkViewport stViewport;
		VkRect2D stScissor;
		VkDynamicState dynamicStates[SGF_PIPELINE_MAX_DYNAMIC_STATES];
		const Device* pDevice;
	};
}