#pragma once

#include "core.h"

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
		inline uint32_t _ViewportOffsetX = 0;
		inline uint32_t _ViewportOffsetY = 0;
#ifdef SVE_RENDER_IN_VIEWPORT
		inline VkRenderPass _ViewportRenderPass = VK_NULL_HANDLE;
		inline VkFramebuffer _ViewportFramebuffers[FRAMES_IN_FLIGHT] = {};
		inline VkImage _ViewportImages[FRAMES_IN_FLIGHT] = {};
		inline VkImageView _ViewportImageViews[FRAMES_IN_FLIGHT] = {};
		inline VkImage _ViewportDepthImage = VK_NULL_HANDLE;
		inline VkImageView _ViewportDepthImageView = VK_NULL_HANDLE;
		inline VkDescriptorSet _ViewportTextureIDs[FRAMES_IN_FLIGHT] = {};
		inline VkSampler _ViewportSampler = VK_NULL_HANDLE;
#endif
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
#ifdef SVE_RENDER_IN_VIEWPORT
			VkSemaphore viewportRenderFinished;
#endif
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
	inline VkDevice getDevice() { return _private::_Logical; }
	inline VkPhysicalDevice getPhysicalDevice() { return _private::_Physical; }
	inline VkRenderPass getRenderPass() { 
#ifdef SVE_RENDER_IN_VIEWPORT
		return _private::_ViewportRenderPass;
#else
		return _private::_RenderPass;
#endif
	}
	inline VkFramebuffer getRenderFramebuffer(uint32_t index = 
#ifdef SVE_RENDER_IN_VIEWPORT
		_private::_InFlightIndex
#else
		_private::_ImageIndex
#endif
	) {
#ifdef SVE_RENDER_IN_VIEWPORT
		return _private::_ViewportFramebuffers[index];
#else
		return _private::_ImageResources[index].framebuffer;
#endif
	}
	inline VkQueue getGraphicsQueue() { return _private::_GraphicsQueue; }
	inline uint32_t getGraphicsFamily() { return _private::_GraphicsIndex; }
	inline uint32_t getWindowWidth() { return _private::_WindowWidth; }
	inline uint32_t getWindowHeight() { return _private::_WindowHeight; }
	inline uint32_t getViewportWidth() { return _private::_ViewportWidth; }
	inline uint32_t getViewportHeight() { return _private::_ViewportHeight; }
	inline uint32_t getViewportOffsetX() { return _private::_ViewportOffsetX; }
	inline uint32_t getViewportOffsetY() { return _private::_ViewportOffsetY; }
	inline uint32_t getFramebufferWidth() { 
#ifdef SVE_RENDER_IN_VIEWPORT
		return _private::_ViewportWidth; 
#else
		return _private::_WindowWidth; 
#endif
	}
	inline uint32_t getFramebufferHeight() { 
#ifdef SVE_RENDER_IN_VIEWPORT
		return _private::_ViewportHeight; 
#else
		return _private::_WindowHeight; 
#endif
	}
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
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
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
#ifdef SVE_RENDER_IN_VIEWPORT

			ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoCollapse);                          // Create a window called "Hello, world!" and append into it.

			//ImGui::SetWindowPos(ImVec2(0.5f, 0.5f));
			ImGui::Image((ImTextureID)SVE::_private::_ViewportTextureIDs[SVE::getInFlightIndex()], { (float)getFramebufferWidth(), (float)getFramebufferHeight()});
			ImGui::End();
			vkl::beginCommandBuffer(imgui_commands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			{
				VkClearValue clear_values[2] = {};
				clear_values[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
				clear_values[1].depthStencil = { 1.0f, 0 };
			
				VkRenderPassBeginInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				info.renderPass = _private::_RenderPass;
				info.framebuffer = res.framebuffer;
				info.renderArea.extent.width = SVE::getWindowWidth();
				info.renderArea.extent.height = SVE::getWindowHeight();
				info.clearValueCount = ARRAY_SIZE(clear_values);
				info.pClearValues = clear_values;
				vkCmdBeginRenderPass(imgui_commands, &info, VK_SUBPASS_CONTENTS_INLINE);
			}
#else
			beginRenderCommands(imgui_commands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
#endif
			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
			// Record dear imgui primitives into command buffer
			ImGui_ImplVulkan_RenderDrawData(draw_data, imgui_commands);
#ifdef SVE_RENDER_IN_VIEWPORT
			vkCmdEndRenderPass(imgui_commands);
#endif
			vkEndCommandBuffer(imgui_commands);
		}
		{
			VkClearValue clear_values[2] = {};
			clear_values[0].color = { {0.1f, 0.5f, 0.1f, 1.0f} };
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
#ifndef SVE_RENDER_IN_VIEWPORT
		vkCmdExecuteCommands(res.primaryCommands, 1, &res.imGuiCommands);
#endif // !SVE_RENDER_IN_VIEWPORT:w
		vkCmdEndRenderPass(res.primaryCommands);
		vkl::endCommandBuffer(res.primaryCommands);
#ifdef SVE_RENDER_IN_VIEWPORT
		vkl::submitCommands(_private::_GraphicsQueue, res.primaryCommands, sync.imageAvailable, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, sync.viewportRenderFinished, VK_NULL_HANDLE);
		vkl::submitCommands(_private::_GraphicsQueue, res.imGuiCommands, sync.viewportRenderFinished, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, sync.renderFinished, sync.fence);
#else
		vkl::submitCommands(_private::_GraphicsQueue, res.primaryCommands, sync.imageAvailable, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, sync.renderFinished, sync.fence);
#endif
		vkl::presentSwapchain(_private::_PresentQueue, _private::_Swapchain, _private::_ImageIndex, 1, &sync.renderFinished);
		_private::_InFlightIndex = (_private::_InFlightIndex + 1) % FRAMES_IN_FLIGHT;
	}
}