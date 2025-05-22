#pragma once

#include "SGF_Core.hpp"
#include "Swapchain.hpp"


namespace SGF {
    class Display {
	public:
		inline operator VkSwapchainKHR() const { return handle; }
		inline VkSwapchainKHR getHandle() const { return handle; }
		void nextFrame(VkSemaphore imageAvailable, VkFence fence);
		void presentFrame(const VkSemaphore* waitSemaphores, uint32_t waitCount) const;
		void presentFrame(VkSemaphore waitSemaphore) const;

		const Device& getDevice();
		inline uint32_t getImageCount() const { return swapchainImageCount; }
		inline uint32_t getImageIndex() const { return currentImageIndex; }
		inline uint32_t getWidth() const { return width; }
		inline uint32_t getHeight() const { return height; }

		inline const VkImage* getSwapchainImages() const { return getImagesMod(); }
		inline VkImage getSwapchainImage(uint32_t index) const { assert(index < getImageCount()); return getImages()[index]; }
		inline VkImage getCurrentSwapchainImage() const { return getImage(getImageIndex()); }

		inline const VkImageView* getSwapchainImageViews() const { return getImageViewsMod(); }
		inline VkImageView getSwapchainImageView(uint32_t index) const { assert(index < attachmentCount); return getImageViews()[index]; }
		inline VkImageView getCurrentSwapchainImageView() const { return getImageView(getImageIndex()); }

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

		void resizeFramebuffers(uint32_t width, uint32_t height);
		inline void resizeFramebuffers(const Window& window) { resizeFramebuffers(window.getWidth(), window.getHeight()); }

	    void updateRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
		inline void updateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses) { updateRenderPass(attachments.data(), (uint32_t)attachments.size(), subpasses.data(), subpasses.size()); }
		void updateRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount);
		inline void updateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const const std::vector<VkSubpassDependency>& dependencies) { updateRenderPass(attachments.data(), (uint32_t)attachments.size(), subpasses.data(), subpasses.size(), dependencies.data(), dependencies.size()); }
    	inline void updateRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription& subpass) { updateRenderPass(pAttachments, attachmentCount, &subpass, 1); }
		inline void updateRenderPass(const std::vector<VkAttachmentDescription> attachments, const VkSubpassDescription& subpass) { updateRenderPass(attachments.data(), attachments.size(), subpass); }
		
		inline void requestPresentMode(VkPresentModeKHR requested) { presentMode = requested; }
		inline bool isInitialized() const { return handle != VK_NULL_HANDLE; }
		void destroy();
		Display(const Device& device, const Window& window, VkPresentModeKHR presentMode, const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, 
			const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
		Display(const Device& device, const Window& window, VkPresentModeKHR presentMode, const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, 
			const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount);
		inline Display(const Device& device, const Window& window, VkPresentModeKHR presentMode, const std::vector<VkAttachmentDescription>& attachments,
			const std::vector<VkSubpassDescription>& subpasses) {
			Display(device, window, presentMode, attachments.data(), attachments.size(), subpasses.data(), subpasses.size());
		}
		inline Display(const Device& device, const Window& window, VkPresentModeKHR presentMode, const std::vector<VkAttachmentDescription>& attachments,
			const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies) {
			Display(device, window, presentMode, attachments.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size(), dependencies.data(), (uint32_t)dependencies.size());
		}
		
		inline ~Display() { destroy(); }
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
        Swapchain swapchain;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		char* attachmentData = nullptr;
		uint32_t attachmentCount = 0;
		uint32_t currentImageIndex = 0;
	};
}