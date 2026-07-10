#include "Viewport.hpp"

namespace SGF {
    Viewport::Viewport(GPU::Format cFormat, GPU::Format dFormat) : colorFormat(cFormat), depthFormat(dFormat) {
		renderPass = GPU::CreateRenderPass()
			.AddColorAttachment(colorFormat, GPU::SampleCount::B1, GPU::ImageLayout::UNDEFINED, GPU::ImageLayout::SHADER_READ_ONLY_OPTIMAL)
			.AddColorAttachment(GPU::Format::R32_UINT, GPU::SampleCount::B1, GPU::ImageLayout::UNDEFINED, GPU::ImageLayout::COLOR_ATTACHMENT_OPTIMAL)
			.AddDepthAttachment(depthFormat, GPU::SampleCount::B1)
			.AddColorReference(0, GPU::ImageLayout::COLOR_ATTACHMENT_OPTIMAL)
			.AddColorReference(1, GPU::ImageLayout::COLOR_ATTACHMENT_OPTIMAL)
			.DepthStencilReference(2, GPU::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.AddSubpassDependency(GPU::SUBPASS_EXTERNAL, 0,
				Flags<GPU::PipelineStage>(GPU::PipelineStage::COLOR_ATTACHMENT_OUTPUT) | GPU::PipelineStage::EARLY_FRAGMENT_TESTS, 
				Flags<GPU::PipelineStage>(GPU::PipelineStage::COLOR_ATTACHMENT_OUTPUT) | GPU::PipelineStage::EARLY_FRAGMENT_TESTS, 
				GPU::Access::NONE, Flags<GPU::Access>(GPU::Access::COLOR_ATTACHMENT_WRITE) | GPU::Access::DEPTH_STENCIL_ATTACHMENT_WRITE)
			.Build();
    }
    Viewport::~Viewport() {
		GPU::Destroy(renderPass);
        DestroyFramebuffer();
    }

    void Viewport::Resize(uint32_t width, uint32_t height) {
        if (extent.width == width && extent.height == height) {
            return;
        } else if (extent.width != 0 && extent.height != 0) {
            DestroyFramebuffer();
        }
        extent.width = width;
        extent.height = height;
        if (width == 0 || height == 0) {
            // Do nothing
        } else {
            CreateFramebuffer();
        }
    }

    void Viewport::DestroyFramebuffer() {
		GPU::Destroy(framebuffer, colorImageView, colorImage, pickImageView, pickImage, depthImageView, depthImage, deviceMemory);
	}
    void Viewport::CreateFramebuffer() {
		colorImage = GPU::CreateImage2D((uint32_t)extent.width, (uint32_t)extent.height, colorFormat, AsFlags(GPU::ImageUsage::COLOR_ATTACHMENT, GPU::ImageUsage::SAMPLED));
		pickImage = GPU::CreateImage2D((uint32_t)extent.width, (uint32_t)extent.height, GPU::Format::R32_UINT, AsFlags(GPU::ImageUsage::COLOR_ATTACHMENT, GPU::ImageUsage::TRANSFER_SRC));
		depthImage = GPU::CreateImage2D((uint32_t)extent.width, (uint32_t)extent.height, depthFormat, GPU::ImageUsage::DEPTH_STENCIL_ATTACHMENT);

		GPU::WaitIdle();
		GPU::Image images[] = { colorImage, pickImage, depthImage };
		deviceMemory = GPU::AllocateMemory(images);
		colorImageView = GPU::CreateImageView2D(colorImage, colorFormat, GPU::ImageAspect::COLOR);
		pickImageView = GPU::CreateImageView2D(pickImage, GPU::Format::R32_UINT, GPU::ImageAspect::COLOR);
		depthImageView = GPU::CreateImageView2D(depthImage, depthFormat, GPU::ImageAspect::DEPTH);
		GPU::ImageView imageViews[] = { colorImageView, pickImageView, depthImageView };
		framebuffer = GPU::CreateFramebuffer(renderPass, imageViews, SGF_ARRAY_SIZE(imageViews), extent.width, extent.height, 1);
    }
}