#pragma once

#include "DescriptorSetLayout.hpp"
#include "DescriptorPool.hpp"
#include "Types.hpp"
#include <map>

namespace SGF::Render {
	class DescriptorAllocator {
	public:
		/**
		* Allocates a DescriptorSet with the given Layout, creates a new DescriptorPool if necessary
		*/
		DescriptorSet Allocate(DescriptorSetLayout layout);
		/**
		* Allocates multiple DescriptorSets with the given Layout, creates a new DescriptorPool if necessary
		*/
		std::vector<DescriptorSet> Allocate(DescriptorSetLayout layout, uint32_t count);
		/**
		* Resets all created DescriptorPools freeing every allocated DescriptorSet
		*/
		void ResetAll();
		/**
		* Resets the created DescriptorPool for the given DescriptorSetLayout freeing every allocated DescriptorSet
		*/
		void Reset(DescriptorSetLayout layout);
		void Free(DescriptorSetLayout layout, DescriptorSet descriptorSet);
		void Clear();
	private:
		std::map<DescriptorSetLayout, DescriptorPool> m_DescriptorPools;
	};
}