#pragma once

class SGF_Window;
class SGF_Device;
class SGF_Image;
class SGF_ImageView;
class SGF_Buffer;
class SGF_Memory;
class SGF_CommandList;

class SGF_Image {
public:
    inline operator VkImage() {return handle;}
    inline VkImage getHandle() {return handle;}
    inline VkFormat getFormat() {return format;}
    inline uint32_t getWidth() {return width;}
    inline uint32_t getHeight() {return height;}
    inline uint32_t getDepth() {return depth;}
    inline uint32_t getMipLevelCount() {return mipLevelCount;}
    inline uint32_t getArraySize() {return arraySize;}
private:
    friend SGF_Device;
    friend SGF_ImageView;
    inline SGF_Image() {};
    VkImage handle;
    VkFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipLevelCount;
    uint32_t arraySize;
};
class SGF_ImageView {
public:
    inline operator VkImageView() {return handle;}
    inline VkImageView getHandle() {return handle;}
    inline uint32_t arrayBase() {return baseArrayLayer;}
    inline uint32_t arraySize() {return arrayLayerSize;}
    inline uint32_t mipBase() {return baseMipLevel;}
    inline uint32_t mipCount() {return mipLevelCount;}
    inline VkImageAspectFlags aspect() {return aspectFlags;}
private:
    friend SGF_Image;
    friend SGF_Device;
    inline SGF_ImageView() {}
    VkImageView handle;
    VkImageViewType viewType;
    VkImageAspectFlags aspectFlags;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerSize;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
};
class SGF_Buffer {
public:
    inline operator VkBuffer() {return handle;}
    inline VkBuffer getHandle() {return handle;}
    inline size_t size() {return memorySize;}
private:
    friend SGF_Device;
    inline SGF_Buffer() {}
    VkBuffer handle;
    VkDeviceSize memorySize;
};
class SGF_Memory {
public:
    inline operator VkDeviceMemory() {return handle;}
    inline VkDeviceMemory getHandle() {return handle;}
    inline size_t size() {return memorySize;}
private:
    friend SGF_Device;
    inline SGF_Memory() {}
    VkDeviceMemory handle;
    VkDeviceSize memorySize;
};