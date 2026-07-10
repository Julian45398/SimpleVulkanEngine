#pragma once

#include <SGF.hpp>
#include <SGF/Core/GPU.hpp>

namespace SGF {
    class Viewport
    {
    public:
        Viewport(GPU::Format colorFormat, GPU::Format depthFormat);
        ~Viewport();
        void Resize(uint32_t width, uint32_t height);
        inline GPU::RenderPass GetRenderPass() const { return renderPass; }
        inline GPU::Image GetColorImage() const { return colorImage; }
        inline GPU::Image GetDepthImage() const { return depthImage; }
        inline GPU::Image GetPickImage() const { return pickImage; }
        inline GPU::ImageView GetColorView() const { return colorImageView; }
        inline GPU::ImageView GetPickView() const { return pickImageView; }
        inline GPU::ImageView GetDepthView() const { return depthImageView; }
        inline GPU::Framebuffer GetFramebuffer() const { return framebuffer; }
        inline GPU::Extent2D GetExtent() const { return extent; }
        inline uint32_t GetWidth() const { return extent.width; }
        inline uint32_t GetHeight() const { return extent.height; }
		inline float GetAspectRatio() const { return (float)extent.width/(float)extent.height; };
    private:
        GPU::RenderPass renderPass = nullptr;
        GPU::Image colorImage = nullptr;
        GPU::Image depthImage = nullptr;
        GPU::Image pickImage = nullptr;
        GPU::Memory deviceMemory = nullptr;
        GPU::ImageView colorImageView = nullptr;
        GPU::ImageView depthImageView = nullptr;
        GPU::ImageView pickImageView = nullptr;
        GPU::Framebuffer framebuffer = nullptr;
        GPU::Extent2D extent = {};
        GPU::Format colorFormat = GPU::Format::MAX_ENUM;
        GPU::Format depthFormat = GPU::Format::MAX_ENUM;
    private:
        void DestroyFramebuffer();
        void CreateFramebuffer();
    };
} // namespace SGF
