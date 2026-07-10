#pragma once

#include "CommandList.hpp"

namespace SGF::GPU {
	class CommandPool {
	public:
		CommandList Allocate() const;
		CommandList AllocateSecondary() const;
		std::vector<CommandList> Allocate(uint32_t count) const;
		std::vector<CommandList> AllocateSecondary(uint32_t count) const;
		void AllocateToBuffer(CommandList* pCommandLists, uint32_t count) const;
		template<uint32_t COUNT>
		inline void AllocateToBuffer(CommandList(&commandLists)[COUNT]) const { AllocateToBuffer(commandLists, COUNT); }
		template<uint32_t COUNT>
		inline void AllocateToBuffer(std::array<CommandList, COUNT>& commandLists) const { AllocateToBuffer(commandLists.data(), COUNT); }
		inline void AllocateToBuffer(std::vector<CommandList>& commandLists) const { AllocateToBuffer(commandLists.data(), (uint32_t)commandLists.size()); }

		void AllocateSecondaryToBuffer(CommandList* pCommandLists, uint32_t count) const;
		template<uint32_t COUNT>
		inline void AllocateSecondaryToBuffer(CommandList(&commandLists)[COUNT]) const { AllocateSecondaryToBuffer(commandLists, COUNT); }
		template<uint32_t COUNT>
		inline void AllocateSecondaryToBuffer(std::array<CommandList, COUNT>& commandLists) const { AllocateSecondaryToBuffer(commandLists.data(), COUNT); }
		inline void AllocateSecondaryToBuffer(std::vector<CommandList>& commandLists) const { AllocateSecondaryToBuffer(commandLists.data(), (uint32_t)commandLists.size()); }

		void Free(const CommandList& commandList) const;
		void Free(const CommandList* pCommandLists, uint32_t count) const;
		template<uint32_t COUNT>
		inline void Free(const CommandList(&commandLists)[COUNT]) const { Free(commandLists, COUNT); }
		template<uint32_t COUNT>
		inline void Free(const std::array<CommandList, COUNT>& commandLists) const { Free(commandLists.data(), COUNT); }
		inline void Free(const std::vector<CommandList>& commandLists) const { Free(commandLists.data(), (uint32_t)commandLists.size()); }

		void Reset();
		void ResetRelease();
		inline void* GetHandle() const { return m_Handle; }
	private:
		void* m_Handle = nullptr;
	};
}