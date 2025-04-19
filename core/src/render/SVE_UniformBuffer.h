#pragma once


#include "core.h"
#include "SVE_Backend.h"

template <typename T>
class SveUniformBuffer {
private:
	VkDeviceMemory deviceMemory;
	std::vector<VkBuffer> buffers;
	uint8_t* mappedMemory;
	VkDeviceSize uniformSize;
public:
	inline SveUniformBuffer() {
		buffers.resize(SVE::getImageCount());

		for (size_t i = 0; i < buffers.size(); ++i) {
			buffers[i] = vkl::createBuffer(SVE::getDevice(), sizeof(T), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, SVE::getGraphicsFamily());
		}
		VkMemoryRequirements req = vkl::getBufferMemoryRequirements(SVE::getDevice(), buffers[0]);
		uniformSize = req.size;
		req.size = SVE::getImageCount() * req.size;
		deviceMemory = vkl::allocateMemory(SVE::getDevice(), SVE::getPhysicalDevice(), req, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		for (size_t i = 0; i < buffers.size(); ++i) {
			vkl::bindBufferMemory(SVE::getDevice(), buffers[i], deviceMemory, i * uniformSize);
		}
		mappedMemory = (uint8_t*)vkl::mapMemory(SVE::getDevice(), deviceMemory, VK_WHOLE_SIZE, 0);
	}
	inline ~SveUniformBuffer() {
		for (size_t i = 0; i < buffers.size(); ++i) {
			vkl::destroyBuffer(SVE::getDevice(), buffers[i]);
		}
		vkl::freeMemory(SVE::getDevice(), deviceMemory);
		uniformSize = 0;
	}
	inline VkDescriptorSetLayoutBinding getDescriptorLayoutBinding(uint32_t binding, VkShaderStageFlags shaderStage) {
		return { binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, shaderStage, nullptr };
	}
	inline void update(const T& updateData) {
		size_t offset = SVE::getImageIndex() * uniformSize;
		memcpy(mappedMemory + offset, &updateData, sizeof(T));
	}
	inline VkBuffer getBuffer(uint32_t index) { return buffers[index]; }
	inline operator VkBuffer() { return buffers[SVE::getImageIndex()]; }
	inline operator const VkBuffer* () { return buffers.data(); }
	inline VkDeviceSize size() { return uniformSize; }
	inline const VkBuffer* getBuffers() { return buffers.data(); }
};