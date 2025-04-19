#pragma once

#include "SVE_RenderPipeline.h"

class SveGridRenderer {
public:
	SveGridRenderer(VkDescriptorSetLayout uniformLayout);
	~SveGridRenderer();
	void draw(VkCommandBuffer commands, VkDescriptorSet uniformSet);
private:
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};