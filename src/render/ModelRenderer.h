#pragma once

#include "engine_core.h"
#include "util.h"
#include "ModelVertexBuffer.h"
#include "StagingBuffer.h"
#include "ui_data.h"


inline const ModelVertex VERTICES[] = {
	{glm::vec3(-0.5f, 0.5f, 1.0f), 0, glm::vec2(0.3f, 0.0f), 0}, 
	{glm::vec3(-0.5f, -0.5f, 1.0f), 0, glm::vec2(0.0f, 1.0f), 0}, 
	{glm::vec3(0.5f, -0.5f, 1.0f), 0, glm::vec2(1.0f, 0.0f), 0}, 
	{glm::vec3(0.5f, 0.5f, 1.0f), 0, glm::vec2(0.5f, 0.5f), 0}
	//{glm::vec4(-0.5f, 0.5f, 1.0f, 1.0f)},
	//{glm::vec4(-0.5f, -0.5f, 1.0f, 1.0f)},
	//{glm::vec4(0.5f, -0.5f, 1.0f, 1.0f)},
	//{glm::vec4(0.5f, 0.5f, 1.0f, 1.0f)}
};

inline const uint32_t INDICES[] = {
	0, 1, 2, 2, 3, 0
};

class ModelRenderer {
private:
	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
	ModelVertexBuffer ModelBuffer;
	std::vector<ModelVertex> Vertices;
	std::vector<uint32_t> Indices;
	struct ChangeRegion {
		uint32_t changeCount = 0;
		uint32_t changeOffset = 0;
	};
	ChangeRegion IndexChange;
	ChangeRegion VertexChange;
	uint32_t IndexCount = 0;

	static const uint32_t MAX_TRANSFER_SIZE = sizeof(ModelVertex) * 0xFFFFF;
	static const uint32_t MAX_VERTEX_COUNT = 0xFFFFF;
	static const uint32_t MAX_INDEX_COUNT = MAX_VERTEX_COUNT * 2;
public:
	ModelRenderer() {
		PipelineLayout = vkl::createPipelineLayout(Core.LogicalDevice, 0, nullptr);
		//Pipeline = util::createGraphicPipeline("resources/shaders/test_triangle.vert", "resources/shaders/test_triangle.frag", PipelineLayout, MODEL_VERTES_INPUT_INFO);
		Pipeline = util::createGraphicPipeline("resources/shaders/model.vert", "resources/shaders/model.frag", PipelineLayout, MODEL_VERTEX_INPUT_INFO);
		ModelBuffer.allocate(MAX_VERTEX_COUNT, MAX_INDEX_COUNT, MAX_VERTEX_COUNT * sizeof(ModelVertex));
		ModelBuffer.stageChanges(ARRAY_SIZE(VERTICES), 0, VERTICES, ARRAY_SIZE(INDICES), 0, INDICES);

		VkCommandPool trans_pool = vkl::createCommandPool(Core.LogicalDevice, Core.GraphicsIndex, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		VkCommandBuffer trans_comm = vkl::createCommandBuffer(Core.LogicalDevice, trans_pool);
		VkFence fence = vkl::createFence(Core.LogicalDevice);
		vkl::beginCommandBuffer(trans_comm, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		ModelBuffer.upload(trans_comm);
		vkl::endCommandBuffer(trans_comm);
		vkl::submitCommands(Core.GraphicsQueue, trans_comm, fence);
		vkl::waitForFence(Core.LogicalDevice, fence);
		vkl::destroyCommandPool(Core.LogicalDevice, trans_pool);
		vkl::destroyFence(Core.LogicalDevice, fence);
		
	}
	void recordCommands(VkCommandBuffer renderCommands, VkCommandBuffer transferCommands) {
		vkCmdBindPipeline(renderCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
		if (ButtonPressed) {
			uint32_t vert_offset = ButtonPressedCount * 4;
			ModelVertex vertices[] = {
				{glm::vec3(0.1f * ButtonPressedCount, 0.1f * ButtonPressedCount, 0.1f * (ButtonPressedCount % 9)), ButtonPressedCount, glm::vec2(0.1f * (ButtonPressedCount % 10), 0.2f * (ButtonPressedCount % 5)), ButtonPressedCount, 0.1f},
				{glm::vec3(0.1f * ButtonPressedCount, -0.1f * ButtonPressedCount, 0.1f * (ButtonPressedCount % 9)), ButtonPressedCount, glm::vec2(0.2f * (ButtonPressedCount % 5), 0.1f * (ButtonPressedCount % 10)), ButtonPressedCount, 0.1f},
				{glm::vec3(-0.1f * ButtonPressedCount, -0.1f * ButtonPressedCount, 0.1f * (ButtonPressedCount % 9)), ButtonPressedCount, glm::vec2(0.2f * (ButtonPressedCount % 5), 0.1f * (ButtonPressedCount % 10)), ButtonPressedCount, 0.1f},
				{glm::vec3(-0.1f * ButtonPressedCount, 0.1f * ButtonPressedCount, 0.1f * (ButtonPressedCount % 9)), ButtonPressedCount, glm::vec2(0.2f * (ButtonPressedCount % 5), 0.1f * (ButtonPressedCount % 10)), ButtonPressedCount, 0.1f},
			};
			uint32_t indices[]{
				vert_offset + 0, 
				vert_offset + 1, 
				vert_offset + 2, 
				vert_offset + 2, 
				vert_offset + 3, 
				vert_offset + 0, 
			};
			ModelBuffer.stageChanges(ARRAY_SIZE(vertices), vert_offset, vertices, ARRAY_SIZE(indices), ButtonPressedCount * ARRAY_SIZE(indices), indices);
			ModelBuffer.upload(transferCommands);
			ButtonPressed = false;
		}
		ModelBuffer.bindAndDraw(renderCommands);
		vkEndCommandBuffer(renderCommands);
	}
};
