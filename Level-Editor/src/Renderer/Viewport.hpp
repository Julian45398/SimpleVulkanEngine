#pragma once

#include <SGF.hpp>

namespace SGF {
    class Viewport
    {
    public:
        Viewport(VkFormat colorFormat, VkFormat depthFormat);
        ~Viewport();
        void Resize(uint32_t width, uint32_t height);
        inline VkRenderPass GetRenderPass() const { return renderPass; }
        inline VkImage GetColorImage() const { return colorImage; }
        inline VkImage GetDepthImage() const { return depthImage; }
        inline VkImage GetPickImage() const { return pickImage; }
        inline VkImageView GetColorView() const { return colorImageView; }
        inline VkImageView GetPickView() const { return pickImageView; }
        inline VkImageView GetDepthView() const { return depthImageView; }
        inline VkFramebuffer GetFramebuffer() const { return framebuffer; }
        inline VkExtent2D GetExtent() const { return extent; }
        inline uint32_t GetWidth() const { return extent.width; }
        inline uint32_t GetHeight() const { return extent.height; }
		inline float GetAspectRatio() const { return (float)extent.width/(float)extent.height; };
    private:
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkImage colorImage = VK_NULL_HANDLE;
        VkImage depthImage = VK_NULL_HANDLE;
        VkImage pickImage = VK_NULL_HANDLE;
        VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
        VkImageView colorImageView = VK_NULL_HANDLE;
        VkImageView depthImageView = VK_NULL_HANDLE;
        VkImageView pickImageView = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkExtent2D extent = {};
        VkFormat colorFormat = VK_FORMAT_MAX_ENUM;
        VkFormat depthFormat = VK_FORMAT_MAX_ENUM;
    private:
        void DestroyFramebuffer();
        void CreateFramebuffer();
    };
} // namespace SGF
