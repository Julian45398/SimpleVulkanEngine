#pragma once

#include "core.h"

class SveGeometryRenderer {
public:
	SveGeometryRenderer(VkDescriptorSetLayout uniformLayout);
	~SveGeometryRenderer();
	void draw(VkCommandBuffer commands, VkDescriptorSet uniformSet);
private:
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	VkDeviceMemory deviceMemory;
	uint32_t indexCount;
};