#pragma once


namespace SGF {
    namespace Vk {
        inline VkClearValue CreateDepthClearValue(float depth, uint32_t stencil) { VkClearValue value; value.depthStencil.depth = depth; value.depthStencil.stencil = stencil; return value; }
        inline VkClearValue CreateColorClearValue(float r, float g, float b, float a) { return { r, g, b , a }; }
        inline VkClearValue CreateColorClearValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a) { VkClearValue c; c.color.uint32[0] = r; c.color.uint32[1] = g;  c.color.uint32[2] = b; c.color.uint32[3] = a; return c; }
        inline VkClearValue CreateColorClearValue(int32_t r, int32_t g, int32_t b, int32_t a) { VkClearValue c; c.color.int32[0] = r; c.color.int32[1] = g;  c.color.int32[2] = b; c.color.int32[3] = a; return c; }

        inline VkDescriptorSetLayoutBinding CreateDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers = nullptr) 
        { return { binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers }; }

        inline VkDescriptorPoolSize CreateDescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount) 
        { return  { type, descriptorCount }; }
        inline VkDescriptorPoolCreateInfo CreateDescriptorPoolInfo(uint32_t maxSets, uint32_t poolSizeCount, const VkDescriptorPoolSize* pPoolSizes, VkDescriptorPoolCreateFlags flags = FLAG_NONE, const void* pNext = nullptr) 
        { return { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, pNext, flags, maxSets, poolSizeCount, pPoolSizes }; }
        inline VkWriteDescriptorSet CreateDescriptorWrite(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const VkDescriptorBufferInfo* pBufferInfos, uint32_t descriptorCount, const void* pNext = nullptr) 
        { return { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pNext, dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, nullptr, pBufferInfos, nullptr }; }
        inline VkWriteDescriptorSet CreateDescriptorWrite(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const VkDescriptorImageInfo* pImageInfos, uint32_t descriptorCount, const void* pNext = nullptr) 
        { return { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pNext, dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, pImageInfos, nullptr, nullptr }; }
        inline VkWriteDescriptorSet CreateDescriptorWrite(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, const VkBufferView* pBufferViews, uint32_t descriptorCount, const void* pNext = nullptr) 
        { return { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pNext, dstSet, dstBinding, dstArrayElement, descriptorCount, descriptorType, nullptr, nullptr, pBufferViews }; }

        VkCommandBufferInheritanceInfo CreateCommandBufferInheritanceInfo(VkRenderPass renderPass, uint32_t subpass, VkFramebuffer framebuffer = VK_NULL_HANDLE, VkBool32 occlusionQueryEnable = VK_FALSE, VkQueryControlFlags queryFlags = FLAG_NONE, VkQueryPipelineStatisticFlags pipelineStatistics = FLAG_NONE, const void* pNext = nullptr);
        void BeginCommandBuffer(VkCommandBuffer commands, VkCommandBufferUsageFlags usage = FLAG_NONE, const void* pNext = nullptr);
        void BeginSecondaryCommands(VkCommandBuffer commands, VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t subpass = 0, VkCommandBufferUsageFlags usage = FLAG_NONE, const void* pNext = nullptr);
        void EndCommandBuffer(VkCommandBuffer commandBuffer);

		

        VkSubmitInfo CreateSubmitInfo(const VkCommandBuffer* pCommandBuffers, uint32_t commandBufferCount, 
            const VkSemaphore* pWaitSemaphores, const VkPipelineStageFlags* pWaitDstStageMask, uint32_t waitSemaphoreCount,
            const VkSemaphore* pSignalSemaphores, uint32_t signalSemaphoreCount, const void* pNext = nullptr);

        void SubmitCommands(VkQueue queue, const VkSubmitInfo& submitInfo, VkFence fence = VK_NULL_HANDLE);
        void SubmitCommands(VkQueue queue, VkCommandBuffer commands, VkFence fence = VK_NULL_HANDLE);
        void SubmitCommands(VkQueue queue, VkCommandBuffer commands, VkSemaphore waitSemaphore, VkPipelineStageFlags waitStage, VkSemaphore signalSemaphore, VkFence fence = VK_NULL_HANDLE);       
        void SubmitCommands(VkQueue queue, VkCommandBuffer commands, const VkSemaphore* pWaitSemaphore, const VkPipelineStageFlags* pWaitStage, uint32_t waitCount,
            const VkSemaphore* pSignalSemaphore, uint32_t signalCount, VkFence fence = VK_NULL_HANDLE);
        template<uint32_t WAIT_COUNT, uint32_t SIGNAL_COUNT>
        inline void SubmitCommands(VkQueue queue, VkCommandBuffer commands, const VkSemaphore(&waitSemaphores)[WAIT_COUNT], const VkPipelineStageFlags(&waitStages)[WAIT_COUNT], const VkSemaphore(&signalSemaphores)[SIGNAL_COUNT], VkFence fence = VK_NULL_HANDLE) 
        { SubmitCommands(queue, commands, waitSemaphores, waitStages, WAIT_COUNT, signalSemaphores, SIGNAL_COUNT, fence); }
        void SubmitCommands(VkQueue queue, const VkCommandBuffer* pCommands, uint32_t commandBufferCount, const VkSemaphore* pWaitSemaphore, const VkPipelineStageFlags* pWaitStage, uint32_t waitCount,
            const VkSemaphore* pSignalSemaphore, uint32_t signalCount, VkFence fence = VK_NULL_HANDLE);
        template<uint32_t COMMAND_COUNT, uint32_t WAIT_COUNT, uint32_t SIGNAL_COUNT>
        inline void SubmitCommands(VkQueue queue, const VkCommandBuffer(&commands)[COMMAND_COUNT], const VkSemaphore(&waitSemaphores)[WAIT_COUNT], const VkPipelineStageFlags(&waitStages)[WAIT_COUNT], const VkSemaphore(&signalSemaphores)[SIGNAL_COUNT], VkFence fence = VK_NULL_HANDLE) 
        { SubmitCommands(queue, commands, COMMAND_COUNT, waitSemaphores, waitStages, WAIT_COUNT, signalSemaphores, SIGNAL_COUNT, fence); }
        void SubmitCommands(VkQueue queue, const VkCommandBuffer* pCommands, uint32_t commandBufferCount, VkFence fence);
        template<uint32_t COUNT>
        inline void SubmitCommands(VkQueue queue, const VkCommandBuffer(&commands)[COUNT], VkFence fence) { SubmitCommands(queue, commands, COUNT, fence); }

        inline VkAttachmentDescription CreateAttachmentDescription(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE, VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE) {
			return { FLAG_NONE, format, sampleCount, loadOp, storeOp, stencilLoadOp, stencilStoreOp, initialLayout, finalLayout };
		}
		inline VkSubpassDescription CreateSubpassDescription(const VkAttachmentReference* pColorAttachments = nullptr, uint32_t colorAttachmentCount = 0, const VkAttachmentReference* pResolveAttachments = nullptr, const VkAttachmentReference* pDepthAttachment = nullptr,
			const VkAttachmentReference* pInputAttachments = nullptr, uint32_t inputAttachmentCount = 0, const uint32_t* pPreserveAttachments = nullptr, uint32_t preserveCount = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) {
			return { FLAG_NONE, bindPoint, inputAttachmentCount, pInputAttachments, colorAttachmentCount, pColorAttachments, pResolveAttachments, pDepthAttachment, preserveCount, pPreserveAttachments };
		}
		inline VkAttachmentReference CreateAttachmentReference(uint32_t index, VkImageLayout layout) {
			return { index, layout };
		}
		inline VkAttachmentReference CreateColorAttachmentReference(uint32_t index) {
			return { index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		}
		inline VkAttachmentReference CreateDepthStencilAttachmentReference(uint32_t index) {
			return { index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		}
		inline VkAttachmentDescription CreateDepthAttachment(VkFormat format = VK_FORMAT_D16_UNORM, VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE) {
			return CreateAttachmentDescription(format, sampleCount, initialLayout, finalLayout, loadOp, storeOp, loadOp, storeOp);
		}
    } // namespace VK
} // namespace SGF 