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
		VkPipeline Build();
		inline GraphicsPipelineBuilder& Layout(VkPipelineLayout layout) { info.layout = layout; return *this; }
		inline GraphicsPipelineBuilder& Layout(VkRenderPass renderPass, uint32_t subpass = 0) { info.renderPass = renderPass; info.subpass = subpass; return *this; }
		GraphicsPipelineBuilder& GeometryShader(const char* filename);
		GraphicsPipelineBuilder& TesselationControlShader(const char* filename);
		GraphicsPipelineBuilder& TesselationEvaluationShader(const char* filename);
		GraphicsPipelineBuilder& VertexShader(const char* filename);
		GraphicsPipelineBuilder& FragmentShader(const char* filename);
		inline GraphicsPipelineBuilder& VertexInput(const VkPipelineVertexInputStateCreateInfo& inputState) { info.pVertexInputState = &inputState; return *this; }
		inline GraphicsPipelineBuilder& PolygonMode(VkPolygonMode mode) { rasterizationState.polygonMode = mode; return *this; }
		inline GraphicsPipelineBuilder& Topology(VkPrimitiveTopology topology) { inputAssemblyState.topology = topology; return *this; }
		inline GraphicsPipelineBuilder& Viewport(float width, float height, float xOffset = 0.f, float yOffset = 0.f, float minDepth = 0.0f, float maxDepth = 1.0f) { stViewport = { xOffset, yOffset, width, height, minDepth, maxDepth };  return *this; }
		inline GraphicsPipelineBuilder& Scissor(uint32_t width, uint32_t height, int32_t xOffset = 0, int32_t yOffset = 0) { stScissor = { {xOffset, yOffset}, {width, height} };  return *this; }
		inline GraphicsPipelineBuilder& Depth(bool test, bool write, VkCompareOp op = VK_COMPARE_OP_LESS) {depthStencilState.depthWriteEnable = (VkBool32)write; depthStencilState.depthTestEnable = (VkBool32)test; depthStencilState.depthCompareOp = op; return*this; }
		inline GraphicsPipelineBuilder& DynamicState(VkDynamicState state) { dynamicStates[dynamicStateInfo.dynamicStateCount] = state; dynamicStateInfo.dynamicStateCount++; return *this; }
		inline GraphicsPipelineBuilder& SampleCount(VkSampleCountFlagBits sampleCount) { multisampleState.rasterizationSamples = sampleCount; return *this; }
		inline GraphicsPipelineBuilder& FrontFace(VkFrontFace front) { rasterizationState.frontFace = front; return *this; }
		//inline GraphicsPipelineBuilder& Rotation() 
		~GraphicsPipelineBuilder();
	private:
		GraphicsPipelineBuilder(const Device* device, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass);
		void AddShaderStage(const char* filename, VkShaderStageFlagBits stage);
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