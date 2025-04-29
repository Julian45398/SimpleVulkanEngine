#pragma once

#include "SGF_Core.hpp"

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <vulkan/vulkan.h>
#include <vector>
#include <assert.h>

#include "SGF_Types.hpp"

#ifndef SGF_MAX_DEVICE_EXTENSION_COUNT
#define SGF_MAX_DEVICE_EXTENSION_COUNT 32
#endif
#ifndef SGF_RENDER_PASS_MAX_SUBPASSES
#define SGF_RENDER_PASS_MAX_SUBPASSES 4
#endif
#ifndef SGF_PIPELINE_MAX_DYNAMIC_STATES 
#define SGF_PIPELINE_MAX_DYNAMIC_STATES 32
#endif
#ifndef SGF_PIPELINE_MAX_PIPELINE_STAGES 
#define SGF_PIPELINE_MAX_PIPELINE_STAGES 4
#endif

enum SGF_DeviceFeature {
    SGF_DEVICE_FEATURE_ROBUST_BUFFER_ACCESS,
    SGF_DEVICE_FEATURE_FULL_DRAW_INDEX_UINT32,
    SGF_DEVICE_FEATURE_IMAGE_CUBE_ARRAY,
    SGF_DEVICE_FEATURE_INDEPENDENT_BLEND,
    SGF_DEVICE_FEATURE_GEOMETRY_SHADER,
    SGF_DEVICE_FEATURE_TESSELLATION_SHADER,
    SGF_DEVICE_FEATURE_SAMPLE_RATE_SHADING,
    SGF_DEVICE_FEATURE_DUAL_SRC_BLEND,
    SGF_DEVICE_FEATURE_LOGIC_OP,
    SGF_DEVICE_FEATURE_MULTI_DRAW_INDIRECT,
    SGF_DEVICE_FEATURE_DRAW_INDIRECT_FIRST_INSTANCE,
    SGF_DEVICE_FEATURE_DEPTH_CLAMP,
    SGF_DEVICE_FEATURE_DEPTH_BIAS_CLAMP,
    SGF_DEVICE_FEATURE_FILL_MODE_NON_SOLID,
    SGF_DEVICE_FEATURE_DEPTH_BOUNDS,
    SGF_DEVICE_FEATURE_WIDE_LINES,
    SGF_DEVICE_FEATURE_LARGE_POINTS,
    SGF_DEVICE_FEATURE_ALPHA_TO_ONE,
    SGF_DEVICE_FEATURE_MULTI_VIEWPORT,
    SGF_DEVICE_FEATURE_SAMPLER_ANISOTROPY,
    SGF_DEVICE_FEATURE_TEXTURE_COMPRESSION_ETC2,
    SGF_DEVICE_FEATURE_TEXTURE_COMPRESSION_ASTC_LDR,
    SGF_DEVICE_FEATURE_TEXTURE_COMPRESSION_BC,
    SGF_DEVICE_FEATURE_OCCLUSION_QUERY_PRECISE,
    SGF_DEVICE_FEATURE_PIPELINE_STATISTICS_QUERY,
    SGF_DEVICE_FEATURE_VERTEX_PIPELINE_STORES_AND_ATOMICS,
    SGF_DEVICE_FEATURE_FRAGMENT_STORES_AND_ATOMICS,
    SGF_DEVICE_FEATURE_SHADER_TESSELLATION_AND_GEOMETRY_POINT_SIZE,
    SGF_DEVICE_FEATURE_SHADER_IMAGE_GATHER_EXTENDED,
    SGF_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_EXTENDED_FORMATS,
    SGF_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_MULTISAMPLE,
    SGF_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_READ_WITHOUT_FORMAT,
    SGF_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_WRITE_WITHOUT_FORMAT,
    SGF_DEVICE_FEATURE_SHADER_UNIFORM_BUFFER_ARRAY_DYNAMIC_INDEXING,
    SGF_DEVICE_FEATURE_SHADER_SAMPLED_IMAGE_ARRAY_DYNAMIC_INDEXING,
    SGF_DEVICE_FEATURE_SHADER_STORAGE_BUFFER_ARRAY_DYNAMIC_INDEXING,
    SGF_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_ARRAY_DYNAMIC_INDEXING,
    SGF_DEVICE_FEATURE_SHADER_CLIP_DISTANCE,
    SGF_DEVICE_FEATURE_SHADER_CULL_DISTANCE,
    SGF_DEVICE_FEATURE_SHADER_FLOAT64,
    SGF_DEVICE_FEATURE_SHADER_INT64,
    SGF_DEVICE_FEATURE_SHADER_INT16,
    SGF_DEVICE_FEATURE_SHADER_RESOURCE_RESIDENCY,
    SGF_DEVICE_FEATURE_SHADER_RESOURCE_MIN_LOD,
    SGF_DEVICE_FEATURE_SPARSE_BINDING,
    SGF_DEVICE_FEATURE_SPARSE_RESIDENCY_BUFFER,
    SGF_DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE2D,
    SGF_DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE3D,
    SGF_DEVICE_FEATURE_SPARSE_RESIDENCY_2_SAMPLES,
    SGF_DEVICE_FEATURE_SPARSE_RESIDENCY_4_SAMPLES,
    SGF_DEVICE_FEATURE_SPARSE_RESIDENCY_8_SAMPLES,
    SGF_DEVICE_FEATURE_SPARSE_RESIDENCY_16_SAMPLES,
    SGF_DEVICE_FEATURE_SPARSE_RESIDENCY_ALIASED,
    SGF_DEVICE_FEATURE_VARIABLE_MULTISAMPLE_RATE,
    SGF_DEVICE_FEATURE_INHERITED_QUERIES,
    SGF_DEVICE_FEATURE_MAX_ENUM
};

class SGF_Device {
public:
    class Builder {
    public:
        Builder();
        SGF_Device build();
        inline Builder& requireExtension(const char* deviceExtension) { deviceExtensions[deviceExtensionCount] = deviceExtension; ++deviceExtensionCount; return *this; }
        inline Builder& requireFeature(SGF_DeviceFeature feature) { VkBool32* feat = (VkBool32*)&features; feat[(uint32_t)feature] = VK_TRUE; return *this; }
        inline Builder& optionalFeature(SGF_DeviceFeature feature) { VkBool32* feat = (VkBool32*)&optional; feat[(uint32_t)feature] = VK_TRUE; return *this; }
        inline Builder& graphicQueues(uint32_t count) { graphicsQueueCount = count; return *this; }
        inline Builder& transferQueues(uint32_t count) { transferQueueCount = count; return *this; }
        inline Builder& computeQueues(uint32_t count) { computeQueueCount = count; return *this; }
        inline Builder& bindWindow(SGF_Window* window) { pWindow = window; requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME); return *this; }
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
        SGF_Window* pWindow;
    };

    /**
     * @brief Creates the device at the specified device index returned by vkEnumeratePhysicalDevices if it matches the requirements
     * 
     * If the device at the specified index does not support the required device extensions another physical device is selected.
     * @param deviceIndex - the index of the physical device returned by vkEnumeratePhysicalDevices
     * @param extensionCount - the number of device extensions requested
     * @param ppDeviceExtensions - pointer to the specified extensions
     * @param graphicsQueueCount - the number of device queues with graphics support
     * @param transferQueueCount - the number of device queues with transfer support - ideally from a unused queue index
     * @param computeQueueCount - the number of device queues with compute support - ideally from a unused queue index
     * @param window - window to bind device to 
     */
    SGF_Device(uint32_t deviceIndex, uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
        const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, SGF_Window* window, 
        uint32_t graphicsQueueCount, uint32_t computeQueueCount, uint32_t transferQueueCount);

    SGF_Device(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
        const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, SGF_Window* window, 
        uint32_t graphicsQueueCount, uint32_t computeQueueCount, uint32_t transferQueueCount);
    
    ~SGF_Device();
    SGF_Device(SGF_Device&& other) noexcept;
    SGF_Device(const SGF_Device& other) = delete;
    SGF_Device(SGF_Device& other) = delete;
    inline bool isFeatureEnabled(SGF_DeviceFeature feature) { return ((enabledFeatures >> (uint32_t)feature) & 1); }
    void waitIdle();
    void waitFence(VkFence fence);
    void waitFences(const VkFence* pFences, uint32_t count);
public:
    class GraphicsPipelineBuilder {
    public:
        VkPipeline build();
        inline GraphicsPipelineBuilder& layout(VkPipelineLayout layout) { info.layout = layout; return *this; }
        inline GraphicsPipelineBuilder& layout(VkRenderPass renderPass, uint32_t subpass = 0) { info.renderPass = renderPass; info.subpass = subpass; return *this; }
        GraphicsPipelineBuilder& geometryShader(const char* filename);
        GraphicsPipelineBuilder& fragmentShader(const char* filename);
        GraphicsPipelineBuilder& vertexShader(const char* filename);
        inline GraphicsPipelineBuilder& vertexInput(const VkPipelineVertexInputStateCreateInfo& inputState) { info.pVertexInputState = &inputState; return *this; }
        inline GraphicsPipelineBuilder& polygonMode(VkPolygonMode mode) { rasterizationState.polygonMode = mode; return *this; }
        inline GraphicsPipelineBuilder& topology(VkPrimitiveTopology topology) { inputAssemblyState.topology = topology; return *this; }
        inline GraphicsPipelineBuilder& depth(bool test, bool write, VkCompareOp op = VK_COMPARE_OP_LESS) {depthStencilState.depthWriteEnable = (VkBool32)write; depthStencilState.depthTestEnable = (VkBool32)test; depthStencilState.depthCompareOp = op; return*this; }
        inline GraphicsPipelineBuilder& dynamicState(VkDynamicState state) {dynamicStates[dynamicStateInfo.dynamicStateCount] = state; dynamicStateInfo.dynamicStateCount++; return *this; }
        ~GraphicsPipelineBuilder();
    private:
        GraphicsPipelineBuilder(SGF_Device* device);
        friend SGF_Device;
        VkGraphicsPipelineCreateInfo info;
        VkPipelineShaderStageCreateInfo pipelineStages[SGF_PIPELINE_MAX_PIPELINE_STAGES];
        //VkPipelineVertexInputStateCreateInfo vertexInputState;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
        VkPipelineTessellationStateCreateInfo tessellationState;
        VkPipelineViewportStateCreateInfo viewportState;
        VkPipelineRasterizationStateCreateInfo rasterizationState;
        VkPipelineMultisampleStateCreateInfo multisampleState;
        VkPipelineDepthStencilStateCreateInfo depthStencilState;
        VkPipelineColorBlendStateCreateInfo colorBlendState;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	    VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
	    VkViewport viewport;
	    VkRect2D scissor;
        VkDynamicState dynamicStates[SGF_PIPELINE_MAX_DYNAMIC_STATES];
        SGF_Device* device;
    };
    class ImageBuilder {
    public: 
        inline SGF_Image build() { return pDevice->image(info); }
        inline ImageBuilder& set1D(uint32_t length) { info.imageType = VK_IMAGE_TYPE_1D; info.extent = {length, 1, 1}; return *this; }
        inline ImageBuilder& set2D(uint32_t width, uint32_t height) { info.imageType = VK_IMAGE_TYPE_2D; info.extent = {width, height, 1}; return *this; }
        inline ImageBuilder& set3D(uint32_t width, uint32_t height, uint32_t depth) { info.imageType = VK_IMAGE_TYPE_3D; info.extent = {width, height, depth}; return *this; }
        inline ImageBuilder& array(uint32_t size) { info.arrayLayers = size; return *this; }
        inline ImageBuilder& mip(uint32_t levelCount) { info.mipLevels = levelCount; return *this; }
        inline ImageBuilder& format(VkFormat format) { info.format = format; return *this; }
        inline ImageBuilder& tiling(VkImageTiling tiling) { info.tiling = tiling; return *this; }
        inline ImageBuilder& layout(VkImageLayout initialLayout) { info.initialLayout = initialLayout; return *this; }
        inline ImageBuilder& sampleCount(VkSampleCountFlagBits samples) { info.samples = samples; return *this; }
        inline ImageBuilder& usage(VkImageUsageFlags usage) { info.usage = usage; return *this; }
        inline ImageBuilder& graphics() { assert(info.queueFamilyIndexCount < 4); queueIndices[info.queueFamilyIndexCount] = pDevice->graphicsIndex(); info.queueFamilyIndexCount++;; return *this; }
        inline ImageBuilder& compute() { assert(info.queueFamilyIndexCount < 4); queueIndices[info.queueFamilyIndexCount] = pDevice->computeIndex(); info.queueFamilyIndexCount++;; return *this; }
        inline ImageBuilder& transfer() { assert(info.queueFamilyIndexCount < 4); queueIndices[info.queueFamilyIndexCount] = pDevice->transferIndex(); info.queueFamilyIndexCount++;; return *this; }
        inline ImageBuilder& present() { assert(info.queueFamilyIndexCount < 4); queueIndices[info.queueFamilyIndexCount] = pDevice->presentIndex(); info.queueFamilyIndexCount++;; return *this; }
    private:
        ImageBuilder(SGF_Device* device, uint32_t length);
        ImageBuilder(SGF_Device* device, uint32_t width, uint32_t height);
        ImageBuilder(SGF_Device* device, uint32_t width, uint32_t height, uint32_t depth);
        friend SGF_Device;
        VkImageCreateInfo info;
        uint32_t queueIndices[4];
        SGF_Device* pDevice;
    };
    class ImageViewBuilder {
    public:
        inline SGF_ImageView build() { return pDevice->imageView(info); }
        inline ImageViewBuilder& swizzle(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a) { info.components.r = r; info.components.g = g; info.components.b = b; info.components.a = a; return *this; }
        inline ImageViewBuilder& aspect(VkImageAspectFlags aspect) { info.subresourceRange.aspectMask = aspect; return *this; }
        inline ImageViewBuilder& mip(uint32_t baseLevel, uint32_t levelCount) { info.subresourceRange.baseMipLevel = baseLevel; info.subresourceRange.levelCount = levelCount; return *this; }
        inline ImageViewBuilder& view1D(uint32_t baseLayer = 0) { info.viewType = VK_IMAGE_VIEW_TYPE_1D; info.subresourceRange.baseArrayLayer = baseLayer; return *this; }
        inline ImageViewBuilder& view2D(uint32_t baseLayer = 0) { info.viewType = VK_IMAGE_VIEW_TYPE_2D; info.subresourceRange.baseArrayLayer = baseLayer; return *this; }
        inline ImageViewBuilder& view3D(uint32_t baseLayer = 0) { info.viewType = VK_IMAGE_VIEW_TYPE_3D; info.subresourceRange.baseArrayLayer = baseLayer; return *this; }
        inline ImageViewBuilder& view1DArray(uint32_t baseLayer = 0, uint32_t arraySize = 1) { info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;info.subresourceRange.baseArrayLayer = baseLayer; info.subresourceRange.layerCount = arraySize; return *this; }
        inline ImageViewBuilder& view2DArray(uint32_t baseLayer = 0, uint32_t arraySize = 1) { info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;info.subresourceRange.baseArrayLayer = baseLayer; info.subresourceRange.layerCount = arraySize; return *this; }
        inline ImageViewBuilder& viewCubeArray(uint32_t baseLayer = 0, uint32_t arraySize = 1) { info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;info.subresourceRange.baseArrayLayer = baseLayer; info.subresourceRange.layerCount = arraySize; return *this; }
    private:
        ImageViewBuilder(SGF_Device* device, const SGF_Image& image);
        friend SGF_Device;
        VkImageViewCreateInfo info;
        SGF_Device* pDevice;
    };
    class RenderPassBuilder {
    public:
        inline VkRenderPass build() {return pDevice->renderPass(info);}
    private:
        VkRenderPassCreateInfo info;
        VkSubpassDescription subpasses[8];
        SGF_Device* pDevice;
    };
    friend GraphicsPipelineBuilder;
    // builder functions:
    inline GraphicsPipelineBuilder graphicsPipeline() { return GraphicsPipelineBuilder(this); };
    inline ImageBuilder image(uint32_t length) { return ImageBuilder(this, length); }
    inline ImageBuilder image(uint32_t width, uint32_t height) { return ImageBuilder(this, width, height); }
    inline ImageBuilder image(uint32_t width, uint32_t height, uint32_t depth) { return ImageBuilder(this, width, height, depth); }
    inline ImageViewBuilder imageView(const SGF_Image& image) { return ImageViewBuilder(this, image); }
    // Queue functions:
    uint32_t graphicsIndex();
    uint32_t computeIndex();
    uint32_t transferIndex();
    uint32_t presentIndex();
    VkQueue graphicsQueue(uint32_t index);
    VkQueue computeQueue(uint32_t index);
    VkQueue transferQueue(uint32_t index);

    VkFence fence();
    VkFence fenceSignaled();
    VkSemaphore semaphore();

    VkBuffer buffer(VkDeviceSize size, VkBufferUsageFlags usage);
    SGF_Image image(const VkImageCreateInfo& info);
    /**
     * @brief creates an 1D-image for use with the graphics queue-family
     * 
     * requires at least one graphics-Queue. If more specific image are to be created use the builder-> SGF_Device::image(...)::foo(...)::build(), or use your own VkImageCreateInfo.
     * 
     * @return 1D-image
     */
    SGF_Image image1D(uint32_t length, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0);
    SGF_Image image2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0);
    SGF_Image image3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0);
    SGF_Image imageArray1D(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0);
    SGF_Image imageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0);
    SGF_Image imageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageCreateFlags flags = 0);
    SGF_ImageView imageView(const VkImageViewCreateInfo& info);
    SGF_ImageView imageView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0);
    SGF_ImageView imageView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0);
    SGF_ImageView imageView3D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0);
    SGF_ImageView imageViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0);
    SGF_ImageView imageArrayView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1);
    SGF_ImageView imageArrayView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1);
    SGF_ImageView imageArrayViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mipLevel = 0, uint32_t levelCount = 1, uint32_t arrayLayer = 0, uint32_t arraySize = 1);

    SGF_Memory allocate(const VkMemoryAllocateInfo& info);
    /**
     * @brief allocates device-memory for the given requirements.
     * 
     * @return allocated memory with the required memory-size
     */
    SGF_Memory allocate(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags flags);
    /**
     * @brief allocates device-memory for the given buffer and binds the buffer to it.
     * 
     * @return allocated memory with the required memory-size for the buffer
     */
    SGF_Memory allocate(VkBuffer buffer, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    /**
     * @brief allocates device-memory for all given buffers and binds them to it.
     * 
     * Buffers are bound in the order they are submitted to the function while respecting the required alignment for each buffer. 
     * 
     * @return allocated memory with the required memory-size for all buffers.
     */
    SGF_Memory allocate(const VkBuffer* pBuffers, uint32_t bufferCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    /**
     * @brief allocates device-memory for the given image and binds them.
     * 
     * @return allocated memory with required size for the image.
     */
    SGF_Memory allocate(VkImage image, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    /**
     * @brief allocates device-memory for all given images and binds them to it.
     * 
     * images are bound in the order they are submitted to the function while respecting the required alignment for each image. 
     * 
     * @return allocated memory with the required memory-size for all images.
     */
    SGF_Memory allocate(const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    /**
     * @brief allocates device-memory for all given buffers and images and binds them to it.
     * 
     * buffers and images are bound in the order they are submitted to the function while respecting the required alignment for each.
     * buffers are bound before images, because their required alignments are typically smaller than that for images - possibly saving memory.
     * 
     * @return allocated memory with the required memory-size for all buffers and images.
     */
    SGF_Memory allocate(const VkBuffer* pBuffers, uint32_t bufferCount, const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkSwapchainKHR swapchain(const VkSwapchainCreateInfoKHR& info);

    VkFramebuffer framebuffer(VkRenderPass renderPass, const VkImageView* pAttachments, uint32_t attachmentCount, uint32_t width, uint32_t height, uint32_t layerCount);

    VkRenderPass renderPass(const VkRenderPassCreateInfo& info);
    VkRenderPass renderPass(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependenfyCount);
    //VkSwapchainKHR swapchain(const SGF_Window window, VkPresentModeKHR presentMode);
    /**
     * @brief gets the first supported format for the requested feature and tiling from supplied candidates.
     * 
     * @return first supported candidate or VK_FORMAT_MAX_ENUM when none are supported.
     */
    VkFormat getSupportedFormat(const VkFormat* pCandidates, uint32_t candidateCount, VkFormatFeatureFlags features, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL);
private:
    uint32_t findMemoryIndex(uint32_t typeBits, VkMemoryPropertyFlags flags);
    void destroyType(VkFence fence);
    void destroyType(VkSemaphore semaphore);
    void destroyType(VkBuffer buffer);
    void destroyType(VkImage image);
    void destroyType(VkImageView imageView);
    void destroyType(VkFramebuffer framebuffer);
    void destroyType(VkRenderPass renderPass);
    void destroyType(VkPipeline pipeline);
    void destroyType(VkPipelineLayout pipelineLayout);
    void destroyType(VkDescriptorSetLayout descriptorSetLayout);
    void destroyType(VkDescriptorSet descriptorSet);
    void destroyType(VkDescriptorPool descriptorPool);
    void destroyType(VkDeviceMemory memory);
    void destroyType(VkCommandPool commandPool);
    void destroyType(VkSampler sampler);
public:
    template<typename T>
    inline void destroy(T type) { destroyType(type); }
    template<typename T, typename ...Args>
    inline void destroy(T type, Args... args) {
        destroyType(type);
        destroy(args...);
    }
private:
    void getQueueCreateInfos(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* pIndexCount, VkDeviceQueueCreateInfo* pQueueCreateInfos, float* pQueuePriorityBuffer);
    void pickPhysicalDevice(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
        const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, VkSurfaceKHR surface);
    void createLogicalDevice(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
        const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, VkSurfaceKHR surface);
    friend SGF_Window;
    friend Builder;
    VkDevice logical = VK_NULL_HANDLE;
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    uint32_t presentFamily = UINT32_MAX;
    uint32_t presentCount = 0;
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t graphicsCount = 0;
    uint32_t transferFamily = UINT32_MAX;
    uint32_t transferCount = 0;
    uint32_t computeFamily = UINT32_MAX;
    uint32_t computeCount = 0;
    uint64_t enabledFeatures = 0;
};
inline constexpr uint32_t DeviceSize = sizeof(SGF_Device);
inline constexpr uint32_t DeviceBuilderSize = sizeof(SGF_Device::Builder);

