#pragma once

#include "SGF_Core.hpp"

namespace SGF {
    /*
    class Image {
    public:
        inline operator VkImage() const { return handle; }
        inline VkImage getHandle() const { return handle; }
        inline VkFormat getFormat() const { return format; }
        inline uint32_t getWidth() const { return extent.width; }
        inline uint32_t getHeight() const { return extent.height; }
        inline uint32_t getDepth() const { return extent.depth; }
        inline VkExtent3D getExtent() const { return extent; }
        inline uint32_t getMipLevelCount() const { return mipLevelCount; }
        inline uint32_t getArraySize() const { return arraySize; }
        inline VkImageUsageFlags getUsage() const { return usage; }
        
        Image(const Device& device, uint32_t length, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, uint32_t arraySize = 1, uint32_t mipLevelCount = 1, QueueFamilyFlags simultaneousUsage = FLAG_NONE);
        Image(const Device& device, uint32_t width, uint32_t height, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, uint32_t arraySize = 1, uint32_t mipLevelCount = 1, QueueFamilyFlags simultaneousUsage = FLAG_NONE);
        Image(const Device& device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, uint32_t arraySize = 1, uint32_t mipLevelCount = 1, QueueFamilyFlags simultaneousUsage = FLAG_NONE);
        ~Image();
    private:
        friend ImageView;
        VkImage handle;
        VkImageUsageFlags usage;
        VkFormat format;
        VkImageLayout layout;
        VkExtent3D extent;
        uint32_t mipLevelCount;
        uint32_t arraySize;
    };
    class ImageView {
    public:
        inline operator VkImageView() const {return handle;}
        inline VkImageView getHandle() const {return handle;}
        inline uint32_t arrayBase() const {return baseArrayLayer;}
        inline uint32_t arraySize() const {return arrayLayerSize;}
        inline uint32_t mipBase() const {return baseMipLevel;}
        inline uint32_t mipCount() const {return mipLevelCount;}
        inline VkImageAspectFlags aspect() const {return aspectFlags;}
        inline ImageView() {}
    private:
        friend Image;
        friend Device;
        VkImageView handle = VK_NULL_HANDLE;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
        uint32_t baseArrayLayer = 0;
        uint32_t arrayLayerSize = 0;
        uint32_t baseMipLevel = 0;
        uint32_t mipLevelCount = 0;
    public:
        class Builder {
        public:
            inline ImageView build();
            inline Builder& swizzle(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a) { info.components = {r, g, b, a}; return *this; }
            inline Builder& aspect(VkImageAspectFlags aspect) { info.subresourceRange.aspectMask = aspect; return *this; }
            inline Builder& mip(uint32_t baseLevel, uint32_t levelCount) { info.subresourceRange.baseMipLevel = baseLevel; info.subresourceRange.levelCount = levelCount; return *this; }
            inline Builder& view1D(uint32_t baseLayer = 0) { info.viewType = VK_IMAGE_VIEW_TYPE_1D; info.subresourceRange.baseArrayLayer = baseLayer; return *this; }
            inline Builder& view2D(uint32_t baseLayer = 0) { info.viewType = VK_IMAGE_VIEW_TYPE_2D; info.subresourceRange.baseArrayLayer = baseLayer; return *this; }
            inline Builder& view3D(uint32_t baseLayer = 0) { info.viewType = VK_IMAGE_VIEW_TYPE_3D; info.subresourceRange.baseArrayLayer = baseLayer; return *this; }
            inline Builder& view1DArray(uint32_t baseLayer = 0, uint32_t arraySize = 1) { info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;info.subresourceRange.baseArrayLayer = baseLayer; info.subresourceRange.layerCount = arraySize; return *this; }
            inline Builder& view2DArray(uint32_t baseLayer = 0, uint32_t arraySize = 1) { info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;info.subresourceRange.baseArrayLayer = baseLayer; info.subresourceRange.layerCount = arraySize; return *this; }
            inline Builder& viewCubeArray(uint32_t baseLayer = 0, uint32_t arraySize = 1) { info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;info.subresourceRange.baseArrayLayer = baseLayer; info.subresourceRange.layerCount = arraySize; return *this; }
            Builder(const Device* device, const Image& image, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
        private:
            friend Device;
            VkImageViewCreateInfo info;
            const Device* pDevice;
        };
    };
    class Buffer {
    public:
        inline operator VkBuffer() const {return handle;}
        inline VkBuffer getHandle() const {return handle;}
        inline size_t size() const {return memorySize;}
        inline Buffer() {}
    private:
        friend Device;
        VkBuffer handle = VK_NULL_HANDLE;
        VkDeviceSize memorySize = 0;
    };
    class DeviceMemory {
    public:
        inline operator VkDeviceMemory() const {return handle;}
        inline VkDeviceMemory getHandle() const {return handle;}
        inline size_t size() const {return memorySize;}
        inline DeviceMemory() {}
    private:
        friend Device;
        VkDeviceMemory handle;
        VkDeviceSize memorySize;
    };
    */
}