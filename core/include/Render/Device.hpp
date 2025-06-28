#pragma once

#include "SGF_Core.hpp"
#include "GraphicsPipeline.hpp"
#include "Vulkan.hpp"

#ifndef SGF_MAX_DEVICE_EXTENSION_COUNT
#define SGF_MAX_DEVICE_EXTENSION_COUNT 32
#endif

namespace SGF {
    struct DeviceRequirements {
        std::vector<const char*> extensions;
        DeviceFeatureFlags requiredFeatures;
        DeviceFeatureFlags optionalFeatures;
        uint32_t graphicsQueueCount;
        uint32_t computeQueueCount;
        uint32_t transferQueueCount;
    };
    class Device {
    public:
        static uint32_t getSupportedDeviceCount();
        static uint32_t getSupportedDeviceCount(const DeviceRequirements& requirements);
        inline static const Device& Get() { return s_Instance; }
        static void RequireFeatures(DeviceFeatureFlags requiredFeatures);
        static void RequestFeatures(DeviceFeatureFlags optionalFeatures);
        static void RequireExtensions(const char* const* pExtensionNames, uint32_t featureCount);
        static void RequireExtensions(const std::vector<const char*> extensionNames);
        template<size_t EXT_COUNT>
        inline static void RequireExtensions(const char*(&extensionNames)[EXT_COUNT]) { RequireExtensions(extensionNames, EXT_COUNT); }

        static void RequireGraphicsQueues(uint32_t queueCount);
        static void RequireComputeQueues(uint32_t queueCount);
        static void RequireTransferQueues(uint32_t queueCount);
        static void PickNew();

        inline static void Shutdown() { s_Instance.Terminate(); }
        inline static bool IsInitialized() { return s_Instance.IsCreated(); }
    public:
        
        inline ~Device() { if (IsCreated()) Terminate(); }
        void WaitIdle() const;

        inline bool HasFeaturesEnabled(DeviceFeatureFlags features) const { return (enabledFeatures & features) == features; }
        inline bool HasFeatureEnabled(DeviceFeatureFlagBits feature) const { return (enabledFeatures & feature); }

        inline bool CheckSurfaceSupport(VkSurfaceKHR surface) const {
            error("TODO: implement surface support function!");
            return true;
        }

        void WaitFence(VkFence fence) const;
        void WaitFences(const VkFence* pFences, uint32_t count) const;
        inline void WaitFences(const std::vector<VkFence>& fences) const { WaitFences(fences.data(), (uint32_t)fences.size()); }
        template<uint32_t COUNT>
        inline void WaitFences(const VkFence(&fences)[COUNT]) const { WaitFences(fences, COUNT); }

        void Reset(const VkFence* pFences, uint32_t count) const;
        inline void Reset(const std::vector<VkFence>& fences) const { Reset(fences.data(), (uint32_t)fences.size()); }
        template<uint32_t COUNT>
        inline void Reset(const VkFence(&fences)[COUNT]) const { Reset(fences, COUNT); }
        inline void Reset(VkFence fence) const { Reset(&fence, 1); }
        bool IsFenceSignaled(VkFence fence) const;

        const char* GetName() const;
        inline bool IsCreated() const {return logical != nullptr; }
    public:
        inline operator VkDevice() const { return logical; }
        inline operator VkPhysicalDevice() const { return physical; }
        inline VkDevice GetLogical() const { return logical; }
        inline VkPhysicalDevice GetPhysical() const { return physical; }
        // Queue functions:
        inline uint32_t GetGraphicsFamily() const { return graphicsFamilyIndex; }
        inline uint32_t GetComputeFamily() const { return computeFamilyIndex; }
        inline uint32_t GetTransferFamily() const { return transferFamilyIndex; }
        inline uint32_t GetPresentFamily() const { return presentFamilyIndex; }

        inline uint32_t GetGraphicsQueueCount() const { return graphicsCount; }
        inline uint32_t GetComputeQueueCount() const { return computeCount; }
        inline uint32_t GetTransferQueueCount() const { return transferCount; }
        inline uint32_t GetPresentQueueCount() const { return presentCount; }

        VkQueue GetGraphicsQueue(uint32_t index) const;
        VkQueue GetComputeQueue(uint32_t index) const;
        VkQueue GetTransferQueue(uint32_t index) const;
        VkQueue GetPresentQueue() const;

        VkFence CreateFence() const;
        VkFence CreateFenceSignaled() const;
        VkSemaphore CreateSemaphore() const;
        //void signalSemaphore(VkSemaphore semaphore, uint64_t value) const;

        VkBuffer CreateBuffer(const VkBufferCreateInfo& info) const;
        VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags createFlags = 0) const;
        VkBuffer CreateBufferShared(VkDeviceSize size, VkBufferUsageFlags usage, QueueFamilyFlags flags, VkBufferCreateFlags createFlags = 0) const;

        VkImage CreateImage(const VkImageCreateInfo& info) const;
        /**
         * @brief creates an 1D-image for use with the graphics queue-family
         * 
         * requires at least one graphics-Queue. If more specific image are to be created use the builder-> Device::image(...)::foo(...)::build(), or use your own VkImageCreateInfo.
         * 
         * @return 1D-image
         */
        VkImage CreateImage1D(uint32_t length, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage CreateImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage CreateImage3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage CreateImageArray1D(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage CreateImageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage CreateImageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;

        VkImage CreateImage1DShared(uint32_t length, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage CreateImage2DShared(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage CreateImage3DShared(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage CreateImageArray1DShared(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage CreateImageArray2DShared(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage CreateImageArray3DShared(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;

        VkImageView CreateImageView(const VkImageViewCreateInfo& info) const;
        VkImageView CreateImageView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        VkImageView CreateImageView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        VkImageView CreateImageView3D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        VkImageView CreateImageViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        VkImageView CreateImageArrayView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1) const;
        VkImageView CreateImageArrayView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1) const;
        VkImageView CreateImageArrayViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1) const;

        VkSampler CreateImageSampler(const VkSamplerCreateInfo& info) const;
        VkSampler CreateImageSampler(VkFilter filterType = VK_FILTER_NEAREST, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 
            float mipLodBias = 0.0f, VkBool32 anisotropyEnable = VK_FALSE, float maxAnisotropy = 0.0f, VkBool32 compareEnable = VK_FALSE, VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS, 
            float minLod = 0.0f, float maxLod = 0.0f, VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VkBool32 unnormalizedCoordinates = VK_FALSE, VkSamplerCreateFlags flags = FLAG_NONE, const void* pNext = nullptr) const;

        VkMemoryRequirements GetMemoryRequirements(VkBuffer buffer) const;
        VkMemoryRequirements GetMemoryRequirements(VkImage image) const;

        void BindMemory(VkDeviceMemory memory, VkBuffer buffer, VkDeviceSize offset = 0) const;
        void BindMemory(VkDeviceMemory memory, VkImage image, VkDeviceSize offset = 0) const;
        void* MapMemory(VkDeviceMemory memory, size_t size = VK_WHOLE_SIZE, size_t offset = 0) const;

        VkDeviceMemory AllocateMemory(const VkMemoryAllocateInfo& info) const;
        VkDeviceMemory AllocateMemory(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags flags) const;

        VkDeviceMemory AllocateMemory(VkBuffer buffer, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        VkDeviceMemory AllocateMemory(VkImage image, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;

        VkDeviceMemory AllocateMemory(const VkBuffer* pBuffers, uint32_t bufferCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        inline VkDeviceMemory AllocateMemory(const std::vector<VkBuffer>& buffers, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const 
        { return AllocateMemory(buffers.data(), (uint32_t)buffers.size(), flags); }
        VkDeviceMemory AllocateMemory(const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        inline VkDeviceMemory AllocateMemory(const std::vector<VkImage>& images, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const 
        { return AllocateMemory(images.data(), (uint32_t)images.size(), flags); }

        VkDeviceMemory AllocateMemory(const VkBuffer* pBuffers, uint32_t bufferCount, const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        inline VkDeviceMemory AllocateMemory(const std::vector<VkBuffer>& buffers, const std::vector<VkImage>& images, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const 
        { return AllocateMemory(buffers.data(), (uint32_t)buffers.size(), images.data(), (uint32_t)images.size(), flags); }


        VkShaderModule CreateShaderModule(const char* filename) const;
        VkShaderModule CreateShaderModule(const VkShaderModuleCreateInfo& info) const;
        VkPipelineLayout CreatePipelineLayout(const VkPipelineLayoutCreateInfo& info) const;
        VkPipelineLayout CreatePipelineLayout(const VkDescriptorSetLayout* pLayouts, uint32_t descriptorLayoutCount, const VkPushConstantRange* pPushConstantRanges = nullptr, uint32_t pushConstantCount = 0) const;

        template<size_t DESCRIPTOR_COUNT, size_t PUSH_CONSTANT_COUNT>
        inline VkPipelineLayout CreatePipelineLayout(const VkDescriptorSetLayout(&layouts)[DESCRIPTOR_COUNT], const VkPushConstantRange(&pushConstants)[PUSH_CONSTANT_COUNT]) const
        { return CreatePipelineLayout(layouts, DESCRIPTOR_COUNT, pushConstants, PUSH_CONSTANT_COUNT); }
        inline VkPipelineLayout CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts, const std::vector<VkPushConstantRange>& pushConstants) const
        { return CreatePipelineLayout(layouts.data(), (uint32_t)layouts.size(), pushConstants.data(), (uint32_t)pushConstants.size()); }
        VkPipeline CreatePipeline(const VkGraphicsPipelineCreateInfo& info) const;
        VkPipeline CreatePipeline(const VkComputePipelineCreateInfo& info) const;
        inline GraphicsPipelineBuilder CreateGraphicsPipeline(VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass) const { return GraphicsPipelineBuilder(this, layout, renderPass, subpass); };

        VkSwapchainKHR CreateSwapchain(const VkSwapchainCreateInfoKHR& info) const;

        VkFramebuffer CreateFramebuffer(const VkFramebufferCreateInfo& info) const;
        VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, const VkImageView* pAttachments, uint32_t attachmentCount, uint32_t width, uint32_t height, uint32_t layerCount) const;

        VkRenderPass CreateRenderPass(const VkRenderPassCreateInfo& info) const;
        VkRenderPass CreateRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount) const;
        VkRenderPass CreateRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies = nullptr, uint32_t dependencyCount = 0) const;

        template<uint32_t ATTACHMENT_COUNT, uint32_t SUBPASS_COUNT, uint32_t DEPENDENCY_COUNT>
        inline VkRenderPass CreateRenderPass(const VkAttachmentDescription(&attachments)[ATTACHMENT_COUNT], const VkSubpassDescription(&subpasses)[SUBPASS_COUNT], const VkSubpassDependency(&dependencies)[DEPENDENCY_COUNT]) const {
            return CreateRenderPass(attachments, ATTACHMENT_COUNT, subpasses, SUBPASS_COUNT, dependencies, DEPENDENCY_COUNT); }
        inline VkRenderPass CreateRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies) const {
            return CreateRenderPass(attachments.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size(), dependencies.data(), (uint32_t)dependencies.size());
        }

        VkCommandPool CreateCommandPool(const VkCommandPoolCreateInfo& info) const;
        VkCommandPool CreateCommandPool(uint32_t queueIndex, VkCommandPoolCreateFlags flags = FLAG_NONE) const;
        inline VkCommandPool CreateGraphicsCommandPool(VkCommandPoolCreateFlags flags = FLAG_NONE) const { return CreateCommandPool(graphicsFamilyIndex, flags); }
        inline VkCommandPool CreateTransferCommandPool(VkCommandPoolCreateFlags flags = FLAG_NONE) const { return CreateCommandPool(transferFamilyIndex, flags); }
        inline VkCommandPool CreateComputeCommandPool(VkCommandPoolCreateFlags flags = FLAG_NONE) const { return CreateCommandPool(computeFamilyIndex, flags); }
        VkCommandBuffer AllocateCommandBuffer(VkCommandPool pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
        void AllocateCommandBuffers(const VkCommandBufferAllocateInfo& info, VkCommandBuffer* pBuffers) const;
        void AllocateCommandBuffers(VkCommandPool pool, VkCommandBufferLevel level, uint32_t allocationCount, VkCommandBuffer* pBuffers) const;
        template<uint32_t COUNT>
        void AllocateCommandBuffers(VkCommandPool pool, VkCommandBufferLevel level, VkCommandBuffer(&buffers)[COUNT]) const { AllocateCommandBuffers(pool, level, COUNT, buffers); }


        VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info) const;
        VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding* pBindings, uint32_t bindingCount, VkDescriptorSetLayoutCreateFlags flags = FLAG_NONE) const;
        VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayoutCreateFlags flags = FLAG_NONE) const;
        template<uint32_t COUNT>
        inline VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding(&bindings)[COUNT], VkDescriptorSetLayoutCreateFlags flags = FLAG_NONE) const { return CreateDescriptorSetLayout(bindings, COUNT, flags); }
        

        VkDescriptorPool CreateDescriptorPool(const VkDescriptorPoolCreateInfo& info) const;
        VkDescriptorPool CreateDescriptorPool(uint32_t maxSets, const VkDescriptorPoolSize* pPoolSizes, uint32_t poolSizeCount, VkDescriptorPoolCreateFlags flags = FLAG_NONE) const;
        VkDescriptorPool CreateDescriptorPool(uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes, VkDescriptorPoolCreateFlags flags = FLAG_NONE) const;
        template<uint32_t COUNT>
        inline VkDescriptorPool CreateDescriptorPool(uint32_t maxSets, const VkDescriptorPoolSize(&poolSizes)[COUNT], VkDescriptorPoolCreateFlags flags = FLAG_NONE) const { return descriptorPool(maxSets, poolSizes, COUNT, flags); }


        VkDescriptorSet CreateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout) const;
        void CreateDescriptorSets(const VkDescriptorSetAllocateInfo& info, VkDescriptorSet* pSets) const;
        void CreateDescriptorSets(VkDescriptorPool pool, const VkDescriptorSetLayout* pSetLayouts, uint32_t setCount, VkDescriptorSet* pDescriptorSets) const;
        void CreateDescriptorSets(VkDescriptorPool pool, const std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSet* pDescriptorSets) const;
        template<uint32_t COUNT>
        inline void CreateDescriptorSets(VkDescriptorPool pool, const VkDescriptorSetLayout(&setLayouts)[COUNT], VkDescriptorSet(&pDescriptorSets)[COUNT]) const { CreateDescriptorSets(pool, setLayouts, COUNT, pDescriptorSets); }

        void UpdateDescriptors(const VkWriteDescriptorSet* pDescriptorWrites, uint32_t writeCount, const VkCopyDescriptorSet* pDescriptorCopies = nullptr, uint32_t copyCount = 0) const;
        template<uint32_t WRITE_COUNT, uint32_t COPY_COUNT>
        inline void UpdateDescriptors(const VkWriteDescriptorSet(&descriptorWrites)[WRITE_COUNT], const VkCopyDescriptorSet(&descriptorCopies)[COPY_COUNT]) const
        { UpdateDescriptors(descriptorWrites, WRITE_COUNT, descriptorCopies, COPY_COUNT); }

        template<uint32_t WRITE_COUNT>
        inline void UpdateDescriptors(const VkWriteDescriptorSet(&descriptorWrites)[WRITE_COUNT]) const
        { UpdateDescriptors(descriptorWrites, WRITE_COUNT, nullptr, 0); }
        

        
        inline void UpdateDescriptor(const VkWriteDescriptorSet& descriptorWrite) const { UpdateDescriptors(&descriptorWrite, 1, nullptr, 0); }

        inline void UpdateDescriptor(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, uint32_t descriptorCount, const VkDescriptorBufferInfo* pBufferInfos, const void* pNext = nullptr) const
        { UpdateDescriptor(Vk::CreateDescriptorWrite(dstSet, dstBinding, dstArrayElement, descriptorType, descriptorCount, pBufferInfos, pNext)); }
        inline void UpdateDescriptor(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, uint32_t descriptorCount, const VkDescriptorImageInfo* pImageInfos, const void* pNext = nullptr) const
        { UpdateDescriptor(Vk::CreateDescriptorWrite(dstSet, dstBinding, dstArrayElement, descriptorType, descriptorCount, pImageInfos, pNext)); }
        inline void UpdateDescriptor(VkDescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, VkDescriptorType descriptorType, uint32_t descriptorCount, const VkBufferView* pBufferViews, const void* pNext = nullptr) const
        { UpdateDescriptor(Vk::CreateDescriptorWrite(dstSet, dstBinding, dstArrayElement, descriptorType, descriptorCount, pBufferViews, pNext)); }
        
        /**
         * @brief gets the first supported format for the requested feature and tiling from supplied candidates.
         * 
         * @return first supported candidate or VK_FORMAT_MAX_ENUM when none are supported.
         */
        VkFormat GetSupportedFormat(const VkFormat* pCandidates, uint32_t candidateCount, VkFormatFeatureFlags features, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL) const;
        VkFormat GetSwapchainFormat(VkSurfaceKHR surface) const;
        void GetSwapchainImages(VkSwapchainKHR swapchain, uint32_t* count, VkImage* pImages) const;
        VkSurfaceFormatKHR PickSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR preference) const;
        VkPresentModeKHR PickPresentMode(VkSurfaceKHR surface, VkPresentModeKHR requested) const;
        uint32_t FindMemoryIndex(uint32_t typeBits, VkMemoryPropertyFlags flags) const;

        VkSampleCountFlagBits GetMaxSupportedSampleCount() const;

        inline void Reset(VkCommandPool commandPool, VkCommandPoolResetFlags flags = FLAG_NONE) const { vkResetCommandPool(logical, commandPool, flags); }

        void Terminate();
    public:
        inline void Destroy() const {}
        void Destroy(VkFence fence) const;
        void Destroy(VkSemaphore semaphore) const;
        void Destroy(VkBuffer buffer) const;
        void Destroy(VkImage image) const;
        void Destroy(VkImageView imageView) const;
        void Destroy(VkFramebuffer framebuffer) const;
        void Destroy(VkRenderPass renderPass) const;
        void Destroy(VkPipeline pipeline) const;
        void Destroy(VkPipelineLayout pipelineLayout) const;
        void Destroy(VkDescriptorSetLayout descriptorSetLayout) const;
        void Destroy(VkDescriptorPool descriptorPool) const;
        void Destroy(VkDeviceMemory memory) const;
        void Destroy(VkCommandPool commandPool) const;
        void Destroy(VkSampler sampler) const;
        void Destroy(VkSwapchainKHR swapchain) const;
        void Destroy(VkShaderModule shaderModule) const;
        template<typename T, typename ...Args>
        inline void Destroy(T type, Args... args) const {
            Destroy(type);
            Destroy(args...);
        }
    private:
        inline Device() {};
        Device(Device&& other) noexcept;
        Device(const Device& other) = delete;
        Device(Device& other) = delete;
        void CreateNew(const DeviceRequirements& requirements);
        void getQueueCreateInfos(VkPhysicalDevice device, uint32_t* pIndexCount, VkDeviceQueueCreateInfo* pQueueCreateInfos, float* pQueuePriorityBuffer);
        void pickPhysicalDevice(const DeviceRequirements& requirements);
        void createLogicalDevice(const DeviceRequirements& requirements);
        friend Window;
        friend Swapchain;
        //friend Builder;
        VkDevice logical = VK_NULL_HANDLE;
        VkPhysicalDevice physical = VK_NULL_HANDLE;
        uint32_t presentFamilyIndex = UINT32_MAX;
        uint32_t presentCount = 0;
        uint32_t graphicsFamilyIndex = UINT32_MAX;
        uint32_t graphicsCount = 0;
        uint32_t transferFamilyIndex = UINT32_MAX;
        uint32_t transferCount = 0;
        uint32_t computeFamilyIndex = UINT32_MAX;
        uint32_t computeCount = 0;
        DeviceFeatureFlags enabledFeatures = 0;
        char name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE] = {};
    private:
        static DeviceRequirements s_Requirements;
        static Device s_Instance;
    };
}