#pragma once

#include "SVE_ImageMemoryAllocator.h"
#include "SVE_UniformBuffer.h"
#include "SVE_Model.h"


class SveModelRenderer {
public:
	static constexpr uint32_t MAX_TEXTURE_COUNT = 128;
	SveModelRenderer(VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout);
	~SveModelRenderer();
	void addModel(const SveModel& model);
	void draw(VkCommandBuffer commands, VkDescriptorSet uniformSet);
	void changePipelineSettings(VkPolygonMode mode, VkCullModeFlags cull);
private:
	// Models:
	std::vector<const SveModel*> models;
	// Vertex buffers:
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexDeviceMemory;
	// Images:
	VkSampler sampler;
	std::vector<TextureImage> textures;
	SveImageMemoryAllocator textureAllocator;
	// TransferResources:
	VkFence fence;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;
	uint8_t* stagingMapped;
	// Descriptors:
	struct Descriptor {
		VkDescriptorSet set;
		bool invalidated;
	};
	VkDescriptorSetLayout descriptorLayout;
	std::vector<Descriptor> descriptorSets;
	// Pipeline:
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	VkPolygonMode polygonMode;
	uint32_t transformCount;
	uint32_t vertexCount;
	uint32_t indexCount;
	size_t freeVertexBufferMemory;


	void invalidateDescriptors();
	void checkTransferStatus();
	void createPipeline();
	void updateTextureDescriptors();
};
