#pragma once

#include "SVE_Backend.h"

class SveDescriptorInfo {
private:
	union ImageOrBuffer {
		inline ImageOrBuffer(VkSampler imageSampler, VkImageView imageView, VkImageLayout imageLayout) {
			imageInfo.sampler = imageSampler;
			imageInfo.imageView = imageView;
			imageInfo.imageLayout = imageLayout;
		}
		inline ImageOrBuffer(VkBuffer buffer, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
			bufferInfo.buffer = buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = size;
		}
		VkDescriptorImageInfo imageInfo;
		VkDescriptorBufferInfo bufferInfo;
		inline operator VkDescriptorImageInfo() const { return imageInfo; }
		inline operator VkDescriptorBufferInfo() const { return bufferInfo; }
	};
	VkDescriptorType type;
	VkShaderStageFlags stage;
	ImageOrBuffer info;
public:
	inline SveDescriptorInfo(VkDescriptorType descriptorType, VkShaderStageFlags shaderStages, VkSampler imageSampler, VkImageView imageView, VkImageLayout imageLayout) : 
		type(descriptorType), stage(shaderStages), info(imageSampler, imageView, imageLayout) {}
	inline SveDescriptorInfo(VkDescriptorType descriptorType, VkShaderStageFlags shaderStages, VkBuffer buffer, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) : 
		type(descriptorType), stage(shaderStages), info(buffer, size, offset) {}
	
	inline VkDescriptorType getType() const { return type; }
	inline VkShaderStageFlags getShaderStage() const { return stage; }
	inline VkDescriptorSetLayoutBinding createBinding(uint32_t binding) const {
		return { binding, type, 1, stage, nullptr };
	}
};


class SveDescriptor {
public:
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	inline void create(uint32_t infoCount, const SveDescriptorInfo* pDescriptorInfos) {
		std::vector<VkDescriptorSetLayoutBinding> bindings(infoCount);
		for (uint32_t i = 0; i < infoCount; ++i) {
			bindings[i].binding = i;
			bindings[i].descriptorCount = 1;
			bindings[i].descriptorType = pDescriptorInfos[i].getType();
			bindings[i].stageFlags = pDescriptorInfos[i].getShaderStage();
		}
		//descriptorSetLayout = vkl::createDescriptorSetLayout(SVE::getDevice(), bufferInfoCount + imageInfoCount, )
	}
};