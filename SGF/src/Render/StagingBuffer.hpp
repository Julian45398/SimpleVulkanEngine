#pragma once

#include <SGF/Core/GPU.hpp>

namespace SGF {
    class StagingBuffer {
    public:
        inline operator GPU::Buffer() { return m_Buffer; }
        inline operator GPU::Memory() { return m_Memory; }
        inline uint8_t* Data() { return m_MappedMemory; }
        inline const uint8_t* Data() const { return m_MappedMemory; }
        inline size_t GetSize() const { return m_AllocationSize; }
        inline bool IsInitialized() const { return m_AllocationSize != 0; }
        inline StagingBuffer(size_t size) { Allocate(size); }
        inline ~StagingBuffer() { if (m_MappedMemory) GPU::Destroy(m_Memory, m_Buffer); }
        inline StagingBuffer(StagingBuffer&& other) noexcept : m_Buffer(other.m_Buffer), m_Memory(other.m_Memory), m_MappedMemory(other.m_MappedMemory), m_AllocationSize(other.m_AllocationSize) {
            other.m_MappedMemory = nullptr;
            other.m_AllocationSize = 0;
        }
        inline StagingBuffer() : m_MappedMemory(nullptr), m_Memory(nullptr), m_Buffer(nullptr), m_AllocationSize(0) {}
        inline StagingBuffer& operator=(StagingBuffer&& other) noexcept {
            if (this != &other) {
                m_Buffer = other.m_Buffer;
                m_Memory = other.m_Memory;
                m_MappedMemory = other.m_MappedMemory;
                other.m_MappedMemory = nullptr;
            }
            return *this;
        }
        inline void Resize(size_t size) {
            if (IsInitialized()) Clear();
            if (size == 0) return;
            Allocate(size);
        }
        inline void Clear() {
            GPU::Destroy(m_Memory, m_Buffer);
            m_MappedMemory = nullptr;
            m_AllocationSize = 0;
        }
        inline size_t CopyData(const void* data, size_t size, size_t offset = 0) {
            assert(size + offset <= m_AllocationSize);
            memcpy(m_MappedMemory + offset, data, size);
            return size + offset;
        }
        inline size_t CopyData(const void* data, const GPU::BufferCopy& copyRegion) {
            return CopyData(data, copyRegion.size, copyRegion.srcOffset);
        }
        inline void Allocate(size_t size) {
            assert(m_MappedMemory == nullptr && !IsInitialized());
            m_Buffer = GPU::CreateBuffer(size, GPU::BufferUsage::TRANSFER_SRC);
            m_Memory = GPU::AllocateMemory(m_Buffer, Flags(GPU::MemoryProperty::HOST_COHERENT | GPU::MemoryProperty::HOST_VISIBLE);
            m_MappedMemory = (uint8_t*)GPU::MapMemory(m_Memory);
            m_AllocationSize = size;
        }
    private:
        /* data */
        GPU::Buffer m_Buffer;
        GPU::Memory m_Memory;
        uint8_t* m_MappedMemory;
        size_t m_AllocationSize;
    };
}