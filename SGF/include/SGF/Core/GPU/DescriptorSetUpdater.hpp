#pragma once

#include "Types.hpp"

namespace SGF::GPU {
	class DescriptorSetUpdater {
	public:
		void Update();
		void AddBuffer(DescriptorSet dstSet,
			uint32_t dstBinding, 
			uint32_t dstArrayElement, 
			uint32_t descriptorCount, 
			DescriptorType descriptorType, 
			Buffer buffer, 
			size_t offset, 
			size_t size);
		void AddImage(DescriptorSet dstSet,
			uint32_t dstBinding, 
			uint32_t dstArrayElement, 
			uint32_t descriptorCount, 
			DescriptorType descriptorType, 
			Sampler sampler, 
			ImageView view, 
			ImageLayout layout);
		void AddImage(DescriptorSet dstSet,
			uint32_t dstBinding, 
			uint32_t dstArrayElement, 
			uint32_t descriptorCount, 
			DescriptorType descriptorType, 
			Sampler sampler, 
			ImageView view, 
			ImageLayout layout);
	private:
		void* m_Handle;
	};
}