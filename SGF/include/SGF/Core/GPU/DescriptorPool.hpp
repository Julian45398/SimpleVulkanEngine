#pragma once

#include <stdint.h>
#include <vector>
#include <array>

#include "Types.hpp"

namespace SGF {
	namespace GPU {
		class DescriptorPool {
			DescriptorSet AllocateDescriptorSet(DescriptorSetLayout descriptorSetLayout);
			//void AllocateDescriptorSets(const DescriptorSetAllocateInfo& info, DescriptorSet* pSets);
			std::vector<DescriptorSet> AllocateDescriptorSets(const DescriptorSetLayout* pSetLayouts, uint32_t setCount) const;
			inline std::vector<DescriptorSet> AllocateDescriptorSets(const std::vector<DescriptorSetLayout> setLayouts) const { return AllocateDescriptorSets(setLayouts.data(), (uint32_t)setLayouts.size()); }
			template<uint32_t COUNT>
			inline std::vector<DescriptorSet> AllocateDescriptorSets(const std::array<DescriptorSetLayout, COUNT> setLayouts) const { return AllocateDescriptorSets(setLayouts.data(), COUNT); }
			template<uint32_t COUNT>
			inline std::vector<DescriptorSet> AllocateDescriptorSets(const DescriptorSetLayout(&setLayouts)[COUNT]) const { return AllocateDescriptorSets(setLayouts, COUNT); }

			void FreeDescriptorSets(const DescriptorSet* pDescriptorSets, uint32_t count) const;
			template<uint32_t COUNT>
			inline void FreeDescriptorSets(const DescriptorSet(&descriptorSets)[COUNT]) const { FreeDescriptorSets(descriptorSets, COUNT); }
			template<uint32_t COUNT>
			inline void FreeDescriptorSets(const std::array<DescriptorSet, COUNT>& descriptorSets) const { FreeDescriptorSets(descriptorSets.data(), COUNT); }
			inline void FreeDescriptorSets(const std::vector<DescriptorSet>& descriptorSets) const { FreeDescriptorSets(descriptorSets.data(), (uint32_t)descriptorSets.size()); }
			inline void FreeDescriptorSet(DescriptorSet descriptorSet) const { FreeDescriptorSets(&descriptorSet, 1); }
		private:
			void* m_Handle = nullptr;
		};
	}
}