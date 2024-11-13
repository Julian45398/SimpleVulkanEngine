#pragma once

#include "core.h"

void initBackend();

class RenderCore {
public:
	inline static const uint32_t FRAMES_IN_FLIGHT = 2;
	inline static const uint32_t COMMAND_BUFFER_COUNT = 4;
private:
	inline static const VkFormat POSSIBLE_DEPTH_FORMATS[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	inline static const VkPresentModeKHR PRESENT_MODES[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
	inline static const VkFormat SURFACE_FORMATS[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };

	struct SubmitSynchronization {
		VkSemaphore ImageAvailable = VK_NULL_HANDLE;
		VkSemaphore RenderFinished = VK_NULL_HANDLE;
		VkFence Fence = VK_NULL_HANDLE;
	};

	struct ImageResources {
		VkImage image = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VkCommandPool secondaryPools[COMMAND_BUFFER_COUNT] = {};
		VkCommandBuffer secondaryCommands[COMMAND_BUFFER_COUNT] = {};
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	};

	GLFWwindow* Window = nullptr;
	VkInstance Instance = VK_NULL_HANDLE;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
public:
	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	VkDevice LogicalDevice = VK_NULL_HANDLE;
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	VkQueue GraphicsQueue = VK_NULL_HANDLE;
private:
	VkQueue PresentQueue = VK_NULL_HANDLE;
	uint32_t PresentIndex = 0;
public:
	uint32_t GraphicsIndex = 0;
	uint32_t WindowWidth = 0;
	uint32_t WindowHeight = 0;
	uint32_t FrameIndex = 0;
private:
	uint32_t ImageIndex = 0;
	SubmitSynchronization Sync[FRAMES_IN_FLIGHT] = {};
	std::vector<ImageResources> ImageRes;
	VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
	VkImage DepthImage = VK_NULL_HANDLE;
	VkImageView DepthImageView = VK_NULL_HANDLE;
	VkDeviceMemory DepthMemory = VK_NULL_HANDLE;
	uint32_t CommandCount = 0;
public:
	bool shouldRun() {
		return !glfwWindowShouldClose(Window);
	}
	void initialize(uint32_t windowWidth, uint32_t windowHeight);
	void terminate();
	/**
	* Starts new Frame and returns the primary commandbuffer for the frame
	*/
	VkCommandBuffer beginRendering() {
		vkl::waitForFence(LogicalDevice, Sync[FrameIndex].Fence);
		vkResetFences(LogicalDevice, 1, &Sync[FrameIndex].Fence);
		while (true) {
			VkResult err = vkAcquireNextImageKHR(LogicalDevice, Swapchain, UINT64_MAX, Sync[FrameIndex].ImageAvailable, VK_NULL_HANDLE, &ImageIndex);
			if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
			{
				int width, height;
				glfwGetFramebufferSize(Window, &width, &height);
				onWindowResize(width, height);
				shl::logWarn("swapchain out of date!");
			}
			else if (err != VK_SUCCESS) {
				shl::logFatal("failed to acquire swapchain image!");
			}
			else {
				break;
			}
		}
		//ImageIndex = vkl::acquireNextImage(LogicalDevice, Swapchain, Sync[FrameIndex].ImageAvailable, Sync[FrameIndex].Fence);
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		vkl::resetCommandPool(LogicalDevice, ImageRes[ImageIndex].commandPool);
		vkl::beginCommandBuffer(ImageRes[ImageIndex].commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		return ImageRes[ImageIndex].commandBuffer;
	}
	/*
	* Returns the next secondary commandbuffer for render-commands
	*/
	VkCommandBuffer getNextSecondaryCommands() {
		vkl::resetCommandPool(LogicalDevice, ImageRes[ImageIndex].secondaryPools[CommandCount]);
		VkCommandBufferInheritanceInfo inheritance_info = vkl::createCommandBufferInheritanceInfo(RenderPass, 0, ImageRes[ImageIndex].framebuffer);
		VkCommandBuffer commands = ImageRes[ImageIndex].secondaryCommands[CommandCount];
		vkl::beginCommandBuffer(commands, VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, &inheritance_info);
		++CommandCount;
		return commands;
	}
	void finalizeRendering() {
		{
			auto imgui_commands = getNextSecondaryCommands();
			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
			// Record dear imgui primitives into command buffer
			ImGui_ImplVulkan_RenderDrawData(draw_data, imgui_commands);
			vkEndCommandBuffer(imgui_commands);
		}
		const auto& res = ImageRes[ImageIndex];
		const auto& sync = Sync[FrameIndex];
		{
			VkClearValue clear_values[2] = {};
			clear_values[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
			clear_values[1].depthStencil = { 1.0f, 0 };
		
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = RenderPass;
			info.framebuffer = res.framebuffer;
			info.renderArea.extent.width = WindowWidth;
			info.renderArea.extent.height = WindowHeight;
			info.clearValueCount = ARRAY_SIZE(clear_values);
			info.pClearValues = clear_values;
			vkCmdBeginRenderPass(res.commandBuffer, &info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		}
		vkCmdExecuteCommands(res.commandBuffer, CommandCount, res.secondaryCommands);
		vkCmdEndRenderPass(res.commandBuffer);
		vkl::endCommandBuffer(res.commandBuffer);
		vkl::submitCommands(GraphicsQueue, res.commandBuffer, sync.ImageAvailable, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, sync.RenderFinished, sync.Fence);
		vkl::presentSwapchain(PresentQueue, Swapchain, ImageIndex, 1, &sync.RenderFinished);
		FrameIndex = (FrameIndex + 1) % FRAMES_IN_FLIGHT;
		CommandCount = 0;
	}
	void onWindowResize(uint32_t newWidth, uint32_t newHeight);
private:
	void createPresentResources();
	void destroyPresentResources();
	void setupWindow(uint32_t width, uint32_t height);
	void setupVulkanInstance();
	void setupVulkanDevice();
	void setupImGui();
	void setupSynchronization();
	void destroySynchronization();
};


inline RenderCore Core;
