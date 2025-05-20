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
		void begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* pInheritanceInfo = nullptr);
		void end();
		void beginRenderPass(const VkRenderPassBeginInfo& info);
		void beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D renderArea, const VkClearValue* pClearValues, uint32_t clearValueCount);
		void beginRenderPass(const Swapchain& swapchain, const VkClearValue* pClearValues, uint32_t clearValueCount);
		void endRenderPass();

		void bindGraphicsPipeline(VkPipeline pipeline);
		void bindComputePipeline(VkPipeline pipeline);
		void bindVertexBuffers(const VkBuffer* pBuffers, uint32_t bufferCount, uint32_t firstBinding);

		void clear();
	private:
		VkCommandBuffer	commands;
		VkCommandPool commandPool;
		VkQueue queue;
	};
}