#include "Viewport.hpp"

namespace SGF {

    Viewport::Viewport(VkFormat cFormat, VkFormat dFormat) : colorFormat(cFormat), depthFormat(dFormat) {
        auto& device = Device::Get();
		const std::vector<VkAttachmentDescription> attachments = {
			SGF::Vk::CreateAttachmentDescription(colorFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_DONT_CARE),
			SGF::Vk::CreateAttachmentDescription(VK_FORMAT_R32_UINT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE),
			SGF::Vk::CreateAttachmentDescription(VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE),
		};
		auto colorRef = SGF::Vk::CreateAttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		auto depthRef = SGF::Vk::CreateAttachmentReference(2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		auto modelRef = SGF::Vk::CreateAttachmentReference(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference colorRefs[] = {
			colorRef, modelRef
		};

		const std::vector<VkSubpassDescription> subpasses = {
			SGF::Vk::CreateSubpassDescription(colorRefs, ARRAY_SIZE(colorRefs), nullptr, &depthRef)
		};
		const std::vector<VkSubpassDependency> dependencies = {
			{ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT }
		};
		renderPass = device.CreateRenderPass(attachments, subpasses, dependencies);
    }
    Viewport::~Viewport() {
        auto& device = Device::Get();
        device.Destroy(renderPass);
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
		auto& device = Device::Get();
		device.Destroy(framebuffer, colorImageView, colorImage, pickImageView, pickImage, depthImageView, depthImage, deviceMemory);
	}
    void Viewport::CreateFramebuffer() {
        auto& device = Device::Get();

		colorImage = device.CreateImage2D((uint32_t)extent.width, (uint32_t)extent.height, colorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		pickImage = device.CreateImage2D((uint32_t)extent.width, (uint32_t)extent.height, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		depthImage = device.CreateImage2D((uint32_t)extent.width, (uint32_t)extent.height, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		//depthImage = CreateImage(info);

		device.WaitIdle();
		VkImage images[] = { colorImage, pickImage, depthImage };
		deviceMemory = device.AllocateMemory(images, ARRAY_SIZE(images));
		colorImageView = device.CreateImageView2D(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		pickImageView = device.CreateImageView2D(pickImage, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT);
		depthImageView = device.CreateImageView2D(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		VkImageView imageViews[] = { colorImageView, pickImageView, depthImageView };
		framebuffer = device.CreateFramebuffer(renderPass, imageViews, ARRAY_SIZE(imageViews), extent.width, extent.height, 1);
    }
}