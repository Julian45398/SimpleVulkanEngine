#pragma once


namespace SGF {
    namespace Vk {
        inline VkClearValue CreateDepthClearValue(float depth, uint32_t stencil) { VkClearValue value; value.depthStencil.depth = depth; value.depthStencil.stencil = stencil; return value; }
        inline VkClearValue CreateColorClearValue(float r, float g, float b, float a) { return { r, g, b , a }; }

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
    } // namespace VK
} // namespace SGF 