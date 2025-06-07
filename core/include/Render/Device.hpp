#pragma once

#include "SGF_Core.hpp"
#include "GraphicsPipeline.hpp"

#ifndef SGF_MAX_DEVICE_EXTENSION_COUNT
#define SGF_MAX_DEVICE_EXTENSION_COUNT 32
#endif

namespace SGF {
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

        inline static void Shutdown() { s_Instance.shutdown(); }
        inline static bool IsInitialized() { return s_Instance.isCreated(); }
    public:
        
        inline ~Device() { shutdown(); }
        void waitIdle() const;

        inline bool hasFeaturesEnabled(DeviceFeatureFlags features) const { return (enabledFeatures & features) == features; }
        inline bool hasFeatureEnabled(DeviceFeatureFlagBits feature) const { return (enabledFeatures & feature); }

        void waitFence(VkFence fence) const;
        void waitFences(const VkFence* pFences, uint32_t count) const;
        inline void waitFences(const std::vector<VkFence>& fences) const { waitFences(fences.data(), (uint32_t)fences.size()); }

        void reset(const VkFence* pFences, uint32_t count) const;
        inline void reset(const std::vector<VkFence>& fences) const { waitFences(fences.data(), (uint32_t)fences.size()); }
        inline void reset(VkFence fence) const { reset(&fence, 1); }

        const char* getName() const;
        bool isCreated() const;
    public:
        inline operator VkDevice() const { return logical; }
        inline operator VkPhysicalDevice() const { return physical; }
        inline VkDevice getLogical() const { return logical; }
        inline VkPhysicalDevice getPhysical() const { return physical; }
        // Queue functions:
        inline uint32_t graphicsFamily() const { return graphicsFamilyIndex; }
        inline uint32_t computeFamily() const { return computeFamilyIndex; }
        inline uint32_t transferFamily() const { return transferFamilyIndex; }
        inline uint32_t presentFamily() const { return presentFamilyIndex; }

        inline uint32_t graphicsQueueCount() const { return graphicsCount; }
        inline uint32_t computeQueueCount() const { return computeCount; }
        inline uint32_t transferQueueCount() const { return transferCount; }
        inline uint32_t presentQueueCount() const { return presentCount; }

        VkQueue graphicsQueue(uint32_t index) const;
        VkQueue computeQueue(uint32_t index) const;
        VkQueue transferQueue(uint32_t index) const;
        VkQueue presentQueue() const;

        VkFence fence() const;
        VkFence fenceSignaled() const;
        VkSemaphore semaphore() const;
        //void signalSemaphore(VkSemaphore semaphore, uint64_t value) const;

        VkBuffer buffer(const VkBufferCreateInfo& info) const;
        VkBuffer buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags createFlags = 0) const;
        VkBuffer bufferShared(VkDeviceSize size, VkBufferUsageFlags usage, QueueFamilyFlags flags, VkBufferCreateFlags createFlags = 0) const;

        VkImage image(const VkImageCreateInfo& info) const;
        /**
         * @brief creates an 1D-image for use with the graphics queue-family
         * 
         * requires at least one graphics-Queue. If more specific image are to be created use the builder-> Device::image(...)::foo(...)::build(), or use your own VkImageCreateInfo.
         * 
         * @return 1D-image
         */
        VkImage image1D(uint32_t length, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage image2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage image3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage imageArray1D(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage imageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;
        VkImage imageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, VkImageCreateFlags flags = 0) const;

        VkImage image1DShared(uint32_t length, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage image2DShared(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage image3DShared(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage imageArray1DShared(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage imageArray2DShared(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        VkImage imageArray3DShared(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevelCount = 1, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;

        VkImageView imageView(const VkImageViewCreateInfo& info) const;
        VkImageView imageView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        VkImageView imageView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        VkImageView imageView3D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        VkImageView imageViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        VkImageView imageArrayView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1) const;
        VkImageView imageArrayView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1) const;
        VkImageView imageArrayViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1) const;

        VkSampler imageSampler(const VkSamplerCreateInfo& info) const;
        VkSampler imageSampler(VkFilter filterType = VK_FILTER_NEAREST, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 
            float mipLodBias = 0.0f, VkBool32 anisotropyEnable = VK_FALSE, float maxAnisotropy = 0.0f, VkBool32 compareEnable = VK_FALSE, VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS, 
            float minLod = 0.0f, float maxLod = 0.0f, VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, VkBool32 unnormalizedCoordinates = VK_FALSE, VkSamplerCreateFlags flags = FLAG_NONE, const void* pNext = nullptr) const;

        VkDeviceMemory allocate(const VkMemoryAllocateInfo& info) const;
        VkDeviceMemory allocate(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags flags) const;

        VkDeviceMemory allocate(VkBuffer buffer, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        VkDeviceMemory allocate(VkImage image, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;

        VkDeviceMemory allocate(const VkBuffer* pBuffers, uint32_t bufferCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        inline VkDeviceMemory allocate(const std::vector<VkBuffer>& buffers, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const 
        { return allocate(buffers.data(), (uint32_t)buffers.size(), flags); }
        VkDeviceMemory allocate(const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        inline VkDeviceMemory allocate(const std::vector<VkImage>& images, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const 
        { return allocate(images.data(), (uint32_t)images.size(), flags); }

        VkDeviceMemory allocate(const VkBuffer* pBuffers, uint32_t bufferCount, const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        inline VkDeviceMemory allocate(const std::vector<VkBuffer>& buffers, const std::vector<VkImage>& images, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const 
        { return allocate(buffers.data(), (uint32_t)buffers.size(), images.data(), (uint32_t)images.size(), flags); }


        VkShaderModule shaderModule(const char* filename) const;
        VkShaderModule shaderModule(const VkShaderModuleCreateInfo& info) const;
        VkPipelineLayout pipelineLayout(const VkPipelineLayoutCreateInfo& info) const;
        VkPipelineLayout pipelineLayout(const VkDescriptorSetLayout* pLayouts, uint32_t descriptorLayoutCount, const VkPushConstantRange* pPushConstantRanges = nullptr, uint32_t pushConstantCount = 0) const;
        template<size_t DESCRIPTOR_COUNT, size_t PUSH_CONSTANT_COUNT>
        VkPipelineLayout pipelineLayout(const VkDescriptorSetLayout(&layouts)[DESCRIPTOR_COUNT], const VkPushConstantRange(&pushConstants)[PUSH_CONSTANT_COUNT]) const;
        VkPipelineLayout pipelineLayout(const std::vector<VkDescriptorSetLayout>& layouts, const std::vector<VkPushConstantRange>& pushConstants) const;
        VkPipeline pipeline(const VkGraphicsPipelineCreateInfo& info) const;
        VkPipeline pipeline(const VkComputePipelineCreateInfo& info) const;
        inline GraphicsPipelineBuilder graphicsPipeline(VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass) const { return GraphicsPipelineBuilder(this, layout, renderPass, subpass); };

        VkSwapchainKHR swapchain(const VkSwapchainCreateInfoKHR& info) const;

        VkFramebuffer framebuffer(const VkFramebufferCreateInfo& info) const;
        VkFramebuffer framebuffer(VkRenderPass renderPass, const VkImageView* pAttachments, uint32_t attachmentCount, uint32_t width, uint32_t height, uint32_t layerCount) const;

        VkRenderPass renderPass(const VkRenderPassCreateInfo& info) const;
        VkRenderPass renderPass(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies = nullptr, uint32_t dependencyCount = 0) const;

        template<size_t ATTACHMENT_COUNT, size_t SUBPASS_COUNT, size_t DEPENDENCY_COUNT>
        inline VkRenderPass renderPass(const VkAttachmentDescription(&attachments)[ATTACHMENT_COUNT], const VkSubpassDescription(&subpasses)[SUBPASS_COUNT], const VkSubpassDependency(&dependencies)[DEPENDENCY_COUNT]) const {
            return renderPass(attachments, ATTACHMENT_COUNT, subpasses, SUBPASS_COUNT, dependencies, DEPENDENCY_COUNT); }
        inline VkRenderPass renderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies) const {
            return renderPass(attachments.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size(), dependencies.data(), (uint32_t)dependencies.size());
        }

        VkCommandPool commandPool(const VkCommandPoolCreateInfo& info) const;
        VkCommandPool commandPool(uint32_t queueIndex, VkCommandPoolCreateFlags flags = FLAG_NONE) const;
        VkCommandBuffer commandBuffer(VkCommandPool pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
        void commandBuffers(const VkCommandBufferAllocateInfo& info, VkCommandBuffer* pBuffers) const;
        void commandBuffers(VkCommandPool pool, VkCommandBufferLevel level, uint32_t allocationCount, VkCommandBuffer* pBuffers) const;

        VkDescriptorSetLayout descriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info) const;
        VkDescriptorSetLayout descriptorSetLayout(const VkDescriptorSetLayoutBinding* pBindings, uint32_t bindingCount, VkDescriptorSetLayoutCreateFlags flags = FLAG_NONE) const;
        VkDescriptorSetLayout descriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayoutCreateFlags flags = FLAG_NONE) const;

        VkDescriptorPool descriptorPool(const VkDescriptorPoolCreateInfo& info) const;
        VkDescriptorPool descriptorPool(uint32_t maxSets, const VkDescriptorPoolSize* pPoolSizes, uint32_t poolSizeCount, VkDescriptorPoolCreateFlags flags = FLAG_NONE) const;
        VkDescriptorPool descriptorPool(uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes, VkDescriptorPoolCreateFlags flags = FLAG_NONE) const;

        VkDescriptorSet descriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout) const;
        void descriptorSets(const VkDescriptorSetAllocateInfo& info, VkDescriptorSet* pSets) const;
        void descriptorSets(VkDescriptorPool pool, const VkDescriptorSetLayout* pSetLayouts, uint32_t setCount, VkDescriptorSet* pDescriptorSets) const;
        void descriptorSets(VkDescriptorPool pool, const std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSet* pDescriptorSets) const;
        /**
         * @brief gets the first supported format for the requested feature and tiling from supplied candidates.
         * 
         * @return first supported candidate or VK_FORMAT_MAX_ENUM when none are supported.
         */
        VkFormat getSupportedFormat(const VkFormat* pCandidates, uint32_t candidateCount, VkFormatFeatureFlags features, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL) const;
        VkFormat getSwapchainFormat(VkSurfaceKHR surface) const;
        void getSwapchainImages(VkSwapchainKHR swapchain, uint32_t* count, VkImage* pImages) const;
        VkSurfaceFormatKHR pickSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR preference) const;
        VkPresentModeKHR pickPresentMode(VkSurfaceKHR surface, VkPresentModeKHR requested) const;
        uint32_t findMemoryIndex(uint32_t typeBits, VkMemoryPropertyFlags flags) const;

        VkSampleCountFlagBits getMaxSupportedSampleCount() const;

        inline void reset(VkCommandPool commandPool, VkCommandPoolResetFlags flags = FLAG_NONE) const { vkResetCommandPool(logical, commandPool, flags); }

        void shutdown();
    private:
        inline void destroyType() const {}
        void destroyType(VkFence fence) const;
        void destroyType(VkSemaphore semaphore) const;
        void destroyType(VkBuffer buffer) const;
        void destroyType(VkImage image) const;
        void destroyType(VkImageView imageView) const;
        void destroyType(VkFramebuffer framebuffer) const;
        void destroyType(VkRenderPass renderPass) const;
        void destroyType(VkPipeline pipeline) const;
        void destroyType(VkPipelineLayout pipelineLayout) const;
        void destroyType(VkDescriptorSetLayout descriptorSetLayout) const;
        void destroyType(VkDescriptorPool descriptorPool) const;
        void destroyType(VkDeviceMemory memory) const;
        void destroyType(VkCommandPool commandPool) const;
        void destroyType(VkSampler sampler) const;
        void destroyType(VkSwapchainKHR swapchain) const;
        void destroyType(VkShaderModule shaderModule) const;
    public:
        template<typename T>
        inline void destroy(T type) const {
            destroyType(type);
        }
        template<typename T, typename ...Args>
        inline void destroy(T type, Args... args) const {
            destroyType(type);
            destroy(args...);
        }
    private:
        inline Device() {};
        Device(Device&& other) noexcept;
        Device(const Device& other) = delete;
        Device(Device& other) = delete;
        void createNew(const DeviceRequirements& requirements);
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

    class DeviceDestroyEvent {
    public:
        inline DeviceDestroyEvent(const Device& d) : device(d) {}
        inline const Device& getDevice() const { return device; }
    private:
        const Device& device;
    };
}