#pragma once

#include "render/SVE_UniformBuffer.h"
#include "SVE_ModelRenderer.h"
#include "SVE_GridRenderer.h"
#include "SVE_Model.h"

class SveSceneRenderer {
private:
	// UniformBuffer:
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout uniformLayout;
	SveUniformBuffer<glm::mat4> uniformBuffer;
	std::vector<VkDescriptorSet> uniformDescriptors;

	//SveModelRenderer modelRenderer;
	SveGridRenderer gridRenderer;
public:
	SveSceneRenderer();
	~SveSceneRenderer();

	inline void addModel(const SveModel& model) {
		//modelRenderer.addModel(model);
	}

	inline void draw(VkCommandBuffer commands, const glm::mat4& viewMatrix) {
		uniformBuffer.update(viewMatrix);
		//modelRenderer.draw(commands, uniformDescriptors[SVE::getImageIndex()]);
		gridRenderer.draw(commands, uniformDescriptors[SVE::getImageIndex()]);
	}
};
