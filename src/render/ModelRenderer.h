#pragma once

#include "engine_core.h"
#include "util.h"
#include "ModelVertexBuffer.h"
#include "StagingBuffer.h"
#include "ui_data.h"
#include "Camera.h"
#include "SVE_Model.h"

struct UniformData {
	glm::mat4 Transform;
};

class ModelRenderer {
private:
	struct ImageResource {
		VkBuffer Uniform = VK_NULL_HANDLE;
		VkDescriptorSet Descriptor = VK_NULL_HANDLE;
		VkCommandPool CommandPool = VK_NULL_HANDLE;
		VkCommandBuffer Commands = VK_NULL_HANDLE;
	};
	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
	ModelVertexBuffer ModelBuffer;
	std::vector<ModelVertex> Vertices;
	std::vector<uint32_t> Indices;
	std::vector<ImageResource> PerImage;
	VkDeviceMemory UniformMemory = VK_NULL_HANDLE;
	uint8_t* UniformMapped = nullptr;
	uint32_t UniformSize = 0;
	uint32_t IndexCount = 0;

	static const uint32_t MAX_TRANSFER_SIZE = sizeof(ModelVertex) * 0xFFFFF;
	static const uint32_t MAX_VERTEX_COUNT = 0xFFFFF;
	static const uint32_t MAX_INDEX_COUNT = MAX_VERTEX_COUNT * 2;
private:
	void setupPerImage() {
		PerImage.resize(Core.getImageCount());
	
		for (size_t i = 0; i < PerImage.size(); ++i) {
			PerImage[i].Uniform = vkl::createBuffer(Core.LogicalDevice, sizeof(UniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Core.GraphicsIndex);
		}
		VkMemoryRequirements req = vkl::getBufferMemoryRequirements(Core.LogicalDevice, PerImage[0].Uniform);
		UniformSize = req.size;
		req.size = PerImage.size() * req.size;
		UniformMemory = vkl::allocateMemory(Core.LogicalDevice, Core.PhysicalDevice, req, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		for (size_t i = 0; i < PerImage.size(); ++i) {
			vkBindBufferMemory(Core.LogicalDevice, PerImage[i].Uniform, UniformMemory, i * UniformSize);
			PerImage[i].Descriptor = vkl::allocateDescriptorSet(Core.LogicalDevice, DescriptorPool, 1, &DescriptorSetLayout);
			VkDescriptorBufferInfo buf_info = {PerImage[i].Uniform, 0, VK_WHOLE_SIZE};
			VkWriteDescriptorSet write = vkl::createDescriptorWrite(PerImage[i].Descriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, 1, &buf_info);
			vkUpdateDescriptorSets(Core.LogicalDevice, 1, &write, 0, nullptr);
			PerImage[i].CommandPool = vkl::createCommandPool(Core.LogicalDevice, Core.GraphicsIndex);
			PerImage[i].Commands = vkl::createCommandBuffer(Core.LogicalDevice, PerImage[i].CommandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
			//PerImage[i].CommandsRecorded = false;
		}
		UniformMapped = (uint8_t*)vkl::mapMemory(Core.LogicalDevice, UniformMemory, VK_WHOLE_SIZE, 0);
	}
public:
	void updateUniformBuffer(const UniformData& data) {
		memcpy(UniformMapped + Core.ImageIndex * UniformSize, &data, sizeof(UniformData));
	}

	void init() {
		ModelBuffer.allocate(MAX_VERTEX_COUNT, MAX_INDEX_COUNT, MAX_VERTEX_COUNT * sizeof(ModelVertex));

		VkDescriptorPoolSize pool_sizes[] = {
			vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Core.getImageCount())
		};
		VkDescriptorSetLayoutBinding bindings[] = {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
		};
		DescriptorPool = vkl::createDescriptorPool(Core.LogicalDevice, Core.getImageCount(), ARRAY_SIZE(pool_sizes), pool_sizes);
		DescriptorSetLayout = vkl::createDescriptorSetLayout(Core.LogicalDevice, ARRAY_SIZE(bindings), bindings);
		
		PipelineLayout = vkl::createPipelineLayout(Core.LogicalDevice, 1, &DescriptorSetLayout);
		//Pipeline = util::createGraphicPipeline("resources/shaders/test_triangle.vert", "resources/shaders/test_triangle.frag", PipelineLayout, MODEL_VERTES_INPUT_INFO);
		Pipeline = util::createGraphicPipeline("resources/shaders/model.vert", "resources/shaders/model.frag", PipelineLayout, SVE_MODEL_VERTEX_INPUT_INFO);
		setupPerImage();
	}

	void getRenderCommands(VkCommandBuffer renderCommands, VkCommandBuffer transferCommands) {
		vkCmdBindPipeline(renderCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
		uint32_t offset = 0;
		vkCmdBindDescriptorSets(renderCommands, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &PerImage[Core.ImageIndex].Descriptor, 0, &offset);
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
			ButtonPressedCount++;
		}
		ModelBuffer.bindAndDraw(renderCommands);
	}
};
