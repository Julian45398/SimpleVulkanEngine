#pragma once

#include "SGF_Core.hpp"

#ifndef SGF_PIPELINE_MAX_DYNAMIC_STATES 
#define SGF_PIPELINE_MAX_DYNAMIC_STATES 32
#endif
#ifndef SGF_PIPELINE_MAX_PIPELINE_STAGES 
#define SGF_PIPELINE_MAX_PIPELINE_STAGES 4
#endif
#ifndef SGF_PIPELINE_MAX_COLOR_BLEND_ATTACHMENTS
#define SGF_PIPELINE_MAX_COLOR_BLEND_ATTACHMENTS 4
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
		inline GraphicsPipelineBuilder& PolygonMode(VkPolygonMode mode = VK_POLYGON_MODE_FILL) { rasterizationState.polygonMode = mode; return *this; }
		inline GraphicsPipelineBuilder& Topology(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) { inputAssemblyState.topology = topology; return *this; }
		inline GraphicsPipelineBuilder& Viewport(float width, float height, float xOffset = 0.f, float yOffset = 0.f, float minDepth = 0.0f, float maxDepth = 1.0f) { stViewport = { xOffset, yOffset, width, height, minDepth, maxDepth };  return *this; }
		inline GraphicsPipelineBuilder& Scissor(uint32_t width, uint32_t height, int32_t xOffset = 0, int32_t yOffset = 0) { stScissor = { {xOffset, yOffset}, {width, height} };  return *this; }
		inline GraphicsPipelineBuilder& Depth(bool test = true, bool write = true, VkCompareOp op = VK_COMPARE_OP_LESS) {depthStencilState.depthWriteEnable = (VkBool32)write; depthStencilState.depthTestEnable = (VkBool32)test; depthStencilState.depthCompareOp = op; return*this; }
		inline GraphicsPipelineBuilder& DynamicState(VkDynamicState state) { dynamicStates[dynamicStateInfo.dynamicStateCount] = state; dynamicStateInfo.dynamicStateCount++; return *this; }
		inline GraphicsPipelineBuilder& SampleCount(VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT) { multisampleState.rasterizationSamples = sampleCount; return *this; }
		inline GraphicsPipelineBuilder& FrontFace(VkFrontFace front = VK_FRONT_FACE_COUNTER_CLOCKWISE) { rasterizationState.frontFace = front; return *this; }
		inline GraphicsPipelineBuilder& SetColorBlendAttachment(bool blendEnable = false, VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD, VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			VkBlendOp colorBlendOp = VK_BLEND_OP_ADD, VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		{ auto& s = colorBlendAttachmentStates[colorBlendState.attachmentCount-1]; s.blendEnable = blendEnable; s.alphaBlendOp = alphaBlendOp; s.srcAlphaBlendFactor = srcAlphaBlendFactor; s.dstAlphaBlendFactor = dstAlphaBlendFactor; 
			s.colorBlendOp = colorBlendOp; s.colorWriteMask = colorWriteMask; s.srcColorBlendFactor = srcColorBlendFactor; s.dstColorBlendFactor = dstColorBlendFactor; return *this; }
		inline GraphicsPipelineBuilder& AddColorBlendAttachment(bool blendEnable = false, VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD, VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			VkBlendOp colorBlendOp = VK_BLEND_OP_ADD, VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		{  colorBlendState.attachmentCount++; return SetColorBlendAttachment(blendEnable, colorWriteMask, alphaBlendOp, srcAlphaBlendFactor, dstAlphaBlendFactor, colorBlendOp, srcColorBlendFactor, dstColorBlendFactor); }
		
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
		VkPipelineColorBlendAttachmentState colorBlendAttachmentStates[SGF_PIPELINE_MAX_COLOR_BLEND_ATTACHMENTS];
		VkPipelineShaderStageCreateInfo pipelineStages[SGF_PIPELINE_MAX_PIPELINE_STAGES];
		VkViewport stViewport;
		VkRect2D stScissor;
		VkDynamicState dynamicStates[SGF_PIPELINE_MAX_DYNAMIC_STATES];
		const Device* pDevice;
	};
}