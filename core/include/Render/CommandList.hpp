#pragma once

#include "SGF_Core.hpp"

#include "Device.hpp"
#include "Window.hpp"

namespace SGF {
//inline VkClearValue createColorClearValue(int32_t r, int32_t g, int32_t b, int32_t a) { return { r, g, b , a }; }
	//inline VkClearValue createColorClearValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a) { return { r, g, b , a }; }

	class CommandList {
	public:
		inline CommandList(const Device& device, QueueFamilyFlagBits queueType, uint32_t queueIndex, VkCommandBufferLevel level, VkCommandPoolCreateFlags flags) {
			uint32_t queueFamilyIndex;
			assert(queueType == QUEUE_FAMILY_COMPUTE || queueType == QUEUE_FAMILY_GRAPHICS || queueType == QUEUE_FAMILY_PRESENT);
			if (queueType == QUEUE_FAMILY_GRAPHICS) {
				queueFamilyIndex = device.GetGraphicsFamily();
				queue = device.GetGraphicsQueue(queueIndex);
			} else if (queueType == QUEUE_FAMILY_COMPUTE) {
				queueFamilyIndex = device.GetComputeFamily();
				queue = device.GetComputeQueue(queueIndex);
			} else if (queueType == QUEUE_FAMILY_TRANSFER) {
				queueFamilyIndex = device.GetTransferFamily();
				queue = device.GetTransferQueue(queueIndex);
			}
			else {
				queueFamilyIndex = UINT32_MAX;
				queue = VK_NULL_HANDLE;
			}
			fence = device.CreateFenceSignaled();
			commandPool = device.CreateCommandPool(queueFamilyIndex, flags);
			commands = device.AllocateCommandBuffer(commandPool, level);
		}
		inline ~CommandList() {
			auto& device = Device::Get();
			device.WaitFence(fence);
			device.Destroy(commandPool, fence);
		}
		inline operator VkCommandBuffer() const { return commands; }
		inline operator VkCommandPool() const { return commandPool; }
		inline VkCommandBuffer GetCommands() const { return commands; }
		inline VkCommandPool GetCommandPool() const { return commandPool; }
		inline void Begin(VkCommandBufferUsageFlags flags = FLAG_NONE) {
			auto& device = Device::Get();
			device.WaitFence(fence);
			device.Reset(fence);
			device.Reset(commandPool);
			VkCommandBufferBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = flags;
			info.pInheritanceInfo = nullptr;
			info.pNext = nullptr;
			vkBeginCommandBuffer(commands, &info);
		}
		inline void ContinueRenderPass(VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer, VkCommandBufferUsageFlags flags, VkBool32 occlusionQueryEnable = VK_FALSE, VkQueryControlFlags queryFlags = FLAG_NONE, VkQueryPipelineStatisticFlags pipelineStatistics = FLAG_NONE) {
			VkCommandBufferInheritanceInfo inheritanceInfo;
			inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritanceInfo.pNext = nullptr;
			inheritanceInfo.renderPass = renderPass;
			inheritanceInfo.framebuffer = framebuffer;
			inheritanceInfo.queryFlags = queryFlags;
			inheritanceInfo.occlusionQueryEnable = occlusionQueryEnable;
			inheritanceInfo.pipelineStatistics = pipelineStatistics;
			VkCommandBufferBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = flags | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			info.pInheritanceInfo = &inheritanceInfo;
			info.pNext = nullptr;
			vkBeginCommandBuffer(commands, &info);
		}
		inline void End() {
			vkEndCommandBuffer(commands);
		}
		inline void BeginRenderPass(const VkRenderPassBeginInfo& info, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
			vkCmdBeginRenderPass(commands, &info, subpassContents);
		}
		inline void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea, const VkClearValue* pClearValues, uint32_t clearValueCount, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
			VkRenderPassBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.renderPass = renderPass;
			info.framebuffer = framebuffer;
			info.renderArea = renderArea;
			info.pClearValues = pClearValues;
			info.clearValueCount = clearValueCount;
			BeginRenderPass(info, subpassContents);
		}
		inline void BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea, const std::vector<VkClearValue>& clearValues, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const {
			BeginRenderPass(renderPass, framebuffer, renderArea, clearValues.data(), (uint32_t)clearValues.size(), subpassContents);
		}
		inline void BeginRenderPass(const Window& window) const {
			VkRect2D renderArea;
			renderArea.extent.width = window.GetWidth();
			renderArea.extent.height = window.GetHeight();
			renderArea.offset.x = 0;
			renderArea.offset.y = 0;
			BeginRenderPass(window.GetRenderPass(), window.GetCurrentFramebuffer(), renderArea, window.GetClearValues(), window.GetClearValueCount(), VK_SUBPASS_CONTENTS_INLINE);
		}
		inline void EndRenderPass() const {
			vkCmdEndRenderPass(commands);
		}
		inline void BindGraphicsPipeline(VkPipeline pipeline) const {
			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		}
		inline void BindComputePipeline(VkPipeline pipeline) const {
			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
		}
		inline void BindVertexBuffers(const VkBuffer* pBuffers, uint32_t bufferCount, const VkDeviceSize* pOffsets, uint32_t firstBinding) const {
			vkCmdBindVertexBuffers(commands, firstBinding, bufferCount, pBuffers, pOffsets);
		}
		inline void BindIndexBuffer(VkBuffer indexBuffer, VkDeviceSize indexOffset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32) const {
			vkCmdBindIndexBuffer(commands, indexBuffer, indexOffset, indexType);
		}
		inline void SetScissors(const VkRect2D* pScissors, uint32_t scissorCount, uint32_t firstScissor = 0) const {
			vkCmdSetScissor(commands, firstScissor, scissorCount, pScissors);
		}
		
		inline void SetScissor(const VkRect2D& scissor) const { SetScissors(&scissor, 1, 0); }
		inline void SetViewports(const VkViewport* pViewports, uint32_t viewportCount, uint32_t firstViewport = 0) const {
			vkCmdSetViewport(commands, firstViewport, viewportCount, pViewports);
		}
		inline void SetViewport(const VkViewport& viewport) const { SetViewports(&viewport, 1, 0); }
		inline void SetRenderArea(const VkViewport& viewport) const {
			SetViewport(viewport);
			VkRect2D area;
			area.extent.width = (uint32_t)viewport.width;
			area.extent.height = (uint32_t)viewport.height;
			area.offset.x = (int32_t)viewport.x;
			area.offset.y = (int32_t)viewport.y;
			SetScissor(area);
		}
		inline void SetRenderArea(uint32_t width, uint32_t height, int32_t xOffset = 0, int32_t yOffset = 0, float minDepth = 0.f, float maxDepth = 1.f) const {
			VkRect2D area;
			area.extent.width = width;
			area.extent.height = height;
			area.offset.x = xOffset;
			area.offset.y = yOffset;
			SetScissor(area);
			VkViewport viewport;
			viewport.width = (float)width;
			viewport.height = (float)height;
			viewport.x = (float)xOffset;
			viewport.y = (float)yOffset;
			viewport.maxDepth = maxDepth;
			viewport.minDepth = minDepth;
			SetViewport(viewport);
		}
		inline void SetRenderArea(const Window& window) const {
			SetRenderArea(window.GetWidth(), window.GetHeight());
		}
		inline void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) const { 
			vkCmdDraw(commands, vertexCount, instanceCount, firstVertex, firstInstance);
		}
		inline void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) const {
			vkCmdDrawIndexed(commands, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}

		inline void Reset() {
			auto& device = Device::Get();
			device.Reset(commandPool);
		}
		inline void Submit(const VkSemaphore* pWaitSemaphores, const VkShaderStageFlags* pWaitStages, uint32_t waitCount, const VkSemaphore* pSignalSemaphores, uint32_t signalCount) {
			VkSubmitInfo info;
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.pNext = nullptr;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &commands;
			info.waitSemaphoreCount = waitCount;
			info.signalSemaphoreCount = signalCount;
			info.pWaitDstStageMask = pWaitStages;
			info.pWaitSemaphores = pWaitSemaphores;
			info.pSignalSemaphores = pSignalSemaphores;
			if (vkQueueSubmit(queue, 1, &info, fence) != VK_SUCCESS) {
				fatal(ERROR_QUEUE_SUBMIT);
			}
		}
		inline void Submit(const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkShaderStageFlags>& waitStages, const std::vector<VkSemaphore>& signalSemaphores) {
			assert(waitSemaphores.size() == waitStages.size());
			Submit(waitSemaphores.data(), waitStages.data(), (uint32_t)waitStages.size(), signalSemaphores.data(), signalSemaphores.size());
		}
		inline void Submit(VkSemaphore waitSemaphore, VkShaderStageFlags waitStage, VkSemaphore signalSemaphore) {
			VkSubmitInfo info;
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.pNext = nullptr;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &commands;
			info.waitSemaphoreCount = (waitSemaphore != nullptr) ? 1 : 0;
			info.signalSemaphoreCount = (signalSemaphore != nullptr) ? 1 : 0;
			info.pWaitDstStageMask = &waitStage;
			info.pWaitSemaphores = &waitSemaphore;
			info.pSignalSemaphores = &signalSemaphore;
			if (vkQueueSubmit(queue, 1, &info, fence) != VK_SUCCESS) {
				fatal(ERROR_QUEUE_SUBMIT);
			}
		}
	private:
		VkCommandBuffer	commands;
		VkCommandPool commandPool;
		VkFence fence;
		VkQueue queue;
	};
}