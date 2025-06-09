#include <cstring>
#include <vector>
#include <volk.h>
#include "Render/Device.hpp"
#include "Events/Event.hpp"
#include "Events/InputEvents.hpp"
#include "Filesystem/File.hpp"

#include "Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace SGF {
    extern VkInstance VulkanInstance;
    extern VkAllocationCallbacks* VulkanAllocator;
#ifdef SGF_ENABLE_VALIDATION
	extern const char* VULKAN_MESSENGER_NAME;
    extern VkDebugUtilsMessengerEXT VulkanMessenger;
    extern VkDebugUtilsMessengerEXT createDebugUtilsMessengerEXT(const VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
#endif
#ifdef SGF_LOG_VULKAN_DEVICE_OBJECTS
    int32_t _TRACK_RENDER_PASS = 0;
#define TRACK_RENDER_PASS(COUNT) do { _TRACK_RENDER_PASS += COUNT;if (COUNT < 0) {SGF::debug("Destroying Renderpasses: ", _TRACK_RENDER_PASS);} else{SGF::debug("creating Renderpasses: ", _TRACK_RENDER_PASS);} } while(0)
    int32_t _TRACK_FENCE = 0;
#define TRACK_FENCE(COUNT) do{_TRACK_FENCE += COUNT; if (COUNT < 0){SGF::debug("destroying fences: ", _TRACK_FENCE);}else{SGF::debug("creating fences: ", _TRACK_FENCE);}  }while(0)
    int32_t _TRACK_SEMAPHORE = 0;
#define TRACK_SEMAPHORE(COUNT) do{ _TRACK_SEMAPHORE += COUNT;if(COUNT<0){SGF::debug("destroying semaphore: ", _TRACK_SEMAPHORE);}else{SGF::debug("creating semaphore: ", _TRACK_SEMAPHORE);} }while(0)
    int32_t _TRACK_BUFFER = 0;
#define TRACK_BUFFER(COUNT) do{ _TRACK_BUFFER += COUNT;if(COUNT<0){SGF::debug("destroying buffer: ", _TRACK_BUFFER);}else{SGF::debug("creating buffer: ", _TRACK_BUFFER);} }while(0)
    int32_t _TRACK_IMAGE = 0;
#define TRACK_IMAGE(COUNT) do{ _TRACK_IMAGE += COUNT;if(COUNT<0){SGF::debug("destroying image: ", _TRACK_IMAGE);}else{SGF::debug("creating image: ", _TRACK_IMAGE);} }while(0)
    int32_t _TRACK_IMAGE_VIEW = 0;
#define TRACK_IMAGE_VIEW(COUNT) do{ _TRACK_IMAGE_VIEW += COUNT;if(COUNT<0){SGF::debug("destroying image view: ", _TRACK_IMAGE_VIEW);}else{SGF::debug("creating image view: ", _TRACK_IMAGE_VIEW);} }while(0)
    int32_t _TRACK_DEVICE_MEMORY = 0;
#define TRACK_DEVICE_MEMORY(COUNT) do{ _TRACK_DEVICE_MEMORY += COUNT;if(COUNT<0){SGF::debug("destroying device memory: ", _TRACK_DEVICE_MEMORY);}else{SGF::debug("creating device memory: ", _TRACK_DEVICE_MEMORY);} }while(0)
    int32_t _TRACK_COMMAND_POOL = 0;
#define TRACK_COMMAND_POOL(COUNT) do{ _TRACK_COMMAND_POOL += COUNT;if(COUNT<0){SGF::debug("destroying command pool: ", _TRACK_COMMAND_POOL);}else{SGF::debug("creating command pool: ", _TRACK_COMMAND_POOL);} }while(0)
    int32_t _TRACK_DESCRIPTOR_POOL = 0;
#define TRACK_DESCRIPTOR_POOL(COUNT) do{ _TRACK_DESCRIPTOR_POOL += COUNT;if(COUNT<0){SGF::debug("destroying descriptor pool: ", _TRACK_DESCRIPTOR_POOL);}else{SGF::debug("creating descriptor pool: ", _TRACK_DESCRIPTOR_POOL);} }while(0)
    int32_t _TRACK_DESCRIPTOR_SET_LAYOUT = 0;
#define TRACK_DESCRIPTOR_SET_LAYOUT(COUNT) do{ _TRACK_DESCRIPTOR_SET_LAYOUT += COUNT;if(COUNT<0){SGF::debug("destroying descriptor set layout: ", _TRACK_DESCRIPTOR_SET_LAYOUT);}else{SGF::debug("creating descriptor set layout: ", _TRACK_DESCRIPTOR_SET_LAYOUT);} }while(0)
    int32_t _TRACK_PIPELINE_LAYOUT = 0;
#define TRACK_PIPELINE_LAYOUT(COUNT) do{ _TRACK_PIPELINE_LAYOUT += COUNT;if(COUNT<0){SGF::debug("destroying pipeline layout: ", _TRACK_PIPELINE_LAYOUT);}else{SGF::debug("creating pipeline layout: ", _TRACK_PIPELINE_LAYOUT);} }while(0)
    int32_t _TRACK_PIPELINE = 0;
#define TRACK_PIPELINE(COUNT) do{ _TRACK_PIPELINE += COUNT;if(COUNT<0){SGF::debug("destroying pipeline: ", _TRACK_PIPELINE);}else{SGF::debug("creating pipeline: ", _TRACK_PIPELINE);} }while(0)
    int32_t _TRACK_FRAMEBUFFER = 0;
#define TRACK_FRAMEBUFFER(COUNT) do{ _TRACK_FRAMEBUFFER += COUNT;if(COUNT<0){SGF::debug("destroying framebuffer: ", _TRACK_FRAMEBUFFER);}else{SGF::debug("creating framebuffer: ", _TRACK_FRAMEBUFFER);} }while(0)
    int32_t _TRACK_SWAPCHAIN = 0;
#define TRACK_SWAPCHAIN(COUNT) do{ _TRACK_SWAPCHAIN += COUNT;if(COUNT<0){SGF::debug("destroying swapchain: ", _TRACK_SWAPCHAIN);}else{SGF::debug("creating swapchain: ", _TRACK_SWAPCHAIN);} }while(0)
    int32_t _TRACK_SAMPLER = 0;
#define TRACK_SAMPLER(COUNT) do{ _TRACK_SAMPLER += COUNT;if(COUNT<0){SGF::debug("destroying sampler: ", _TRACK_SAMPLER);}else{SGF::debug("creating sampler: ", _TRACK_SAMPLER);} }while(0)
    int32_t _TRACK_SHADER_MODULE = 0;
#define TRACK_SHADER_MODULE(COUNT) do{ _TRACK_SHADER_MODULE += COUNT;if(COUNT<0){SGF::debug("destroying shader module: ", _TRACK_SHADER_MODULE);}else{SGF::debug("creating shader module: ", _TRACK_SHADER_MODULE);} }while(0)
#else
#define TRACK_RENDER_PASS(COUNT)
#define TRACK_FENCE(COUNT)
#define TRACK_SEMAPHORE(COUNT)
#define TRACK_BUFFER(COUNT)
#define TRACK_IMAGE(COUNT)
#define TRACK_IMAGE_VIEW(COUNT)
#define TRACK_DEVICE_MEMORY(COUNT)
#define TRACK_COMMAND_POOL(COUNT)
#define TRACK_DESCRIPTOR_POOL(COUNT)
#define TRACK_DESCRIPTOR_SET_LAYOUT(COUNT)
#define TRACK_PIPELINE_LAYOUT(COUNT)
#define TRACK_PIPELINE(COUNT)
#define TRACK_FRAMEBUFFER(COUNT)
#define TRACK_SWAPCHAIN(COUNT)
#define TRACK_SAMPLER(COUNT)
#define TRACK_SHADER_MODULE(COUNT)
#
#endif
    Device Device::s_Instance;
    DeviceRequirements Device::s_Requirements = {
        .extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
        .requiredFeatures = 0, 
        .optionalFeatures = 0,
        .graphicsQueueCount = 1, 
        .computeQueueCount = 0, 
        .transferQueueCount = 0
    };


    void Device::RequireFeatures(DeviceFeatureFlags requiredFeatures) {
        s_Requirements.requiredFeatures = requiredFeatures;
    }
    void Device::RequestFeatures(DeviceFeatureFlags optionalFeatures) {
        s_Requirements.optionalFeatures = optionalFeatures;
    }
    void Device::RequireExtensions(const char* const* pExtensionNames, uint32_t featureCount) {
        s_Requirements.extensions.clear();
        s_Requirements.extensions.resize(featureCount + 1);
        if (pExtensionNames != nullptr) {
            memcpy(s_Requirements.extensions.data(), pExtensionNames, sizeof(pExtensionNames[0]) * featureCount);
        }
        else {
            assert(featureCount == 0);
            s_Requirements.extensions.shrink_to_fit();
        }
        s_Requirements.extensions[featureCount] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    }

    void Device::RequireGraphicsQueues(uint32_t queueCount) {
        s_Requirements.graphicsQueueCount = queueCount;
    }
    void Device::RequireComputeQueues(uint32_t queueCount) {
        s_Requirements.computeQueueCount = queueCount;
    }
    void Device::RequireTransferQueues(uint32_t queueCount) {
        s_Requirements.transferQueueCount = queueCount;
    }
        
    void Device::PickNew() {
        s_Instance.createNew(s_Requirements);
    }
#pragma region HELPER_FUNCTIONS
    void createDefaultBufferInfo(VkBufferCreateInfo* pInfo, VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags) {
        pInfo->sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        pInfo->pNext = nullptr;
        pInfo->flags = flags;
        pInfo->usage = usage;
        pInfo->size = size;
        pInfo->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        pInfo->queueFamilyIndexCount = 0;
        pInfo->pQueueFamilyIndices = nullptr;
    }
    void createDefaultImageInfo(VkImageCreateInfo* pInfo,const VkExtent3D& extent, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, VkImageCreateFlags flags) {
        pInfo->pNext = nullptr;
        pInfo->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        pInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        pInfo->arrayLayers = arraySize;
        pInfo->mipLevels = mipLevelCount;
        pInfo->tiling = VK_IMAGE_TILING_OPTIMAL;
        pInfo->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        pInfo->pQueueFamilyIndices = nullptr;
        pInfo->queueFamilyIndexCount = 0;
        pInfo->samples = samples;
        pInfo->extent = extent;
        pInfo->format = format;
        pInfo->usage = usage;
        pInfo->flags = flags;
    }
    template<typename CREATE_INFO>
    void setupQueueFamilySharing(const Device* pDevice, CREATE_INFO* pInfo, uint32_t* pIndices, QueueFamilyFlags familyFlags) {
        pInfo->pQueueFamilyIndices = pIndices;
        if (familyFlags & QUEUE_FAMILY_GRAPHICS) {
            assert(pDevice->graphicsQueueCount() != 0);
            pIndices[pInfo->queueFamilyIndexCount] = pDevice->graphicsFamily();
            pInfo->queueFamilyIndexCount++;
        }
        if (familyFlags & QUEUE_FAMILY_COMPUTE) {
            assert(pDevice->computeQueueCount() != 0);
            pIndices[pInfo->queueFamilyIndexCount] = pDevice->computeFamily();
            pInfo->queueFamilyIndexCount++;
        }
        if (familyFlags & QUEUE_FAMILY_TRANSFER) {
            assert(pDevice->transferQueueCount() != 0);
            pIndices[pInfo->queueFamilyIndexCount] = pDevice->transferFamily();
            pInfo->queueFamilyIndexCount++;
        }
        if (familyFlags & QUEUE_FAMILY_PRESENT) {
            assert(pDevice->presentQueueCount() != 0);
            if (pDevice->presentFamily() != pDevice->graphicsFamily()) {
				pIndices[pInfo->queueFamilyIndexCount] = pDevice->presentFamily();
				pInfo->queueFamilyIndexCount++;
            }
        }
        if (pInfo->queueFamilyIndexCount > 1) {
            pInfo->sharingMode = VK_SHARING_MODE_CONCURRENT;
        }
    }
#pragma region DEVICE_CREATION
#pragma region PHYSICAL_DEVICE_HELPER 
    bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice physicalDevice, uint32_t extensionCount, const char* const* ppDeviceExtensions) {
        assert(physicalDevice != VK_NULL_HANDLE);
        uint32_t count;
        if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr) != VK_SUCCESS) {
            SGF::error("failed to enumerate extension properties for the device!");
            return false;
        }
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions.data());
        count = 0;
        for (size_t i = 0; i < extensions.size(); ++i) {
            uint32_t found = 0;
            for (uint32_t j = 0; j < extensionCount; ++j) {
                if (strcmp(ppDeviceExtensions[j], extensions[i].extensionName) == 0) {
                    count++;
                    if (found != 0) {
                        SGF::error("device extension: \"", ppDeviceExtensions[i], "\" is not allowed to be include twice in the extension list");
                        return false;
                    }
                    found++;
                }
            }
        }
        if (count != extensionCount) {
            SGF::warn("Extensions not supported!");
            return false;
        } else {
            return true;
        }
        return count == extensionCount;
    }

    bool checkPhysicalDeviceFeatureSupport(VkPhysicalDevice device, DeviceFeatureFlags flags) {
        assert(device != VK_NULL_HANDLE);
        if (flags == 0) {
            return true;
        }
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);
        uint32_t array_size = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
        const VkBool32* available = (VkBool32*)&features;
        for(uint32_t i = 0; i < array_size; ++i) {
            if ((BIT(i) & flags) && !available[i]) {
                return false;
            }
        }
        return true;
    }

    #define VK_CHECK_DEVICE_PROPERTIES(TYPE, MEMBER_TYPE, START, END, AVAILABLE, OPERATOR, REQUESTED) do {  \
        const MEMBER_TYPE* _available = (MEMBER_TYPE*)AVAILABLE;\
        const MEMBER_TYPE* _requested = (MEMBER_TYPE*)REQUESTED;\
        uint32_t _start = offsetof(TYPE, START)/sizeof(MEMBER_TYPE);\
        uint32_t _end = offsetof(TYPE,END)/sizeof(MEMBER_TYPE);\
        for (uint32_t i = _start; i < _end; ++i) {\
            if (_available[i] OPERATOR _requested[i]) {\
                return false;\
            }\
        }\
    } while(0)

    #define COMPARE_MEMBER(AVAIL, OPERATOR, REQ, MEMBER) (AVAIL.MEMBER OPERATOR REQ.MEMBER)
    #define BIT_COMPARE_MEMBER(AVAIL, REQ, MEMBER) ((AVAIL.MEMBER & REQ.MEMBER) != REQ.MEMBER)
    #define BOOL_COMPARE_MEMBER(AVAIL, REQ, MEMBER) (REQ.MEMBER && AVAIL.MEMBER != REQ.MEMBER)

    bool checkPhysicalDeviceRequiredLimits(VkPhysicalDevice device, const VkPhysicalDeviceLimits* minimalLimits) {
        assert(device != VK_NULL_HANDLE);
        if (minimalLimits == nullptr) {
            return true;
        }

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        static_assert(sizeof(float) == sizeof(uint32_t) && sizeof(size_t) == sizeof(VkDeviceSize));
        const VkPhysicalDeviceLimits& avail = properties.limits;
        const VkPhysicalDeviceLimits& req = *minimalLimits;

        VK_CHECK_DEVICE_PROPERTIES(VkPhysicalDeviceLimits, uint32_t, maxImageDimension1D, bufferImageGranularity, &properties.limits, <, minimalLimits);
        VK_CHECK_DEVICE_PROPERTIES(VkPhysicalDeviceLimits, uint32_t, maxBoundDescriptorSets, maxSamplerLodBias, &properties.limits, <, minimalLimits);
        VK_CHECK_DEVICE_PROPERTIES(VkPhysicalDeviceLimits, float, maxSamplerLodBias, maxViewports, &properties.limits, <, minimalLimits);
        VK_CHECK_DEVICE_PROPERTIES(VkPhysicalDeviceLimits, uint32_t, maxViewports, viewportBoundsRange, &properties.limits, <, minimalLimits);
        VK_CHECK_DEVICE_PROPERTIES(VkPhysicalDeviceLimits, size_t, minMemoryMapAlignment, minTexelOffset, &properties.limits, >, minimalLimits);
        if (COMPARE_MEMBER(avail, <, req, sparseAddressSpaceSize) ||
            COMPARE_MEMBER(avail, <, req, bufferImageGranularity) ||
            COMPARE_MEMBER(avail, >, req, viewportBoundsRange[0]) ||
            COMPARE_MEMBER(avail, <, req, viewportBoundsRange[1]) ||
            COMPARE_MEMBER(avail, <, req, viewportSubPixelBits) ||
            COMPARE_MEMBER(avail, >, req, minTexelOffset) ||
            COMPARE_MEMBER(avail, <, req, maxTexelOffset) ||
            COMPARE_MEMBER(avail, >, req, minTexelGatherOffset) ||
            COMPARE_MEMBER(avail, <, req, maxTexelGatherOffset) ||
            COMPARE_MEMBER(avail, >, req, minInterpolationOffset) ||
            COMPARE_MEMBER(avail, <, req, maxInterpolationOffset) ||
            COMPARE_MEMBER(avail, >, req, subPixelInterpolationOffsetBits) ||
            COMPARE_MEMBER(avail, <, req, maxFramebufferWidth) || 
            COMPARE_MEMBER(avail, <, req, maxFramebufferHeight) || 
            COMPARE_MEMBER(avail, <, req, maxFramebufferLayers) || 
            BIT_COMPARE_MEMBER(avail, req, framebufferColorSampleCounts) || 
            BIT_COMPARE_MEMBER(avail, req, framebufferDepthSampleCounts) || 
            BIT_COMPARE_MEMBER(avail, req, framebufferStencilSampleCounts) || 
            BIT_COMPARE_MEMBER(avail, req, framebufferNoAttachmentsSampleCounts) || 
            COMPARE_MEMBER(avail, <, req, maxColorAttachments) || 
            COMPARE_MEMBER(avail, <, req, sampledImageColorSampleCounts) || 
            COMPARE_MEMBER(avail, <, req, sampledImageDepthSampleCounts) || 
            COMPARE_MEMBER(avail, <, req, sampledImageStencilSampleCounts) || 
            COMPARE_MEMBER(avail, <, req, storageImageSampleCounts) || 
            COMPARE_MEMBER(avail, <, req, maxSampleMaskWords) || 
            BOOL_COMPARE_MEMBER(avail, req, timestampComputeAndGraphics) || 
            //COMPARE_MEMBER(avail, >, req, timestampPeriod) || 
            COMPARE_MEMBER(avail, <, req, maxClipDistances) || 
            COMPARE_MEMBER(avail, <, req, maxCullDistances) || 
            COMPARE_MEMBER(avail, <, req, maxCombinedClipAndCullDistances) || 
            COMPARE_MEMBER(avail, <, req, discreteQueuePriorities) || 
            COMPARE_MEMBER(avail, >, req, pointSizeRange[0]) || 
            COMPARE_MEMBER(avail, <, req, pointSizeRange[1]) || 
            COMPARE_MEMBER(avail, >, req, lineWidthRange[0]) || 
            COMPARE_MEMBER(avail, <, req, lineWidthRange[1]) || 
            COMPARE_MEMBER(avail, >, req, pointSizeGranularity) || 
            COMPARE_MEMBER(avail, >, req, lineWidthGranularity)) {
                SGF::warn("Device does not have required device limits");
            //COMPARE_MEMBER(avail, >=, req, strictLines) || 
            //COMPARE_MEMBER(avail, <, req, standardSampleLocations) || 
            //COMPARE_MEMBER(avail, >, req, optimalBufferCopyOffsetAlignment) || 
            //COMPARE_MEMBER(avail, >, req, optimalBufferCopyRowPitchAlignment) || 
            //COMPARE_MEMBER(avail, >, req, nonCoherentAtomSize)) {
            return false;
        }

        return true;
    }
    bool checkPhysicalDeviceSurfaceSupport(VkPhysicalDevice physicalDevice) {
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (glfwGetPhysicalDevicePresentationSupport(VulkanInstance, physicalDevice, i) == GLFW_TRUE) {
                return true;
            }
        }
        return false;
    }

    constexpr VkQueueFlags graphicsQueueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    constexpr VkQueueFlags computeQueueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    constexpr VkQueueFlags transferQueueFlags = VK_QUEUE_TRANSFER_BIT;

    bool checkPhysicalDeviceQueueSupport(VkPhysicalDevice device, uint32_t graphicsCount, uint32_t computeCount, uint32_t transferCount) {
        assert(device != VK_NULL_HANDLE);
        bool support = false;
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        std::vector<VkQueueFamilyProperties> properties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());
        bool hasGraphics = false;
        bool hasCompute = false;
        bool hasTransfer = false;
        bool hasPresent = false;
        for (uint32_t i = 0; i < count; ++i) {
            if (!hasPresent) {
                hasPresent = glfwGetPhysicalDevicePresentationSupport(VulkanInstance, device, i) == GLFW_TRUE;
            }
            if (!hasGraphics && (properties[i].queueFlags & graphicsQueueFlags) == graphicsQueueFlags && properties[i].queueCount >= graphicsCount) {
                hasGraphics = true;
                continue;
            }
            if (!hasCompute && (properties[i].queueFlags & computeQueueFlags) == computeQueueFlags && properties[i].queueCount >= computeCount) {
                hasCompute = true;
                continue;
            }
            if (!hasTransfer && (properties[i].queueFlags & transferQueueFlags) == transferQueueFlags && properties[i].queueCount >= computeCount) {
                hasTransfer = true;
                continue;
            }
        }
        if (!hasPresent || (!hasGraphics && graphicsCount != 0) || (!hasCompute && computeCount != 0) || (!hasTransfer && transferCount != 0)) {
            SGF::warn("Device is missing queue support!");
            return false;
        } else {
            return true;
        }
    }

    bool checkPhysicalDeviceRequirements(VkPhysicalDevice device, const DeviceRequirements& r) {
        assert(device != VK_NULL_HANDLE);
        return checkPhysicalDeviceExtensionSupport(device, r.extensions.size(), r.extensions.data()) && checkPhysicalDeviceFeatureSupport(device, r.requiredFeatures) &&
            //checkPhysicalDeviceRequiredLimits(device, minLimits) && 
            checkPhysicalDeviceSurfaceSupport(device) && 
            checkPhysicalDeviceQueueSupport(device, r.graphicsQueueCount, r.computeQueueCount, r.transferQueueCount);
    }

    DeviceFeatureFlags getEnabledFeatures(VkPhysicalDevice device, const DeviceRequirements& requirements, VkPhysicalDeviceFeatures* f) {
        constexpr size_t featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
        static_assert(featureCount < sizeof(DeviceFeatureFlags)*8);
        assert(device != VK_NULL_HANDLE);
        DeviceFeatureFlags enabledFeatures = 0;
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
        const VkBool32* supported = (VkBool32*)&supportedFeatures;
        VkBool32* ef = (VkBool32*)f;
        for (size_t i = 0; i < featureCount; ++i) {
            if (BIT(i) & requirements.requiredFeatures) {
                if (!supported[i]) {
                    fatal("device feature required but not supported!");
                } else {
                    enabledFeatures |= BIT(i);
                    ef[i] = VK_TRUE;
                }
            }
            else {
                ef[i] = VK_FALSE;
            }
            if ((BIT(i) & requirements.optionalFeatures) && supported[i]) {
                enabledFeatures |= BIT(i);
                ef[i] = VK_TRUE;
            }
        }
        return enabledFeatures;
    }

    #define CHECK_QUEUE(PROPERTIES, INDEX, QUEUE_FLAGS, QUEUE_COUNT, INFO, QUEUE_BUFFER, BUFFER_OFFSET, HAS_QUEUE) {\
        if (!HAS_QUEUE && QUEUE_COUNT != 0) {\
            if ((PROPERTIES[INDEX].queueFlags & QUEUE_FLAGS) == QUEUE_FLAGS && PROPERTIES[INDEX].queueCount >= QUEUE_COUNT) {\
                const float _priorityStepSize = 1.0f / (float)QUEUE_COUNT; \
                INFO.queueFamilyIndex = INDEX;\
                INFO.queueCount = QUEUE_COUNT; \
                INFO.pQueuePriorities = &QUEUE_BUFFER[BUFFER_OFFSET];\
                for (uint32_t k = 0; k < QUEUE_COUNT; ++k) {\
                    QUEUE_BUFFER[k + BUFFER_OFFSET] = std::clamp(1.0f - (float)k * _priorityStepSize, 0.0f, 1.0f);\
                }\
                BUFFER_OFFSET += QUEUE_COUNT;\
                HAS_QUEUE = true;\
                continue;\
            }\
        }\
    }

    void Device::getQueueCreateInfos(VkPhysicalDevice device, uint32_t* pIndexCount, VkDeviceQueueCreateInfo* pQueueCreateInfos, float* pQueuePriorityBuffer) {
        assert(device != VK_NULL_HANDLE);
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        std::vector<VkQueueFamilyProperties> properties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());

        bool hasGraphics = false;
        bool hasCompute = false;
        bool hasTransfer = false;
        bool hasPresent = false;
        VkDeviceQueueCreateInfo graphicsInfo{};
        graphicsInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        VkDeviceQueueCreateInfo computeInfo = graphicsInfo;
        VkDeviceQueueCreateInfo transferInfo = graphicsInfo;
        VkDeviceQueueCreateInfo presentInfo = graphicsInfo;
        uint32_t floatBufferOffset = 0;

        for (uint32_t i = 0; i < count; ++i) {
            bool this_present = false;
            // search present queue when surface available
            if (!hasPresent && glfwGetPhysicalDevicePresentationSupport(VulkanInstance, device, i)) {
                this_present = true;
                hasPresent = true;
            }
            CHECK_QUEUE(properties, i, graphicsQueueFlags, graphicsCount, graphicsInfo, pQueuePriorityBuffer, floatBufferOffset, hasGraphics);
            if (this_present) continue;
            CHECK_QUEUE(properties, i, computeQueueFlags, computeCount, computeInfo, pQueuePriorityBuffer, floatBufferOffset, hasCompute);
            CHECK_QUEUE(properties, i, transferQueueFlags, transferCount, transferInfo, pQueuePriorityBuffer, floatBufferOffset, hasTransfer);
        }
        uint32_t uniqueQueueIndexCount = 0;
        if (hasGraphics) { 
            graphicsFamilyIndex = graphicsInfo.queueFamilyIndex;
            pQueueCreateInfos[uniqueQueueIndexCount] = graphicsInfo;
            uniqueQueueIndexCount++; 
        }
        if (hasCompute) { 
            computeFamilyIndex = computeInfo.queueFamilyIndex;
            pQueueCreateInfos[uniqueQueueIndexCount] = computeInfo;
            uniqueQueueIndexCount++; 
        }
        if (hasTransfer) {
            transferFamilyIndex = transferInfo.queueFamilyIndex;
            pQueueCreateInfos[uniqueQueueIndexCount] = transferInfo;
            uniqueQueueIndexCount++; 
        }
        if (hasPresent) {
            presentFamilyIndex = presentInfo.queueFamilyIndex;
            presentCount = 1;
            if (presentInfo.queueFamilyIndex != graphicsInfo.queueFamilyIndex && presentInfo.queueFamilyIndex != computeInfo.queueFamilyIndex &&
                    presentInfo.queueFamilyIndex != transferInfo.queueFamilyIndex) {
                pQueueCreateInfos[uniqueQueueIndexCount] = presentInfo;
                uniqueQueueIndexCount++;
            }
        }
        *pIndexCount = uniqueQueueIndexCount;
    }
    #pragma endregion PHYSICAL_DEVICE_HELPER 

#pragma endregion HELPER_FUNCTIONS
    void Device::pickPhysicalDevice(const DeviceRequirements& r) {
        uint32_t count;
        vkEnumeratePhysicalDevices(SGF::VulkanInstance, &count, nullptr);
        std::vector<VkPhysicalDevice> physical_devices(count);
        vkEnumeratePhysicalDevices(SGF::VulkanInstance, &count, physical_devices.data());
        
        VkPhysicalDevice picked = VK_NULL_HANDLE;
        int32_t max_score = -1;
        for (size_t i = 0; i < physical_devices.size(); ++i) {
            int32_t score = 0;
            VkPhysicalDevice device = physical_devices[i];
            if (!checkPhysicalDeviceRequirements(device, r)) {
                continue;
            }
            SGF::debug("device supports minimal requirements!");
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            if (checkPhysicalDeviceFeatureSupport(device, r.optionalFeatures)) {
                score += 2000;
            }
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                score += 1000;
            }
            if (max_score < score) {
                picked = device;
                max_score = score;
                strncpy(name, properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
            }
        }
        if (picked == VK_NULL_HANDLE) {
            SGF::fatal(ERROR_FIND_PHYSICAL_DEVICE);
        }
        physical = picked;
    }

    void Device::createLogicalDevice(const DeviceRequirements& r) {
        VkDeviceCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.enabledExtensionCount = (uint32_t)r.extensions.size();
        info.ppEnabledExtensionNames = r.extensions.data();
        VkPhysicalDeviceFeatures enabled;
        uint32_t indexCount;
        VkDeviceQueueCreateInfo queueCreateInfos[4];
        std::vector<float> queuePriorityBuffer(graphicsCount + computeCount + transferCount + presentCount);
        getQueueCreateInfos(physical, &indexCount, queueCreateInfos, queuePriorityBuffer.data());
        info.pQueueCreateInfos = queueCreateInfos;
        info.queueCreateInfoCount = indexCount;
        enabledFeatures = getEnabledFeatures(physical, r, &enabled);
        info.pEnabledFeatures = &enabled;
    #ifdef SGF_ENABLE_VALIDATION
        info.enabledLayerCount = 1;
        info.ppEnabledLayerNames = &SGF::VULKAN_MESSENGER_NAME;
    #else
        info.enabledLayerCount = 0;
        info.ppEnabledLayerNames = nullptr;
    #endif
        if (vkCreateDevice(physical, &info, SGF::VulkanAllocator, &logical) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_LOGICAL_DEVICE);
        }
    #ifdef SGF_SINGLE_GPU
        volkLoadDevice(logical);
    #endif
        SGF::info("Logical device created!");
    }
        
    void Device::createNew(const DeviceRequirements& r) {
        shutdown();
        graphicsCount = r.graphicsQueueCount;
        computeCount = r.computeQueueCount;
        transferCount = r.transferQueueCount;
        presentCount = 1;
        pickPhysicalDevice(r);
        createLogicalDevice(r);
        // bind window
        //Device& d = (*this);
        //if (window != nullptr) {
            //(*window).bindDevice(d);
        //}
    }
    void Device::shutdown() {
        SGF::debug("device shutdown requested...");
        TRACK_RENDER_PASS(0);
        TRACK_FENCE(0);
        TRACK_SEMAPHORE(0);
		TRACK_BUFFER(0);
        TRACK_IMAGE(0);
        TRACK_IMAGE_VIEW(0);
        TRACK_DEVICE_MEMORY(0);
        TRACK_COMMAND_POOL(0);
        TRACK_DESCRIPTOR_POOL(0);
        TRACK_DESCRIPTOR_SET_LAYOUT(0);
        TRACK_PIPELINE_LAYOUT(0);
        TRACK_PIPELINE(0);
        TRACK_FRAMEBUFFER(0);
        TRACK_SWAPCHAIN(0);
        TRACK_SAMPLER(0);
        TRACK_SHADER_MODULE(0);

        if (physical != VK_NULL_HANDLE) {
            SGF::debug("destroying device...");
            waitIdle();
            {
				DeviceDestroyEvent event(*this);
				EventManager::dispatch(event);
            }
            vkDestroyDevice(logical, SGF::VulkanAllocator);
            logical = VK_NULL_HANDLE;
            physical = VK_NULL_HANDLE;
            presentFamilyIndex = UINT32_MAX;
            presentCount = 0;
            graphicsFamilyIndex = UINT32_MAX;
            graphicsCount = 0;
            transferFamilyIndex = UINT32_MAX;
            transferCount = 0;
            computeFamilyIndex = UINT32_MAX;
            computeCount = 0;
        }
    }
    
    Device::Device(Device&& other) noexcept {
        SGF::info("Moved device!");
        logical = other.logical;
        physical = other.physical;
        presentFamilyIndex = other.presentFamilyIndex;
        presentCount = other.presentCount;
        graphicsFamilyIndex = other.graphicsFamilyIndex;
        graphicsCount = other.graphicsCount;
        transferFamilyIndex = other.transferFamilyIndex;
        transferCount = other.transferCount;
        computeFamilyIndex = other.computeFamilyIndex;
        computeCount = other.computeCount;
        other.logical = VK_NULL_HANDLE;
        other.physical = VK_NULL_HANDLE;
        other.presentFamilyIndex = UINT32_MAX;
        other.presentCount = 0;
        other.graphicsFamilyIndex = UINT32_MAX;
        other.graphicsCount = 0;
        other.transferFamilyIndex = UINT32_MAX;
        other.transferCount = 0;
        other.computeFamilyIndex = UINT32_MAX;
        other.computeCount = 0;
    }
#pragma endregion DEVICE_CREATION

#pragma region DEVICE_BUILDER

	constexpr VkPhysicalDeviceLimits DEFAULT_DEVICE_LIMITS {
		0U,     //uint32_t              maxImageDimension1D;
		0U,     //uint32_t              maxImageDimension2D;
		0U,     //uint32_t              maxImageDimension3D;
		0U,     //uint32_t              maxImageDimensionCube;
		0U,     //uint32_t              maxImageArrayLayers;
		0U,     //uint32_t              maxTexelBufferElements;
		0U,     //uint32_t              maxUniformBufferRange;
		0U,     //uint32_t              maxStorageBufferRange;
		0U,     //uint32_t              maxPushConstantsSize;
		0U,     //uint32_t              maxMemoryAllocationCount;
		0U,     //uint32_t              maxSamplerAllocationCount;
		0U,     //VkDeviceSize          bufferImageGranularity;
		0U,     //VkDeviceSize          sparseAddressSpaceSize;
		0U,     //uint32_t              maxBoundDescriptorSets;
		0U,     //uint32_t              maxPerStageDescriptorSamplers;
		0U,     //uint32_t              maxPerStageDescriptorUniformBuffers;
		0U,     //uint32_t              maxPerStageDescriptorStorageBuffers;
		0U,     //uint32_t              maxPerStageDescriptorSampledImages;
		0U,     //uint32_t              maxPerStageDescriptorStorageImages;
		0U,     //uint32_t              maxPerStageDescriptorInputAttachments;
		0U,     //uint32_t              maxPerStageResources;
		0U,     //uint32_t              maxDescriptorSetSamplers;
		0U,     //uint32_t              maxDescriptorSetUniformBuffers;
		0U,     //uint32_t              maxDescriptorSetUniformBuffersDynamic;
		0U,     //uint32_t              maxDescriptorSetStorageBuffers;
		0U,     //uint32_t              maxDescriptorSetStorageBuffersDynamic;
		0U,     //uint32_t              maxDescriptorSetSampledImages;
		0U,     //uint32_t              maxDescriptorSetStorageImages;
		0U,     //uint32_t              maxDescriptorSetInputAttachments;
		0U,     //uint32_t              maxVertexInputAttributes;
		0U,     //uint32_t              maxVertexInputBindings;
		0U,     //uint32_t              maxVertexInputAttributeOffset;
		0U,     //uint32_t              maxVertexInputBindingStride;
		0U,     //uint32_t              maxVertexOutputComponents;
		0U,     //uint32_t              maxTessellationGenerationLevel;
		0U,     //uint32_t              maxTessellationPatchSize;
		0U,     //uint32_t              maxTessellationControlPerVertexInputComponents;
		0U,     //uint32_t              maxTessellationControlPerVertexOutputComponents;
		0U,     //uint32_t              maxTessellationControlPerPatchOutputComponents;
		0U,     //uint32_t              maxTessellationControlTotalOutputComponents;
		0U,     //uint32_t              maxTessellationEvaluationInputComponents;
		0U,     //uint32_t              maxTessellationEvaluationOutputComponents;
		0U,     //uint32_t              maxGeometryShaderInvocations;
		0U,     //uint32_t              maxGeometryInputComponents;
		0U,     //uint32_t              maxGeometryOutputComponents;
		0U,     //uint32_t              maxGeometryOutputVertices;
		0U,     //uint32_t              maxGeometryTotalOutputComponents;
		0U,     //uint32_t              maxFragmentInputComponents;
		0U,     //uint32_t              maxFragmentOutputAttachments;
		0U,     //uint32_t              maxFragmentDualSrcAttachments;
		0U,     //uint32_t              maxFragmentCombinedOutputResources;
		0U,     //uint32_t              maxComputeSharedMemorySize;
		{},     //uint32_t              maxComputeWorkGroupCount[3];
		0U,     //uint32_t              maxComputeWorkGroupInvocations;
		{},     //uint32_t              maxComputeWorkGroupSize[3];
		0U,     //uint32_t              subPixelPrecisionBits;
		0U,     //uint32_t              subTexelPrecisionBits;
		0U,     //uint32_t              mipmapPrecisionBits;
		0U,     //uint32_t              maxDrawIndexedIndexValue;
		0U,     //uint32_t              maxDrawIndirectCount;
		0.f,    //float                 maxSamplerLodBias;
		0.f,    //float                 maxSamplerAnisotropy;
		0U,     //uint32_t              maxViewports;
		{0, 0},     //uint32_t              maxViewportDimensions[2];
		{std::numeric_limits<float>::infinity(), 
		-std::numeric_limits<float>::infinity()},     //float viewportBoundsRange[2];
		0U,     //uint32_t              viewportSubPixelBits;
		0U,     //size_t                minMemoryMapAlignment;
		0U,     //VkDeviceSize          minTexelBufferOffsetAlignment;
		0U,     //VkDeviceSize          minUniformBufferOffsetAlignment;
		0U,     //VkDeviceSize          minStorageBufferOffsetAlignment;
		INT32_MAX,     //int32_t               minTexelOffset;
		0U,     //uint32_t              maxTexelOffset;
		INT32_MAX,     //int32_t               minTexelGatherOffset;
		0U,     //uint32_t              maxTexelGatherOffset;
		std::numeric_limits<float>::infinity(),     //float                 minInterpolationOffset;
		-std::numeric_limits<float>::infinity(),     //float                 maxInterpolationOffset;
		UINT32_MAX,     //uint32_t              subPixelInterpolationOffsetBits;
		0U,     //uint32_t              maxFramebufferWidth;
		0U,     //uint32_t              maxFramebufferHeight;
		0U,     //uint32_t              maxFramebufferLayers;
		0U,     //VkSampleCountFlagBits    framebufferColorSampleCounts;
		0U,     //VkSampleCountFlagBits    framebufferDepthSampleCounts;
		0U,     //VkSampleCountFlagBits    framebufferStencilSampleCounts;
		0U,     //VkSampleCountFlagBits    framebufferNoAttachmentsSampleCounts;
		0U,     //uint32_t              maxColorAttachments;
		0U,     //VkSampleCountFlagBits    sampledImageColorSampleCounts;
		0U,     //VkSampleCountFlagBits    sampledImageIntegerSampleCounts;
		0U,     //VkSampleCountFlagBits    sampledImageDepthSampleCounts;
		0U,     //VkSampleCountFlagBits    sampledImageStencilSampleCounts;
		0U,     //VkSampleCountFlagBits    storageImageSampleCounts;
		0U,     //uint32_t              maxSampleMaskWords;
		VK_FALSE,     //VkBool32              timestampComputeAndGraphics;
		0.f,     //float                 timestampPeriod;
		0U,     //uint32_t              maxClipDistances;
		0U,     //uint32_t              maxCullDistances;
		0U,     //uint32_t              maxCombinedClipAndCullDistances;
		0U,     //uint32_t              discreteQueuePriorities;
		{std::numeric_limits<float>::infinity(), 
		-std::numeric_limits<float>::infinity()},     //float                 pointSizeRange[2];
		{std::numeric_limits<float>::infinity(), 
		-std::numeric_limits<float>::infinity()},     //float                 lineWidthRange[2];
		std::numeric_limits<float>::infinity(),     //float                 pointSizeGranularity;
		std::numeric_limits<float>::infinity(),     //float                 lineWidthGranularity;
		VK_FALSE,     //VkBool32              strictLines;
		VK_FALSE,     //VkBool32              standardSampleLocations;
		0U,     //VkDeviceSize          optimalBufferCopyOffsetAlignment;
		0U,     //VkDeviceSize          optimalBufferCopyRowPitchAlignment;
		0U     //VkDeviceSize          nonCoherentAtomSize;
	};

#pragma endregion DEVICE_BUILDER

#pragma region DEVICE_USER_FUNCTIONS
    void Device::waitIdle() const {
        if (vkDeviceWaitIdle(logical) != VK_SUCCESS) {
            fatal(ERROR_DEVICE_WAIT_IDLE);
        }
    }
    void Device::waitFence(VkFence fence) const {
        assert(fence != VK_NULL_HANDLE);
        if (vkWaitForFences(logical, 1, &fence, VK_TRUE, UINT32_MAX) != VK_SUCCESS) {
            fatal(ERROR_WAIT_FENCE);
        }
    }
    void Device::waitFences(const VkFence* pFences, uint32_t count) const {
        assert(pFences != nullptr && count != 0);
        if (vkWaitForFences(logical, count, pFences, VK_TRUE, UINT32_MAX) != VK_SUCCESS) {
            fatal(ERROR_WAIT_FENCE);
        }
    }
    void Device::reset(const VkFence* pFences, uint32_t count) const {
        assert(pFences != nullptr && count != 0);
		if (vkResetFences(logical, count, pFences) != VK_SUCCESS) {
			fatal(ERROR_RESET_FENCE);
		}
	}


#pragma region DEVICE_MEMORY
    uint32_t Device::findMemoryIndex(uint32_t typeBits, VkMemoryPropertyFlags flags) const {
        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties(physical, &mem_properties);

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
            if ((typeBits & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & flags) == flags) {
                return i;
            }
        }
        SGF::error(ERROR_UNSUPPORTED_MEMORY_TYPE);
        return UINT32_MAX;
    }
    VkSurfaceFormatKHR Device::pickSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat) const {
        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
        if (format_count != 0) {
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &format_count, surface_formats.data());
        }
        else {
            SGF::fatal(ERROR_DEVICE_NO_SURFACE_SUPPORT);
        }
        for (const auto& available_format : surface_formats) {
            if (available_format.format == surfaceFormat.format && available_format.colorSpace == surfaceFormat.colorSpace)
            {
                return available_format;
            }
        }
        return surface_formats[0];
    }
    VkPresentModeKHR Device::pickPresentMode(VkSurfaceKHR surface, VkPresentModeKHR requested) const {
		uint32_t presentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &presentCount, nullptr);
		std::vector<VkPresentModeKHR> available(presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface, &presentCount, available.data());
		for (const auto& mode : available) {
			if (mode == requested) {
				return requested;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}
    VkSampleCountFlagBits Device::getMaxSupportedSampleCount() const {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physical, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
		return VK_SAMPLE_COUNT_1_BIT;
	}
    VkDeviceMemory Device::allocate(const VkMemoryAllocateInfo& info) const {
        VkDeviceMemory mem;
        if (vkAllocateMemory(logical, &info, SGF::VulkanAllocator, &mem) != VK_SUCCESS) {
            SGF::fatal(ERROR_DEVICE_MEM_ALLOCATION);
        }
        SGF::info("Allocated device Memory with size: ", info.allocationSize);
        TRACK_DEVICE_MEMORY(1);
        return mem;
    }

    VkDeviceMemory Device::allocate(const VkMemoryRequirements& req, VkMemoryPropertyFlags flags) const {
        VkMemoryAllocateInfo info;
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.pNext = nullptr;
        info.allocationSize = req.size;
        info.memoryTypeIndex = findMemoryIndex(req.memoryTypeBits, flags);
        return allocate(info);
    }

    VkDeviceMemory Device::allocate(VkBuffer buffer, VkMemoryPropertyFlags flags) const {
        assert(buffer != VK_NULL_HANDLE);
        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(logical, buffer, &req);
        VkDeviceMemory mem = allocate(req, flags);
        if (vkBindBufferMemory(logical, buffer, mem, 0) != VK_SUCCESS) {
            SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
        }
        return mem;
    }
    VkDeviceSize calcOffset(const VkMemoryRequirements& prevReq, const VkMemoryRequirements& objReq) {
        if (prevReq.size % objReq.alignment == 0) {
            return prevReq.size;
        } else {
            return prevReq.size + (objReq.alignment - (prevReq.size % objReq.alignment));
        }
    }
    VkDeviceMemory Device::allocate(const VkBuffer* pBuffers, uint32_t bufferCount, VkMemoryPropertyFlags flags) const {
        assert(bufferCount != 0);
        assert(pBuffers != nullptr);
        VkMemoryRequirements req = {};
        VkMemoryRequirements bufReq;
        VkDeviceSize* offsets = new VkDeviceSize[bufferCount];
        for (uint32_t i = 0; i < bufferCount; ++i) {
            vkGetBufferMemoryRequirements(logical, pBuffers[i], &bufReq);
            req.memoryTypeBits |= bufReq.memoryTypeBits;
            req.alignment = (bufReq.alignment < req.alignment ? req.alignment : bufReq.alignment);
            offsets[i] = calcOffset(req, bufReq);
            req.size = offsets[i] + bufReq.size;
            assert(offsets[i] % bufReq.alignment == 0);
        }
        req.size += ((req.alignment - req.size % req.alignment) ^ req.alignment);
        assert(req.size % req.alignment == 0);
        VkDeviceMemory mem = allocate(req, flags);
        for (uint32_t i = 0; i < bufferCount; ++i) {
            if (vkBindBufferMemory(logical, pBuffers[i], mem, offsets[i]) != VK_SUCCESS) {
                SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
            }
        }
        delete[] offsets;
        return mem;
    }
    VkDeviceMemory Device::allocate(VkImage image, VkMemoryPropertyFlags flags) const {
        assert(image != VK_NULL_HANDLE);
        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(logical, image, &req);
        VkDeviceMemory mem = allocate(req, flags);
        if (vkBindImageMemory(logical, image, mem, 0) != VK_SUCCESS) {
            SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
        }
        return mem;
    }
    VkDeviceMemory Device::allocate(const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags) const {
        assert(imageCount != 0);
        assert(pImages != nullptr);
        VkMemoryRequirements req = {};
        VkMemoryRequirements imageReq;
        VkDeviceSize* offsets = new VkDeviceSize[imageCount];
        for (uint32_t i = 0; i < imageCount; ++i) {
            vkGetImageMemoryRequirements(logical, pImages[i], &imageReq);
            req.memoryTypeBits |= imageReq.memoryTypeBits;
            req.alignment = (imageReq.alignment < req.alignment ? req.alignment : imageReq.alignment);
            //VkDeviceSize offset = ((imageReq.alignment - req.size % imageReq.alignment) ^ imageReq.alignment) + req.size;
            offsets[i] = calcOffset(req, imageReq);
            req.size = offsets[i] + imageReq.size;
            assert(offsets[i] % imageReq.alignment == 0);
        }
        req.size += ((req.alignment - req.size % req.alignment) ^ req.alignment);
        assert(req.size % req.alignment == 0);
        VkDeviceMemory mem = allocate(req, flags);
        for (uint32_t i = 0; i < imageCount; ++i) {
            if (vkBindImageMemory(logical, pImages[i], mem, offsets[i]) != VK_SUCCESS) {
                SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
            }
        }
        delete[] offsets;
        return mem;
    }
    
    VkDeviceMemory Device::allocate(const VkBuffer* pBuffers, uint32_t bufferCount, const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags) const {
        assert(imageCount != 0);
        assert(pImages != nullptr);
        assert(bufferCount != 0);
        assert(pBuffers != nullptr);
        VkMemoryRequirements req = {};
        VkMemoryRequirements memReq;
        VkDeviceSize* offsets = new VkDeviceSize[bufferCount+imageCount];
        for (uint32_t i = 0; i < bufferCount; ++i) {
            vkGetBufferMemoryRequirements(logical, pBuffers[i], &memReq);
            req.memoryTypeBits |= memReq.memoryTypeBits;
            req.alignment = (memReq.alignment < req.alignment ? req.alignment : memReq.alignment);
            offsets[i] = calcOffset(req, memReq);
            //offsets[i] = ((memReq.alignment - req.size % memReq.alignment) ^ memReq.alignment) + req.size;
            req.size = offsets[i] + memReq.size;
            //req.size += offsets[i] + memReq.size;
            assert(offsets[i] % memReq.alignment == 0);
        }
        for (uint32_t i = bufferCount; i < imageCount + bufferCount; ++i) {
            vkGetImageMemoryRequirements(logical, pImages[i], &memReq);
            req.memoryTypeBits |= memReq.memoryTypeBits;
            req.alignment = (memReq.alignment < req.alignment ? req.alignment : memReq.alignment);
            //offsets[i] = ((memReq.alignment - req.size % memReq.alignment) ^ memReq.alignment) + req.size;
            offsets[i] = calcOffset(req, memReq);
            //req.size += (req.size % req.alignment) + memReq.size;
            req.size = offsets[i] + memReq.size;
            assert(offsets[i] % memReq.alignment == 0);
        }
        req.size += ((req.alignment - req.size % req.alignment) ^ req.alignment);
        assert(req.size % req.alignment == 0);
        VkDeviceMemory mem = allocate(req, flags);
        for (uint32_t i = 0; i < bufferCount; ++i) {
            if (vkBindBufferMemory(logical, pBuffers[i], mem, offsets[i]) != VK_SUCCESS) {
                SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
            }
        }
        for (uint32_t i = bufferCount; i < imageCount + bufferCount; ++i) {
            if (vkBindImageMemory(logical, pImages[i], mem, offsets[i]) != VK_SUCCESS) {
                SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
            }
        }
        delete[] offsets;
        return mem;
    }
#pragma endregion DEVICE_MEMORY

    VkBuffer Device::buffer(const VkBufferCreateInfo& info) const {
        VkBuffer buffer;
        if (vkCreateBuffer(logical, &info, SGF::VulkanAllocator, &buffer) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_BUFFER);
        }
        TRACK_BUFFER(1);
        return buffer;
    }
    VkBuffer Device::buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags) const {
        VkBufferCreateInfo info;
        createDefaultBufferInfo(&info, size, usage, flags);
        return buffer(info);
    }
    VkBuffer Device::bufferShared(VkDeviceSize size, VkBufferUsageFlags usage, QueueFamilyFlags familyFlags, VkBufferCreateFlags flags) const {
        uint32_t indices[4] = {};
        VkBufferCreateInfo info;
        createDefaultBufferInfo(&info, size, usage, flags);
        setupQueueFamilySharing(this, &info, indices, familyFlags);
        return buffer(info);
    }


#pragma region IMAGE
    VkImage Device::image(const VkImageCreateInfo& info) const {
        assert(info.extent.width != 0);
        assert(info.extent.height != 0);
        assert(info.extent.depth != 0);
        assert(info.sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
        VkImage image;
        if (vkCreateImage(logical, &info, SGF::VulkanAllocator, &image) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_IMAGE);
        }
        TRACK_IMAGE(1);
        return image;
    }
    VkImage Device::image1D(uint32_t length, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, {length, 1, 1}, 1, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_1D;
        return image(info);
    }
    VkImage Device::image2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, {width, height, 1}, 1, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_2D;
        return image(info);
    }
    VkImage Device::image3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, {width, height, depth}, 1, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_3D;
        return image(info);
    }
    VkImage Device::imageArray1D(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, {length, 1, 1}, arraySize, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_1D;
        return image(info);
    }
    VkImage Device::imageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, VkImageCreateFlags flags) const {
        VkImageCreateInfo info{};
        createDefaultImageInfo(&info, {width, height, 1}, arraySize, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_2D;
        return image(info);
    }
    VkImage Device::imageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, VkImageCreateFlags flags) const {
        VkImageCreateInfo info{};
        createDefaultImageInfo(&info, { width, height, depth }, arraySize, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_3D;
        return image(info);
    }
    VkImage Device::image1DShared(uint32_t length, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { length, 1, 1 }, 1, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_1D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    VkImage Device::image2DShared(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        assert(graphicsCount != 0);
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { width, height, 1 }, 1, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_2D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    VkImage Device::image3DShared(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        assert(graphicsCount != 0);
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { width, height, depth }, 1, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_3D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    VkImage Device::imageArray1DShared(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { length, 1, 1 }, arraySize, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_1D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    VkImage Device::imageArray2DShared(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { width, height, 1 }, arraySize, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_2D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    VkImage Device::imageArray3DShared(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, uint32_t mipLevelCount, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { width, height, depth }, arraySize, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_3D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }

#define IMAGE_VIEW_CREATE_INFO(IMAGE,TYPE,FORMAT,ASPECT,MIP,LEVELS,BASE,COUNT) {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,nullptr,0, IMAGE,TYPE,FORMAT,\
{VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY},\
{ASPECT,MIP,LEVELS,BASE,COUNT}}

    VkImageView Device::imageView(const VkImageViewCreateInfo& info) const {
        VkImageView view;
        assert(info.sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
        if (vkCreateImageView(logical, &info, SGF::VulkanAllocator, &view) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_IMAGE_VIEW);
        }
        TRACK_IMAGE_VIEW(1);
        return view;
    }
    VkImageView Device::imageView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_1D, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
        return imageView(info);
    }

    VkImageView Device::imageView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_2D, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
        return imageView(info);
    }

    VkImageView Device::imageView3D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_3D, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
        return imageView(info);
    }

    VkImageView Device::imageViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_CUBE, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
        return imageView(info);
    }
    VkImageView Device::imageArrayView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer, uint32_t arraySize) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_1D_ARRAY, format, imageAspect, mipLevel, levelCount, arrayLayer, arraySize);
        return imageView(info);
    }

    VkImageView Device::imageArrayView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer, uint32_t arraySize) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_2D_ARRAY, format, imageAspect, mipLevel, levelCount, arrayLayer, arraySize);
        return imageView(info);
    }
    VkImageView Device::imageArrayViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer, uint32_t arraySize) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, format, imageAspect, mipLevel, levelCount, arrayLayer, arraySize);
        return imageView(info);
    }
#pragma endregion IMAGE_CREATION

    VkSampler Device::imageSampler(const VkSamplerCreateInfo& info) const {
        VkSampler sampler;
        if (vkCreateSampler(logical, &info, VulkanAllocator, &sampler) != VK_SUCCESS) {
            fatal(ERROR_CREATE_SAMPLER);
        }
        return sampler;
    }

    VkSampler Device::imageSampler(VkFilter filterType, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, float mipLodBias, VkBool32 anisotropyEnable, 
        float maxAnisotropy, VkBool32 compareEnable, VkCompareOp compareOp, float minLod, float maxLod, VkBorderColor borderColor, VkBool32 unnormalizedCoordinates, VkSamplerCreateFlags flags, const void* pNext) const {
        VkSamplerCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.pNext = pNext;
        info.flags = flags;
		info.magFilter = filterType;
		info.minFilter = filterType;
		info.mipmapMode = mipmapMode;
		info.addressModeU = addressMode;
		info.addressModeV = addressMode;
		info.addressModeW = addressMode;
		info.mipLodBias = mipLodBias;
		info.anisotropyEnable = anisotropyEnable;
		info.maxAnisotropy = maxAnisotropy;
		info.compareEnable = compareEnable;
		info.compareOp = compareOp;
		info.minLod = minLod;
		info.maxLod = maxLod;
		info.borderColor = borderColor;
		info.unnormalizedCoordinates = unnormalizedCoordinates;
        return imageSampler(info);
    }


    VkQueue Device::graphicsQueue(uint32_t index) const {
        assert(graphicsCount != 0);
        assert(graphicsFamilyIndex != UINT32_MAX);
        VkQueue queue;
        vkGetDeviceQueue(logical, graphicsFamilyIndex, index, &queue);
        return queue;
    }
    VkQueue Device::computeQueue(uint32_t index) const {
        assert(computeCount != 0);
        assert(computeFamilyIndex != UINT32_MAX);
        VkQueue queue;
        vkGetDeviceQueue(logical, computeFamilyIndex, index, &queue);
        return queue;
    }
    VkQueue Device::transferQueue(uint32_t index) const {
        assert(transferCount != 0);
        assert(transferFamilyIndex != UINT32_MAX);
        VkQueue queue;
        vkGetDeviceQueue(logical, transferFamilyIndex, index, &queue);
        return queue;
    }
    VkQueue Device::presentQueue() const {
        assert(presentCount != 0);
        assert(presentFamilyIndex != UINT32_MAX);
        VkQueue queue;
        vkGetDeviceQueue(logical, graphicsFamilyIndex, 0, &queue);
        return queue;
    }

    VkFence Device::fence() const {
        assert(logical != VK_NULL_HANDLE);
        VkFenceCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        VkFence fence_r;
        if (vkCreateFence(logical, &info, VulkanAllocator, &fence_r) != VK_SUCCESS) {
            fatal(ERROR_CREATE_FENCE);
        }
        TRACK_FENCE(1);
        return fence_r;
    }

    VkFence Device::fenceSignaled() const {
        assert(logical != VK_NULL_HANDLE);
        VkFenceCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkFence fence_r;
        if (vkCreateFence(logical, &info, VulkanAllocator, &fence_r) != VK_SUCCESS) {
            fatal(ERROR_CREATE_FENCE);
        }
        TRACK_FENCE(1);
        return fence_r;
    }
    VkSemaphore Device::semaphore() const {
        VkSemaphoreCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        VkSemaphore sem;
        if (vkCreateSemaphore(logical, &info, VulkanAllocator, &sem) != VK_SUCCESS) {
            fatal(ERROR_CREATE_SEMAPHORE);
        }
        TRACK_SEMAPHORE(1);
        return sem;
    }
    /*
    void Device::signalSemaphore(VkSemaphore semaphore, uint64_t value) const {
        VkSemaphoreSignalInfo signalInfo{};
        signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signalInfo.semaphore = semaphore;
        signalInfo.value = 0;
        signalInfo.pNext = nullptr;
        if (vkSignalSemaphore(logical, &signalInfo) != VK_SUCCESS) {
            fatal("failed to signal semaphore");
        }
    }*/

    VkShaderModule Device::shaderModule(const char* filename) const {
        const auto& code = LoadBinaryFile(filename);
        VkShaderModuleCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = FLAG_NONE;
        info.codeSize = code.size();
        info.pCode = (uint32_t*)code.data();
        return shaderModule(info);
    }
    VkShaderModule Device::shaderModule(const VkShaderModuleCreateInfo& info) const {
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(logical, &info, VulkanAllocator, &shaderModule) != VK_SUCCESS) {
            fatal(ERROR_CREATE_SHADER_MODULE);
        }
        TRACK_SHADER_MODULE(1);
        return shaderModule;
    }

    VkPipelineLayout Device::pipelineLayout(const VkPipelineLayoutCreateInfo& info) const {
        VkPipelineLayout layout;
        if (vkCreatePipelineLayout(logical, &info, VulkanAllocator, &layout) != VK_SUCCESS) {
            fatal(ERROR_CREATE_PIPELINE_LAYOUT);
        }
        TRACK_PIPELINE_LAYOUT(1);
        return layout;
    }
    VkPipelineLayout Device::pipelineLayout(const VkDescriptorSetLayout* pLayouts, uint32_t descriptorLayoutCount, const VkPushConstantRange* pPushConstantRanges, uint32_t pushConstantCount) const {
        VkPipelineLayoutCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = FLAG_NONE;
        info.setLayoutCount = descriptorLayoutCount;
        info.pSetLayouts = pLayouts;
        info.pushConstantRangeCount = pushConstantCount;
        info.pPushConstantRanges = pPushConstantRanges;
        return pipelineLayout(info);
    }
    VkPipeline Device::pipeline(const VkGraphicsPipelineCreateInfo& info) const {
        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(logical, VK_NULL_HANDLE, 1, &info, VulkanAllocator, &pipeline) != VK_SUCCESS) {
            fatal(ERROR_CREATE_RENDER_PIPELINE);
        }
        TRACK_PIPELINE(1);
        return pipeline;
    }

    VkPipeline Device::pipeline(const VkComputePipelineCreateInfo& info) const {
        VkPipeline pipeline;
        if (vkCreateComputePipelines(logical, VK_NULL_HANDLE, 1, &info, VulkanAllocator, &pipeline) != VK_SUCCESS) {
            fatal(ERROR_CREATE_COMPUTE_PIPELINE);
        }
        TRACK_PIPELINE(1);
        return pipeline;
    }

    VkFramebuffer Device::framebuffer(const VkFramebufferCreateInfo& info) const {
        VkFramebuffer framebuffer;
        if (vkCreateFramebuffer(logical, &info, SGF::VulkanAllocator, &framebuffer) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_FRAMEBUFFER);
        }
        TRACK_FRAMEBUFFER(1);
        return framebuffer;
    }
    VkFramebuffer Device::framebuffer(VkRenderPass renderPass, const VkImageView* pAttachments, uint32_t attachmentCount, uint32_t width, uint32_t height, uint32_t layerCount) const {
        VkFramebufferCreateInfo info{};
        info.attachmentCount = attachmentCount;
        info.renderPass = renderPass;
        info.pAttachments = pAttachments;
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.width = width;
        info.height = height;
        info.layers = layerCount;
        return framebuffer(info);
    }

    VkRenderPass Device::renderPass(const VkRenderPassCreateInfo& info) const {
        VkRenderPass renderPass;
        if (vkCreateRenderPass(logical, &info, SGF::VulkanAllocator, &renderPass) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_RENDER_PASS);
        }
        TRACK_RENDER_PASS(1);
        return renderPass;
    }
    VkRenderPass Device::renderPass(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependenfyCount) const {
        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = attachmentCount;
        info.pAttachments = pAttachments;
        info.subpassCount = subpassCount;
        info.pSubpasses = pSubpasses;
        info.dependencyCount = dependenfyCount;
        info.pDependencies = pDependencies;
        return renderPass(info);
    }
    VkSwapchainKHR Device::swapchain(const VkSwapchainCreateInfoKHR& info) const {
        VkSwapchainKHR swapchain;
        if (vkCreateSwapchainKHR(logical, &info, SGF::VulkanAllocator, &swapchain) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_SWAPCHAIN);
        }
        TRACK_SWAPCHAIN(1);
        return swapchain;
    }

    VkCommandPool Device::commandPool(const VkCommandPoolCreateInfo& info) const {
        VkCommandPool pool;
        if (vkCreateCommandPool(logical, &info, SGF::VulkanAllocator, &pool) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_COMMAND_POOL);
        }
        return pool;
    }
    VkCommandPool Device::commandPool(uint32_t queueIndex, VkCommandPoolCreateFlags flags) const {
        VkCommandPoolCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        info.queueFamilyIndex = queueIndex;
        return commandPool(info);
    }
    VkCommandBuffer Device::commandBuffer(VkCommandPool pool, VkCommandBufferLevel level) const {
        VkCommandBuffer commands;
        commandBuffers(pool, level, 1, &commands);
        return commands;
    }
    void Device::commandBuffers(VkCommandPool pool, VkCommandBufferLevel level, uint32_t allocationCount, VkCommandBuffer* pBuffers) const {
        VkCommandBufferAllocateInfo info;
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;
        info.commandPool = pool;
        info.commandBufferCount = allocationCount;
        info.level = level;
        commandBuffers(info, pBuffers);
    }
    void Device::commandBuffers(const VkCommandBufferAllocateInfo& info, VkCommandBuffer* pBuffers) const {
        if (vkAllocateCommandBuffers(logical, &info, pBuffers) != VK_SUCCESS) {
            fatal(ERROR_ALLOCATE_COMMAND_BUFFERS);
        }
    }

    VkDescriptorSetLayout Device::descriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info) const {
        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(logical, &info, VulkanAllocator, &layout) != VK_SUCCESS) {
            fatal(ERROR_CREATE_DESCRIPTOR_LAYOUT);
        }
        return layout;
    }
    VkDescriptorSetLayout Device::descriptorSetLayout(const VkDescriptorSetLayoutBinding* pBindings, uint32_t bindingCount, VkDescriptorSetLayoutCreateFlags flags) const {
        VkDescriptorSetLayoutCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.pNext = nullptr;
        info.pBindings = pBindings;
        info.bindingCount = bindingCount;
        info.flags = flags;
        return descriptorSetLayout(info);
    }
    VkDescriptorSetLayout Device::descriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayoutCreateFlags flags) const {
        return descriptorSetLayout(bindings.data(), (uint32_t)bindings.size(), flags);
    }

    VkDescriptorPool Device::descriptorPool(const VkDescriptorPoolCreateInfo& info) const {
        assert(info.sType == VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
        assert(info.pPoolSizes != nullptr);
        assert(info.poolSizeCount != 0);
        VkDescriptorPool pool;
        if (vkCreateDescriptorPool(logical, &info, VulkanAllocator, &pool) != VK_SUCCESS) {
            fatal(ERROR_CREATE_DESCRIPTOR_POOL);
        }
        return pool;
    }

    VkDescriptorPool Device::descriptorPool(uint32_t maxSets, const VkDescriptorPoolSize* pPoolSizes, uint32_t poolSizeCount, VkDescriptorPoolCreateFlags flags) const {
        VkDescriptorPoolCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.flags = flags;
        info.pNext = nullptr;
        info.poolSizeCount = poolSizeCount;
        info.pPoolSizes = pPoolSizes;
        info.maxSets = maxSets;
        return descriptorPool(info);
    }
    VkDescriptorPool Device::descriptorPool(uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes, VkDescriptorPoolCreateFlags flags) const {
        return descriptorPool(maxSets, poolSizes.data(), (uint32_t)poolSizes.size(), flags);
    }

    
    VkDescriptorSet Device::descriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout) const {
        VkDescriptorSet set;
        descriptorSets(pool, &descriptorSetLayout, 1, &set);
        return set;
    }
    void Device::descriptorSets(const VkDescriptorSetAllocateInfo& info, VkDescriptorSet* descriptorSets) const {
        if (vkAllocateDescriptorSets(logical, &info, descriptorSets) != VK_SUCCESS) {
            fatal(ERROR_ALLOCATE_DESCRIPTOR_SETS);
        }
    }
    void Device::descriptorSets(VkDescriptorPool pool, const VkDescriptorSetLayout* pSetLayouts, uint32_t setCount, VkDescriptorSet* pDescriptorSets) const {
        VkDescriptorSetAllocateInfo info;
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.descriptorPool = pool;
        info.pSetLayouts = pSetLayouts;
        info.descriptorSetCount = setCount;
        info.pSetLayouts = pSetLayouts;
        info.pNext = nullptr;
        descriptorSets(info, pDescriptorSets);
    }
    void Device::descriptorSets(VkDescriptorPool pool, const std::vector<VkDescriptorSetLayout> setLayouts, VkDescriptorSet* pDescriptorSets) const {
        descriptorSets(pool, setLayouts.data(), (uint32_t)setLayouts.size(), pDescriptorSets);
    }
        

    void Device::getSwapchainImages(VkSwapchainKHR swapchain, uint32_t* pCount, VkImage* pImages) const {
        if (vkGetSwapchainImagesKHR(logical, swapchain, pCount, pImages) != VK_SUCCESS) {
            fatal(ERROR_GET_SWAPCHAIN_IMAGES);
        }
    }
    VkFormat Device::getSupportedFormat(const VkFormat* pCandidates, uint32_t candidateCount, VkFormatFeatureFlags features, VkImageTiling tiling) const {
        assert(tiling == VK_IMAGE_TILING_LINEAR || tiling == VK_IMAGE_TILING_OPTIMAL);
        for (uint32_t i = 0; i < candidateCount; i++) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physical, pCandidates[i], &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return pCandidates[i];
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return pCandidates[i];
            }
        }
        return VK_FORMAT_MAX_ENUM;
    }

#pragma region DESTRUCTORS
    void Device::destroyType(VkFence fence) const {
        assert(fence != VK_NULL_HANDLE);
        vkDestroyFence(logical, fence, SGF::VulkanAllocator);
        TRACK_FENCE(-1);
    }
    void Device::destroyType(VkSemaphore semaphore) const {
        assert(semaphore != VK_NULL_HANDLE);
        vkDestroySemaphore(logical, semaphore, SGF::VulkanAllocator);
        TRACK_SEMAPHORE(-1);
    }
    void Device::destroyType(VkBuffer buffer) const {
        assert(buffer != VK_NULL_HANDLE);
        vkDestroyBuffer(logical, buffer, SGF::VulkanAllocator);
        TRACK_BUFFER(-1);
    }
    void Device::destroyType(VkImage image) const {
        assert(image != VK_NULL_HANDLE);
        vkDestroyImage(logical, image, SGF::VulkanAllocator);
        TRACK_IMAGE(-1);
    }
    void Device::destroyType(VkImageView imageView) const {
        assert(imageView != VK_NULL_HANDLE);
        vkDestroyImageView(logical, imageView, SGF::VulkanAllocator);
        TRACK_IMAGE_VIEW(-1);
    }
    void Device::destroyType(VkFramebuffer framebuffer) const {
        assert(framebuffer != VK_NULL_HANDLE);
        vkDestroyFramebuffer(logical, framebuffer, SGF::VulkanAllocator);
        TRACK_FRAMEBUFFER(-1);
    }
    void Device::destroyType(VkRenderPass renderPass) const {
        assert(renderPass != VK_NULL_HANDLE);
        vkDestroyRenderPass(logical, renderPass, SGF::VulkanAllocator);
        TRACK_RENDER_PASS(-1);
    }
    void Device::destroyType(VkPipeline pipeline) const {
        assert(pipeline != VK_NULL_HANDLE);
        vkDestroyPipeline(logical, pipeline, SGF::VulkanAllocator);
        TRACK_PIPELINE(-1);
    }
    void Device::destroyType(VkPipelineLayout pipelineLayout) const {
        assert(pipelineLayout != VK_NULL_HANDLE);
        vkDestroyPipelineLayout(logical, pipelineLayout, SGF::VulkanAllocator);
        TRACK_PIPELINE_LAYOUT(-1);
    }
    void Device::destroyType(VkDescriptorSetLayout descriptorSetLayout) const {
        assert(descriptorSetLayout != VK_NULL_HANDLE);
        vkDestroyDescriptorSetLayout(logical, descriptorSetLayout, SGF::VulkanAllocator);
        TRACK_DESCRIPTOR_SET_LAYOUT(-1);
    }
    void Device::destroyType(VkDescriptorPool descriptorPool) const {
        assert(descriptorPool != VK_NULL_HANDLE);
        vkDestroyDescriptorPool(logical, descriptorPool, SGF::VulkanAllocator);
        TRACK_DESCRIPTOR_POOL(-1);
    }
    void Device::destroyType(VkDeviceMemory memory) const {
        assert(memory != VK_NULL_HANDLE);
        vkFreeMemory(logical, memory, SGF::VulkanAllocator);
        TRACK_DEVICE_MEMORY(-1);
    }
    void Device::destroyType(VkCommandPool commandPool) const {
        assert(commandPool != VK_NULL_HANDLE);
        vkDestroyCommandPool(logical, commandPool, SGF::VulkanAllocator);
        TRACK_COMMAND_POOL(-1);
    }
    void Device::destroyType(VkSampler sampler) const {
        assert(sampler != VK_NULL_HANDLE);
        vkDestroySampler(logical, sampler, SGF::VulkanAllocator);
        TRACK_SAMPLER(-1);
    }
    void Device::destroyType(VkSwapchainKHR swapchain) const {
        assert(swapchain != VK_NULL_HANDLE);
        vkDestroySwapchainKHR(logical, swapchain, SGF::VulkanAllocator);
        TRACK_SWAPCHAIN(-1);
    }
    void Device::destroyType(VkShaderModule module) const {
        assert(module != VK_NULL_HANDLE);
        vkDestroyShaderModule(logical, module, SGF::VulkanAllocator);
        TRACK_SHADER_MODULE(-1);
    }
#pragma endregion DESTRUCTORS

#pragma region GETTERS
    const char* Device::getName() const {
        return name;
    }
#pragma endregion SETTERS
#pragma endregion DEVICE_USER_FUNCTIONS

} // namespace SGF