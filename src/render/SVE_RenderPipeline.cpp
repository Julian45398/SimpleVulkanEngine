#include "SVE_RenderPipeline.h"


void pipelineFramebufferResizeCallback(void* data) {
	SveRenderPipeline& pipeline = *(SveRenderPipeline*)data;
	pipeline.recreatePipeline();
}

SveRenderPipeline::SveRenderPipeline(const char* vertexFileName, const char* fragmentFileName, const VkPipelineVertexInputStateCreateInfo& vertexInfo, const VkBuffer* uniformBuffers, VkSampler imageSampler, VkImageView imageView, VkPolygonMode polygonMode_T, VkCullModeFlags cullMode_T, VkPrimitiveTopology primitiveTopology) :
	vertShaderFile(vertexFileName), fragShaderFile(fragmentFileName), vertexInput(vertexInfo)
{
	polygonMode = polygonMode_T;
	topology = primitiveTopology;
	cullMode = cullMode_T;
	if (windowResizeCallbackFunctionIndex == UINT32_MAX) {
		windowResizeCallbackFunctionIndex = SVE::addFramebufferResizeCallbackFunction(pipelineFramebufferResizeCallback);
	}
	SVE::addFramebufferResizeCallbackListener(windowResizeCallbackFunctionIndex, this);
	create(uniformBuffers, imageSampler, imageView);
}
SveRenderPipeline::~SveRenderPipeline()
{
	// TODO: remove callback listener!
	destroy();
}
void SveRenderPipeline::create(const VkBuffer* uniformBuffers, VkSampler imageSampler, VkImageView imageView) {
	VkDescriptorPoolSize pool_sizes[] = {
		vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SVE::getImageCount()),
		vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SVE::getImageCount())
	};
	descriptorPool = vkl::createDescriptorPool(SVE::getDevice(), SVE::getImageCount(), ARRAY_SIZE(pool_sizes), pool_sizes);
	VkDescriptorSetLayoutBinding bindings[]{
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
	};
	descriptorSetLayout = vkl::createDescriptorSetLayout(SVE::getDevice(), ARRAY_SIZE(bindings), bindings);
	descriptorSets.resize(SVE::getImageCount());
	for (uint32_t i = 0; i < SVE::getImageCount(); ++i) {
		descriptorSets[i] = vkl::allocateDescriptorSet(SVE::getDevice(), descriptorPool, 1, &descriptorSetLayout);
		VkDescriptorBufferInfo buf_info = { uniformBuffers[i], 0, VK_WHOLE_SIZE};
		VkDescriptorImageInfo image_info = { imageSampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		VkWriteDescriptorSet writes[] = {
			vkl::createDescriptorWrite(descriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, 1, &buf_info),
			vkl::createDescriptorWrite(descriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, 0, 1, &image_info)
		};
		vkUpdateDescriptorSets(SVE::getDevice(), ARRAY_SIZE(writes), writes, 0, nullptr);
	}
	pipelineLayout = vkl::createPipelineLayout(SVE::getDevice(), 1, &descriptorSetLayout);
	createGraphicPipeline();
}
void SveRenderPipeline::destroy() {
	vkl::destroyDescriptorPool(SVE::getDevice(), descriptorPool);
	vkl::destroyDescriptorSetLayout(SVE::getDevice(), descriptorSetLayout);
	vkl::destroyPipeline(SVE::getDevice(), pipelineHandle);
	vkl::destroyPipelineLayout(SVE::getDevice(), pipelineLayout);
}
void SveRenderPipeline::recreatePipeline() {
	vkDeviceWaitIdle(SVE::getDevice());
	vkl::destroyPipeline(SVE::getDevice(), pipelineHandle);
	createGraphicPipeline();
}
void SveRenderPipeline::createGraphicPipeline() {

	auto vertex_data = util::readBinaryFile(vertShaderFile);
	auto fragment_data = util::readBinaryFile(fragShaderFile);
	VkShaderModule vertex_module = vkl::createShaderModule(SVE::getDevice(), vertex_data.size(), (const uint32_t*)vertex_data.data());
	VkShaderModule fragment_module = vkl::createShaderModule(SVE::getDevice(), fragment_data.size(), (const uint32_t*)fragment_data.data());
	VkPipelineShaderStageCreateInfo stages[] = {
		vkl::createPipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, vertex_module),
		vkl::createPipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_module)
	};
	VkPipelineInputAssemblyStateCreateInfo assembly_info = vkl::createPipelineInputAssemblyInfo(topology, VK_FALSE);
	VkViewport viewport = { 0.0f, 0.0f, (float)SVE::getFramebufferWidth(), (float)SVE::getFramebufferHeight(), 0.0f, 1.0f };
	VkRect2D scissor = { {0, 0}, {SVE::getFramebufferWidth(), SVE::getFramebufferHeight()} };
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
	float blend_constants[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	VkPipelineColorBlendStateCreateInfo color_blend = vkl::createPipelineColorBlendStateInfo(VK_FALSE, VK_LOGIC_OP_COPY, 1, &color_blend_attachement, blend_constants);
	//VkPipelineDynamicStateCreateInfo dynamic_state = vkl::createPipelineDynamiceStateCreateInfo(0, nullptr);
	VkGraphicsPipelineCreateInfo info = vkl::createGraphicsPipelineInfo(ARRAY_SIZE(stages), stages, &vertexInput, &assembly_info, nullptr, &viewport_info, &rasterization, &multisample, &depth_stencil, &color_blend, nullptr, pipelineLayout, SVE::getRenderPass(), 0);

	pipelineHandle = vkl::createGraphicsPipeline(SVE::getDevice(), info);
	vkl::destroyShaderModule(SVE::getDevice(), vertex_module);
	vkl::destroyShaderModule(SVE::getDevice(), fragment_module);
}