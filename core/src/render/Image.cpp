#include "Image.hpp"

#include "Device.hpp"

namespace SGF {
    Image Image::Builder::build() {
        info.sharingMode = info.queueFamilyIndexCount > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        return pDevice->image(info);
    }
    Image::Builder& Image::Builder::graphics() { 
        indices[info.queueFamilyIndexCount] = pDevice->graphicsIndex(); 
        info.queueFamilyIndexCount++; 
        return *this;
    }
    Image::Builder& Image::Builder::compute() { 
        indices[info.queueFamilyIndexCount] = pDevice->computeIndex(); 
        info.queueFamilyIndexCount++; 
        return *this; 
    }
    Image::Builder& Image::Builder::transfer() { 
        indices[info.queueFamilyIndexCount] = pDevice->transferIndex(); 
        info.queueFamilyIndexCount++; 
        return *this; 
    }
    Image::Builder& Image::Builder::present() { 
        indices[info.queueFamilyIndexCount] = pDevice->presentIndex(); 
        for(uint32_t i = 0;i < info.queueFamilyIndexCount; ++i) {
            if(indices[i]==indices[info.queueFamilyIndexCount]) 
                return *this;
        } 
        info.queueFamilyIndexCount++; 
        return *this; 
    }
    Image::Builder::Builder(Device& device, uint32_t length) : pDevice(&device) {
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.format = VK_FORMAT_R8G8B8A8_SRGB;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.pQueueFamilyIndices = indices;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.extent = {length, 1, 1};
        info.imageType = VK_IMAGE_TYPE_1D;
        info.pNext = nullptr;
        info.flags = 0; 
    }
    Image::Builder::Builder(Device& device, uint32_t width, uint32_t height) : pDevice(&device) {
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.format = VK_FORMAT_R8G8B8A8_SRGB;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.pQueueFamilyIndices = indices;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.extent = {width, height, 1};
        info.imageType = VK_IMAGE_TYPE_2D;
        info.pNext = nullptr;
        info.flags = 0; 
    }
    Image::Builder::Builder(Device& device, uint32_t width, uint32_t height, uint32_t depth) : pDevice(&device) {
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.format = VK_FORMAT_R8G8B8A8_SRGB;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.pQueueFamilyIndices = indices;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.extent = {width, height, depth};
        info.imageType = VK_IMAGE_TYPE_3D;
        info.pNext = nullptr;
        info.flags = 0; 
    }

    ImageView ImageView::Builder::build() { 
        return pDevice->imageView(info); 
    }
    ImageView::Builder::Builder(Device& device, const Image& image, VkImageViewType viewType) {
        pDevice = &device;
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = image.handle;
        info.pNext = nullptr;
        info.format = image.format;
        info.flags = 0;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = image.arraySize;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = image.mipLevelCount;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    }
}