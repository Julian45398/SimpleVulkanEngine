#pragma once

#include <SGF.hpp>

namespace SGF {
    class Viewport
    {
    public:
        Viewport(VkFormat colorFormat, VkFormat depthFormat);
        ~Viewport();
        void Resize(uint32_t width, uint32_t height);
        inline VkRenderPass GetRenderPass() { return renderPass; }
        inline VkImage GetColorImage() { return colorImage; }
        inline VkImage GetDepthImage() { return depthImage; }
        inline VkImageView GetColorView() { return colorImageView; }
        inline VkImageView GetDepthView() { return depthImageView; }
        inline VkFramebuffer GetFramebuffer() { return framebuffer; }
        inline VkExtent2D GetExtent() const { return extent; }
        inline uint32_t GetWidth() const { return extent.width; }
        inline uint32_t GetHeight() const { return extent.height; }
		inline float GetAspectRatio() { return (float)extent.width/(float)extent.height; };
    private:
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkImage colorImage = VK_NULL_HANDLE;
        VkImage depthImage = VK_NULL_HANDLE;
        VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
        VkImageView colorImageView = VK_NULL_HANDLE;
        VkImageView depthImageView = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkExtent2D extent = {};
        VkFormat colorFormat = VK_FORMAT_MAX_ENUM;
        VkFormat depthFormat = VK_FORMAT_MAX_ENUM;
    private:
        void DestroyFramebuffer();
        void CreateFramebuffer();
    };
} // namespace SGF
