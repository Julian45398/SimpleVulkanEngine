#pragma once

#include "Types.hpp" 

namespace SGF::GPU {
	class DescriptorSetLayout {
	public:
		static constexpr uint32_t MAX_BINDINGS = 16;
		DescriptorType GetDescriptorType(uint32_t index) const;
		DescriptorSetBinding GetBinding(uint32_t index) const;
		const DescriptorSetBinding* GetBindings() const;
		uint32_t GetBindingCount() const;
		inline bool IsValid() const noexcept { return m_Index != UINT64_MAX; }
		void* GetNativeHandle() const;
	private:
		uint64_t m_Index = UINT64_MAX;
	};
}
