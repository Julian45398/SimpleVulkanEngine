#include "SVE_RenderPipeline.h"
#include "SVE_Backend.h"

VkPipeline SveRenderPipelineBuilder::build() const {
	return SVE::createRenderPipeline(createInfo);
}
SveRenderPipelineBuilder& SveRenderPipelineBuilder::defaultMultisampleState() {
	multisampleState = vkl::createPipelineMultisampleStateInfo(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, nullptr, VK_FALSE, VK_FALSE);
	return *this;
}
SveRenderPipelineBuilder& SveRenderPipelineBuilder::defaultDepthStencilState() {
	depthStencilState = vkl::createPipelineDepthStencilStateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE, VK_FALSE);
	return *this;
}
SveRenderPipelineBuilder& SveRenderPipelineBuilder::defaultRasterizationState() {
	rasterizationState = vkl::createPipelineRasterizationStateInfo(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
	return *this;
}
SveRenderPipelineBuilder& SveRenderPipelineBuilder::defaultDynamicState() {
	dynamicStates.resize(2);
	dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
	dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.flags = VKL_FLAG_NONE;
	dynamicState.pNext = nullptr;
	dynamicState.pDynamicStates = dynamicStates.data();
	dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
	return *this;
}
SveRenderPipelineBuilder& SveRenderPipelineBuilder::defaultInputAssemblyState() {
	inputAssembly = vkl::createPipelineInputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
	return *this;
}
SveRenderPipelineBuilder& SveRenderPipelineBuilder::defaultColorBlend() {
	colorBlendStates.resize(1);
	colorBlendStates[0] = {};
	colorBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendStates[0].blendEnable = VK_TRUE;
	colorBlendStates[0].colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	float blend_constants[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	colorBlendState = vkl::createPipelineColorBlendStateInfo(VK_FALSE, VK_LOGIC_OP_COPY, (uint32_t)colorBlendStates.size(), colorBlendStates.data(), blend_constants);
	return *this;
}

SveRenderPipelineBuilder::SveRenderPipelineBuilder(const char* vertexShaderFile, const char* fragmentShaderFile, VkPipelineLayout pipelineLayout, const VkPipelineVertexInputStateCreateInfo& vertexInputState) {
	auto vertex_data = shl::readBinaryFile(vertexShaderFile);
	auto fragment_data = shl::readBinaryFile(fragmentShaderFile);
	VkShaderModule vertex_module = SVE::createShaderModule(vertex_data.size(), (const uint32_t*)vertex_data.data());
	VkShaderModule fragment_module = SVE::createShaderModule(fragment_data.size(), (const uint32_t*)fragment_data.data());
	shaderStages[0] = vkl::createPipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, vertex_module);
	shaderStages[1] = vkl::createPipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_module);
	shaderStageCount = 2;
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.basePipelineHandle = nullptr;
	createInfo.basePipelineIndex = 0;
	createInfo.flags = VKL_FLAG_NONE;
	createInfo.layout = pipelineLayout;
	createInfo.pColorBlendState = &colorBlendState;
	createInfo.pDepthStencilState = &depthStencilState;
	createInfo.pDynamicState = &dynamicState;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pMultisampleState = &multisampleState;
	createInfo.pRasterizationState = &rasterizationState;
	createInfo.pTessellationState = nullptr; //&tessellationState;
	createInfo.pVertexInputState = &vertexInputState;
	createInfo.pViewportState = &viewportState;
	createInfo.pStages = shaderStages;
	createInfo.stageCount = shaderStageCount;
	createInfo.subpass = 0;
	createInfo.renderPass = SVE::getRenderPass();
	defaultColorBlend();
	defaultDepthStencilState();
	defaultDynamicState();
	defaultMultisampleState();
	defaultRasterizationState();
	defaultInputAssemblyState();
	viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.scissorCount = 1;
	viewportState.viewportCount = 1;
}
SveRenderPipelineBuilder::~SveRenderPipelineBuilder() {
	for (uint32_t i = 0; i < shaderStageCount; ++i) {
		SVE::destroyShaderModule(shaderStages[i].module);
	}
}

SveRenderPipelineBuilder& SveRenderPipelineBuilder::addShaderStage(const VkPipelineShaderStageCreateInfo& shaderStageInfo) {
	if (shaderStageCount < MAX_SHADER_STAGES) {
		shaderStages[shaderStageCount] = shaderStageInfo;
		++shaderStageCount;
	}
	else {
		shl::logWarn("Pipeline Builder: max amound of shader stages reached!");
	}
	return *this;
}
