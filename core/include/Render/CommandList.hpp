#pragma once

#include "SGF_Core.hpp"

#include "Device.hpp"

namespace SGF {
	enum QueueTypes {
		QUEUE_TYPE_GRAPHICS,
		QUEUE_TYPE_TRANSFER,
		QUEUE_TYPE_COMPUTE,
		QUEUE_TYPE_PRESENT
	};
	inline VkClearValue createDepthClearValue(float depth, uint32_t stencil) { VkClearValue value; value.depthStencil.depth = depth; value.depthStencil.stencil = stencil; return value; }
	inline VkClearValue createColorClearValue(float r, float g, float b, float a) { return { r, g, b , a }; }
	inline VkClearValue createColorClearValue(int32_t r, int32_t g, int32_t b, int32_t a) { return { r, g, b , a }; }
	inline VkClearValue createColorClearValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a) { return { r, g, b , a }; }

	class CommandList {
	public:
		inline CommandList(const Device& device, QueueTypes queueType, uint32_t queueIndex, VkCommandBufferLevel level, VkCommandPoolCreateFlags flags) {
			uint32_t queueFamilyIndex;
			assert(queueType == QUEUE_TYPE_GRAPHICS || queueType == QUEUE_TYPE_COMPUTE || queueType == QUEUE_TYPE_TRANSFER);
			if (queueType == QUEUE_TYPE_GRAPHICS) {
				queueFamilyIndex = device.graphicsFamily();
				queue = device.graphicsQueue(queueIndex);
			} else if (queueType == QUEUE_TYPE_COMPUTE) {
				queueFamilyIndex = device.computeFamily();
				queue = device.computeQueue(queueIndex);
			} else if (queueType == QUEUE_TYPE_TRANSFER) {
				queueFamilyIndex = device.transferFamily();
				queue = device.transferQueue(queueIndex);
			}
			commandPool = device.commandPool(queueFamilyIndex, flags);
			commands = device.commandBuffer(commandPool, level);
		}
		inline ~CommandList() {
			auto& device = getDevice();
			device.destroy(commandPool);
		}
		inline void begin(VkCommandBufferUsageFlags flags = FLAG_NONE) {
			VkCommandBufferBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = flags;
			info.pInheritanceInfo = nullptr;
			info.pNext = nullptr;
			vkBeginCommandBuffer(commands, &info);
		}
		inline void continueRenderPass(VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer, VkCommandBufferUsageFlags flags, VkBool32 occlusionQueryEnable = VK_FALSE, VkQueryControlFlags queryFlags = FLAG_NONE, VkQueryPipelineStatisticFlags pipelineStatistics = FLAG_NONE) {
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
		inline void end() {
			vkEndCommandBuffer(commands);
		}
		inline void beginRenderPass(const VkRenderPassBeginInfo& info, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) {
			vkCmdBeginRenderPass(commands, &info, subpassContents);
		}
		inline void beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea, const VkClearValue* pClearValues, uint32_t clearValueCount, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) {
			VkRenderPassBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.renderPass = renderPass;
			info.framebuffer = framebuffer;
			info.renderArea = renderArea;
			info.pClearValues = pClearValues;
			info.clearValueCount = clearValueCount;
			beginRenderPass(info, subpassContents);
		}
		inline void beginRenderPass(const Swapchain& swapchain, const VkClearValue* pClearValues, uint32_t clearValueCount) {
			VkRect2D renderArea;
			renderArea.extent.width = swapchain.getWidth();
			renderArea.extent.height = swapchain.getHeight();
			renderArea.offset.x = 0;
			renderArea.offset.y = 0;
			beginRenderPass(swapchain.getRenderPass(), swapchain.getCurrentFramebuffer(), renderArea, pClearValues, clearValueCount);
		}
		inline void endRenderPass() {
			vkCmdEndRenderPass(commands);
		}
		inline void bindGraphicsPipeline(VkPipeline pipeline) {
			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		}
		inline void bindComputePipeline(VkPipeline pipeline) {
			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
		}
		inline void bindVertexBuffers(const VkBuffer* pBuffers, uint32_t bufferCount, const VkDeviceSize* pOffsets, uint32_t firstBinding) {
			vkCmdBindVertexBuffers(commands, firstBinding, bufferCount, pBuffers, pOffsets);
		}
		inline void bindIndexBuffer(VkBuffer indexBuffer, VkDeviceSize indexOffset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32) {
			vkCmdBindIndexBuffer(commands, indexBuffer, indexOffset, indexType);
		}
		inline void setScissors(const VkRect2D* pScissors, uint32_t scissorCount, uint32_t firstScissor = 0) {
			vkCmdSetScissor(commands, firstScissor, scissorCount, pScissors);
		}
		
		inline void setScissor(const VkRect2D& scissor) { setScissors(&scissor, 1, 0); }
		inline void setViewports(const VkViewport* pViewports, uint32_t viewportCount, uint32_t firstViewport = 0) {
			vkCmdSetViewport(commands, firstViewport, viewportCount, pViewports);
		}
		inline void setViewport(const VkViewport& viewport) { setViewports(&viewport, 1, 0); }
		inline void setRenderArea(const VkViewport& viewport) {
			setViewport(viewport);
			VkRect2D area;
			area.extent.width = (uint32_t)viewport.width;
			area.extent.height = (uint32_t)viewport.height;
			area.offset.x = (int32_t)viewport.x;
			area.offset.y = (int32_t)viewport.y;
			setScissor(area);
		}
		inline void setRenderArea(uint32_t width, uint32_t height, int32_t xOffset = 0, int32_t yOffset = 0, float minDepth = 0.f, float maxDepth = 1.f) {
			VkRect2D area;
			area.extent.width = width;
			area.extent.height = height;
			area.offset.x = xOffset;
			area.offset.y = yOffset;
			setScissor(area);
			VkViewport viewport;
			viewport.width = (float)width;
			viewport.height = (float)height;
			viewport.x = (float)xOffset;
			viewport.y = (float)yOffset;
			viewport.maxDepth = maxDepth;
			viewport.minDepth = minDepth;
			setViewport(viewport);
		}
		inline void setRenderArea(const Swapchain& swapchain) {
			setRenderArea(swapchain.getWidth(), swapchain.getHeight());
		}
		inline void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) { 
			vkCmdDraw(commands, vertexCount, instanceCount, firstVertex, firstInstance);
		}
		inline void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) {
			vkCmdDrawIndexed(commands, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}

		inline void reset() {
			auto& device = getDevice();
			device.reset(commandPool);
		}
		inline void submit(VkSemaphore waitSemaphore, VkShaderStageFlags waitStage, VkSemaphore signalSemaphore, VkFence fence) {
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
		VkQueue queue;
	};
}