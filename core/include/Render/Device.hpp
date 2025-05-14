#pragma once

#include "SGF_Core.hpp"

#ifndef SGF_MAX_DEVICE_EXTENSION_COUNT
#define SGF_MAX_DEVICE_EXTENSION_COUNT 32
#endif

#include "Image.hpp"
#include "GraphicsPipeline.hpp"
#include "RenderPass.hpp"

namespace SGF {
    enum DeviceFeature {
        DEVICE_FEATURE_ROBUST_BUFFER_ACCESS,
        DEVICE_FEATURE_FULL_DRAW_INDEX_UINT32,
        DEVICE_FEATURE_IMAGE_CUBE_ARRAY,
        DEVICE_FEATURE_INDEPENDENT_BLEND,
        DEVICE_FEATURE_GEOMETRY_SHADER,
        DEVICE_FEATURE_TESSELLATION_SHADER,
        DEVICE_FEATURE_SAMPLE_RATE_SHADING,
        DEVICE_FEATURE_DUAL_SRC_BLEND,
        DEVICE_FEATURE_LOGIC_OP,
        DEVICE_FEATURE_MULTI_DRAW_INDIRECT,
        DEVICE_FEATURE_DRAW_INDIRECT_FIRST_INSTANCE,
        DEVICE_FEATURE_DEPTH_CLAMP,
        DEVICE_FEATURE_DEPTH_BIAS_CLAMP,
        DEVICE_FEATURE_FILL_MODE_NON_SOLID,
        DEVICE_FEATURE_DEPTH_BOUNDS,
        DEVICE_FEATURE_WIDE_LINES,
        DEVICE_FEATURE_LARGE_POINTS,
        DEVICE_FEATURE_ALPHA_TO_ONE,
        DEVICE_FEATURE_MULTI_VIEWPORT,
        DEVICE_FEATURE_SAMPLER_ANISOTROPY,
        DEVICE_FEATURE_TEXTURE_COMPRESSION_ETC2,
        DEVICE_FEATURE_TEXTURE_COMPRESSION_ASTC_LDR,
        DEVICE_FEATURE_TEXTURE_COMPRESSION_BC,
        DEVICE_FEATURE_OCCLUSION_QUERY_PRECISE,
        DEVICE_FEATURE_PIPELINE_STATISTICS_QUERY,
        DEVICE_FEATURE_VERTEX_PIPELINE_STORES_AND_ATOMICS,
        DEVICE_FEATURE_FRAGMENT_STORES_AND_ATOMICS,
        DEVICE_FEATURE_SHADER_TESSELLATION_AND_GEOMETRY_POINT_SIZE,
        DEVICE_FEATURE_SHADER_IMAGE_GATHER_EXTENDED,
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_EXTENDED_FORMATS,
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_MULTISAMPLE,
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_READ_WITHOUT_FORMAT,
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_WRITE_WITHOUT_FORMAT,
        DEVICE_FEATURE_SHADER_UNIFORM_BUFFER_ARRAY_DYNAMIC_INDEXING,
        DEVICE_FEATURE_SHADER_SAMPLED_IMAGE_ARRAY_DYNAMIC_INDEXING,
        DEVICE_FEATURE_SHADER_STORAGE_BUFFER_ARRAY_DYNAMIC_INDEXING,
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_ARRAY_DYNAMIC_INDEXING,
        DEVICE_FEATURE_SHADER_CLIP_DISTANCE,
        DEVICE_FEATURE_SHADER_CULL_DISTANCE,
        DEVICE_FEATURE_SHADER_FLOAT64,
        DEVICE_FEATURE_SHADER_INT64,
        DEVICE_FEATURE_SHADER_INT16,
        DEVICE_FEATURE_SHADER_RESOURCE_RESIDENCY,
        DEVICE_FEATURE_SHADER_RESOURCE_MIN_LOD,
        DEVICE_FEATURE_SPARSE_BINDING,
        DEVICE_FEATURE_SPARSE_RESIDENCY_BUFFER,
        DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE2D,
        DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE3D,
        DEVICE_FEATURE_SPARSE_RESIDENCY_2_SAMPLES,
        DEVICE_FEATURE_SPARSE_RESIDENCY_4_SAMPLES,
        DEVICE_FEATURE_SPARSE_RESIDENCY_8_SAMPLES,
        DEVICE_FEATURE_SPARSE_RESIDENCY_16_SAMPLES,
        DEVICE_FEATURE_SPARSE_RESIDENCY_ALIASED,
        DEVICE_FEATURE_VARIABLE_MULTISAMPLE_RATE,
        DEVICE_FEATURE_INHERITED_QUERIES,
        DEVICE_FEATURE_MAX_ENUM
    };

    enum QueueFamilyFlags {
        QUEUE_FAMILY_GRAPHICS = BIT(0),
        QUEUE_FAMILY_COMPUTE = BIT(1),
        QUEUE_FAMILY_TRANSFER = BIT(2),
        QUEUE_FAMILY_PRESENT = BIT(3)
    };

    class Device {
    public:
        static uint32_t getSupportedDeviceCount(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures, Window* windowSupport, uint32_t graphicsQueueCount, uint32_t computeQueueCount, uint32_t transferQueueCount);
    public:
        class Builder {
        public:
            Builder();
            Device build();
            inline Builder& requireExtension(const char* deviceExtension) { deviceExtensions[deviceExtensionCount] = deviceExtension; ++deviceExtensionCount; return *this; }
            inline Builder& requireFeature(DeviceFeature feature) { VkBool32* feat = (VkBool32*)&features; feat[(uint32_t)feature] = VK_TRUE; return *this; }
            inline Builder& optionalFeature(DeviceFeature feature) { VkBool32* feat = (VkBool32*)&optional; feat[(uint32_t)feature] = VK_TRUE; return *this; }
            inline Builder& graphicQueues(uint32_t count) { graphicsQueueCount = count; return *this; }
            inline Builder& transferQueues(uint32_t count) { transferQueueCount = count; return *this; }
            inline Builder& computeQueues(uint32_t count) { computeQueueCount = count; return *this; }
            inline Builder& bindWindow(Window* window) { pWindow = window; requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME); return *this; }
            inline Builder& requireImageSize1D(uint32_t size) { limits.maxImageDimension1D = size; return *this; }
            inline Builder& requireImageSize2D(uint32_t size) { limits.maxImageDimension2D = size; return *this; }
            inline Builder& requireImageSize3D(uint32_t size) { limits.maxImageDimension3D = size; return *this; }
            inline Builder& requireImageSizeCube(uint32_t size) { limits.maxImageDimensionCube = size; return *this; }
            inline Builder& requireImageArraySize(uint32_t size) { limits.maxImageArrayLayers = size; return *this; }
            inline Builder& requireUniformBufferSize(uint32_t size) { limits.maxUniformBufferRange = size; return *this; }
            inline Builder& requireStorageBufferSize(uint32_t size) { limits.maxStorageBufferRange = size; return *this; }
            inline Builder& requirePushConstantSize(uint32_t size) { limits.maxPushConstantsSize = size; return *this; }
            inline Builder& requireMemoryAllocationCount(uint32_t count) { limits.maxMemoryAllocationCount = count; return *this; }
            inline Builder& requireSampleAllocationCount(uint32_t count) { limits.maxSamplerAllocationCount = count; return *this; }
            inline Builder& requireBoundDescriptorSets(uint32_t count) { limits.maxBoundDescriptorSets = count; return *this; }
        private:
            const char* deviceExtensions[SGF_MAX_DEVICE_EXTENSION_COUNT] = {};
            VkPhysicalDeviceLimits* pLimits = nullptr;
            VkPhysicalDeviceLimits limits = {};
            VkPhysicalDeviceFeatures* pFeatures = nullptr; 
            VkPhysicalDeviceFeatures features = {};
            VkPhysicalDeviceFeatures* pOptional = nullptr;
            VkPhysicalDeviceFeatures optional = {};
            uint32_t graphicsQueueCount = 0;
            uint32_t transferQueueCount = 0;
            uint32_t computeQueueCount = 0;
            uint32_t deviceExtensionCount = 0;
            Window* pWindow;
        };

        /**
         * @brief Creates the device at the specified device index returned by vkEnumeratePhysicalDevices if it matches the requirements
         * 
         * If the device at the specified index does not support the required device extensions another physical device is selected.
         * @param deviceIndex - the index of the physical device returned by vkEnumeratePhysicalDevices
         * @param extensionCount - the number of device extensions requested
         * @param ppDeviceExtensions - pointer to the specified extensions
         * @param pRequiredFeatures - pointer to the required device features can be nullptr 
         * @param pOptionalFeatures - pointer to optional device features will be activated when supported can be nullptr 
         * @param graphicsQueueCount - the number of device queues with graphics support
         * @param transferQueueCount - the number of device queues with transfer support - ideally from a unused queue index
         * @param computeQueueCount - the number of device queues with compute support - ideally from a unused queue index
         * @param window - window to bind device to 
         */
        Device(uint32_t deviceIndex, uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* pRequiredFeatures,
            const VkPhysicalDeviceFeatures* pOptionalFeatures, const VkPhysicalDeviceLimits* minLimits, Window* window, 
            uint32_t graphicsQueueCount, uint32_t computeQueueCount, uint32_t transferQueueCount);

        Device(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* pRequiredFeatures,
            const VkPhysicalDeviceFeatures* pOptionalFeatures, const VkPhysicalDeviceLimits* minLimits, Window* window, 
            uint32_t graphicsQueueCount = 1, uint32_t computeQueueCount = 0, uint32_t transferQueueCount = 0);
        
        typedef void (*DeviceDescructionCallback)(Device& device);
        typedef void (*DeviceCreationCallback)(Device& device);
        void setDestructionCallback(DeviceDescructionCallback func);
        inline ~Device() { destroy(); }

        Device(Device&& other) noexcept;
        Device(const Device& other) = delete;
        Device(Device& other) = delete;
        inline bool isFeatureEnabled(DeviceFeature feature) const { return ((enabledFeatures >> (uint32_t)feature) & 1); }
        void waitIdle() const;
        void waitFence(VkFence fence) const;
        void waitFences(const VkFence* pFences, uint32_t count) const;

        const char* getName() const;
        bool isCreated() const;
        void destroy();
        void create(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
            const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, Window* window, 
            uint32_t graphicsQueueCount, uint32_t computeQueueCount, uint32_t transferQueueCount);
    public:
        
        friend RenderPass::Builder;
        friend GraphicsPipeline::Builder;
        // builder functions:
        inline GraphicsPipeline::Builder graphicsPipeline(VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass) { return GraphicsPipeline::Builder(this, layout, renderPass, subpass); };

        inline Image::Builder image(uint32_t length) { return Image::Builder(*this, length); }
        inline Image::Builder image(uint32_t width, uint32_t height) { return Image::Builder(*this, width, height); }
        inline Image::Builder image(uint32_t width, uint32_t height, uint32_t depth) { return Image::Builder(*this, width, height, depth); }
        inline ImageView::Builder imageView(const Image& image) { return ImageView::Builder(*this, image); }
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
        Buffer buffer(const VkBufferCreateInfo& info) const;
        Buffer buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags createFlags = 0) const;
        Buffer buffer(VkDeviceSize size, VkBufferUsageFlags usage, QueueFamilyFlags flags, VkBufferCreateFlags createFlags = 0) const;
        Image image(const VkImageCreateInfo& info) const;
        /**
         * @brief creates an 1D-image for use with the graphics queue-family
         * 
         * requires at least one graphics-Queue. If more specific image are to be created use the builder-> Device::image(...)::foo(...)::build(), or use your own VkImageCreateInfo.
         * 
         * @return 1D-image
         */
        Image image1D(uint32_t length, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0) const;
        Image image2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0) const;
        Image image3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0) const;
        Image imageArray1D(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0) const;
        Image imageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0) const;
        Image imageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0) const;

        Image image1D(uint32_t length, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        Image image2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        Image image3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        Image imageArray1D(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        Image imageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        Image imageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, QueueFamilyFlags queueFamilies = QUEUE_FAMILY_GRAPHICS, VkImageCreateFlags flags = 0) const;
        
        ImageView imageView(const VkImageViewCreateInfo& info) const;
        ImageView imageView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        ImageView imageView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        ImageView imageView3D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        ImageView imageViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0) const;
        ImageView imageArrayView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1) const;
        ImageView imageArrayView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1) const;
        ImageView imageArrayViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1) const;

        DeviceMemory allocate(const VkMemoryAllocateInfo& info) const;
        /**
         * @brief allocates device-memory for the given requirements.
         * 
         * @return allocated memory with the required memory-size
         */
        DeviceMemory allocate(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags flags) const;
        /**
         * @brief allocates device-memory for the given buffer and binds the buffer to it.
         * 
         * @return allocated memory with the required memory-size for the buffer
         */
        DeviceMemory allocate(VkBuffer buffer, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        /**
         * @brief allocates device-memory for all given buffers and binds them to it.
         * 
         * Buffers are bound in the order they are submitted to the function while respecting the required alignment for each buffer. 
         * 
         * @return allocated memory with the required memory-size for all buffers.
         */
        DeviceMemory allocate(const VkBuffer* pBuffers, uint32_t bufferCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        /**
         * @brief allocates device-memory for the given image and binds them.
         * 
         * @return allocated memory with required size for the image.
         */
        DeviceMemory allocate(VkImage image, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        /**
         * @brief allocates device-memory for all given images and binds them to it.
         * 
         * images are bound in the order they are submitted to the function while respecting the required alignment for each image. 
         * 
         * @return allocated memory with the required memory-size for all images.
         */
        DeviceMemory allocate(const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;
        /**
         * @brief allocates device-memory for all given buffers and images and binds them to it.
         * 
         * buffers and images are bound in the order they are submitted to the function while respecting the required alignment for each.
         * buffers are bound before images, because their required alignments are typically smaller than that for images - possibly saving memory.
         * 
         * @return allocated memory with the required memory-size for all buffers and images.
         */
        DeviceMemory allocate(const VkBuffer* pBuffers, uint32_t bufferCount, const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) const;

        VkShaderModule shaderModule(const char* filename) const;
        VkShaderModule shaderModule(const VkShaderModuleCreateInfo& info) const;
        VkPipelineLayout pipelineLayout(const VkPipelineLayoutCreateInfo& info) const;
        VkPipelineLayout pipelineLayout(uint32_t descriptorLayoutCount, const VkDescriptorSetLayout* pLayouts, uint32_t pushConstantCount = 0, const VkPushConstantRange* pPushConstantRanges = nullptr) const;
        VkPipeline pipeline(const VkGraphicsPipelineCreateInfo& info) const;
        VkPipeline pipeline(const VkComputePipelineCreateInfo& info) const;
        VkSwapchainKHR swapchain(const VkSwapchainCreateInfoKHR& info) const;

        VkFramebuffer framebuffer(const VkFramebufferCreateInfo& info) const;
        VkFramebuffer framebuffer(VkRenderPass renderPass, const VkImageView* pAttachments, uint32_t attachmentCount, uint32_t width, uint32_t height, uint32_t layerCount) const;

        VkRenderPass renderPass(const VkRenderPassCreateInfo& info) const;
        VkRenderPass renderPass(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependenfyCount) const;
        /**
         * @brief gets the first supported format for the requested feature and tiling from supplied candidates.
         * 
         * @return first supported candidate or VK_FORMAT_MAX_ENUM when none are supported.
         */
        VkFormat getSupportedFormat(const VkFormat* pCandidates, uint32_t candidateCount, VkFormatFeatureFlags features, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL) const;
    private:
        uint32_t findMemoryIndex(uint32_t typeBits, VkMemoryPropertyFlags flags) const;
    public:
        void destroy(VkFence fence) const;
        void destroy(VkSemaphore semaphore) const;
        void destroy(VkBuffer buffer) const;
        void destroy(VkImage image) const;
        void destroy(VkImageView imageView) const;
        void destroy(VkFramebuffer framebuffer) const;
        void destroy(VkRenderPass renderPass) const;
        void destroy(VkPipeline pipeline) const;
        void destroy(VkPipelineLayout pipelineLayout) const;
        void destroy(VkDescriptorSetLayout descriptorSetLayout) const;
        void destroy(VkDescriptorPool descriptorPool) const;
        void destroy(VkDeviceMemory memory) const;
        void destroy(VkCommandPool commandPool) const;
        void destroy(VkSampler sampler) const;
        void destroy(VkSwapchainKHR swapchain) const;
        inline void destroy() const {}
        template<typename T, typename ...Args>
        inline void destroy(T type, Args... args) const {
            destroy(type);
            destroy(args...);
        }
    private:
        void getQueueCreateInfos(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* pIndexCount, VkDeviceQueueCreateInfo* pQueueCreateInfos, float* pQueuePriorityBuffer);
        void pickPhysicalDevice(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
            const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, VkSurfaceKHR surface);
        void createLogicalDevice(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
            const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, VkSurfaceKHR surface);
        friend Window;
        friend Builder;
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
        uint64_t enabledFeatures = 0;
        char name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE] = {};
    };

    class DeviceDestroyEvent {
    public:
        inline DeviceDestroyEvent(Device* dev) : device(dev) {}
        inline Device* getDevice() const { return device; }
    private:
        Device* device;
    };
    inline constexpr uint32_t DeviceSize = sizeof(Device);
    inline constexpr uint32_t DeviceBuilderSize = sizeof(Device::Builder);
}