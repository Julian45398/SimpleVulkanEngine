#pragma once

#include <stdint.h>
#include <string>
#include <array>
#include <vector>
#include <SGF/Core/Platform/WindowHandle.hpp>
#include <SGF/Core/Flags.hpp>

#include "Types.hpp"
#include "CommandList.hpp"
#include "CommandPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "DescriptorPool.hpp"
#include "GraphicsPipeline.hpp"
#include "RenderPass.hpp"

namespace SGF {
    namespace GPU {
        bool InitializeAPI(Flags<DriverFeature> flags = DriverFeature::NONE);

        std::vector<PhysicalDevice> GetAvailableDevices();
        std::string GetDeviceName(PhysicalDevice device);
        PhysicalDevice GetCurrentDevice();

		PhysicalDevice PickPhysicalDevice(Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const WindowHandle* pPresentationWindows = nullptr, uint32_t presentationWindowCount = 0);
		bool CheckPhysicalDeviceSupport(PhysicalDevice device, Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const WindowHandle* pPresentationWindows = nullptr, uint32_t presentationWindowCount = 0);

        bool Initialize(PhysicalDevice device, Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const WindowHandle* pPresentationWindows = nullptr, uint32_t presentationWindowCount = 0);
		bool Initialize(Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const WindowHandle* pPresentationWindows = nullptr, uint32_t presentationWindowCount = 0);

        bool IsInitialized();
        void Terminate();


		bool SupportsPresentation(const WindowHandle* pWindows, uint32_t windowCount);
		inline bool SupportsPresentation(const std::vector<WindowHandle>& windows) { return SupportsPresentation(windows.data(), (uint32_t)windows.size()); }
		template<uint32_t COUNT>
		inline bool SupportsPresentation(const std::array<WindowHandle, COUNT>& windows) { return SupportsPresentation(windows.data(), COUNT); }
		template<uint32_t COUNT>
		inline bool SupportsPresentation(const WindowHandle(&windows)[COUNT]) { return SupportsPresentation(windows, COUNT); }
		inline bool SupportsPresentation(const WindowHandle& window) { return SupportsPresentation(&window, 1); }

        bool HasFeaturesEnabled(Flags<DeviceFeature> features);

		void Submit(QueueType queueType, const CommandList* pCommands, uint32_t commandCount, const Semaphore* pWaitSemaphores, const Flags<PipelineStage>* pWaitStages, uint32_t waitSemaphoreCount, const Semaphore* pSignalSemaphores, uint32_t signalSemaphoreCount, Fence fence);
		template<uint32_t COMMAND_COUNT, uint32_t WAIT_COUNT, uint32_t SIGNAL_COUNT>
		inline void Submit(QueueType queueType, const CommandList(&commands)[COMMAND_COUNT], const Semaphore(&waitSemaphores)[WAIT_COUNT], const Flags<PipelineStage>(&waitStages)[WAIT_COUNT], const Semaphore(&signalSemaphores)[SIGNAL_COUNT], Fence fence) { Submit(queueType, commands, COMMAND_COUNT, waitSemaphores, waitStages, WAIT_COUNT, signalSemaphores, SIGNAL_COUNT, fence); }
		template<uint32_t COMMAND_COUNT, uint32_t WAIT_COUNT, uint32_t SIGNAL_COUNT>
		inline void Submit(QueueType queueType, const std::array<CommandList, COMMAND_COUNT>& commands, const std::array<Semaphore, WAIT_COUNT>& waitSemaphores, const std::array<Flags<PipelineStage>, WAIT_COUNT>& waitStages, const std::array<Semaphore, SIGNAL_COUNT>& signalSemaphores, Fence fence) { Submit(queueType, commands.data(), COMMAND_COUNT, waitSemaphores.data(), waitStages.data(), WAIT_COUNT, signalSemaphores.data(), SIGNAL_COUNT, fence); }
		inline void Submit(QueueType queueType, const std::vector<CommandList>& commands, const std::vector<Semaphore>& waitSemaphores, const std::vector<Flags<PipelineStage>>& waitStages, const std::vector<Semaphore>& signalSemaphores, Fence fence) { Submit(queueType, commands.data(), (uint32_t)commands.size(), waitSemaphores.data(), waitStages.data(), (uint32_t)waitSemaphores.size(), signalSemaphores.data(), (uint32_t)signalSemaphores.size(), fence); }

		inline void Submit(QueueType queueType, CommandList commands, const Semaphore* pWaitSemaphores, const Flags<PipelineStage>* pWaitStages, uint32_t waitSemaphoreCount, const Semaphore* pSignalSemaphores, uint32_t signalSemaphoreCount, Fence fence) { Submit(queueType, &commands, 1, pWaitSemaphores, pWaitStages, waitSemaphoreCount, pSignalSemaphores, signalSemaphoreCount, fence); }
		template<uint32_t WAIT_COUNT, uint32_t SIGNAL_COUNT>
		inline void Submit(QueueType queueType, CommandList commands, const Semaphore(&waitSemaphores)[WAIT_COUNT], const Flags<PipelineStage>(&waitStages)[WAIT_COUNT], const Semaphore(&signalSemaphores)[SIGNAL_COUNT], Fence fence) { Submit(queueType, commands, waitSemaphores, waitStages, (uint32_t)WAIT_COUNT, signalSemaphores, (uint32_t)SIGNAL_COUNT, fence);  }
		template<uint32_t WAIT_COUNT, uint32_t SIGNAL_COUNT>
		inline void Submit(QueueType queueType, CommandList commands, const std::array<Semaphore, WAIT_COUNT>& waitSemaphores, const std::array<Flags<PipelineStage>, WAIT_COUNT>& waitStages, const std::array<Semaphore, SIGNAL_COUNT>& signalSemaphores, Fence fence) { Submit(queueType, commands, waitSemaphores.data(), waitStages.data(), (uint32_t)WAIT_COUNT, signalSemaphores.data(), SIGNAL_COUNT, fence); }
		inline void Submit(QueueType queueType, CommandList commands, const std::vector<Semaphore>& waitSemaphores, const std::vector<Flags<PipelineStage>>& waitStages, const std::vector<Semaphore>& signalSemaphores, Fence fence) { Submit(queueType, commands, waitSemaphores.data(), waitStages.data(), (uint32_t)waitStages.size(), signalSemaphores.data(), (uint32_t)signalSemaphores.size(), fence); }

		inline void Submit(QueueType queueType, const CommandList* pCommands, uint32_t commandCount, Fence fence) { Submit(queueType, pCommands, commandCount, nullptr, nullptr, 0, nullptr, 0, fence); }
		template <uint32_t COUNT>
		inline void Submit(QueueType queueType, const CommandList(&commands)[COUNT], Fence fence) { Submit(queueType, commands, COUNT, fence); }
		template <uint32_t COUNT>
		inline void Submit(QueueType queueType, std::array<CommandList, COUNT>& commands, Fence fence) { Submit(queueType, commands.data(), COUNT, fence); }
		inline void Submit(QueueType queueType, const std::vector<CommandList>& commands, Fence fence) { Submit(queueType, commands.data(), (uint32_t)commands.size(), fence); }
		inline void Submit(QueueType queueType, CommandList commands, Fence fence = nullptr) { Submit(queueType, &commands, 1, nullptr, nullptr, 0, nullptr, 0, fence); }

		template<typename... Args>
		inline void SubmitGraphics(Args&&... args) { Submit(QueueType::GRAPHICS, std::forward<Args>(args)...); }
		template<typename... Args>
		inline void SubmitCompute(Args&&... args) { Submit(QueueType::COMPUTE, std::forward<Args>(args)...); }
		template<typename... Args>
		inline void SubmitTransfer(Args&&... args) { Submit(QueueType::TRANSFER, std::forward<Args>(args)...); }

		bool Present(const Swapchain* pSwapchains, uint32_t swapchainCount, const Semaphore* pWaitSemaphores, uint32_t waitSemaphoreCount);
		template<uint32_t COUNT>
		inline bool Present(const Swapchain(&swapchains)[COUNT], const Semaphore(&waitSemaphores)[COUNT]) { Present(swapchains, COUNT, waitSemaphores, COUNT); }
		template<uint32_t COUNT>
		inline bool Present(const std::array<Swapchain, COUNT>& swapchains, const std::array<Semaphore, COUNT>& waitSemaphores) { Present(swapchains.data(), COUNT, waitSemaphores.data(), COUNT); }
		inline bool Present(const std::vector<Swapchain>& swapchains, const std::vector<Semaphore>& waitSemaphores) { Present(swapchains.data(), (uint32_t)swapchains.size(), waitSemaphores.data(), (uint32_t)waitSemaphores.size()); }
        bool Present(Swapchain surface, Semaphore waitSemaphore = nullptr);

		void QueueWaitIdle(QueueType queueType, uint64_t timeout = UINT64_MAX);

        Fence CreateFence();
        Fence CreateFenceSignaled();
        Semaphore CreateSemaphore();

        void WaitFences(const Fence* pFences, uint32_t count, uint64_t timeout = UINT64_MAX);
        inline void WaitFences(const std::vector<Fence>& fences, uint64_t timeout = UINT64_MAX) { WaitFences(fences.data(), (uint32_t)fences.size(), timeout); }
        template<uint32_t COUNT>
        inline void WaitFences(const Fence(&fences)[COUNT], uint64_t timeout = UINT64_MAX) { WaitFences(fences, COUNT, timeout); }
        template<uint32_t COUNT>
        inline void WaitFences(const std::array<Fence, COUNT>& fences, uint64_t timeout = UINT64_MAX) { WaitFences(fences.data(), COUNT, timeout); }
		inline void WaitFence(Fence fence, uint64_t timeout = UINT64_MAX) { WaitFences(&fence, 1, timeout); }

        bool IsFenceSignaled(Fence fence);

        void ResetFences(const Fence* pFences, uint32_t count);
        inline void ResetFences(const std::vector<Fence>& fences) { ResetFences(fences.data(), (uint32_t)fences.size()); }
        template<uint32_t COUNT>
        inline void ResetFences(const Fence(&fences)[COUNT]) { ResetFences(fences, COUNT); }
        template<uint32_t COUNT>
        inline void ResetFences(const std::array<Fence, COUNT>& fences) { ResetFences(fences.data(), COUNT); }
        inline void ResetFence(Fence fence) { ResetFences(&fence, 1); }

        void WaitIdle(uint64_t timeout = UINT64_MAX);

        Buffer CreateBuffer(size_t size, Flags<BufferUsage> usage, Flags<BufferCreate> createFlags = BufferCreate::NONE);
        Buffer CreateBufferShared(size_t size, Flags<BufferUsage> usage, Flags<QueueType> flags, Flags<BufferCreate> createFlags = BufferCreate::NONE);

        /**
         * @brief creates an 1D-image for use with the graphics queue-family
         *
         * requires at least one graphics-Queue. If more specific image are to be created use the builder-> Device::image(...)::foo(...)::build(), or use your own ImageCreateInfo.
         *
         * @return 1D-image
         */
        Image CreateImage1D(uint32_t length, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImage2D(uint32_t width, uint32_t height, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImage3D(uint32_t width, uint32_t height, uint32_t depth, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImageArray1D(uint32_t length, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImage1DShared(uint32_t length, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<QueueType> queueFamilies = QueueType::GRAPHICS, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImage2DShared(uint32_t width, uint32_t height, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<QueueType> queueFamilies = QueueType::GRAPHICS, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImage3DShared(uint32_t width, uint32_t height, uint32_t depth, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<QueueType> queueFamilies = QueueType::GRAPHICS, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImageArray1DShared(uint32_t length, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<QueueType> queueFamilies = QueueType::GRAPHICS, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImageArray2DShared(uint32_t width, uint32_t height, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<QueueType> queueFamilies = QueueType::GRAPHICS, Flags<ImageCreate> flags = ImageCreate::NONE);
        Image CreateImageArray3DShared(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples = SampleCount::B1, uint32_t mipLevelCount = 1, Flags<QueueType> queueFamilies = QueueType::GRAPHICS, Flags<ImageCreate> flags = ImageCreate::NONE);
        ImageView CreateImageView1D(Image image, Format format, Flags<ImageAspect> imageAspect = ImageAspect::COLOR, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0);
        ImageView CreateImageView2D(Image image, Format format, Flags<ImageAspect> imageAspect = ImageAspect::COLOR, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0);
        ImageView CreateImageView3D(Image image, Format format, Flags<ImageAspect> imageAspect = ImageAspect::COLOR, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0);
        ImageView CreateImageViewCube(Image image, Format format, Flags<ImageAspect> imageAspect = ImageAspect::COLOR, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0);
        ImageView CreateImageArrayView1D(Image image, Format format, Flags<ImageAspect> imageAspect = ImageAspect::COLOR, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1);
        ImageView CreateImageArrayView2D(Image image, Format format, Flags<ImageAspect> imageAspect = ImageAspect::COLOR, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1);
        ImageView CreateImageArrayViewCube(Image image, Format format, Flags<ImageAspect> imageAspect = ImageAspect::COLOR, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1);

        Sampler CreateImageSampler(FilterType filterType = FilterType::NEAREST, SamplerMipmapMode mipmapMode = SamplerMipmapMode::LINEAR, SamplerAddressMode addressMode = SamplerAddressMode::CLAMP_TO_BORDER,
            float mipLodBias = 0.0f, float maxAnisotropy = 0.0f, CompareOp compareOp = CompareOp::ALWAYS,
            float minLod = 0.0f, float maxLod = 0.0f, BorderColor borderColor = BorderColor::FLOAT_OPAQUE_WHITE);

        MemoryRequirements GetMemoryRequirements(Buffer buffer);
        MemoryRequirements GetMemoryRequirements(Image image);

        void BindMemory(Memory memory, Buffer buffer, size_t offset = 0);
        void BindMemory(Memory memory, Image image, size_t offset = 0);
        void* MapMemory(Memory memory, size_t size = SIZE_MAX, size_t offset = 0);

        //Memory AllocateMemory(const MemoryRequirements& memReq, Flags<MemoryProperty> flags);

        Memory AllocateMemory(Buffer buffer, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL);
        Memory AllocateMemory(Image image, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL);

        Memory AllocateMemory(const Buffer* pBuffers, uint32_t bufferCount, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL);
        inline Memory AllocateMemory(const std::vector<Buffer>& buffers, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL) { return AllocateMemory(buffers.data(), (uint32_t)buffers.size(), flags); }
		template<uint32_t BUFFER_COUNT>
		inline Memory AllocateMemory(const std::array<Buffer, BUFFER_COUNT>& buffers, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL) { return AllocateMemory(buffers.data(), BUFFER_COUNT, flags); }
		template<uint32_t BUFFER_COUNT>
		inline Memory AllocateMemory(const Buffer(&buffers)[BUFFER_COUNT], Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL) { return AllocateMemory(buffers, BUFFER_COUNT, flags); }

        Memory AllocateMemory(const Image* pImages, uint32_t imageCount, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL);
        inline Memory AllocateMemory(const std::vector<Image>& images, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL) { return AllocateMemory(images.data(), (uint32_t)images.size(), flags); }
		template<uint32_t IMAGE_COUNT>
		inline Memory AllocateMemory(const std::array<Image, IMAGE_COUNT>& images, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL) { return AllocateMemory(images.data(), IMAGE_COUNT, flags); }
		template<uint32_t IMAGE_COUNT>
		inline Memory AllocateMemory(const Image(&images)[IMAGE_COUNT], Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL) { return AllocateMemory(images, IMAGE_COUNT, flags); }

        Memory AllocateMemory(const Buffer* pBuffers, uint32_t bufferCount, const Image* pImages, uint32_t imageCount, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL);
        inline Memory AllocateMemory(const std::vector<Buffer>& buffers, const std::vector<Image>& images, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL) { return AllocateMemory(buffers.data(), (uint32_t)buffers.size(), images.data(), (uint32_t)images.size(), flags); }
		template<uint32_t BUFFER_COUNT, uint32_t IMAGE_COUNT>
		inline Memory AllocateMemory(const std::array<Buffer, BUFFER_COUNT>& buffers, const std::array<Image, IMAGE_COUNT>& images, Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL) { return AllocateMemory(buffers.data(), BUFFER_COUNT, images.data(), IMAGE_COUNT, flags); }
		template<uint32_t BUFFER_COUNT, uint32_t IMAGE_COUNT>
		inline Memory AllocateMemory(const Buffer(&buffers)[BUFFER_COUNT], const Image(&images)[IMAGE_COUNT], Flags<MemoryProperty> flags = MemoryProperty::DEVICE_LOCAL) { return AllocateMemory(buffers, BUFFER_COUNT, images, IMAGE_COUNT, flags); }

        QueryPool CreateQueryPool(QueryType queryType, uint32_t queryCount, Flags<QueryPipelineStatistic> pipelineStatistics);

        ShaderModule CreateShaderModule(const char* filename);
        ShaderModule CreateShaderModule(const void* pData, size_t dataSize);

        PipelineLayout CreatePipelineLayout(const DescriptorSetLayout* pLayouts, uint32_t descriptorLayoutCount, const PushConstantRange* pPushConstantRanges = nullptr, uint32_t pushConstantCount = 0);
        inline PipelineLayout CreatePipelineLayout(DescriptorSetLayout descriptorSetLayout, PushConstantRange pushConstantRange) { return CreatePipelineLayout(&descriptorSetLayout, 1, &pushConstantRange, 1); }
        inline PipelineLayout CreatePipelineLayout(DescriptorSetLayout descriptorSetLayout) { return CreatePipelineLayout(&descriptorSetLayout, 1, nullptr, 0); }
        inline PipelineLayout CreatePipelineLayout(PushConstantRange pushConstantRange) { return CreatePipelineLayout(nullptr, 0, &pushConstantRange, 1); }
        inline PipelineLayout CreatePipelineLayout(const std::vector<DescriptorSetLayout>& layouts, const std::vector<PushConstantRange>& pushConstants) { return CreatePipelineLayout(layouts.data(), (uint32_t)layouts.size(), pushConstants.data(), (uint32_t)pushConstants.size()); }
        template<size_t DESCRIPTOR_COUNT>
        inline PipelineLayout CreatePipelineLayout(const DescriptorSetLayout(&layouts)[DESCRIPTOR_COUNT]) { return CreatePipelineLayout(layouts, DESCRIPTOR_COUNT, nullptr, 0); }
        template<size_t DESCRIPTOR_COUNT, size_t PUSH_CONSTANT_COUNT = 0>
        inline PipelineLayout CreatePipelineLayout(const DescriptorSetLayout(&layouts)[DESCRIPTOR_COUNT], const PushConstantRange(&pushConstants)[PUSH_CONSTANT_COUNT]) { return CreatePipelineLayout(layouts, DESCRIPTOR_COUNT, pushConstants, PUSH_CONSTANT_COUNT); }

        GraphicsPipelineBuilder CreateGraphicsPipeline(PipelineLayout layout, RenderPass renderPass, uint32_t subpass);
        ComputePipeline CreateComputePipeline(PipelineLayout layout, ShaderModule computeShader);
        ComputePipeline CreateComputePipeline(PipelineLayout layout, const char* filename);
        ComputePipeline CreateComputePipeline(PipelineLayout layout, const void* pData, size_t dataSize);


        RenderPassBuilder CreateRenderPass();

        Framebuffer CreateFramebuffer(RenderPass renderPass, const ImageView* pAttachments, uint32_t attachmentCount, uint32_t width, uint32_t height, uint32_t layerCount = 1);

        CommandPool CreateCommandPool(QueueType queueFamily = QueueType::GRAPHICS, Flags<CommandPoolCreate> flags = CommandPoolCreate::NONE);
        inline CommandPool CreateGraphicsCommandPool(Flags<CommandPoolCreate> flags = CommandPoolCreate::NONE) { return CreateCommandPool(QueueType::GRAPHICS, flags); }
		inline CommandPool CreateComputeCommandPool(Flags<CommandPoolCreate> flags = CommandPoolCreate::NONE) { return CreateCommandPool(QueueType::COMPUTE, flags); }
        inline CommandPool CreateTransferCommandPool(Flags<CommandPoolCreate> flags = CommandPoolCreate::NONE) { return CreateCommandPool(QueueType::TRANSFER, flags); }

        DescriptorSetLayout CreateDescriptorSetLayout(const DescriptorSetBinding* pBindings, uint32_t bindingCount, Flags<DescriptorSetLayoutCreate> flags = DescriptorSetLayoutCreate::NONE);
		inline DescriptorSetLayout CreateDescriptorSetLayout(const std::vector<DescriptorSetBinding>& bindings, Flags<DescriptorSetLayoutCreate> flags = DescriptorSetLayoutCreate::NONE) { return CreateDescriptorSetLayout(bindings.data(), (uint32_t)bindings.size(), flags); }
        template<uint32_t COUNT>
        inline DescriptorSetLayout CreateDescriptorSetLayout(const DescriptorSetBinding(&bindings)[COUNT], Flags<DescriptorSetLayoutCreate> flags = DescriptorSetLayoutCreate::NONE) { return CreateDescriptorSetLayout(bindings, COUNT, flags); }
        template<uint32_t COUNT>
        inline DescriptorSetLayout CreateDescriptorSetLayout(const std::array<DescriptorSetBinding, COUNT>& bindings, Flags<DescriptorSetLayoutCreate> flags = DescriptorSetLayoutCreate::NONE) { return CreateDescriptorSetLayout(bindings.data(), COUNT, flags); }

        DescriptorPool CreateDescriptorPool(DescriptorSetLayout descriptorLayout, uint32_t maxSets, Flags<DescriptorPoolCreate> flags = DescriptorPoolCreate::NONE);

		void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const DescriptorBufferInfo* pBufferInfos, uint32_t descriptorCount);
		inline void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const std::vector<DescriptorBufferInfo>& bufferInfos) { UpdateDescriptor(dstSet, dstBinding, dstArrayElement, descriptorType, bufferInfos.data(), (uint32_t)bufferInfos.size());   }
		template<uint32_t COUNT>
		inline void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const DescriptorBufferInfo(&bufferInfos)[COUNT]) { UpdateDescriptor(dstSet, dstBinding, dstArrayElement, descriptorType, bufferInfos, COUNT); }
		template<uint32_t COUNT>
		inline void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const std::array<DescriptorBufferInfo, COUNT>& bufferInfos) { UpdateDescriptor(dstSet, dstBinding, dstArrayElement, descriptorType, bufferInfos.data(), COUNT); }

        void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const DescriptorImageInfo* pImageInfos, uint32_t descriptorCount);
        inline void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const std::vector<DescriptorImageInfo>& bufferInfos) { UpdateDescriptor(dstSet, dstBinding, dstArrayElement, descriptorType, bufferInfos.data(), (uint32_t)bufferInfos.size());   }
		template<uint32_t COUNT>
		inline void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const DescriptorImageInfo(&bufferInfos)[COUNT]) { UpdateDescriptor(dstSet, dstBinding, dstArrayElement, descriptorType, bufferInfos, COUNT); }
		template<uint32_t COUNT>
		inline void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const std::array<DescriptorImageInfo, COUNT>& bufferInfos) { UpdateDescriptor(dstSet, dstBinding, dstArrayElement, descriptorType, bufferInfos.data(), COUNT); }
        
        void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const BufferView* pBufferViews, uint32_t descriptorCount);
        inline void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const std::vector<BufferView>& bufferInfos) { UpdateDescriptor(dstSet, dstBinding, dstArrayElement, descriptorType, bufferInfos.data(), (uint32_t)bufferInfos.size());   }
		template<uint32_t COUNT>
		inline void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const BufferView(&bufferInfos)[COUNT]) { UpdateDescriptor(dstSet, dstBinding, dstArrayElement, descriptorType, bufferInfos, COUNT); }
		template<uint32_t COUNT>
		inline void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const std::array<BufferView, COUNT>& bufferInfos) { UpdateDescriptor(dstSet, dstBinding, dstArrayElement, descriptorType, bufferInfos.data(), COUNT); }

        /**
         * @brief gets the first supported format for the requested feature and tiling from supplied candidates.
         *
         * @return first supported candidate or VK_FORMAT_MAX_ENUM when none are supported.
         */
        Format GetSupportedFormat(const Format* pCandidates, uint32_t candidateCount, Flags<FormatFeature> features, ImageTiling tiling = ImageTiling::OPTIMAL);
        template<uint32_t COUNT>
        inline Format GetSupportedFormat(const Format(candidates)[COUNT], Flags<FormatFeature> features, ImageTiling tiling = ImageTiling::OPTIMAL) { return GetSupportedFormat(candidates, COUNT, features, tiling); }
		inline Format GetSupportedFormat(const std::vector<Format>& candidates, Flags<FormatFeature> features, ImageTiling tiling = ImageTiling::OPTIMAL) { return GetSupportedFormat(candidates.data(), (uint32_t)candidates.size(), features, tiling); }
		inline bool IsFormatSupported(Format format, Flags<FormatFeature> features, ImageTiling tiling = ImageTiling::OPTIMAL) { return GetSupportedFormat(&format, 1, features, tiling) != Format::MAX_ENUM; }


        Swapchain CreateSwapchain(WindowHandle windowHandle, Flags<SwapchainCreate> flags = SwapchainCreate::NONE, Flags<ImageUsage> imageUsage = ImageUsage::COLOR_ATTACHMENT);
        Swapchain RecreateSwapchain(Swapchain oldSwapchain, glm::uvec2 size, Flags<SwapchainCreate> flags, Flags<ImageUsage> imageUsage);

        const Image* GetSwapchainImages(Swapchain swapchain, uint32_t* pCount);
        const ImageView* GetSwapchainImageViews(Swapchain swapchain, uint32_t* pCount);
        bool SwapchainOutOfDate(Swapchain swapchain);

        SampleCount GetMaxSupportedSampleCount();

        void ClearDescriptorSetLayouts();

        void Destroy(Fence fence);
        void Destroy(Semaphore semaphore);
        void Destroy(Buffer buffer);
        void Destroy(Image image);
        void Destroy(ImageView imageView);
        void Destroy(Framebuffer framebuffer);
        void Destroy(RenderPass renderPass);
        void Destroy(GraphicsPipeline pipeline);
        void Destroy(ComputePipeline pipeline);
        void Destroy(RayTracingPipeline pipeline);
        void Destroy(PipelineLayout pipelineLayout);
        //void Destroy(DescriptorSetLayout descriptorSetLayout);
        void Destroy(DescriptorPool descriptorPool);
        void Destroy(Memory memory);
        void Destroy(CommandPool commandPool);
        void Destroy(Sampler sampler);
        void Destroy(Swapchain swapchain);
        void Destroy(ShaderModule shaderModule);
        void Destroy(QueryPool queryPool);
        inline void Destroy() {}
        template<typename T, typename ...Args>
        inline void Destroy(T type, Args... args) {
            Destroy(type);
            Destroy(args...);
        }
        template<typename T, GLSLLayout Layout = GLSLLayout::STD140>
        inline constexpr uint32_t Alignment() {
            //
            // Scalars
            //
            if constexpr (std::is_same_v<T, float> ||
                std::is_same_v<T, int32_t> ||
                std::is_same_v<T, uint32_t> ||
                std::is_same_v<T, bool>) {
                return 4;
            }
            if constexpr (std::is_same_v<T, double> ||
                std::is_same_v<T, int64_t> ||
                std::is_same_v<T, uint64_t>) {
                return 8;
            }
            //
            // float/int/bool vectors
            //
            if constexpr (std::is_same_v<T, glm::vec2> ||
                std::is_same_v<T, glm::ivec2> ||
                std::is_same_v<T, glm::uvec2> ||
                std::is_same_v<T, glm::bvec2>) {
                return Layout == GLSLLayout::SCALAR ? 8 : 8;
            }
            if constexpr (std::is_same_v<T, glm::vec3> ||
                std::is_same_v<T, glm::ivec3> ||
                std::is_same_v<T, glm::uvec3> ||
                std::is_same_v<T, glm::bvec3> ||
                std::is_same_v<T, glm::vec4> ||
                std::is_same_v<T, glm::ivec4> ||
                std::is_same_v<T, glm::uvec4> ||
                std::is_same_v<T, glm::bvec4>) {
                return Layout == GLSLLayout::SCALAR ? 4 : 16;
            }
            //
            // double vectors
            //
            if constexpr (std::is_same_v<T, glm::dvec2>) {
                return Layout == GLSLLayout::SCALAR ? 16 : 16;
            }
            if constexpr (std::is_same_v<T, glm::dvec3> ||
                std::is_same_v<T, glm::dvec4>) {
                return Layout == GLSLLayout::SCALAR ? 8 : 32;
            }
            //
            // float matrices
            //
            if constexpr (std::is_same_v<T, glm::mat2> || 
                std::is_same_v<T, glm::mat2x2>) {
                if constexpr (Layout == GLSLLayout::STD140)
                    return 16;
                if constexpr (Layout == GLSLLayout::STD430)
                    return 8;
                return 4;
            }
            if constexpr (std::is_same_v<T, glm::mat2x3> ||
                std::is_same_v<T, glm::mat2x4> ||
                std::is_same_v<T, glm::mat3> ||
                std::is_same_v<T, glm::mat3x2> ||
                std::is_same_v<T, glm::mat3x4> ||
                std::is_same_v<T, glm::mat4> ||
                std::is_same_v<T, glm::mat4x2> ||
                std::is_same_v<T, glm::mat4x3>) {
                if constexpr (Layout == GLSLLayout::SCALAR)
                    return 4;

                return 16;
            }
            //
            // double matrices
            //
            if constexpr (std::is_same_v<T, glm::dmat2> || std::is_same_v<T, glm::dmat2x2>) {
				if constexpr (Layout == GLSLLayout::STD140)
					return 32;
				if constexpr (Layout == GLSLLayout::STD430)
					return 16;
				return 8;
            }

            if constexpr (std::is_same_v<T, glm::dmat2x3> ||
                std::is_same_v<T, glm::dmat2x4> ||
                std::is_same_v<T, glm::dmat3> ||
                std::is_same_v<T, glm::dmat3x2> ||
                std::is_same_v<T, glm::dmat3x4> ||
                std::is_same_v<T, glm::dmat4> ||
                std::is_same_v<T, glm::dmat4x2> ||
                std::is_same_v<T, glm::dmat4x3>) {
                if constexpr (Layout == GLSLLayout::SCALAR)
                    return 8;
                return 32;
            }
            return alignof(T);
        }
	}
}