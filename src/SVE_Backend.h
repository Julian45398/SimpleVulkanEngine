#pragma once


#include "core.h"
#include <thread>

#define SVE_RENDER_IN_VIEWPORT

namespace SVE {
	typedef void (*CallbackFunction)(void* data);
	inline constexpr uint32_t FRAMES_IN_FLIGHT = 2;
	void onWindowResize(uint32_t width, uint32_t height);
	void init(uint32_t windowWidth, uint32_t windowHeight);
	void terminate();
	uint32_t addFramebufferResizeCallbackFunction(CallbackFunction callback);
	uint32_t addFramebufferResizeCallbackListener(uint32_t callbackFunctionIndex, void* listener);
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
#ifdef SVE_RENDER_IN_VIEWPORT
		inline uint32_t _ViewportWidth = 0;
		inline uint32_t _ViewportHeight = 0;
		inline VkRenderPass _ViewportRenderPass = VK_NULL_HANDLE;
		inline VkFramebuffer _ViewportFramebuffers[FRAMES_IN_FLIGHT] = {};
		inline VkImage _ViewportImages[FRAMES_IN_FLIGHT] = {};
		inline VkImageView _ViewportImageViews[FRAMES_IN_FLIGHT] = {};
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
	inline VkRenderPass getRenderPass() { return _private::_RenderPass; }
	inline VkQueue getGraphicsQueue() { return _private::_GraphicsQueue; }
	inline uint32_t getGraphicsFamily() { return _private::_GraphicsIndex; }
	inline uint32_t getWindowWidth() { return _private::_WindowWidth; }
	inline uint32_t getWindowHeight() { return _private::_WindowHeight; }
	inline uint32_t getFramebufferWidth() { return _private::_WindowWidth; }
	inline uint32_t getFramebufferHeight() { return _private::_WindowHeight; }
	inline double getFrameTime() { return _private::_FrameTime; }
	inline uint32_t getImageIndex() { return _private::_ImageIndex; }
	inline uint32_t getImageCount() { return (uint32_t)_private::_ImageResources.size(); }
	inline uint32_t getInFlightIndex() { return _private::_InFlightIndex; }
	inline bool shouldClose() { return glfwWindowShouldClose(_private::_Window); }
	inline bool isKeyPressed(int keycode) { return glfwGetKey(_private::_Window, keycode) == GLFW_PRESS; }
	inline bool isMouseClicked(int mousecode) { return glfwGetMouseButton(_private::_Window, mousecode) == GLFW_PRESS; }
	inline float getAspectRatio() { return ((float)_private::_WindowWidth) / ((float)_private::_WindowHeight); }
	inline void hideCursor() { glfwSetInputMode(_private::_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
	inline void showCursor() { glfwSetInputMode(_private::_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
	inline glm::dvec2 getCursorPos() {
		glm::dvec2 pos;
		glfwGetCursorPos(_private::_Window, &pos.x, &pos.y);
		return pos;
	}
	inline void beginRenderCommands(VkCommandBuffer commands, VkCommandBufferUsageFlags flags = VKL_FLAG_NONE) {
		auto inheritance = vkl::createCommandBufferInheritanceInfo(_private::_RenderPass, 0, _private::_ImageResources[_private::_ImageIndex].framebuffer);
		vkl::beginCommandBuffer(commands, VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | flags, &inheritance);
	}
	inline VkCommandBuffer newFrame() {
		_private::_FrameTime = _private::_FrameTimer.ellapsedSeconds();
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
		{
			auto& imgui_commands = _private::_ImageResources[_private::_ImageIndex].imGuiCommands;
			beginRenderCommands(imgui_commands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
			// Record dear imgui primitives into command buffer
			ImGui_ImplVulkan_RenderDrawData(draw_data, imgui_commands);
			vkEndCommandBuffer(imgui_commands);
		}
		const auto& res = _private::_ImageResources[_private::_ImageIndex];
		const auto& sync = _private::_Synchronization[_private::_InFlightIndex];
		{
			VkClearValue clear_values[2] = {};
			clear_values[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
			clear_values[1].depthStencil = { 1.0f, 0 };
		
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = _private::_RenderPass;
			info.framebuffer = res.framebuffer;
			info.renderArea.extent.width = _private::_WindowWidth;
			info.renderArea.extent.height = _private::_WindowHeight;
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
		//std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}