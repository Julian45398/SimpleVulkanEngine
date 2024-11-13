#pragma once

#include "engine_core.h"


#include <fstream>

namespace util {
	inline std::vector<char> readBinaryFile(const char* filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			shl::logError("Failed to open file: ", filename);
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
	inline VkPipeline createGraphicPipeline(const char* vertexShaderFile, const char* fragShaderFile, VkPipelineLayout layout, const VkPipelineVertexInputStateCreateInfo& inputInfo, VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, VkCullModeFlags cullMode = VK_CULL_MODE_NONE, VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		const VkViewport viewport = {0.0f, 0.0f, (float)Core.WindowWidth, (float)Core.WindowHeight, 0.0f, 1.0f}, const VkRect2D scissor = { {0,0}, {Core.WindowWidth, Core.WindowHeight} }) {

		auto vertex_data = readBinaryFile(vertexShaderFile);
		auto fragment_data = readBinaryFile(fragShaderFile);
		VkShaderModule vertex_module = vkl::createShaderModule(Core.LogicalDevice, vertex_data.size(), (const uint32_t*)vertex_data.data());
		VkShaderModule fragment_module = vkl::createShaderModule(Core.LogicalDevice, fragment_data.size(), (const uint32_t*)fragment_data.data());
		VkPipelineShaderStageCreateInfo stages[] = {
			vkl::createPipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, vertex_module),
			vkl::createPipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_module)
		};
		VkPipelineInputAssemblyStateCreateInfo assembly_info = vkl::createPipelineInputAssemblyInfo(topology, VK_FALSE);
		VkPipelineViewportStateCreateInfo viewport_info = vkl::createPipelineViewportStateInfo(1, &viewport, 1, &scissor);
		VkPipelineRasterizationStateCreateInfo rasterization = vkl::createPipelineRasterizationStateInfo(VK_FALSE, VK_FALSE, polygonMode, cullMode, VK_FRONT_FACE_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
		VkPipelineMultisampleStateCreateInfo multisample = vkl::createPipelineMultisampleStateInfo(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, nullptr, VK_FALSE, VK_FALSE);
		VkPipelineDepthStencilStateCreateInfo depth_stencil = vkl::createPipelineDepthStencilStateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE, VK_FALSE);
		VkPipelineColorBlendAttachmentState color_blend_attachement{};
		color_blend_attachement.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
#ifdef DISABLE_COLOR_BLEND
		color_blend_attachement.blendEnable = VK_FALSE;
#else
		color_blend_attachement.blendEnable = VK_TRUE;
		color_blend_attachement.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachement.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachement.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachement.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachement.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachement.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
#endif
		float blend_constants[] = {0.0f, 0.0f, 0.0f, 0.0f};
		VkPipelineColorBlendStateCreateInfo color_blend = vkl::createPipelineColorBlendStateInfo(VK_FALSE, VK_LOGIC_OP_COPY, 1, &color_blend_attachement, blend_constants);
		//VkPipelineDynamicStateCreateInfo dynamic_state = vkl::createPipelineDynamiceStateCreateInfo(0, nullptr);
		VkGraphicsPipelineCreateInfo info = vkl::createGraphicsPipelineInfo(ARRAY_SIZE(stages), stages, &inputInfo, &assembly_info, nullptr, &viewport_info, &rasterization, &multisample, &depth_stencil, &color_blend, nullptr, layout, Core.RenderPass, 0);

		VkPipeline pipeline = vkl::createGraphicsPipeline(Core.LogicalDevice, info);
		vkDestroyShaderModule(Core.LogicalDevice, vertex_module, vkl::VKL_Callbacks);
		vkDestroyShaderModule(Core.LogicalDevice, fragment_module, vkl::VKL_Callbacks);
		return pipeline;
	}
}