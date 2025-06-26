#include "SVE_SceneRenderer.h"

// Descriptor Pool:
VkDescriptorPool createDescriptorPool() {
	VkDescriptorPoolSize pool_sizes[] = {
		vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SVE::getImageCount()),
		vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, SVE::getImageCount()),
		vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, SVE::getImageCount() * SveModelRenderer::MAX_TEXTURE_COUNT),
	};
	return vkl::createDescriptorPool(SVE::getDevice(), 2 * SVE::getImageCount() + 1, ARRAY_SIZE(pool_sizes), pool_sizes);
}

// Descriptor Set Layout:
VkDescriptorSetLayout createUniformLayout() {
	VkDescriptorSetLayoutBinding uniform_bindings[] = {
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
	};
	return vkl::createDescriptorSetLayout(SVE::getDevice(), ARRAY_SIZE(uniform_bindings), uniform_bindings);
}

SveSceneRenderer::SveSceneRenderer() : descriptorPool(createDescriptorPool()), uniformLayout(createUniformLayout()), gridRenderer(uniformLayout)/*, modelRenderer(descriptorPool, uniformLayout)*/ 
{
	// Allocating Descriptor Sets:
	uniformDescriptors.resize(SVE::getImageCount());
	for (auto& set : uniformDescriptors) {
		set = vkl::allocateDescriptorSet(SVE::getDevice(), descriptorPool, 1, &uniformLayout);
	}
	// Writing Descriptor Sets:
	std::vector<VkWriteDescriptorSet> descriptor_writes(uniformDescriptors.size());
	VkDescriptorBufferInfo uniform_info = {VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
	for (size_t i = 0; i < uniformDescriptors.size(); ++i) {
		uniform_info.buffer = uniformBuffer.getBuffer((uint32_t)i);
		VkWriteDescriptorSet writes[] = {
			vkl::createDescriptorWrite(uniformDescriptors[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, 1, &uniform_info),
		};
		vkUpdateDescriptorSets(SVE::getDevice(), ARRAY_SIZE(writes), writes, 0, nullptr);
	}

}

SveSceneRenderer::~SveSceneRenderer() {
	SVE::destroyDescriptorPool(descriptorPool);
	SVE::destroyDescriptorSetLayout(uniformLayout);
}
