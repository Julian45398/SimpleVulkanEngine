#pragma once

#include <stddef.h>
#include <vector>
#include <array>

#include "Types.hpp"

namespace SGF::GPU {
	class DescriptorPool {
	public:
		DescriptorSet Allocate() const;
		std::vector<DescriptorSet> Allocate(uint32_t count) const;
		void AllocateToBuffer(uint32_t count, DescriptorSet* pDescriptorSetBuffer) const;

		void Free(DescriptorSet descriptorSet);
		void Free(const DescriptorSet* pDescriptorSets, uint32_t count) const;
		template<uint32_t COUNT>
		inline void Free(const DescriptorSet(&descriptorSets)[COUNT]) const { Free(descriptorSets, COUNT); }
		template<uint32_t COUNT>
		inline void Free(const std::array<DescriptorSet, COUNT>& descriptorSets) const { Free(descriptorSets.data(), COUNT); }
		inline void Free(const std::vector<DescriptorSet>& descriptorSets) const { Free(descriptorSets.data(), (uint32_t)descriptorSets.size()); }
		void Reset();
		void ShrinkToFit();
	private:
		void* m_Handle;
	};
}