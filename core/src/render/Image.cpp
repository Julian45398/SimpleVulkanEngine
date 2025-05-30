#include "Render/Image.hpp"
#include "Render/Device.hpp"

namespace SGF {
    Image::Image(const Device& device, uint32_t length, VkFormat f, VkImageUsageFlags u, VkSampleCountFlagBits samples, uint32_t arrSize, uint32_t mipLevels, QueueFamilyFlags simultaneousUsage) {
        if (simultaneousUsage == FLAG_NONE) {
            handle = device.image1D(length, f, u, samples, mipLevelCount);
        }
        else {
            handle = device.image1DShared(length, f, u, samples, mipLevelCount, simultaneousUsage);
        }
        layout = VK_IMAGE_LAYOUT_UNDEFINED;
        usage = u;
        arraySize = arrSize;
        mipLevelCount = mipLevels;
        format = f;
        extent.width = length;
        extent.height = 1;
        extent.depth = 1;
    }
    Image::Image(const Device& device, uint32_t width, uint32_t height, VkFormat f, VkImageUsageFlags u, VkSampleCountFlagBits samples, uint32_t arrSize, uint32_t mipLevels, QueueFamilyFlags simultaneousUsage) {
        if (simultaneousUsage == FLAG_NONE) {
            if (arrSize == 1) {
                handle = device.image2D(width, height, f, u, samples, mipLevelCount);
            }
            else {
                handle = device.imageArray2D(width, height, arrSize, f, u, samples, mipLevelCount);
            }
        }
        else {
            if (arrSize == 1) {
                handle = device.image2DShared(width, height, f, u, samples, mipLevelCount, simultaneousUsage);
            }
            else {
                handle = device.imageArray2DShared(width, height, arrSize, f, u, samples, mipLevelCount, simultaneousUsage);
            }
        }
        layout = VK_IMAGE_LAYOUT_UNDEFINED;
        usage = u;
        arraySize = arrSize;
        mipLevelCount = mipLevels;
        format = f;
        extent.width = width;
        extent.height = height;
        extent.depth = 1;
    }
    Image::Image(const Device& device, uint32_t width, uint32_t height, uint32_t depth, VkFormat f, VkImageUsageFlags u, VkSampleCountFlagBits samples, uint32_t arrSize, uint32_t mipLevels, QueueFamilyFlags simultaneousUsage) {
        if (simultaneousUsage == FLAG_NONE) {
            if (arrSize == 1) {
                handle = device.image3D(width, height, depth, f, u, samples, mipLevelCount);
            }
            else {
                handle = device.imageArray3D(width, height, depth, arrSize, f, u, samples, mipLevelCount);
            }
        }
        else {
            if (arrSize == 1) {
                handle = device.image3DShared(width, height, depth, f, u, samples, mipLevelCount, simultaneousUsage);
            }
            else {
                handle = device.imageArray3DShared(width, height, depth, arrSize, f, u, samples, mipLevelCount, simultaneousUsage);
            }
        }
        layout = VK_IMAGE_LAYOUT_UNDEFINED;
        usage = u;
        arraySize = arrSize;
        mipLevelCount = mipLevels;
        format = f;
        extent.width = width;
        extent.height = height;
        extent.depth = depth;
    }
    Image::~Image() {
        auto& device = Device::Get();
        device.destroy(handle);
    }
}