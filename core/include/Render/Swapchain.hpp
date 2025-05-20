#pragma once

#include "SGF_Core.hpp"

namespace SGF {

#ifndef SGF_SWAPCHAIN_MAX_ATTACHMENT_COUNT
#define SGF_SWAPCHAIN_MAX_ATTACHMENT_COUNT 16
#endif 
	class Swapchain {
		inline static VkSurfaceFormatKHR DEFAULT_SURFACE_FORMAT = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	public:
		inline operator VkSwapchainKHR() const { return handle; }
		inline VkSwapchainKHR getHandle() const { return handle; }
		void nextImage(VkSemaphore imageAvailable, VkFence fence);
		void presentImage(const VkSemaphore* waitSemaphores, uint32_t waitCount) const;
		void presentImage(VkSemaphore waitSemaphore) const;

		inline uint32_t getImageCount() const { return swapchainImageCount; }
		inline uint32_t getImageIndex() const { return currentImageIndex; }

		inline const VkImage* getImages() const { return getImagesMod(); }
		inline VkImage getImage(uint32_t index) const { assert(index < getImageCount()); return getImages()[index]; }
		inline VkImage getCurrentImage() const { return getImage(getImageIndex()); }

		inline const VkImageView* getImageViews() const { return getImageViewsMod(); }
		inline VkImageView getImageView(uint32_t index) const { assert(index < attachmentCount); return getImageViews()[index]; }
		inline VkImageView getCurrentImageView() const { return getImageView(getImageIndex()); }

		inline const VkFramebuffer* getFramebuffers() const { return getFramebuffersMod(); }
		inline VkFramebuffer getFramebuffer(uint32_t index) const { assert(index < getImageCount()); return getFramebuffers()[index]; }
		inline VkFramebuffer getCurrentFramebuffer() const { return getFramebuffer(getImageIndex()); }

		inline uint32_t getAttachmentCount() const { return attachmentCount; }

		inline const VkFormat* getAttachmentFormats() const { return getAttachmentFormatsMod(); }
		inline VkFormat getAttachmentFormat(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentFormats()[index]; }
		inline const VkSampleCountFlagBits* getAttachmentSampleCounts() const { return getAttachmentSampleCountsMod(); }
		inline VkSampleCountFlagBits getAttachmentSampleCount(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentSampleCounts()[index]; }
		inline const VkImageUsageFlags* getAttachmentUsages() const { return getAttachmentUsagesMod(); }
		inline VkImageUsageFlags getAttachmentUsage(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentUsages()[index]; }

		inline const VkImage* getAttachmentImages() const { return getAttachmentImagesMod(); }
		inline VkImage getAttachmentImage(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentImages()[index]; }
		inline const VkImageView* getAttachmentImageViews() const { return getAttachmentImageViewsMod(); }
		inline VkImageView getAttachmentImageView(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentImageViews()[index]; }

		inline VkRenderPass getRenderPass() const { return renderPass; }
		inline VkPresentModeKHR getPresentMode() const { return presentMode; }
		inline VkSurfaceFormatKHR getSurfaceFormat() const { return surfaceFormat; }
		inline VkFormat getImageFormat() const { return surfaceFormat.format; }
		inline VkColorSpaceKHR getColorSpace() const { return surfaceFormat.colorSpace; }

		void updateSwapchain(const Window& window);
		void updateSwapchain(const Window& window, uint32_t width, uint32_t height);
		void updateRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount);

		inline void requestPresentMode(VkPresentModeKHR requested) { presentMode = requested; }
		inline bool isInitialized() const { return handle != VK_NULL_HANDLE; }
		inline ~Swapchain() { destroy(); }
		Swapchain(const Device& device, const Window& window, VkPresentModeKHR presentMode, const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, 
			const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
		void destroy();
	private:
		void createFramebuffers();
		void destroyFramebuffers();
		void freeAttachmentData();
		void allocateAttachmentData(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
		void createRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
		void createSwapchain(const Window& window, uint32_t width, uint32_t height);

		VkImage* getImagesMod() const;
		VkImageView* getImageViewsMod() const;
		VkFramebuffer* getFramebuffersMod() const;
		VkImage* getAttachmentImagesMod() const;
		VkImageView* getAttachmentImageViewsMod() const;
		VkDeviceMemory getAttachmentMemory() const;
		void setAttachmentMemory(VkDeviceMemory memory) const;
		VkFormat* getAttachmentFormatsMod() const;
		VkImageUsageFlags* getAttachmentUsagesMod() const;
		VkSampleCountFlagBits* getAttachmentSampleCountsMod() const;
	private:
		friend Device;
		VkSwapchainKHR handle = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkSurfaceFormatKHR surfaceFormat = DEFAULT_SURFACE_FORMAT;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		char* attachmentData = nullptr;
		uint32_t attachmentCount = 0;
		uint32_t swapchainImageCount = 0;
		uint32_t currentImageIndex = 0;
		uint32_t width = 0;
		uint32_t height = 0;
	};
}