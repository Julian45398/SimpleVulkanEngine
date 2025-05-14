#include <cstring>
#include <vector>
#include <volk.h>
#include "Render/Device.hpp"
#include "Events/Event.hpp"
#include "Events/InputEvents.hpp"
#include "Filesystem/File.hpp"

#include "Window.hpp"
namespace SGF {
    extern VkInstance VulkanInstance;
    extern VkAllocationCallbacks* VulkanAllocator;
#ifdef SGF_ENABLE_VALIDATION
	extern const char* VULKAN_MESSENGER_NAME;
    extern VkDebugUtilsMessengerEXT VulkanMessenger;
    extern VkDebugUtilsMessengerEXT createDebugUtilsMessengerEXT(const VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
#endif

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

    bool checkPhysicalDeviceFeatureSupport(VkPhysicalDevice device, const VkPhysicalDeviceFeatures* requestedFeatures) {
        assert(device != VK_NULL_HANDLE);
        if (requestedFeatures == nullptr) {
            return true;
        }
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);
        uint32_t array_size = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
        const VkBool32* requested = (VkBool32*)requestedFeatures;
        const VkBool32* available = (VkBool32*)&features;
        for(uint32_t i = 0; i < array_size; ++i) {
            if (requested[i] && !available[i]) {
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

    bool checkPhysicalDeviceSurfaceSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        assert(device != VK_NULL_HANDLE);
        if (surface == VK_NULL_HANDLE) {
            return true;
        }
        bool surface_support = false;
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        for (uint32_t i = 0; i < count; ++i) {
            VkBool32 support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &support);
            if (support == VK_TRUE) {
                surface_support = true;
                break;
            }
        }
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
        if (!surface_support || count == 0) {
            SGF::info("Device is missing surface support!");
            return false;
        } else {
            return true;
        }
        //return surface_support && count != 0;
    }

    constexpr VkQueueFlags graphicsQueueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    constexpr VkQueueFlags computeQueueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    constexpr VkQueueFlags transferQueueFlags = VK_QUEUE_TRANSFER_BIT;

    bool checkPhysicalDeviceQueueSupport(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t graphicsCount, uint32_t computeCount, uint32_t transferCount) {
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
                VkBool32 support = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &support);
                if (support == VK_TRUE) {
                    hasPresent = true;
                }
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
        if (!(hasPresent || surface == VK_NULL_HANDLE) && 
            (hasGraphics || graphicsCount == 0) && 
            (hasCompute || computeCount == 0) && 
            (hasTransfer || transferCount == 0)) {
            SGF::warn("Device is missing queue support!");
            return false;
        } else {
            return true;
        }
    }

    bool checkPhysicalDeviceRequirements(VkPhysicalDevice device, uint32_t extensionCount, const char* const* pExtensions,
            const VkPhysicalDeviceFeatures* requiredFeatures, const VkPhysicalDeviceLimits* minLimits, VkSurfaceKHR surface, 
            uint32_t graphicsQueueCount, uint32_t computeQueueCount, uint32_t transferQueueCount) {
        assert(device != VK_NULL_HANDLE);
        return checkPhysicalDeviceExtensionSupport(device, extensionCount, pExtensions) && checkPhysicalDeviceFeatureSupport(device, requiredFeatures) &&
            //checkPhysicalDeviceRequiredLimits(device, minLimits) && 
            checkPhysicalDeviceSurfaceSupport(device, surface) && 
            checkPhysicalDeviceQueueSupport(device, surface, graphicsQueueCount, computeQueueCount, transferQueueCount);
    }

    void getEnabledFeatures(VkPhysicalDevice device, const VkPhysicalDeviceFeatures* requiredFeatures, const VkPhysicalDeviceFeatures* optionalFeatures, VkPhysicalDeviceFeatures* pEnabledFeatures, uint64_t* pFeatureFlags) {
        constexpr size_t featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
        static_assert(featureCount < sizeof(*pFeatureFlags)*8);
        assert(device != VK_NULL_HANDLE);
        *pFeatureFlags = 0;
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
        const VkBool32* supported = (VkBool32*)&supportedFeatures;
        const VkBool32* required = (const VkBool32*)requiredFeatures;
        const VkBool32* optional = (const VkBool32*)optionalFeatures;
        VkBool32* enabled = (VkBool32*)pEnabledFeatures;
        for (size_t i = 0; i < featureCount; ++i) {
            if (requiredFeatures != nullptr) {
                enabled[i] = required[i];
            } else {
                enabled[i] = VK_FALSE;
            }
            if (optionalFeatures != nullptr) {
                enabled[i] = supported[i] & optional[i];
            }
            if (enabled[i] == VK_TRUE) {
                *pFeatureFlags |= 1 << i;
            }
        }
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

    void Device::getQueueCreateInfos(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* pIndexCount, VkDeviceQueueCreateInfo* pQueueCreateInfos, float* pQueuePriorityBuffer) {
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
            if (surface != VK_NULL_HANDLE && !hasPresent) {
                VkBool32 support = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &support);
                if (support == VK_TRUE) {
                    pQueuePriorityBuffer[floatBufferOffset] = 1.0f;
                    presentInfo.queueFamilyIndex = i;
                    presentInfo.pQueuePriorities = &pQueuePriorityBuffer[floatBufferOffset];
                    floatBufferOffset++;
                    hasPresent = true;
                    this_present = true;
                }
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
    void Device::pickPhysicalDevice(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
            const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, VkSurfaceKHR surface) {
        uint32_t count;
        vkEnumeratePhysicalDevices(SGF::VulkanInstance, &count, nullptr);
        std::vector<VkPhysicalDevice> physical_devices(count);
        vkEnumeratePhysicalDevices(SGF::VulkanInstance, &count, physical_devices.data());
        
        VkPhysicalDevice picked = VK_NULL_HANDLE;
        int32_t max_score = -1;
        for (size_t i = 0; i < physical_devices.size(); ++i) {
            int32_t score = 0;
            VkPhysicalDevice device = physical_devices[i];
            if (!checkPhysicalDeviceRequirements(device, extensionCount, pExtensions, requiredFeatures, minLimits,
                    surface, graphicsCount, computeCount, transferCount)) {
                continue;
            }
            SGF::debug("device supports minimal requirements!");
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            if (checkPhysicalDeviceFeatureSupport(device, optionalFeatures)) {
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

    void Device::createLogicalDevice(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
            const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, VkSurfaceKHR surface) {
        VkDeviceCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.enabledExtensionCount = extensionCount;
        info.ppEnabledExtensionNames = pExtensions;
        VkPhysicalDeviceFeatures enabled;
        uint32_t indexCount;
        VkDeviceQueueCreateInfo queueCreateInfos[4];
        std::vector<float> queuePriorityBuffer(graphicsCount + computeCount + transferCount + presentCount);
        getQueueCreateInfos(physical, surface, &indexCount, queueCreateInfos, queuePriorityBuffer.data());
        info.pQueueCreateInfos = queueCreateInfos;
        info.queueCreateInfoCount = indexCount;
        getEnabledFeatures(physical, requiredFeatures, optionalFeatures, &enabled, &enabledFeatures);
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
        
    Device::Device(uint32_t deviceIndex, uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
            const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, Window* window, uint32_t g, uint32_t c, uint32_t t) 
            : graphicsCount(g), computeCount(c), transferCount(t), presentCount((window == nullptr ? 0 : 1)) {
        VkSurfaceKHR surface = (window != nullptr ? window->surface : VK_NULL_HANDLE);
        {
            uint32_t count;
            vkEnumeratePhysicalDevices(SGF::VulkanInstance, &count, nullptr);
            std::vector<VkPhysicalDevice> physical_devices(count);
            vkEnumeratePhysicalDevices(SGF::VulkanInstance, &count, physical_devices.data());
            if (count <= deviceIndex) {
                SGF::error("device index out of range for available devices!");
            } else {
                physical = physical_devices[deviceIndex];
                if (!checkPhysicalDeviceRequirements(physical, extensionCount, pExtensions, requiredFeatures, minLimits,
                        surface, g, c, t)) {
                    SGF::info("physical device at index: ", deviceIndex, " not suitable!");
                    physical = VK_NULL_HANDLE;
                }
            }
        }
        if (physical == VK_NULL_HANDLE) {
            pickPhysicalDevice(extensionCount, pExtensions, requiredFeatures,
                optionalFeatures, minLimits, surface);
        }
        createLogicalDevice(extensionCount, pExtensions, requiredFeatures,
                optionalFeatures, minLimits, surface);
        // bind window
        Device& d = (*this);
        if (window != nullptr) {
            (*window).bindDevice(d);
        }
    }
    Device::Device(uint32_t extensionCount, const char* const* pExtensions, const VkPhysicalDeviceFeatures* requiredFeatures,
            const VkPhysicalDeviceFeatures* optionalFeatures, const VkPhysicalDeviceLimits* minLimits, Window* window, 
            uint32_t g, uint32_t c, uint32_t t)
        : graphicsCount(g), computeCount(c), transferCount(t), presentCount((window == nullptr ? 0 : 1)) {
        VkSurfaceKHR surface = (window != nullptr ? window->surface : VK_NULL_HANDLE);
        pickPhysicalDevice(extensionCount, pExtensions, requiredFeatures,
            optionalFeatures, minLimits, surface);
        createLogicalDevice(extensionCount, pExtensions, requiredFeatures,
                optionalFeatures, minLimits, surface);
        // bind window
        Device& d = (*this);
        if (window != nullptr) {
            (*window).bindDevice(d);
        }
    }
    void Device::destroy() {
        if (physical != VK_NULL_HANDLE) {
            DeviceDestroyEvent event(this);
            EventManager::dispatch(event);
            SGF::debug("destroying device...");
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

constexpr VkPhysicalDeviceLimits defaultDeviceLimits {
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

Device::Builder::Builder() : limits(defaultDeviceLimits) {}

constexpr VkPhysicalDeviceFeatures emptyFeatures = {};
Device Device::Builder::build() {
    return Device(deviceExtensionCount, deviceExtensions, &features, &optional, &limits,
            pWindow, graphicsQueueCount, computeQueueCount, transferQueueCount);
}

#pragma endregion DEVICE_BUILDER

#pragma region DEVICE_USER_FUNCTIONS
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

    DeviceMemory Device::allocate(const VkMemoryAllocateInfo& info) const {
        DeviceMemory mem;
        mem.memorySize = info.allocationSize;
        if (vkAllocateMemory(logical, &info, SGF::VulkanAllocator, &mem.handle) != VK_SUCCESS) {
            SGF::fatal(ERROR_DEVICE_MEM_ALLOCATION);
        }
        return mem;
    }

    DeviceMemory Device::allocate(const VkMemoryRequirements& req, VkMemoryPropertyFlags flags) const {
        VkMemoryAllocateInfo info;
        info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.pNext = nullptr;
        info.allocationSize = req.size;
        info.memoryTypeIndex = findMemoryIndex(req.memoryTypeBits, flags);
        return allocate(info);
    }

    DeviceMemory Device::allocate(VkBuffer buffer, VkMemoryPropertyFlags flags) const {
        assert(buffer != VK_NULL_HANDLE);
        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(logical, buffer, &req);
        DeviceMemory mem = allocate(req, flags);
        if (vkBindBufferMemory(logical, buffer, mem, 0) != VK_SUCCESS) {
            SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
        }
        return mem;
    }
    DeviceMemory Device::allocate(const VkBuffer* pBuffers, uint32_t bufferCount, VkMemoryPropertyFlags flags) const {
        assert(bufferCount != 0);
        assert(pBuffers != nullptr);
        VkMemoryRequirements req = {};
        VkMemoryRequirements bufReq;
        VkDeviceSize* offsets = new VkDeviceSize[bufferCount];
        for (uint32_t i = 0; i < bufferCount; ++i) {
            vkGetBufferMemoryRequirements(logical, pBuffers[i], &bufReq);
            req.memoryTypeBits |= bufReq.memoryTypeBits;
            req.alignment = (bufReq.alignment < req.alignment ? req.alignment : bufReq.alignment);
            offsets[i] = ((bufReq.alignment - req.size % bufReq.alignment) ^ bufReq.alignment) + req.size;
            req.size += offsets[i] + bufReq.size;
            assert(offsets[i] % bufReq.alignment == 0);
        }
        req.size += ((req.alignment - req.size % req.alignment) ^ req.alignment);
        assert(req.size % req.alignment == 0);
        DeviceMemory mem = allocate(req, flags);
        for (uint32_t i = 0; i < bufferCount; ++i) {
            if (vkBindBufferMemory(logical, pBuffers[i], mem, offsets[i]) != VK_SUCCESS) {
                SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
            }
        }
        delete[] offsets;
        return mem;
    }
    DeviceMemory Device::allocate(VkImage image, VkMemoryPropertyFlags flags) const {
        assert(image != VK_NULL_HANDLE);
        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(logical, image, &req);
        DeviceMemory mem = allocate(req, flags);
        if (vkBindImageMemory(logical, image, mem, 0) != VK_SUCCESS) {
            SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
        }
        return mem;
    }
    DeviceMemory Device::allocate(const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags) const {
        assert(imageCount != 0);
        assert(pImages != nullptr);
        VkMemoryRequirements req = {};
        VkMemoryRequirements imageReq;
        VkDeviceSize* offsets = new VkDeviceSize[imageCount];
        for (uint32_t i = 0; i < imageCount; ++i) {
            vkGetImageMemoryRequirements(logical, pImages[i], &imageReq);
            req.memoryTypeBits |= imageReq.memoryTypeBits;
            req.alignment = (imageReq.alignment < req.alignment ? req.alignment : imageReq.alignment);
            offsets[i] = ((imageReq.alignment - req.size % imageReq.alignment) ^ imageReq.alignment) + req.size;
            req.size += (req.size % req.alignment) + imageReq.size;
            assert(offsets[i] % imageReq.alignment == 0);
        }
        req.size += ((req.alignment - req.size % req.alignment) ^ req.alignment);
        assert(req.size % req.alignment == 0);
        DeviceMemory mem = allocate(req, flags);
        for (uint32_t i = 0; i < imageCount; ++i) {
            if (vkBindImageMemory(logical, pImages[i], mem, offsets[i]) != VK_SUCCESS) {
                SGF::fatal(ERROR_BIND_DEVICE_MEMORY);
            }
        }
        delete[] offsets;
        return mem;
    }
    DeviceMemory Device::allocate(const VkBuffer* pBuffers, uint32_t bufferCount, const VkImage* pImages, uint32_t imageCount, VkMemoryPropertyFlags flags) const {
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
            offsets[i] = ((memReq.alignment - req.size % memReq.alignment) ^ memReq.alignment) + req.size;
            req.size += offsets[i] + memReq.size;
            assert(offsets[i] % memReq.alignment == 0);
        }
        for (uint32_t i = bufferCount; i < imageCount + bufferCount; ++i) {
            vkGetImageMemoryRequirements(logical, pImages[i], &memReq);
            req.memoryTypeBits |= memReq.memoryTypeBits;
            req.alignment = (memReq.alignment < req.alignment ? req.alignment : memReq.alignment);
            offsets[i] = ((memReq.alignment - req.size % memReq.alignment) ^ memReq.alignment) + req.size;
            req.size += (req.size % req.alignment) + memReq.size;
            assert(offsets[i] % memReq.alignment == 0);
        }
        req.size += ((req.alignment - req.size % req.alignment) ^ req.alignment);
        assert(req.size % req.alignment == 0);
        DeviceMemory mem = allocate(req, flags);
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

    Buffer Device::buffer(const VkBufferCreateInfo& info) const {
        Buffer buffer;
        buffer.memorySize = info.size;
        if (vkCreateBuffer(logical, &info, SGF::VulkanAllocator, &buffer.handle) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_BUFFER);
        }
        return buffer;
    }
    Buffer Device::buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags) const {
        VkBufferCreateInfo info;
        createDefaultBufferInfo(&info, size, usage, flags);
        return buffer(info);
    }
    Buffer Device::buffer(VkDeviceSize size, VkBufferUsageFlags usage, QueueFamilyFlags familyFlags, VkBufferCreateFlags flags) const {
        uint32_t indices[4] = {};
        VkBufferCreateInfo info;
        createDefaultBufferInfo(&info, size, usage, flags);
        setupQueueFamilySharing(this, &info, indices, familyFlags);
        return buffer(info);
    }


#pragma region IMAGE
    Image Device::image(const VkImageCreateInfo& info) const {
        assert(info.extent.width != 0);
        assert(info.extent.height != 0);
        assert(info.extent.depth != 0);
        assert(info.sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
        assert(info.pQueueFamilyIndices != nullptr);
        Image image;
        if (vkCreateImage(logical, &info, SGF::VulkanAllocator, &image.handle) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_IMAGE);
        }
        image.arraySize = info.arrayLayers;
        image.width = info.extent.width;
        image.height = info.extent.height;
        image.depth = info.extent.depth;
        image.format = info.format;
        image.mipLevelCount = info.mipLevels;
        return image;
    }
        Image Device::image1D(uint32_t length, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, {length, 1, 1}, 1, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_1D;
        return image(info);
    }
    Image Device::image2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, {width, height, 1}, 1, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_2D;
        return image(info);
    }
    Image Device::image3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, {width, height, depth}, 1, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_3D;
        return image(info);
    }
    Image Device::imageArray1D(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, {length, 1, 1}, arraySize, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_1D;
        return image(info);
    }
    Image Device::imageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, VkImageCreateFlags flags) const {
        VkImageCreateInfo info{};
        createDefaultImageInfo(&info, {width, height, 1}, arraySize, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_2D;
        return image(info);
    }
    Image Device::imageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, VkImageCreateFlags flags) const {
        VkImageCreateInfo info{};
        createDefaultImageInfo(&info, { width, height, depth }, arraySize, format, usage, mipLevelCount, samples, flags);
        info.imageType = VK_IMAGE_TYPE_3D;
        return image(info);
    }
    Image Device::image1D(uint32_t length, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { length, 1, 1 }, 1, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_1D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    Image Device::image2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        assert(graphicsCount != 0);
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { width, height, 1 }, 1, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_2D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    Image Device::image3D(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        assert(graphicsCount != 0);
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { width, height, depth }, 1, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_3D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    Image Device::imageArray1D(uint32_t length, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { length, 1, 1 }, arraySize, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_1D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    Image Device::imageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
        VkImageCreateInfo info;
        createDefaultImageInfo(&info, { width, height, 1 }, arraySize, format, usage, mipLevelCount, samples, flags);
        uint32_t indices[4] = {};
        info.imageType = VK_IMAGE_TYPE_2D;
        setupQueueFamilySharing(this, &info, indices, queueFlags);
        return image(info);
    }
    Image Device::imageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevelCount, VkSampleCountFlagBits samples, QueueFamilyFlags queueFlags, VkImageCreateFlags flags) const {
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

    ImageView Device::imageView(const VkImageViewCreateInfo& info) const {
        ImageView view;
        assert(info.sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
        if (!vkCreateImageView(logical, &info, SGF::VulkanAllocator, &view.handle) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_IMAGE_VIEW);
        }
        return view;
    }
    ImageView Device::imageView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_1D, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
        return imageView(info);
    }

    ImageView Device::imageView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_2D, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
        return imageView(info);
    }

    ImageView Device::imageView3D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_3D, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
        return imageView(info);
    }

    ImageView Device::imageViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_CUBE, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
        return imageView(info);
    }
    ImageView Device::imageArrayView1D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer, uint32_t arraySize) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_1D_ARRAY, format, imageAspect, mipLevel, levelCount, arrayLayer, arraySize);
        return imageView(info);
    }

    ImageView Device::imageArrayView2D(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer, uint32_t arraySize) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_2D_ARRAY, format, imageAspect, mipLevel, levelCount, arrayLayer, arraySize);
        return imageView(info);
    }
    ImageView Device::imageArrayViewCube(VkImage image, VkFormat format, VkImageAspectFlags imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer, uint32_t arraySize) const {
        VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, format, imageAspect, mipLevel, levelCount, arrayLayer, arraySize);
        return imageView(info);
    }

#pragma endregion IMAGE_CREATION

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
        return sem;
    }

    VkShaderModule Device::shaderModule(const char* filename) const {
        const auto& code = loadBinaryFile(filename);
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
        return shaderModule;
    }

    VkPipelineLayout Device::pipelineLayout(const VkPipelineLayoutCreateInfo& info) const {
        VkPipelineLayout layout;
        if (vkCreatePipelineLayout(logical, &info, VulkanAllocator, &layout) != VK_SUCCESS) {
            fatal(ERROR_CREATE_PIPELINE_LAYOUT);
        }
        return layout;
    }
    VkPipelineLayout Device::pipelineLayout(uint32_t descriptorLayoutCount, const VkDescriptorSetLayout* pLayouts, uint32_t pushConstantCount, const VkPushConstantRange* pPushConstantRanges) const {
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
        return pipeline;
    }

    VkPipeline Device::pipeline(const VkComputePipelineCreateInfo& info) const {
        VkPipeline pipeline;
        if (vkCreateComputePipelines(logical, VK_NULL_HANDLE, 1, &info, VulkanAllocator, &pipeline) != VK_SUCCESS) {
            fatal(ERROR_CREATE_COMPUTE_PIPELINE);
        }
        return pipeline;
    }

    VkFramebuffer Device::framebuffer(const VkFramebufferCreateInfo& info) const {
        VkFramebuffer framebuffer;
        if (vkCreateFramebuffer(logical, &info, SGF::VulkanAllocator, &framebuffer) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_FRAMEBUFFER);
        }
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
        if (!vkCreateRenderPass(logical, &info, SGF::VulkanAllocator, &renderPass) != VK_SUCCESS) {
            SGF::fatal(ERROR_CREATE_RENDER_PASS);
        }
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
        return swapchain;
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
    void Device::destroy(VkFence fence) const {
        assert(fence != VK_NULL_HANDLE);
        vkDestroyFence(logical, fence, SGF::VulkanAllocator);
    }
    void Device::destroy(VkSemaphore semaphore) const {
        assert(semaphore != VK_NULL_HANDLE);
        vkDestroySemaphore(logical, semaphore, SGF::VulkanAllocator);
    }
    void Device::destroy(VkBuffer buffer) const {
        assert(buffer != VK_NULL_HANDLE);
        vkDestroyBuffer(logical, buffer, SGF::VulkanAllocator);
    }
    void Device::destroy(VkImage image) const {
        assert(image != VK_NULL_HANDLE);
        vkDestroyImage(logical, image, SGF::VulkanAllocator);
    }
    void Device::destroy(VkImageView imageView) const {
        assert(imageView != VK_NULL_HANDLE);
        vkDestroyImageView(logical, imageView, SGF::VulkanAllocator);
    }
    void Device::destroy(VkFramebuffer framebuffer) const {
        assert(framebuffer != VK_NULL_HANDLE);
        vkDestroyFramebuffer(logical, framebuffer, SGF::VulkanAllocator);
    }
    void Device::destroy(VkRenderPass renderPass) const {
        assert(renderPass != VK_NULL_HANDLE);
        vkDestroyRenderPass(logical, renderPass, SGF::VulkanAllocator);
    }
    void Device::destroy(VkPipeline pipeline) const {
        assert(pipeline != VK_NULL_HANDLE);
        vkDestroyPipeline(logical, pipeline, SGF::VulkanAllocator);
    }
    void Device::destroy(VkPipelineLayout pipelineLayout) const {
        assert(pipelineLayout != VK_NULL_HANDLE);
        vkDestroyPipelineLayout(logical, pipelineLayout, SGF::VulkanAllocator);
    }
    void Device::destroy(VkDescriptorSetLayout descriptorSetLayout) const {
        assert(descriptorSetLayout != VK_NULL_HANDLE);
        vkDestroyDescriptorSetLayout(logical, descriptorSetLayout, SGF::VulkanAllocator);
    }
    void Device::destroy(VkDescriptorPool descriptorPool) const {
        assert(descriptorPool != VK_NULL_HANDLE);
        vkDestroyDescriptorPool(logical, descriptorPool, SGF::VulkanAllocator);
    }
    void Device::destroy(VkDeviceMemory memory) const {
        assert(memory != VK_NULL_HANDLE);
        vkFreeMemory(logical, memory, SGF::VulkanAllocator);
    }
    void Device::destroy(VkCommandPool commandPool) const {
        assert(commandPool != VK_NULL_HANDLE);
        vkDestroyCommandPool(logical, commandPool, SGF::VulkanAllocator);
    }
    void Device::destroy(VkSampler sampler) const {
        assert(sampler != VK_NULL_HANDLE);
        vkDestroySampler(logical, sampler, SGF::VulkanAllocator);
    }
    void Device::destroy(VkSwapchainKHR swapchain) const {
        assert(swapchain != VK_NULL_HANDLE);
        vkDestroySwapchainKHR(logical, swapchain, SGF::VulkanAllocator);
    }

#pragma endregion DESTRUCTORS

#pragma region GETTERS
    const char* Device::getName() const {
        return name;
    }
#pragma endregion SETTERS
#pragma endregion DEVICE_USER_FUNCTIONS

} // namespace SGF