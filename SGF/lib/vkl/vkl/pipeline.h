#pragma once

#include "vkl_core.h"


namespace vkl {
	inline VkPipelineLayoutCreateInfo createPipelineLayoutCreateInfo(uint32_t setLayoutCount, const VkDescriptorSetLayout* pSetLayouts, 
		uint32_t pushConstantRangeCount, const VkPushConstantRange* pPushConstantRanges, VkPipelineLayoutCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, pNext, flags, setLayoutCount, pSetLayouts, pushConstantRangeCount, pPushConstantRanges
		};
	}
	inline VkPipelineLayout createPipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo& createInfo) {
		VkPipelineLayout layout;
		VKL_CHECK(vkCreatePipelineLayout(device, &createInfo, VKL_Callbacks, &layout), VKL_ERROR_PIPELINE_LAYOUT_CREATION_FAILED);
		return layout;
	}
	inline VkPipelineLayout createPipelineLayout(VkDevice device, uint32_t descriptorCount, const VkDescriptorSetLayout* descriptorLayouts, uint32_t pushConstantCount = 0, const VkPushConstantRange* ranges = nullptr) {
		VkPipelineLayoutCreateInfo info = createPipelineLayoutCreateInfo(descriptorCount, descriptorLayouts, pushConstantCount, ranges);
		return vkl::createPipelineLayout(device, info);
	}
	inline void destroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout) {
		vkDestroyPipelineLayout(device, pipelineLayout, VKL_Callbacks);
	}
	inline VkShaderModuleCreateInfo createShaderModuleInfo(size_t codeSize, const uint32_t* pCode, VkShaderModuleCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, pNext, flags, codeSize, pCode
		};
	}
	inline VkShaderModule createShaderModule(VkDevice device, size_t codeSize, const uint32_t* pCode, VkShaderModuleCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		VkShaderModuleCreateInfo info = createShaderModuleInfo(codeSize, pCode, flags, pNext);
		VkShaderModule module;
		VKL_CHECK(vkCreateShaderModule(device, &info, VKL_Callbacks, &module), VKL_ERROR_SHADER_MODULE_CREATION_FAILED);
		return module;
	}
	inline VkShaderModule createShaderModule(VkDevice device, const VkShaderModuleCreateInfo& info) {
		VkShaderModule module;
		VKL_CHECK(vkCreateShaderModule(device, &info, VKL_Callbacks, &module), VKL_ERROR_SHADER_MODULE_CREATION_FAILED);
		return module;
	}
	inline void destroyShaderModule(VkDevice device, VkShaderModule module) {
		vkDestroyShaderModule(device, module, VKL_Callbacks);
	}
	inline VkPipelineShaderStageCreateInfo createPipelineShaderStageInfo(VkShaderStageFlagBits stage, VkShaderModule module, const char* pName, const VkSpecializationInfo* pSpecializationInfo = nullptr, VkPipelineShaderStageCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, pNext, flags, stage, module, pName, pSpecializationInfo
		};
	}
	inline VkPipelineShaderStageCreateInfo createPipelineShaderStageInfo(VkShaderStageFlagBits stage, VkShaderModule module, const VkSpecializationInfo* pSpecializationInfo = nullptr, VkPipelineShaderStageCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, pNext, flags, stage, module, "main", pSpecializationInfo
		};
	}
	inline VkPipelineInputAssemblyStateCreateInfo createPipelineInputAssemblyInfo(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable, VkPipelineInputAssemblyStateCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, pNext, flags, topology, primitiveRestartEnable
		};
	}
	inline VkPipelineTessellationStateCreateInfo createPipelineTessellationStateInfo(uint32_t patchControlPoints, VkPipelineTessellationStateCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, pNext, flags, patchControlPoints
		};
	}
	inline VkPipelineViewportStateCreateInfo createPipelineViewportStateInfo(uint32_t viewportCount, const VkViewport* pViewports, uint32_t scissorCount, const VkRect2D* pScissors, VkPipelineViewportStateCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, pNext, flags, viewportCount, pViewports, scissorCount, pScissors
		};
	}
	inline VkPipelineRasterizationStateCreateInfo createPipelineRasterizationStateInfo(VkBool32 depthClampEnable, VkBool32 rasterizerDiscardEnable,
		VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkBool32 depthBiasEnable, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor, float lineWidth,
		VkPipelineRasterizationStateCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, pNext, flags, depthClampEnable, rasterizerDiscardEnable,
			polygonMode, cullMode, frontFace, depthBiasEnable, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor, lineWidth
		};
	}
	inline VkPipelineMultisampleStateCreateInfo createPipelineMultisampleStateInfo(VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable, float minSampleShading, const VkSampleMask* sampleMask, VkBool32 alphaToCoverageEnable, VkBool32 alphaToOneEnable, 
		VkPipelineMultisampleStateCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, pNext, flags, rasterizationSamples, sampleShadingEnable, minSampleShading, sampleMask, alphaToCoverageEnable, alphaToOneEnable
		};
	}
	inline VkPipelineColorBlendStateCreateInfo createPipelineColorBlendStateInfo(VkBool32 logicOpEnable, VkLogicOp logicOp, uint32_t attachmentCount, 
		const VkPipelineColorBlendAttachmentState* pAttachments, float blendConstants[4], VkPipelineColorBlendStateCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, pNext, flags, logicOpEnable, logicOp, attachmentCount, pAttachments, {blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]}
		};
	}

	inline VkPipelineDynamicStateCreateInfo createPipelineDynamiceStateCreateInfo(uint32_t dynamicStateCount, const VkDynamicState* pDynamicStates, 
		VkPipelineDynamicStateCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, pNext, flags, dynamicStateCount, pDynamicStates
		};
	}
	inline VkPipelineDepthStencilStateCreateInfo createPipelineDepthStencilStateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp, VkBool32 depthBoundsTestEnable = VK_FALSE,
		VkBool32 stencilTestEnable = VK_FALSE, const VkStencilOpState& front = {}, const VkStencilOpState& back = {}, float minDepthBounds = 0.0f, float maxDepthBounds = 0.0f,
		VkPipelineDepthStencilStateCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, pNext, flags, depthTestEnable, depthWriteEnable,
			depthCompareOp, depthBoundsTestEnable, stencilTestEnable, front, back, minDepthBounds, maxDepthBounds
		};
	}
	inline VkPipelineVertexInputStateCreateInfo createPipelineVertexInputStateInfo(uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription* pVertexBindingDescriptions,
		uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription* pVertexAttributeDescriptions,
		VkPipelineVertexInputStateCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, pNext, flags, vertexBindingDescriptionCount, pVertexBindingDescriptions,
			vertexAttributeDescriptionCount, pVertexAttributeDescriptions
		};
	}
	inline VkGraphicsPipelineCreateInfo createGraphicsPipelineInfo(uint32_t stageCount, const VkPipelineShaderStageCreateInfo* pStages, 
		const VkPipelineVertexInputStateCreateInfo* pVertexInputState, const VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState,
		const VkPipelineTessellationStateCreateInfo* pTessellationState,
		const VkPipelineViewportStateCreateInfo* pViewportState, const VkPipelineRasterizationStateCreateInfo* pRasterizationState,
		const VkPipelineMultisampleStateCreateInfo* pMultisampleState, const VkPipelineDepthStencilStateCreateInfo* pDepthStencilState,
		const VkPipelineColorBlendStateCreateInfo* pColorBlendState, const VkPipelineDynamicStateCreateInfo* pDynamicState,
		VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpassIndex, VkPipeline basePipelineHandle = VK_NULL_HANDLE, int32_t basePipelineIndex = 0,
		VkPipelineCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, pNext, flags,
			stageCount, pStages, pVertexInputState, pInputAssemblyState, pTessellationState, pViewportState, pRasterizationState, pMultisampleState, pDepthStencilState, pColorBlendState, pDynamicState,
			layout, renderPass, subpassIndex, basePipelineHandle, basePipelineIndex
		};
	}
	inline VkPipeline createGraphicsPipeline(VkDevice device, const VkGraphicsPipelineCreateInfo& info, VkPipelineCache pipelineCache = VK_NULL_HANDLE) {
		VkPipeline pipeline;
		VKL_CHECK(vkCreateGraphicsPipelines(device, pipelineCache, 1, &info, VKL_Callbacks, &pipeline), VKL_ERROR_PIPELINE_CREATION_FAILED);
		return pipeline;
	}
	inline VkComputePipelineCreateInfo createComputePipelineInfo(VkShaderModule module, VkPipelineLayout layout, VkPipeline basePipelineHandle = VK_NULL_HANDLE, int32_t basePipelineIndex = 0, VkPipelineCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, pNext, flags, 
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, VKL_FLAG_NONE, VK_SHADER_STAGE_COMPUTE_BIT, module, "main", nullptr },
			layout, basePipelineHandle, basePipelineIndex
		};
	}
	inline VkComputePipelineCreateInfo createComputePipelineInfo(const VkPipelineShaderStageCreateInfo& stage, VkPipelineLayout layout, VkPipeline basePipelineHandle = VK_NULL_HANDLE, int32_t basePipelineIndex = 0, VkPipelineCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, pNext, flags, stage, layout, basePipelineHandle, basePipelineIndex 
		};
	}
	inline VkPipeline createComputePipeline(VkDevice device, const VkComputePipelineCreateInfo& info, VkPipelineCache pipelineCache = VK_NULL_HANDLE) {
		VkPipeline pipeline;
		VKL_CHECK(vkCreateComputePipelines(device, pipelineCache, 1, &info, VKL_Callbacks, &pipeline), VKL_ERROR_PIPELINE_CREATION_FAILED);
		return pipeline;
	}
	inline VkPipeline createComputePipeline(VkDevice device, const VkPipelineShaderStageCreateInfo& stage, VkPipelineLayout layout, VkPipeline basePipelineHandle = VK_NULL_HANDLE, int32_t basePipelineIndex = 0, VkPipelineCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr, VkPipelineCache pipelineCache = VK_NULL_HANDLE) {
		VkComputePipelineCreateInfo info = createComputePipelineInfo(stage, layout, basePipelineHandle, basePipelineIndex, flags, pNext);
		VkPipeline pipeline;
		VKL_CHECK(vkCreateComputePipelines(device, pipelineCache, 1, &info, VKL_Callbacks, &pipeline), VKL_ERROR_PIPELINE_CREATION_FAILED);
		return pipeline;
	}
	inline VkPipeline createComputePipeline(VkDevice device, VkShaderModule module, VkPipelineLayout layout, VkPipeline basePipelineHandle = VK_NULL_HANDLE, int32_t basePipelineIndex = 0, VkPipelineCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr, VkPipelineCache pipelineCache = VK_NULL_HANDLE) {
		VkComputePipelineCreateInfo info = createComputePipelineInfo(module, layout, basePipelineHandle, basePipelineIndex, flags, pNext);
		VkPipeline pipeline;
		VKL_CHECK(vkCreateComputePipelines(device, pipelineCache, 1, &info, VKL_Callbacks, &pipeline), VKL_ERROR_PIPELINE_CREATION_FAILED);
		return pipeline;
	}
	inline void destroyPipeline(VkDevice device, VkPipeline pipeline) {
		vkDestroyPipeline(device, pipeline, VKL_Callbacks);
	}
}