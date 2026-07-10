#pragma once

#include <SGF/Core/GPU.hpp>
#include <SGF/Core/Flags.hpp>
#include <SGF/Core/Macros.hpp>
#include <stddef.h>

namespace SGF {
	template<size_t PAGE_COUNT>
	class HostCoherentRingBuffer {
	private:
		GPU::Buffer m_Buffer;
		GPU::Memory m_Memory;
		void* m_MappedMemory;
		size_t m_PageSize;
		size_t m_CurrentIndex;
		Flags<GPU::BufferUsage> m_UsageFlags;
	public:
		inline HostCoherentRingBuffer(size_t size, Flags<GPU::BufferUsage> usage) {
			m_UsageFlags = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			m_Buffer = GPU::CreateBuffer(size * PAGE_COUNT, m_UsageFlags);
			m_Memory = GPU::AllocateMemory(m_Buffer, GPU::MemoryProperty::HOST_VISIBLE | GPU::MemoryProperty::HOST_COHERENT);
			m_MappedMemory = GPU::MapMemory(m_Memory);
			m_PageSize = size;
			m_CurrentIndex = 0;
		}
		inline ~HostCoherentRingBuffer() {
			GPU::Destroy(m_Buffer, m_Memory);
		}
		inline void Write(const void* data, size_t dataSize, size_t offset = 0) {
			SGF_ASSERT(dataSize <= m_PageSize);
			offset = offset + m_CurrentIndex * m_PageSize;
			memcpy((char*)m_MappedMemory + offset, data, dataSize);
		}
		inline void SetPageIndex(size_t index) { m_CurrentIndex = index % PAGE_COUNT; }
		inline void Resize(size_t allocSize) {
			GPU::Destroy(m_Buffer, m_Memory);
			m_Buffer = GPU::CreateBuffer(allocSize * PAGE_COUNT, m_UsageFlags);
			m_Memory = GPU::AllocateMemory(m_Buffer, AsFlags(GPU::MemoryProperty::HOST_COHERENT, GPU::MemoryProperty::HOST_VISIBLE);
			m_MappedMemory = GPU::MapMemory(m_Memory);
			m_PageSize = allocSize;
		}
		inline void BindCurrentAsVertexBuffer(GPU::CommandList commandBuffer, uint32_t binding) const {
			size_t offset = m_CurrentIndex * m_PageSize;
		}
		inline void BindCurrentAsIndexBuffer32Bit(GPU::CommandList commandBuffer) const {
			commandBuffer.BindIndexBuffer32Bit(m_Buffer, m_CurrentIndex * m_PageSize);
		}
		inline void BindCurrentAsIndexBuffer16Bit(GPU::CommandList commandBuffer) const {
			commandBuffer.BindIndexBuffer32Bit(m_Buffer, m_CurrentIndex * m_PageSize);
		}
		inline void* GetCurrentPagePointer() const { return (char*)m_MappedMemory + m_CurrentIndex * m_PageSize; }
		inline void* GetPagePointer(size_t pageIndex) const {
			SGF_ASSERT(pageIndex < PAGE_COUNT);
			return (char*)m_MappedMemory + pageIndex * m_PageSize;
		}
		inline void* GetMappedMemory() const { return m_MappedMemory; }
		inline GPU::Memory GetMemory() const { return m_Memory; }
		inline GPU::Buffer GetBuffer() const { return m_Buffer; }
		inline size_t GetBufferOffset(uint32_t pageIndex) const { return pageIndex * m_PageSize; }
		inline size_t GetCurrentBufferOffset() const { return GetBufferOffset(m_CurrentIndex); }
		inline size_t GetCurrentPageIndex() const { return m_CurrentIndex; }
		inline size_t GetPageSize() const { return m_PageSize; }
		inline void NextPage() { m_CurrentIndex = (m_CurrentIndex + 1) % PAGE_COUNT; }
	};
}