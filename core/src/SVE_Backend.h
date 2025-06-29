#pragma once

#include "core.h"
#include <nfd.h>
//#define SVE_RENDER_IN_VIEWPORT


namespace SVE {
	typedef void (*CallbackFunction)(void* data);
	inline constexpr uint32_t FRAMES_IN_FLIGHT = 2;
	void onWindowResize(uint32_t width, uint32_t height);
	void init(uint32_t windowWidth, uint32_t windowHeight);
	void terminate();
	uint32_t addFramebufferResizeCallbackFunction(CallbackFunction callback);
	uint32_t addFramebufferResizeCallbackListener(uint32_t callbackFunctionIndex, void* listener);
	void setViewport(uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset);
	//void removeCallbackListener(uint32_t listenerIndex);
	namespace _private {
		inline VkInstance _Instance = VK_NULL_HANDLE;
		inline GLFWwindow* _Window = nullptr;
		inline VkSurfaceKHR _Surface = VK_NULL_HANDLE;
		inline VkPhysicalDevice _Physical = VK_NULL_HANDLE;
		inline VkDevice _Logical = VK_NULL_HANDLE;
		inline VkSwapchainKHR _Swapchain = VK_NULL_HANDLE;
		inline VkRenderPass _RenderPass = VK_NULL_HANDLE;
		inline VkImage _DepthImage = VK_NULL_HANDLE;
		inline VkImageView _DepthImageView = VK_NULL_HANDLE;
		inline VkDeviceMemory _AttachmentMemory = VK_NULL_HANDLE;
		inline VkQueue _GraphicsQueue = VK_NULL_HANDLE;
		inline VkQueue _PresentQueue = VK_NULL_HANDLE;
		inline shl::Timer _FrameTimer;
		inline double _FrameTime = 0;
		inline uint32_t _PresentIndex = 0;
		inline uint32_t _GraphicsIndex = 0;
		inline uint32_t _WindowWidth = 0;
		inline uint32_t _WindowHeight = 0;
		inline uint32_t _ViewportWidth = 1;
		inline uint32_t _ViewportHeight = 1;
		inline int32_t _ViewportOffsetX = 0;
		inline int32_t _ViewportOffsetY = 0;

		struct ImageResource {
			VkImage image;
			VkImageView imageView;
			VkFramebuffer framebuffer;
			VkCommandPool commandPool;
			VkCommandBuffer primaryCommands;
			VkCommandBuffer imGuiCommands;
		};
		inline std::vector<ImageResource> _ImageResources;
		struct InFlightSynchronization {
			VkFence fence;
			VkSemaphore imageAvailable;
			VkSemaphore renderFinished;
		};
		inline InFlightSynchronization _Synchronization[FRAMES_IN_FLIGHT] = {};
		inline uint32_t _ImageIndex = 0;
		inline uint32_t _InFlightIndex = 0;
		struct CallbackListener {
			void* data;
			uint32_t callbackIndex;
		};
		inline std::vector<CallbackFunction> _FramebufferResizeCallbackFunctions;
		inline std::vector<CallbackListener> _FramebufferResizeCallbackListeners;
	}

#pragma region GETTERS
	inline VkDevice getDevice() { return _private::_Logical; }
	inline VkPhysicalDevice getPhysicalDevice() { return _private::_Physical; }
	inline VkRenderPass getRenderPass() { 
		return _private::_RenderPass;
	}
	inline VkFramebuffer getRenderFramebuffer(uint32_t index = _private::_ImageIndex) { return _private::_ImageResources[index].framebuffer; }
	inline VkQueue getGraphicsQueue() { return _private::_GraphicsQueue; }
	inline uint32_t getGraphicsFamily() { return _private::_GraphicsIndex; }
	inline uint32_t getWindowWidth() { return _private::_WindowWidth; }
	inline uint32_t getWindowHeight() { return _private::_WindowHeight; }
	inline uint32_t getViewportWidth() { return _private::_ViewportWidth; }
	inline uint32_t getViewportHeight() { return _private::_ViewportHeight; }
	inline int32_t getViewportOffsetX() { return _private::_ViewportOffsetX; }
	inline int32_t getViewportOffsetY() { return _private::_ViewportOffsetY; }
	inline uint32_t getFramebufferWidth() { return _private::_WindowWidth; }
	inline uint32_t getFramebufferHeight() { return _private::_WindowHeight; }
	inline double getFrameTime() { return _private::_FrameTime; }
	inline uint32_t getImageIndex() { return _private::_ImageIndex; }
	inline uint32_t getImageCount() { return (uint32_t)_private::_ImageResources.size(); }
	inline uint32_t getInFlightIndex() { return _private::_InFlightIndex; }
	inline bool shouldClose() { return glfwWindowShouldClose(_private::_Window); }
	inline bool isKeyPressed(int keycode) { return glfwGetKey(_private::_Window, keycode) == GLFW_PRESS; }
	inline bool isMouseClicked(int mousecode) { return glfwGetMouseButton(_private::_Window, mousecode) == GLFW_PRESS; }
	inline float getAspectRatio() { return ((float)_private::_ViewportWidth) / ((float)_private::_ViewportHeight); }
	inline void hideCursor() { glfwSetInputMode(_private::_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
	inline void showCursor() { glfwSetInputMode(_private::_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
	inline glm::dvec2 getCursorPos() {
		glm::dvec2 pos;
		glfwGetCursorPos(_private::_Window, &pos.x, &pos.y);
		return pos;
	}
#pragma endregion GETTERS

#pragma region VULKAN
	// Commands: 
	inline VkCommandPool createCommandPool(VkCommandPoolCreateFlags flags = VKL_FLAG_NONE) {
		return vkl::createCommandPool(_private::_Logical, _private::_GraphicsIndex, flags);
	}
	inline void destroyCommandPool(VkCommandPool commandPool) {
		vkl::destroyCommandPool(_private::_Logical, commandPool);
	}
	inline void resetCommandPool(VkCommandPool commandPool, VkCommandPoolResetFlags resetFlags = VKL_FLAG_NONE) {
		vkl::resetCommandPool(_private::_Logical, commandPool, resetFlags);
	}
	inline VkCommandBuffer createCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
		return vkl::createCommandBuffer(_private::_Logical, commandPool, level);
	}
	inline std::vector<VkCommandBuffer> createCommandBuffers(VkCommandPool commandPool, uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
		std::vector<VkCommandBuffer> buffers(count);
		auto info = vkl::createCommandBufferAllocateInfo(commandPool, count, level);
		if (vkAllocateCommandBuffers(_private::_Logical, &info, buffers.data()) != VK_SUCCESS) {
			shl::logError("failed to create commandBuffers");
			buffers.resize(0);
		}
		return buffers;
	}
	inline void destroyCommandBuffers(VkCommandPool commandPool, uint32_t count, VkCommandBuffer* commandBuffers) {
		vkFreeCommandBuffers(_private::_Logical, commandPool, count, commandBuffers);
	}
	inline void destroyCommandBuffer(VkCommandPool commandPool, VkCommandBuffer commandBuffer) {
		vkFreeCommandBuffers(_private::_Logical, commandPool, 1, &commandBuffer);
	}

	// Synchronization:
	inline VkFence createFence(VkFenceCreateFlags flags = VKL_FLAG_NONE) { return vkl::createFence(_private::_Logical, flags); }
	inline void destroyFence(VkFence fence) { vkl::destroyFence(_private::_Logical, fence); }
	inline void waitForFence(VkFence fence) { vkl::waitForFence(_private::_Logical, fence); }
	inline void resetFence(VkFence fence) { vkl::resetFence(_private::_Logical, fence); }
	inline bool isFenceSignaled(VkFence fence) { return vkGetFenceStatus(_private::_Logical, fence) == VK_SUCCESS; }
	inline VkSemaphore createSemaphore() { return vkl::createSemaphore(_private::_Logical); }
	inline void destroySemaphore(VkSemaphore semaphore) { vkl::destroySemaphore(_private::_Logical, semaphore); }
	inline void deviceWaitIdle() { vkDeviceWaitIdle(_private::_Logical); }

	// Buffers:
	inline VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags createFlags = VKL_FLAG_NONE) {
		return vkl::createBuffer(_private::_Logical, size, usage, _private::_GraphicsIndex, createFlags);
	}
	inline void destroyBuffer(VkBuffer buffer) {
		vkl::destroyBuffer(_private::_Logical, buffer);
	}
	inline VkImage createImage2D(uint32_t width, uint32_t height, VkImageUsageFlags usage, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB,
		uint32_t mipLevels = 1, uint32_t arrayLayers = 1, uint32_t sampleCount = 1, VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL) {
		return vkl::createImage2D(_private::_Logical, imageFormat, width, height, usage, mipLevels, arrayLayers, sampleCount, imageTiling);
	}
	inline VkImage createImage3D(uint32_t width, uint32_t height, uint32_t depth, VkImageUsageFlags usage, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, uint32_t sampleCount = 1, VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL) {
		return vkl::createImage(_private::_Logical, VK_IMAGE_TYPE_3D, imageFormat, { width, height, depth }, usage, mipLevels, arrayLayers, sampleCount, imageTiling);
	}
	inline VkImageView createImageView2D(VkImage image, VkFormat viewFormat, VkImageAspectFlags imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t mipLevelCount = 1, uint32_t arrayLayer = 0) {
		return vkl::createImageView2D(_private::_Logical, image, viewFormat, imageAspectFlags, mipLevel, mipLevelCount, arrayLayer);
	}
	inline void destroyImage(VkImage image) {
		vkl::destroyImage(_private::_Logical, image);
	}
	inline void destroyImageView(VkImageView imageView) {
		vkl::destroyImageView(_private::_Logical, imageView);
	}
	inline VkSampler createSampler(VkFilter filter = VK_FILTER_NEAREST, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
		return vkl::createSampler(_private::_Logical, filter, mipmapMode, addressMode);
	}
	inline void destroySampler(VkSampler sampler) {
		vkl::destroySampler(_private::_Logical, sampler);
	}
	// Memory:
	inline VkDeviceMemory allocateMemory(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
		return vkl::allocateMemory(_private::_Logical, _private::_Physical, memoryRequirements, properties);
	}
	inline VkMemoryRequirements getBufferMemoryRequirements(VkBuffer buffer) {
		return vkl::getBufferMemoryRequirements(_private::_Logical, buffer);
	}
	inline VkMemoryRequirements getImageMemoryRequirements(VkImage image) {
		return vkl::getImageMemoryRequirements(_private::_Logical, image);
	}
	inline VkDeviceMemory allocateAndBind(uint32_t bufferCount, const VkBuffer* pBuffers, uint32_t imageCount, const VkImage* pImages, VkMemoryPropertyFlags properties) {
		return vkl::allocateAndBind(_private::_Logical, _private::_Physical, bufferCount, pBuffers, imageCount, pImages, properties);
	}
	inline VkDeviceMemory allocateForStagingBuffer(VkBuffer buffer) {
		return vkl::allocateForBuffer(_private::_Logical, _private::_Physical, buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
	inline VkDeviceMemory allocateForBuffer(VkBuffer buffer, VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
		return vkl::allocateForBuffer(_private::_Logical, _private::_Physical, buffer, memoryProperties);
	}
	inline VkDeviceMemory allocateForImage(VkImage image, VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
		return vkl::allocateForImage(_private::_Logical, _private::_Physical, image, memoryProperties);
	}
	inline void bindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize offset = 0) {
		vkl::bindImageMemory(_private::_Logical, image, memory, offset);
	}
	inline void bindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset = 0) {
		vkl::bindBufferMemory(_private::_Logical, buffer, memory, offset);
	}
	inline void* mapMemory(VkDeviceMemory memory, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0, VkMemoryMapFlags mapFlags = VKL_FLAG_NONE) {
		return vkl::mapMemory(_private::_Logical, memory, size, offset, mapFlags);
	}
	inline void unmapMemory(VkDeviceMemory memory) { vkUnmapMemory(_private::_Logical, memory); }
	inline void submitCommands(VkCommandBuffer commands, VkSemaphore waitSemaphore, VkPipelineStageFlags waitStages, VkSemaphore signalSemaphore, VkFence fence) {
		vkl::submitCommands(_private::_GraphicsQueue, commands, waitSemaphore, waitStages, signalSemaphore, fence);
	}
	inline void freeMemory(VkDeviceMemory memory) {
		vkl::freeMemory(_private::_Logical, memory);
	}

	// Pipeline:
	inline VkShaderModule createShaderModule(size_t codeSize, const uint32_t* pCode) {
		return vkl::createShaderModule(_private::_Logical, codeSize, pCode);
	}
	inline void destroyShaderModule(VkShaderModule shaderModule) {
		vkl::destroyShaderModule(_private::_Logical, shaderModule);
	}
	inline VkPipeline createRenderPipeline(const VkGraphicsPipelineCreateInfo& createInfo) {
		return vkl::createGraphicsPipeline(_private::_Logical, createInfo);
	}
	inline void destroyPipeline(VkPipeline pipeline) {
		vkl::destroyPipeline(_private::_Logical, pipeline);
	}
	inline VkPipelineLayout createPipelineLayout(uint32_t setCount, const VkDescriptorSetLayout* sets, uint32_t pushConstantCount = 0, const VkPushConstantRange* ranges = nullptr) {
		return vkl::createPipelineLayout(_private::_Logical, setCount, sets, pushConstantCount, ranges);
	}
	inline void destroyPipelineLayout(VkPipelineLayout pipelineLayout) {
		vkl::destroyPipelineLayout(_private::_Logical, pipelineLayout);
	}

	// Descriptor Sets:
	inline void destroyDescriptorSet(VkDescriptorPool pool, VkDescriptorSet descriptorSet) {
		vkFreeDescriptorSets(_private::_Logical, pool, 1, &descriptorSet);
	}
	inline void destroyDescriptorPool(VkDescriptorPool descriptorPool) {
		vkl::destroyDescriptorPool(_private::_Logical, descriptorPool);
	}
	inline VkDescriptorSetLayout createDescriptorSetLayout(uint32_t bindingCount, const VkDescriptorSetLayoutBinding* pBindings) {
		return vkl::createDescriptorSetLayout(_private::_Logical, bindingCount, pBindings);
	}
	inline void destroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
		vkl::destroyDescriptorSetLayout(_private::_Logical, descriptorSetLayout);
	}

	inline void submitCommands(VkCommandBuffer commands, VkFence fence) {
		vkl::submitCommands(_private::_GraphicsQueue, commands, fence);
	}
	inline void lazyUpload(VkBuffer targetBuffer, VkDeviceSize dataSize, const void* data, VkDeviceSize targetOffset) {
		VkBuffer staging_buf = createBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		VkDeviceMemory staging_mem = allocateForBuffer(staging_buf, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		void* mapped_mem = mapMemory(staging_mem);
		memcpy(mapped_mem, data, dataSize);
		auto fence = createFence();
		auto pool = createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		auto commands = createCommandBuffer(pool);
		vkl::beginCommandBuffer(commands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VkBufferCopy region = { 0, targetOffset, dataSize };
		vkCmdCopyBuffer(commands, staging_buf, targetBuffer, 1, &region);
		vkl::endCommandBuffer(commands);
		submitCommands(commands, fence);
		waitForFence(fence);
		destroyCommandPool(pool);
		freeMemory(staging_mem);
		destroyBuffer(staging_buf);
	}

#pragma endregion VULKAN
	inline void beginRenderCommands(VkCommandBuffer commands, VkCommandBufferUsageFlags flags = VKL_FLAG_NONE) {
		auto inheritance = vkl::createCommandBufferInheritanceInfo(getRenderPass(), 0, getRenderFramebuffer());
		vkl::beginCommandBuffer(commands, VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | flags, &inheritance);
	}
	inline VkCommandBuffer newFrame() {
		_private::_FrameTime = _private::_FrameTimer.ellapsedMillis();
		vkl::waitForFence(_private::_Logical, _private::_Synchronization[_private::_InFlightIndex].fence);
		vkl::resetFence(_private::_Logical, _private::_Synchronization[_private::_InFlightIndex].fence);
		while (true) {
			VkResult err = vkAcquireNextImageKHR(_private::_Logical, _private::_Swapchain, UINT64_MAX, _private::_Synchronization[_private::_InFlightIndex].imageAvailable, VK_NULL_HANDLE, &_private::_ImageIndex);
			if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
			{
				int width, height;
				glfwGetFramebufferSize(_private::_Window, &width, &height);
				//onWindowResize(width, height);
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
		/*
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		*/
		ImGui::NewFrame();
		vkl::resetCommandPool(_private::_Logical, _private::_ImageResources[_private::_ImageIndex].commandPool);
		vkl::beginCommandBuffer(_private::_ImageResources[_private::_ImageIndex].primaryCommands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		return _private::_ImageResources[_private::_ImageIndex].primaryCommands;
	}

	inline void renderFrame(uint32_t commandCount, const VkCommandBuffer* commandBuffer) {
		const auto& res = _private::_ImageResources[_private::_ImageIndex];
		const auto& sync = _private::_Synchronization[_private::_InFlightIndex];
		{
			auto& imgui_commands = _private::_ImageResources[_private::_ImageIndex].imGuiCommands;
			beginRenderCommands(imgui_commands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
			// Record dear imgui primitives into command buffer
			//ImGui_ImplVulkan_RenderDrawData(draw_data, imgui_commands);
			vkEndCommandBuffer(imgui_commands);
		}
		{
			VkClearValue clear_values[2] = {};
			clear_values[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
			clear_values[1].depthStencil = { 1.0f, 0 };
		
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = SVE::getRenderPass();
			info.framebuffer = SVE::getRenderFramebuffer();
			info.renderArea.extent.width = SVE::getFramebufferWidth();
			info.renderArea.extent.height = SVE::getFramebufferHeight();
			info.clearValueCount = ARRAY_SIZE(clear_values);
			info.pClearValues = clear_values;
			vkCmdBeginRenderPass(res.primaryCommands, &info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		}
		vkCmdExecuteCommands(res.primaryCommands, commandCount, commandBuffer);
		vkCmdExecuteCommands(res.primaryCommands, 1, &res.imGuiCommands);
		vkCmdEndRenderPass(res.primaryCommands);
		vkl::endCommandBuffer(res.primaryCommands);
		vkl::submitCommands(_private::_GraphicsQueue, res.primaryCommands, sync.imageAvailable, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, sync.renderFinished, sync.fence);
		vkl::presentSwapchain(_private::_PresentQueue, _private::_Swapchain, _private::_ImageIndex, 1, &sync.renderFinished);
		_private::_InFlightIndex = (_private::_InFlightIndex + 1) % FRAMES_IN_FLIGHT;
	}

	std::string openFileDialog(uint32_t filtercount, const nfdu8filteritem_t* filters);
	std::string saveFileDialog(uint32_t filterCount, const nfdu8filteritem_t* filters);
}
