#pragma once

#include "SGF_Core.hpp"
#include "Swapchain.hpp"

namespace SGF {
    class Display {
	public:
		inline const Swapchain& getSwapchain() const { return swapchain; }

		inline void nextFrame(VkSemaphore imageAvailableSignal, VkFence fence) { currentImageIndex = swapchain.nextImage(imageAvailableSignal, fence); }
		inline void presentFrame(const VkSemaphore* pWaitSemaphores, uint32_t waitCount) const { swapchain.presentImage(currentImageIndex, pWaitSemaphores, waitCount); }
		inline void presentFrame(VkSemaphore waitSemaphore) const { swapchain.presentImage(currentImageIndex, waitSemaphore); }

		inline uint32_t getImageCount() const { return swapchain.getImageCount(); }
		inline uint32_t getImageIndex() const { return currentImageIndex; }

		inline const VkImage* getSwapchainImages() const { return getImagesMod(); }
		inline VkImage getSwapchainImage(uint32_t index) const { assert(index < getImageCount()); return getSwapchainImages()[index]; }
		inline VkImage getCurrentSwapchainImage() const { return getSwapchainImage(getImageIndex()); }

		inline const VkImageView* getSwapchainImageViews() const { return getImageViewsMod(); }
		inline VkImageView getSwapchainImageView(uint32_t index) const { assert(index < attachmentCount); return getSwapchainImageViews()[index]; }
		inline VkImageView getCurrentSwapchainImageView() const { return getSwapchainImageView(getImageIndex()); }

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
		inline VkPresentModeKHR getPresentMode() const { return swapchain.getPresentMode(); }
		inline VkSurfaceFormatKHR getSurfaceFormat() const { return swapchain.getSurfaceFormat(); }
		inline VkFormat getImageFormat() const { return swapchain.getImageFormat(); }
		inline VkColorSpaceKHR getColorSpace() const { return swapchain.getColorSpace(); }

		void updateFramebuffers(VkSurfaceKHR surface, uint32_t width, uint32_t height);
		//inline void updateFramebuffers(const Window& window) { updateFramebuffers(window.getSurface(), window.getWidth(), window.getHeight()); }

	    void updateRenderPass(VkSurfaceKHR surface, uint32_t width, uint32_t height, const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
		inline void updateRenderPass(VkSurfaceKHR surface, uint32_t width, uint32_t height, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses) 
		{ updateRenderPass(surface, width, height, attachments.data(), (uint32_t)attachments.size(), subpasses.data(), subpasses.size()); }

		void updateRenderPass(VkSurfaceKHR surface, uint32_t width, uint32_t height, const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount);
		inline void updateRenderPass(VkSurfaceKHR surface, uint32_t width, uint32_t height, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const const std::vector<VkSubpassDependency>& dependencies) 
		{ updateRenderPass(surface, width, height, attachments.data(), (uint32_t)attachments.size(), subpasses.data(), subpasses.size(), dependencies.data(), dependencies.size()); }
    	inline void updateRenderPass(VkSurfaceKHR surface, uint32_t width, uint32_t height, const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription& subpass) 
		{ updateRenderPass(surface, width, height, pAttachments, attachmentCount, &subpass, 1); }
		inline void updateRenderPass(VkSurfaceKHR surface, uint32_t width, uint32_t height, const std::vector<VkAttachmentDescription>& attachments, const VkSubpassDescription& subpass) 
		{ updateRenderPass(surface, width, height, attachments.data(), attachments.size(), subpass); }

		inline void requestSurfaceFormat(VkSurfaceFormatKHR requested) { swapchain.requestSurfaceFormat(requested); }
		inline void requestColorSpace(VkColorSpaceKHR requested) { swapchain.requestColorSpace(requested); }
		inline void requestImageFormat(VkFormat requested) { swapchain.requestImageFormat(requested); }
		inline void requestPresentMode(VkPresentModeKHR requested) { swapchain.requestPresentMode(requested); }
		inline bool isInitialized() const { return swapchain.isInitialized(); }

		void create(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode, const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, 
			const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
		void create(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode, const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, 
			const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount);
		inline void create(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses)
		{ create(surface, width, height, presentMode, attachments.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size()); }
		inline void create(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode, const std::vector<VkAttachmentDescription>& attachments,
			const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies)
		{ create(surface, width, height, presentMode, attachments.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size(), dependencies.data(), (uint32_t)dependencies.size()); }
		void destroy();
		
		Display(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode, const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, 
			const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
		Display(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode, const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, 
			const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount);
		inline Display(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode, const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses) : 
			Display(surface, width, height, presentMode, attachments.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size()) {}
		inline Display(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode, const std::vector<VkAttachmentDescription>& attachments,
			const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies) : 
			Display(surface, width, height, presentMode, attachments.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size(), dependencies.data(), (uint32_t)dependencies.size()) {}
		inline Display() : renderPass(VK_NULL_HANDLE), attachmentData(nullptr), attachmentCount(0), currentImageIndex(0) {}
		~Display();
	private:
		void createFramebuffers(uint32_t width, uint32_t height);
		void destroyFramebuffers();
		void freeAttachmentData();
		void allocateAttachmentData(uint32_t width, uint32_t height, const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
		void createRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount);

		VkImage* getImagesMod() const;
		VkImageView* getImageViewsMod() const;
		VkFramebuffer* getFramebuffersMod() const;
		VkImage* getAttachmentImagesMod() const;
		VkImageView* getAttachmentImageViewsMod() const;
		VkDeviceMemory& getAttachmentMemory() const;
		void setAttachmentMemory(VkDeviceMemory memory) const;
		VkFormat* getAttachmentFormatsMod() const;
		VkImageUsageFlags* getAttachmentUsagesMod() const;
		VkSampleCountFlagBits* getAttachmentSampleCountsMod() const;
	private:
		friend Device;
        Swapchain swapchain;
		VkRenderPass renderPass;
		char* attachmentData;
		uint32_t attachmentCount;
		uint32_t currentImageIndex;
	};
}