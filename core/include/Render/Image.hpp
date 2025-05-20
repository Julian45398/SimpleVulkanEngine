#pragma once

#include "SGF_Core.hpp"

namespace SGF {
    class Image {
    public:
        inline operator VkImage() const {return handle;}
        inline VkImage getHandle() const {return handle;}
        inline VkFormat getFormat() const {return format;}
        inline uint32_t getWidth() const {return width;}
        inline uint32_t getHeight() const {return height;}
        inline uint32_t getDepth() const {return depth;}
        inline uint32_t getMipLevelCount() const {return mipLevelCount;}
        inline uint32_t getArraySize() const {return arraySize;}
        
        inline Image() {};
    private:
        friend Device;
        friend ImageView;
        VkImage handle;
        VkFormat format;
        VkImageLayout layout;
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t mipLevelCount;
        uint32_t arraySize;
    public:
        class Builder {
        public:
            inline Image build();
            inline Builder& format(VkFormat format) { info.format = format; return *this; }
            inline Builder& extent(uint32_t length) { info.extent = {length, 1, 1}; info.imageType = VK_IMAGE_TYPE_1D; return *this; }
            inline Builder& extent(uint32_t width, uint32_t height) { info.extent = {width, height, 1}; info.imageType = VK_IMAGE_TYPE_2D; return *this; }
            inline Builder& extent(uint32_t width, uint32_t height, uint32_t depth) { info.extent = {width, height, depth}; info.imageType = VK_IMAGE_TYPE_3D; return *this; }
            inline Builder& samples(VkSampleCountFlagBits sampleCount) { info.samples = sampleCount; return *this; }
            inline Builder& mipLevels(uint32_t count) { info.mipLevels = count; return *this; }
            inline Builder& array(uint32_t size) { info.arrayLayers = size; return *this; }
            inline Builder& tiling(VkImageTiling tiling) { info.tiling = tiling; return *this; }
            inline Builder& layout(VkImageLayout initialLayout) { info.initialLayout = initialLayout; return *this; }
            inline Builder& usage(VkImageUsageFlags usage) { info.usage = usage; return *this; }
            inline Builder& createFlags(VkImageCreateFlags flags) { info.flags = flags; return *this; }
            inline Builder& next(void* pNext) { info.pNext = pNext; return *this; }
            Builder& graphics();
            Builder& compute();
            Builder& transfer();
            Builder& present();
            inline Builder& flags(VkImageCreateFlags createFlags) { info.flags = createFlags; return *this; }
            inline Builder& next(const void* pNext) { info.pNext = pNext; return *this; }
        private:
            friend Device;
            Builder(const Device* device, uint32_t length);
            Builder(const Device* device, uint32_t width, uint32_t height);
            Builder(const Device* device, uint32_t width, uint32_t height, uint32_t depth);
            VkImageCreateInfo info = {};
            uint32_t indices[4] = {UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
            const Device* pDevice;
        };
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
}