#pragma once

#include "vkl_core.h"

namespace vkl {
	inline VkDescriptorPoolSize createDescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount) {
		return  {
			type, descriptorCount
		};
	}
	inline VkDescriptorPoolCreateInfo createDescriptorPoolInfo(uint32_t maxSets, uint32_t poolSizeCount, const VkDescriptorPoolSize* pPoolSizes, VkDescriptorPoolCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, pNext, flags, maxSets, poolSizeCount, pPoolSizes
		};
	}
	inline VkDescriptorPool createDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo& info) {
		VkDescriptorPool pool;
		VKL_CHECK(vkCreateDescriptorPool(device, &info, VKL_Callbacks, &pool), VKL_ERROR_DESCRIPTOR_POOL_CREATION_FAILED);
		return pool;
	}
	inline VkDescriptorPool createDescriptorPool(VkDevice device, uint32_t maxSets, uint32_t poolSizeCount, const VkDescriptorPoolSize* pPoolSizes, VkDescriptorPoolCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return createDescriptorPool(device, createDescriptorPoolInfo(maxSets, poolSizeCount, pPoolSizes, flags, pNext));
	}
	inline VkDescriptorSetLayoutCreateInfo createDescriptorSetLayoutInfo(uint32_t bindingCount, const VkDescriptorSetLayoutBinding* pBindings, VkDescriptorSetLayoutCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, pNext, flags, bindingCount, pBindings
		};
	}
	inline void destroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool) {
		vkDestroyDescriptorPool(device, descriptorPool, VKL_Callbacks);
	}
	inline VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo& info) {
		VkDescriptorSetLayout layout;
		VKL_CHECK(vkCreateDescriptorSetLayout(device, &info, VKL_Callbacks, &layout), VKL_ERROR_DESCRIPTOR_SET_LAYOUT_CREATION_FAILED);
		return layout;
	}
	inline VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device, uint32_t bindingCount, const VkDescriptorSetLayoutBinding* pBindings, VkDescriptorSetLayoutCreateFlags flags = VKL_FLAG_NONE, const void* pNext = nullptr) {
		return createDescriptorSetLayout(device, createDescriptorSetLayoutInfo(bindingCount, pBindings, flags, pNext));
	}
	inline VkDescriptorSetAllocateInfo createDescriptorSetAllocateInfo(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts, const void* pNext = nullptr) {
		return { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, pNext, descriptorPool, descriptorSetCount, pSetLayouts };
	}
	inline void destroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout) {
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, VKL_Callbacks);
	}
	inline VkDescriptorSet allocateDescriptorSet(VkDevice device, const VkDescriptorSetAllocateInfo& info) {
		VkDescriptorSet set;
		VKL_CHECK(vkAllocateDescriptorSets(device, &info, &set), VKL_ERROR_DESCRIPTOR_SET_ALLOCATION_FAILED);
		return set;
	}
	inline VkDescriptorSet allocateDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts, const void* pNext = nullptr) {
		return allocateDescriptorSet(device, createDescriptorSetAllocateInfo(descriptorPool, descriptorSetCount, pSetLayouts, pNext));
	}
	inline VkDescriptorBufferInfo createDescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
		return {
			buffer, offset, range
		};
	}
	inline VkWriteDescriptorSet createDescriptorWrite(VkDescriptorSet dstSet, VkDescriptorType descriptorType, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, const VkDescriptorBufferInfo* pBufferInfos, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pNext, dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, nullptr, pBufferInfos, nullptr
		};
	}
	inline VkWriteDescriptorSet createDescriptorWrite(VkDescriptorSet dstSet, VkDescriptorType descriptorType, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, const VkDescriptorImageInfo* pImageInfos, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pNext, dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, pImageInfos, nullptr, nullptr
		};
	}
	inline VkWriteDescriptorSet createDescriptorWrite(VkDescriptorSet dstSet, VkDescriptorType descriptorType, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount, const VkBufferView* pBufferViews, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pNext, dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, nullptr, nullptr, pBufferViews 
		};
	}
}