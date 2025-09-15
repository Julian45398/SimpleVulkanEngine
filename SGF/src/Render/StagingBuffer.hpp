#pragma once

#include "SGF_Core.hpp"
#include "Device.hpp"

namespace SGF {
    class StagingBuffer {
    public:
        inline operator VkBuffer() { return buffer; }
        inline operator VkDeviceMemory() { return deviceMemory; }
        inline uint8_t* Data() { return mappedMemory; }
        inline const uint8_t* Data() const { return mappedMemory; }
        inline size_t GetSize() const { return allocationSize; }
        inline bool IsInitialized() const { return allocationSize != 0; }
        inline StagingBuffer(size_t size) { Allocate(size); }
        inline ~StagingBuffer() { if (mappedMemory) Device::Get().Destroy(deviceMemory, buffer); }
        inline StagingBuffer(StagingBuffer&& other) noexcept : buffer(other.buffer), deviceMemory(other.deviceMemory), mappedMemory(other.mappedMemory), allocationSize(other.allocationSize) {
            other.mappedMemory = nullptr;
            other.allocationSize = 0;
        }
        inline StagingBuffer() : mappedMemory(nullptr), deviceMemory(nullptr), buffer(nullptr), allocationSize(0) {}
        inline StagingBuffer& operator=(StagingBuffer&& other) noexcept {
            if (this != &other) {
                buffer = other.buffer;
                deviceMemory = other.deviceMemory;
                mappedMemory = other.mappedMemory;
                other.mappedMemory = nullptr;
            }
            return *this;
        }
        inline void Resize(size_t size) {
            if (IsInitialized()) Clear();
            if (size == 0) return;
            Allocate(size);
        }
        inline void Clear() {
            Device::Get().Destroy(deviceMemory, buffer);
            mappedMemory = nullptr;
            allocationSize = 0;
        }
        inline size_t CopyData(const void* data, size_t size, size_t offset = 0) {
            assert(size + offset <= allocationSize);
            memcpy(mappedMemory + offset, data, size);
            return size + offset;
        }
        inline size_t CopyData(const void* data, const VkBufferCopy& copyRegion) {
            return CopyData(data, copyRegion.size, copyRegion.srcOffset);
        }
        inline void Allocate(size_t size) {
            assert(mappedMemory == nullptr && !IsInitialized());
            buffer = Device::Get().CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            deviceMemory = Device::Get().AllocateMemory(buffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            mappedMemory = (uint8_t*)Device::Get().MapMemory(deviceMemory);
            allocationSize = size;
        }
    private:
        /* data */
        VkBuffer buffer;
        VkDeviceMemory deviceMemory;
        uint8_t* mappedMemory;
        size_t allocationSize;
    };
}