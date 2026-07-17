#include <SGF/Core/GPU/Device.hpp>

#include <SGF/Core/Debugging/Logger.hpp>
#include <SGF/Core/Debugging/ErrorCodes.hpp>
#include <SGF/Core/Macros.hpp>
#include <SGF/Core/Platform/File.hpp>
#include <SGF/Core/Platform/WindowHandle.hpp>
#include <SGF/Core/GPU/CommandList.hpp>

#include <algorithm>

#include <volk.h>
#include <vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#pragma region DEFINES
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
#define TRACK_CreateImage(COUNT) do{ _TRACK_IMAGE += COUNT;if(COUNT<0){SGF::debug("destroying image: ", _TRACK_IMAGE);}else{SGF::debug("creating image: ", _TRACK_IMAGE);} }while(0)
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
#define TRACK_CreateImage(COUNT)
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
#endif

#ifndef SGF_ENGINE_VERSION
#define SGF_ENGINE_VERSION VK_MAKE_API_VERSION(0, SGF_VERSION_MAJOR, SGF_VERSION_MINOR, SGF_VERSION_PATCH)
#endif
#ifndef SGF_APP_NAME
#define SGF_APP_NAME SGF_ENGINE_NAME
#endif
#ifndef SGF_APP_VERSION
#define SGF_APP_VERSION SGF_ENGINE_VERSION 
#endif
#pragma endregion DEFINES



#pragma region HIDDEN_TYPES
namespace SGF::GPU {
	namespace {
		struct DescriptorPool_T {
			VkDescriptorPool handle;
			DescriptorSetLayout layout;
		};
		class DescriptorSetLayoutBinding_T {
		public:
			VkDescriptorSetLayoutBinding vkBinding{};
			inline bool IsEqual(const DescriptorSetLayoutBinding_T& other) const {
				if (vkBinding.binding != other.vkBinding.binding)
					return false;
				if (vkBinding.descriptorCount != other.vkBinding.descriptorCount)
					return false;
				if (vkBinding.descriptorType != other.vkBinding.descriptorType)
					return false;
				if (vkBinding.stageFlags != other.vkBinding.stageFlags)
					return false;
				if (vkBinding.pImmutableSamplers && other.vkBinding.pImmutableSamplers) {
					for (uint32_t i = 0; i < vkBinding.descriptorCount; ++i) {
						if (vkBinding.pImmutableSamplers[i] != other.vkBinding.pImmutableSamplers[i])
							return false;
					}
				} else {
					return false;
				}
			}
			inline void AllocateSamplers(uint32_t descriptorCount, const VkSampler* pImmutableSamplers) {
				SGF_ASSERT(pImmutableSamplers != nullptr);
				VkSampler* pSamplers = nullptr;
				if (descriptorCount == 1) {
					pSamplers = new VkSampler;
				}
				else {
					pSamplers = new VkSampler[descriptorCount];
				}
				memcpy(pSamplers, pImmutableSamplers, static_cast<size_t>(descriptorCount* sizeof(VkSampler)));
				vkBinding.pImmutableSamplers = pSamplers;
				SGF::Log::Debug("Descriptor Set Layout Binding immutable samplers allocated");
			}
			inline DescriptorSetLayoutBinding_T() : vkBinding{} {}
			inline DescriptorSetLayoutBinding_T(uint32_t binding, VkDescriptorType type, uint32_t descriptorCount, VkPipelineStageFlags stageFlags, const VkSampler* pImmutableSamplers = nullptr) 
				: vkBinding{binding, type, descriptorCount, stageFlags, nullptr} {
				SGF_ASSERT(descriptorCount != 0);
				if (pImmutableSamplers) {
					AllocateSamplers(descriptorCount, pImmutableSamplers);
				}
			}
			inline DescriptorSetLayoutBinding_T(DescriptorSetLayoutBinding_T&& other) {
				vkBinding = other.vkBinding;
				other.vkBinding.pImmutableSamplers = nullptr;
			}
			inline DescriptorSetLayoutBinding_T(const DescriptorSetLayoutBinding_T& other) {
				vkBinding = other.vkBinding;
				if (vkBinding.pImmutableSamplers) {
					AllocateSamplers(other.vkBinding.descriptorCount, other.vkBinding.pImmutableSamplers);
				}
			}
			inline ~DescriptorSetLayoutBinding_T() {
				SGF_ASSERT(vkBinding.descriptorCount != 0);
				SGF::Log::Debug("Descriptor Set Layout Binding descructor called");
				if (vkBinding.pImmutableSamplers) {
					if (vkBinding.descriptorCount == 1) {
						Destroy((Sampler)vkBinding.pImmutableSamplers[0]);
						delete vkBinding.pImmutableSamplers;
					} else {
						for (uint32_t i = 0; i < vkBinding.descriptorCount; ++i) {
							Destroy((Sampler)vkBinding.pImmutableSamplers[i]);
						}
						delete[] vkBinding.pImmutableSamplers;
					}
					vkBinding.pImmutableSamplers = nullptr;
				}
			}
		};
		// Hidden Types
		struct DescriptorSetLayout_T {
			VkDescriptorSetLayout layout = nullptr;
			uint32_t bindingCount = 0;
			VkDescriptorSetLayoutCreateFlags createFlags = 0;
			std::array<DescriptorSetLayoutBinding_T, DescriptorSetLayout::MAX_BINDINGS> bindings{};
			uint32_t hashCollisionIndex = UINT32_MAX;

			bool HasNext() const noexcept {
				return hashCollisionIndex != UINT32_MAX;
			}
			bool IsEqual(const DescriptorSetLayout_T& other) const {
				if (bindingCount != other.bindingCount) 
					return false;
				if (createFlags != other.createFlags)
					return false;
				for (uint32_t i = 0; i < bindingCount; ++i) {
					if (!bindings[i].IsEqual(other.bindings[i]))
						return false;
				}
			}
			DescriptorSetLayout_T(const DescriptorSetBinding* pBindings, uint32_t count, Flags<DescriptorSetLayoutCreate> flags) 
				: layout(nullptr), bindingCount(count), createFlags((VkDescriptorSetLayoutCreateFlags)flags.ToUnderlying()) {
				for (uint32_t i = 0; i < count; ++i) {
					const auto& b = pBindings[i];
					bindings[i] = DescriptorSetLayoutBinding_T(b.binding, (VkDescriptorType)b.descriptorType, b.descriptorCount, (VkPipelineStageFlags)b.stageFlags.ToUnderlying(), (VkSampler*)b.pImmutableSamplers);
				}
			}
			~DescriptorSetLayout_T() {
				if (layout)
					vkDestroyDescriptorSetLayout(s_LogicalDevice, layout, VULKAN_ALLOCATION_CALLBACKS);
			}
		};
		struct Swapchain_T {
			VkSwapchainKHR handle;
			VkSurfaceKHR surface;
			VkImage* pImages;
			VkImageView* pImageViews;
			VkFormat format;
			uint32_t imageCount;
			uint32_t imageIndex;
			bool outOfDate;
			VkExtent2D extent;
			~Swapchain_T() {
				if (pImageViews != nullptr) {
					for (uint32_t i = 0; i < imageCount; ++i) {
						Destroy(pImageViews[i]);
					}
					pImageViews = nullptr;
				}
				if (pImages != nullptr) {
					delete[] pImages;
					pImages = nullptr;
				}
			}
		};
		struct RenderPassBuilder_T {
			struct SubpassData {
				std::vector<VkAttachmentReference> colorReferences;
				std::vector<VkAttachmentReference> inputReferences;
				std::vector<VkAttachmentReference> resolveReferences;
				std::vector<uint32_t> preserveReferences;
				VkAttachmentReference depthStencilReference;
				inline SubpassData() {
					depthStencilReference.attachment = VK_ATTACHMENT_UNUSED;
					depthStencilReference.layout = VK_IMAGE_LAYOUT_UNDEFINED;
				}
			};
			std::vector<VkAttachmentDescription> descriptions;
			std::vector<VkAttachmentReference> references;
			std::vector<VkSubpassDependency> dependencies;
			std::vector<VkSubpassDescription> subpasses;
			std::vector<SubpassData> subpassData;
			VkRenderPassCreateInfo createInfo;
			inline RenderPassBuilder_T() {
				createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				createInfo.pNext = nullptr;
				createInfo.flags = 0;
				createInfo.attachmentCount = 0;
				createInfo.pAttachments = nullptr;
				createInfo.subpassCount = 0;
				createInfo.pSubpasses = nullptr;
				createInfo.dependencyCount = 0;
				createInfo.pDependencies = nullptr;
				subpasses.push_back({});
				subpasses.back().pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpassData.emplace_back();
			}
		};

		struct GraphicsPipelineBuilder_T {
			VkGraphicsPipelineCreateInfo createInfo;
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			std::vector<VkDynamicState> dynamicStates;
			std::vector<VkViewport> viewports;
			std::vector<VkRect2D> scissors;
			std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
			std::vector<ShaderModule> shaderModules;
			std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
			VkPipelineVertexInputStateCreateInfo vertexInputState;
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
			VkPipelineTessellationStateCreateInfo tessellationState;
			VkPipelineViewportStateCreateInfo viewportState;
			VkPipelineRasterizationStateCreateInfo rasterizationState;
			VkPipelineMultisampleStateCreateInfo multisampleState;
			VkPipelineDepthStencilStateCreateInfo depthStencilState;
			VkPipelineColorBlendStateCreateInfo colorBlendState;
			VkPipelineDynamicStateCreateInfo dynamicState;

			~GraphicsPipelineBuilder_T() {
				for (auto& shaderModule : shaderModules) {
					Destroy(shaderModule);
				}
			}
		};
		typedef uint32_t DescriptorLayoutIndex;
	}
	
}
#pragma endregion HIDDEN_TYPES

#pragma region VULKAN_GLOBALS
namespace SGF::GPU {
	namespace {
		constexpr VkAllocationCallbacks* VULKAN_ALLOCATION_CALLBACKS = nullptr;
		VkInstance s_VulkanInstance = VK_NULL_HANDLE;
		VkPhysicalDevice s_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice s_LogicalDevice = VK_NULL_HANDLE;
		VkQueue s_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue s_ComputeQueue = VK_NULL_HANDLE;
		VkQueue s_TransferQueue = VK_NULL_HANDLE;
		VkQueue s_PresentQueue = VK_NULL_HANDLE;
		//VmaAllocator s_MemoryAllocator = VK_NULL_HANDLE;
		Flags<DeviceFeature> s_EnabledFeatures = DeviceFeature::NONE;
		uint32_t s_GraphicsFamilyIndex = UINT32_MAX;
		uint32_t s_ComputeFamilyIndex = UINT32_MAX;
		uint32_t s_TransferFamilyIndex = UINT32_MAX;
		uint32_t s_PresentFamilyIndex = UINT32_MAX;
#ifdef SGF_ENABLE_VALIDATION
		const char* VULKAN_MESSENGER_NAME = "VK_LAYER_KHRONOS_validation";
		VkDebugUtilsMessengerEXT s_VulkanMessenger = VK_NULL_HANDLE;
		std::vector<DescriptorSetLayout_T> s_DescriptorSetLayouts;
		std::map<uint64_t, DescriptorLayoutIndex> s_AllocatedDescriptorMap;
#endif
	}
}
#pragma endregion VULKAN_GLOBALS

#pragma region VULKAN_UTILS 
namespace SGF::GPU::Util {
	VkSurfaceKHR CreateSurface(WindowHandle windowHandle) {
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(s_VulkanInstance, (GLFWwindow*)windowHandle.GetNativeHandle(), VULKAN_ALLOCATION_CALLBACKS, &surface) != VK_SUCCESS) {
			Log::Fatal("{}", ERROR_CREATE_SURFACE);
			return nullptr;
		}
		return surface;
	}
#ifdef SGF_ENABLE_VALIDATION
	VkDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT(const VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback) {
		VkDebugUtilsMessengerCreateInfoEXT create_info = {
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, nullptr, 0,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			debugCallback, nullptr
		};
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		Log::Debug("Creating vulkan debug messenger");

		VkDebugUtilsMessengerEXT messenger;
		if (func != nullptr && func(instance, &create_info, VULKAN_ALLOCATION_CALLBACKS, &messenger) == VK_SUCCESS) {
			return messenger;
		}
		else {
			Log::Warn("Failed to create vulkan messenger!");
			return VK_NULL_HANDLE;
		}
	}
	void DestroyDebugUtilsMessengerEXT(const VkInstance instance, VkDebugUtilsMessengerEXT messenger) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, messenger, VULKAN_ALLOCATION_CALLBACKS);
		}
	}
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		switch (message_severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			Log::Debug(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			Log::Info(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			Log::Warn(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			Log::Error(pCallbackData->pMessage);
			break;
		}
		return VK_FALSE;
	}
#endif
	constexpr VkQueueFlags GRAPHICS_QUEUE_FLAGS = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	constexpr VkQueueFlags COMPUTE_QUEUE_FLAGS = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	constexpr VkQueueFlags TRANSFER_QUEUE_FLAGS = VK_QUEUE_TRANSFER_BIT;

	class QueueSupport {
	public:
		uint32_t graphicsIndex = UINT32_MAX;
		uint32_t computeIndex = UINT32_MAX;
		uint32_t transferIndex = UINT32_MAX;
		uint32_t presentIndex = UINT32_MAX;

	private:
		bool CheckQueuePresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, const std::vector<VkSurfaceKHR>& surfaces);
	public:

		QueueSupport(VkPhysicalDevice physicalDevice, const std::vector<VkSurfaceKHR>& surfaces) : graphicsIndex(UINT32_MAX), computeIndex(UINT32_MAX), transferIndex(UINT32_MAX), presentIndex(UINT32_MAX) {
			SGF_ASSERT(physicalDevice != VK_NULL_HANDLE);
			uint32_t queueCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
			std::vector<VkQueueFamilyProperties> properties(queueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, properties.data());
			for (uint32_t i = 0; i < queueCount; ++i) {
				if (presentIndex == UINT32_MAX) {
					if (CheckQueuePresentationSupport(physicalDevice, i, surfaces)) {
						presentIndex = i;
					}
				}
				if ((properties[i].queueFlags & GRAPHICS_QUEUE_FLAGS) == GRAPHICS_QUEUE_FLAGS && graphicsIndex == UINT32_MAX) {
					graphicsIndex = i;
					if (properties[i].queueCount >= 3) {
						computeIndex = i;
						transferIndex = i;
					}
					else if (properties[i].queueCount >= 2) {
						computeIndex = i;
					}
					continue;
				}
				if (((properties[i].queueFlags & COMPUTE_QUEUE_FLAGS) == COMPUTE_QUEUE_FLAGS) && ((properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && computeIndex == UINT32_MAX) {
					computeIndex = i;
					if (properties[i].queueCount >= 2) {
						transferIndex = i;
					}
					continue;
				}
				if (((properties[i].queueFlags & TRANSFER_QUEUE_FLAGS) == TRANSFER_QUEUE_FLAGS)
					&& ((properties[i].queueFlags & (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT)) == 0) && transferIndex == UINT32_MAX) {
					transferIndex = i;
					continue;
				}
			}
		}
		bool HasSupport(Flags<QueueType> requestedQueues) const {
			if (requestedQueues.Has(QueueType::GRAPHICS) && graphicsIndex == UINT32_MAX) {
				return false;
			}
			if (requestedQueues.Has(QueueType::COMPUTE) && computeIndex == UINT32_MAX) {
				return false;
			}
			if (requestedQueues.Has(QueueType::TRANSFER) && transferIndex == UINT32_MAX) {
				return false;
			}
			if (requestedQueues.Has(QueueType::PRESENT) && presentIndex == UINT32_MAX) {
				return false;
			}
			return true;
		}
		bool HasPresentationSupport() {
			return presentIndex != UINT32_MAX;
		}
	private:
		bool CheckQueuePresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, const std::vector<VkSurfaceKHR>& surfaces) {
			if (surfaces.empty()) {
				return glfwGetPhysicalDevicePresentationSupport(s_VulkanInstance, physicalDevice, queueFamilyIndex);
			}
			for (VkSurfaceKHR surface : surfaces) {
				VkBool32 presentSupported = VK_FALSE;
				VkResult vkResult =
					vkGetPhysicalDeviceSurfaceSupportKHR(
						physicalDevice,
						queueFamilyIndex,
						surface,
						&presentSupported);
				if (vkResult != VK_SUCCESS || presentSupported == VK_FALSE) {
					return false;
				}
			}
			return true;
		}
	};

	uint32_t GetQueueCreateInfos(const QueueSupport& queueSupport, Flags<QueueType> requiredQueues, VkDeviceQueueCreateInfo* pQueueCreateInfos, float* pQueuePriorityBuffer) {
		SGF_ASSERT(device != VK_NULL_HANDLE);
		uint32_t indexCount = 0;
		uint32_t graphicsQueuePosition = UINT32_MAX;
		uint32_t computeQueuePosition = UINT32_MAX;
		uint32_t transferQueuePosition = UINT32_MAX;
		uint32_t presentQueuePosition = UINT32_MAX;
		if (requiredQueues.Has(QueueType::GRAPHICS)) {
			SGF_ASSERT(queueSupport.graphicsIndex != UINT32_MAX);
			uint32_t queueCount = 1;
			pQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			pQueueCreateInfos[0].pNext = nullptr;
			pQueueCreateInfos[0].flags = 0;
			pQueueCreateInfos[0].queueFamilyIndex = queueSupport.graphicsIndex;
			pQueueCreateInfos[0].queueCount = queueCount;
			pQueueCreateInfos[0].pQueuePriorities = pQueuePriorityBuffer;
			graphicsQueuePosition = indexCount;
			for (uint32_t i = 0; i < queueCount; ++i) {
				pQueuePriorityBuffer[i] = 1.0f - (float)i * (1.0f / (float)queueCount);
			}
			indexCount++;
		}
		if (requiredQueues.Has(QueueType::COMPUTE)) {
			SGF_ASSERT(queueSupport.computeIndex != UINT32_MAX);
			if (queueSupport.computeIndex == queueSupport.graphicsIndex && requiredQueues.Has(QueueType::GRAPHICS)) {
				pQueueCreateInfos[graphicsQueuePosition].queueCount++;
				computeQueuePosition = graphicsQueuePosition;
			}
			else {
				pQueueCreateInfos[indexCount] = pQueueCreateInfos[0];
				pQueueCreateInfos[indexCount].queueFamilyIndex = queueSupport.computeIndex;
				pQueueCreateInfos[indexCount].queueCount = 1;
				pQueueCreateInfos[indexCount].pQueuePriorities = pQueuePriorityBuffer;
				computeQueuePosition = indexCount;
				indexCount++;
			}
		}
		if (requiredQueues.Has(QueueType::TRANSFER)) {
			SGF_ASSERT(queueSupport.transferIndex != UINT32_MAX);
			if (queueSupport.transferIndex == queueSupport.graphicsIndex && requiredQueues.Has(QueueType::GRAPHICS)) {
				pQueueCreateInfos[graphicsQueuePosition].queueCount++;
				transferQueuePosition = graphicsQueuePosition;
			}
			else if (queueSupport.transferIndex == queueSupport.computeIndex && requiredQueues.Has(QueueType::COMPUTE)) {
				pQueueCreateInfos[computeQueuePosition].queueCount++;
				transferQueuePosition = computeQueuePosition;
			}
			else {
				pQueueCreateInfos[indexCount] = pQueueCreateInfos[0];
				pQueueCreateInfos[indexCount].queueFamilyIndex = queueSupport.transferIndex;
				pQueueCreateInfos[indexCount].queueCount = 1;
				pQueueCreateInfos[indexCount].pQueuePriorities = pQueuePriorityBuffer;
				transferQueuePosition = indexCount;
				indexCount++;
			}
		}
		if (requiredQueues.Has(QueueType::PRESENT)) {
			SGF_ASSERT(queueSupport.presentIndex != UINT32_MAX);
			if (((queueSupport.presentIndex == queueSupport.graphicsIndex && requiredQueues.Has(QueueType::GRAPHICS)) || (queueSupport.presentIndex == queueSupport.computeIndex && requiredQueues.Has(QueueType::COMPUTE))
				|| (queueSupport.presentIndex == queueSupport.transferIndex && requiredQueues.Has(QueueType::TRANSFER)))) {
				// Do nothing, the present queue is already included in the list of queues to create.
			}
			else {
				pQueueCreateInfos[indexCount] = pQueueCreateInfos[0];
				pQueueCreateInfos[indexCount].queueFamilyIndex = queueSupport.presentIndex;
				pQueueCreateInfos[indexCount].queueCount = 1;
				pQueueCreateInfos[indexCount].pQueuePriorities = pQueuePriorityBuffer;
				presentQueuePosition = indexCount;
				indexCount++;
			}
		}
		return indexCount;
	}
	void GetEnabledFeatures(VkPhysicalDevice physicalDevice, Flags<DeviceFeature> features, VkPhysicalDeviceFeatures* f) {
		constexpr size_t featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
		SGF_ASSERT(physicalDevice != VK_NULL_HANDLE);
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
		const VkBool32* supported = (VkBool32*)&supportedFeatures;
		VkBool32* ef = (VkBool32*)f;
		for (size_t i = 0; i < featureCount; ++i) {
			if (features.Has((DeviceFeature)SGF_BIT(i))) {
				SGF_ASSERT(supported[i] && "device feature required but not supported!");
			}
			ef[i] = (features.Has((DeviceFeature)SGF_BIT(i)) ? VK_TRUE : VK_FALSE);
		}
	}

	std::vector<VkSurfaceKHR> CreateTemporarySurfaces(const WindowHandle* pWindowHandles, uint32_t windowCount) {
		std::vector<VkSurfaceKHR> surfaces;
		surfaces.reserve(windowCount);
		for (uint32_t i = 0; i < windowCount; ++i) {
			auto window = (GLFWwindow*)pWindowHandles[i].GetNativeHandle();
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			if (glfwCreateWindowSurface(s_VulkanInstance, window, nullptr, &surface) != VK_SUCCESS) {
				// Cleanup already-created surfaces.
				for (VkSurfaceKHR s : surfaces) {
					vkDestroySurfaceKHR(s_VulkanInstance, s, nullptr);
				}
				return {};
			}
			surfaces.push_back(surface);
		}
		return surfaces;
	}

	bool InitializeDevice(PhysicalDevice device, Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const std::vector<VkSurfaceKHR>& surfaces) {
		if (device == VK_NULL_HANDLE) {
			Log::Fatal("{}", ERROR_PHYSICAL_DEVICE_INVALID);
			return false;
		}
		Util::QueueSupport queueSupport((VkPhysicalDevice)device, surfaces);
		if (!Util::CheckPhysicalDeviceSupport(device, requiredFeatures, requiredQueues, queueSupport)) {
			Log::Fatal("{}", ERROR_PHYSICAL_DEVICE_NOT_SUITED);
			return false;
		}
		if (IsInitialized()) {
			Log::Info("Device already initialized!");
			Terminate();
		}
		VkDevice logicalDevice = Util::CreateLogicalDevice((VkPhysicalDevice)device, queueSupport, requiredFeatures, requiredQueues);
		s_PhysicalDevice = (VkPhysicalDevice)device;
		s_LogicalDevice = logicalDevice;
		if (requiredQueues.Has(QueueType::GRAPHICS)) {
			s_GraphicsFamilyIndex = queueSupport.graphicsIndex;
			vkGetDeviceQueue(s_LogicalDevice, s_GraphicsFamilyIndex, 0, &s_GraphicsQueue);
		}
		else {
			s_GraphicsFamilyIndex = UINT32_MAX;
			s_GraphicsQueue = nullptr;
		}
		if (requiredQueues.Has(QueueType::COMPUTE)) {
			s_ComputeFamilyIndex = queueSupport.computeIndex;
			vkGetDeviceQueue(s_LogicalDevice, s_ComputeFamilyIndex, 0, &s_ComputeQueue);
		}
		else {
			s_ComputeFamilyIndex = UINT32_MAX;
			s_ComputeQueue = nullptr;
		}
		if (requiredQueues.Has(QueueType::TRANSFER)) {
			s_TransferFamilyIndex = queueSupport.transferIndex;
			vkGetDeviceQueue(s_LogicalDevice, s_TransferFamilyIndex, 0, &s_TransferQueue);
		}
		else {
			s_TransferFamilyIndex = UINT32_MAX;
			s_TransferQueue = nullptr;
		}
		if (requiredQueues.Has(QueueType::PRESENT)) {
			s_PresentFamilyIndex = queueSupport.presentIndex;
			vkGetDeviceQueue(s_LogicalDevice, s_PresentFamilyIndex, 0, &s_PresentQueue);
		}
		else {
			s_PresentFamilyIndex = UINT32_MAX;
			s_PresentQueue = nullptr;
		}
		s_EnabledFeatures = requiredFeatures;
		return true;
	}

	void DestroyTemporarySurfaces(const std::vector<VkSurfaceKHR>& surfaces) {
		for (VkSurfaceKHR surface : surfaces) {
			vkDestroySurfaceKHR(s_VulkanInstance, surface, nullptr);
		}
	}

	bool CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice physicalDevice, uint32_t extensionCount, const char* const* ppDeviceExtensions) {
		assert(physicalDevice != VK_NULL_HANDLE);
		uint32_t count;
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr) != VK_SUCCESS) {
			Log::Error("failed to enumerate extension properties for the device!");
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
						Log::Error("device extension: \"{}\" is not allowed to be included twice in the extension list", ppDeviceExtensions[i]);
						return false;
					}
					found++;
				}
			}
		}
		if (count != extensionCount) {
			Log::Warn("Extensions not supported!");
			return false;
		}
		else {
			return true;
		}
		return count == extensionCount;
	}

	bool CheckPhysicalDeviceFeatureSupport(VkPhysicalDevice device, Flags<DeviceFeature> flags) {
		SGF_ASSERT(device != VK_NULL_HANDLE);
		if (flags == 0) {
			return true;
		}
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);
		uint32_t array_size = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
		const VkBool32* available = (VkBool32*)&features;
		SGF_ASSERT((SGF_BIT(array_size) < MAX_ENUM) && "too many features to check with bitfield!");
		for (uint32_t i = 0; i < array_size; ++i) {
			if ((SGF_BIT(i) & flags) && !available[i]) {
				return false;
			}
		}
		return true;
	}
	bool CheckPhysicalDeviceSupport(PhysicalDevice device, Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const QueueSupport& queueSupport) {
		if (!queueSupport.HasSupport(requiredQueues)) {
			Log::Debug("device doesn't support required queues!");
			return false;
		}
		if (!Util::CheckPhysicalDeviceFeatureSupport((VkPhysicalDevice)device, requiredFeatures)) {
			Log::Debug("device doesn't support required features!");
			return false;
		}
		return true;
	}
	PhysicalDevice PickPhysicalDevice(Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const std::vector<VkSurfaceKHR>& surfaces) {
		auto physicalDevices = GetAvailableDevices();

		PhysicalDevice picked = VK_NULL_HANDLE;
		int32_t max_score = -1;
		for (size_t i = 0; i < physicalDevices.size(); ++i) {
			int32_t score = 0;
			PhysicalDevice device = physicalDevices[i];
			QueueSupport queueSupport((VkPhysicalDevice)device, surfaces);
			if (!CheckPhysicalDeviceSupport(device, requiredFeatures, requiredQueues, queueSupport)) {
				continue;
			}
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties((VkPhysicalDevice)device, &properties);
			Log::Debug("Device: {} supports minimal requirements!", properties.deviceName);
			if (requiredQueues.Has(QueueType::PRESENT) && queueSupport.graphicsIndex == queueSupport.presentIndex && queueSupport.graphicsIndex != UINT32_MAX) {
				score += 100;
			}
			if (requiredQueues.Has(QueueType::COMPUTE) && queueSupport.computeIndex != queueSupport.graphicsIndex) {
				score += 100;
			}
			if (requiredQueues.Has(QueueType::TRANSFER) && (queueSupport.computeIndex != queueSupport.transferIndex || queueSupport.transferIndex != queueSupport.graphicsIndex)) {
				score += 100;
			}
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				score += 1000;
			}
			if (max_score < score) {
				picked = device;
				max_score = score;
			}
		}
		if (picked == VK_NULL_HANDLE) {
			Log::Fatal("{}", ERROR_PHYSICAL_DEVICE_NOT_FOUND);
		}
		return picked;
	}
	VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, const QueueSupport& support, Flags<DeviceFeature> features, Flags<QueueType> requiredQueues) {
		const char* swapchainExtension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		VkDeviceCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		if (requiredQueues.Has(QueueType::PRESENT)) {
			info.enabledExtensionCount = 1;
			info.ppEnabledExtensionNames = &swapchainExtension;
		}
		else {
			info.enabledExtensionCount = 0;
		}
		VkDeviceQueueCreateInfo queueCreateInfos[4];
		VkPhysicalDeviceFeatures enabled;
		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfos[4];
		float queuePriorityBuffer[4] = { queuePriority, queuePriority, queuePriority, queuePriority };
		uint32_t indexCount = GetQueueCreateInfos(support, requiredQueues, queueCreateInfos, queuePriorityBuffer);
		info.pQueueCreateInfos = queueCreateInfos;
		info.queueCreateInfoCount = indexCount;
		GetEnabledFeatures(physicalDevice, features, &enabled);
		info.pEnabledFeatures = &enabled;
#ifdef SGF_ENABLE_VALIDATION
		info.enabledLayerCount = 1;
		info.ppEnabledLayerNames = &VULKAN_MESSENGER_NAME;
#else
		info.enabledLayerCount = 0;
		info.ppEnabledLayerNames = nullptr;
#endif
		VkDevice device = VK_NULL_HANDLE;
		if (vkCreateDevice(physicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &device) != VK_SUCCESS) {
			Log::Fatal("{}", ERROR_LOGICAL_DEVICE_NOT_CREATED);
		}
		volkLoadDevice(device);
	}

	constexpr uint32_t GetQueueTypeIndex(QueueType queueFamily) {
		switch (queueFamily) {
		case QueueType::GRAPHICS:
			return s_GraphicsFamilyIndex;
		case QueueType::COMPUTE:
			return s_ComputeFamilyIndex;
		case QueueType::TRANSFER:
			return s_TransferFamilyIndex;
		case QueueType::PRESENT:
			return s_PresentFamilyIndex;
		}
	}

	constexpr uint32_t GetQueueTypeIndex(QueueType queueFamily) {
		switch (queueFamily) {
		case QueueType::GRAPHICS:
			return s_GraphicsFamilyIndex;
		case QueueType::COMPUTE:
			return s_ComputeFamilyIndex;
		case QueueType::TRANSFER:
			return s_TransferFamilyIndex;
		case QueueType::PRESENT:
			return s_PresentFamilyIndex;
		}
	}
	constexpr VkQueue GetQueue(QueueType queueFamily) {
		switch (queueFamily) {
		case QueueType::GRAPHICS:
			return s_GraphicsQueue;
		case QueueType::COMPUTE:
			return s_ComputeQueue;
		case QueueType::TRANSFER:
			return s_TransferQueue;
		case QueueType::PRESENT:
			return s_PresentQueue;
		}
	}
	VkSurfaceFormatKHR PickSurfaceFormat(VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat) {
		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(s_PhysicalDevice, surface, &format_count, nullptr);
		std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
		if (format_count != 0) {
			vkGetPhysicalDeviceSurfaceFormatsKHR(s_PhysicalDevice, (VkSurfaceKHR)surface, &format_count, surface_formats.data());
		}
		else {
			Log::Fatal("{}", ERROR_DEVICE_NO_SURFACE_SUPPORT);
		}
		for (const auto& available_format : surface_formats) {
			if (available_format.format == surfaceFormat.format && available_format.colorSpace == surfaceFormat.colorSpace)
			{
				return available_format;
			}
		}
		return surface_formats[0];
	}
	VkPresentModeKHR PickPresentMode(VkSurfaceKHR surface, VkPresentModeKHR requested) {
		uint32_t presentCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(s_PhysicalDevice, surface, &presentCount, nullptr);
		std::vector<VkPresentModeKHR> available(presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(s_PhysicalDevice, surface, &presentCount, available.data());
		for (const auto& mode : available) {
			if (mode == requested) {
				return requested;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}
	void PrepareSwapchainRetirement(Swapchain oldSwapchain) {
		SGF_ASSERT(oldSwapchain != nullptr);
		oldSwapchain->outOfDate = true;
		if (oldSwapchain->pImageViews != nullptr) {
			for (uint32_t i = 0; i < oldSwapchain->imageCount; ++i) {
				Destroy(oldSwapchain->pImageViews[i]);
			}
			oldSwapchain->pImageViews = nullptr;
		}
		if (oldSwapchain->pImages != nullptr) {
			delete[] oldSwapchain->pImages;
			oldSwapchain->pImages = nullptr;
		}
		oldSwapchain->imageCount = 0;
	}
	void CreateSwapchainInfo(VkSurfaceKHR surface, glm::uvec2 size, Flags<SwapchainCreate> flags, Flags<ImageUsage> imageUsage, VkSwapchainCreateInfoKHR* pInfo) {
		SGF_ASSERT(pInfo != nullptr);
		VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		surfaceFormat = PickSurfaceFormat(surface, surfaceFormat);
		auto& info = *pInfo;
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.pNext = nullptr;
		info.presentMode = flags.Has(SwapchainCreate::VSYNC) ? VK_PRESENT_MODE_FIFO_KHR : PickPresentMode(surface, VK_PRESENT_MODE_MAILBOX_KHR);
		info.imageFormat = surfaceFormat.format;
		info.imageColorSpace = surfaceFormat.colorSpace;
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_PhysicalDevice, surface, &capabilities);
		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
			imageCount = capabilities.maxImageCount;
		}
		info.minImageCount = imageCount;
		{
			auto e = size;
			info.imageExtent.width = std::min(capabilities.maxImageExtent.width, std::max(e.x, capabilities.minImageExtent.width));
			info.imageExtent.height = std::min(capabilities.maxImageExtent.height, std::max(e.y, capabilities.minImageExtent.height));
		}
		if (s_GraphicsFamilyIndex != s_PresentFamilyIndex) {
			SGF_ASSERT(s_GraphicsFamilyIndex != UINT32_MAX && s_PresentFamilyIndex != UINT32_MAX);
			SGF_ASSERT(s_GraphicsQueue != VK_NULL_HANDLE && s_PresentQueue != VK_NULL_HANDLE);
			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = 2;
			uint32_t queueFamilyIndices[] = { s_GraphicsFamilyIndex, s_PresentFamilyIndex };
			info.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 0;
			info.pQueueFamilyIndices = nullptr;
		}
		info.clipped = flags.Has(SwapchainCreate::CLIPPED) ? VK_TRUE : VK_FALSE;
		info.imageArrayLayers = 1;
		info.imageUsage = (VkImageUsageFlags)imageUsage;
		info.oldSwapchain = nullptr;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.preTransform = capabilities.currentTransform;
	}
	void FinalizeSwapchain(Swapchain swapchain, VkSurfaceKHR surface, const VkSwapchainCreateInfoKHR& info) {
		swapchain->surface = surface;
		swapchain->extent = info.imageExtent;
		swapchain->format = info.imageFormat;
		vkGetSwapchainImagesKHR(s_LogicalDevice, swapchain->handle, &swapchain->imageCount, nullptr);
		swapchain->pImages = new VkImage[swapchain->imageCount * 2];
		vkGetSwapchainImagesKHR(s_LogicalDevice, swapchain->handle, &swapchain->imageCount, swapchain->pImages);
		swapchain->pImageViews = (VkImageView*)&swapchain->pImages[swapchain->imageCount];
		for (uint32_t i = 0; i < swapchain->imageCount; ++i) {
			swapchain->pImageViews[i] = (VkImageView)CreateImageView2D((Image)swapchain->pImages[i], (Format)swapchain->format, ImageAspect::COLOR);
		}
		swapchain->outOfDate = false;
	}
	uint32_t FindMemoryIndex(uint32_t typeBits, Flags<MemoryProperty> flags) {
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(s_PhysicalDevice, &mem_properties);

		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
			if ((typeBits & (1 << i)) && (flags.Has(Flags<MemoryProperty>(mem_properties.memoryTypes[i].propertyFlags)))) {
				return i;
			}
		}
		Log::Error("{}", ERROR_UNSUPPORTED_MEMORY_TYPE);
		return UINT32_MAX;
	}
	void AllocateCommandBuffers(VkCommandPool pool, VkCommandBufferLevel level, uint32_t allocationCount, VkCommandBuffer* pBuffers) {
		VkCommandBufferAllocateInfo info;
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = nullptr;
		info.commandPool = pool;
		info.commandBufferCount = allocationCount;
		info.level = level;
		AllocateCommandBuffers(info, pBuffers);
	}
	void AllocateCommandBuffers(const VkCommandBufferAllocateInfo& info, VkCommandBuffer* pBuffers) {
		if (vkAllocateCommandBuffers(s_LogicalDevice, &info, pBuffers) != VK_SUCCESS) {
			Log::Fatal("{}", ERROR_ALLOCATE_COMMAND_BUFFERS);
		}
	}

	[[nodiscard]]
	constexpr uint64_t Mix64(uint64_t value) noexcept {
		value += 0x9E3779B97F4A7C15ull;
		value = (value ^ (value >> 30u)) * 0xBF58476D1CE4E5B9ull;
		value = (value ^ (value >> 27u)) * 0x94D049BB133111EBull;

		return value ^ (value >> 31u);
	}

	constexpr void HashCombine(uint64_t& seed, uint64_t value) noexcept {
		seed ^= Mix64(value + 0x9E3779B97F4A7C15ull + (seed << 6u) + (seed >> 2u));
	}

	template<typename VulkanHandle>
	[[nodiscard]]
	uint64_t HandleToUInt64(VulkanHandle handle) noexcept {
		if constexpr (std::is_pointer_v<VulkanHandle>) {
			return static_cast<uint64_t>(
				reinterpret_cast<uintptr_t>(handle));
		}
		else {
			return static_cast<uint64_t>(handle);
		}
	}
	[[nodiscard]]
	uint64_t CreateHash(const DescriptorSetLayout_T l) {
		uint64_t hash = 0x243F6A8885A308D3ull;
		Util::HashCombine(hash, static_cast<uint64_t>(l.createFlags));

		Util::HashCombine(hash, l.bindingCount);

		for (uint32_t index = 0; index < l.bindingCount; ++index) {
			const DescriptorSetLayoutBinding_T& binding = l.bindings[index];

			Util::HashCombine(hash, static_cast<uint64_t>(binding.vkBinding.binding));
			Util::HashCombine(hash, static_cast<uint64_t>(binding.vkBinding.descriptorCount));
			Util::HashCombine(hash, static_cast<uint64_t>(binding.vkBinding.descriptorType));
			Util::HashCombine(hash, static_cast<uint64_t>(binding.vkBinding.stageFlags));

			// Distinguishes no immutable samplers from an immutable array.
			if (binding.vkBinding.pImmutableSamplers) {
				for (uint32_t i = 0; i < binding.vkBinding.descriptorCount; ++i) {
					auto sampler = binding.vkBinding.pImmutableSamplers[i];
					Util::HashCombine(hash, Util::HandleToUInt64(sampler));
				}
			}
			else {
				Util::HashCombine(hash, static_cast<uint64_t>(0));
			}
		}
		return hash;
	}
}
#pragma endregion VULKAN_UTILS

#pragma region GPU_FUNCTIONS 
namespace SGF::GPU {
		uint32_t GetPhysicalDeviceCount() {
			uint32_t count;
			if (vkEnumeratePhysicalDevices(s_VulkanInstance, &count, nullptr) != VK_SUCCESS) {
				Log::Error("Failed to retrieve physical devices!");
				return 0;
			}
			return count;
		}
		void WaitIdle(uint64_t timeout) {
			if (vkDeviceWaitIdle(s_LogicalDevice) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_DEVICE_WAIT_IDLE);
			}
		}
		void WaitFence(Fence fence) {
			SGF_ASSERT(fence != VK_NULL_HANDLE);
			if (vkWaitForFences(s_LogicalDevice, 1, (VkFence*)&fence, VK_TRUE, UINT32_MAX) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_WAIT_FENCE);
			}
		}
		void WaitFences(const Fence* pFences, uint32_t count) {
			SGF_ASSERT(pFences != nullptr && count != 0);
			if (vkWaitForFences(s_LogicalDevice, count, (VkFence*)pFences, VK_TRUE, UINT32_MAX) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_WAIT_FENCE);
			}
		}
		void ResetFences(const Fence* pFences, uint32_t count) {
			SGF_ASSERT(pFences != nullptr && count != 0);
			if (vkResetFences(s_LogicalDevice, count, (VkFence*)pFences) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_RESET_FENCE);
			}
		}
		bool IsFenceSignaled(Fence fence) {
			return vkGetFenceStatus(s_LogicalDevice, (VkFence)fence) == VK_SUCCESS;
		}
#pragma region DEVICE_MEMORY
		
		SampleCount GetMaxSupportedSampleCount() {
			VkPhysicalDeviceProperties s_PhysicalDeviceDeviceProperties;
			vkGetPhysicalDeviceProperties(s_PhysicalDevice, &s_PhysicalDeviceDeviceProperties);

			Flags<SampleCount> counts = SampleCount(s_PhysicalDeviceDeviceProperties.limits.framebufferColorSampleCounts & s_PhysicalDeviceDeviceProperties.limits.framebufferDepthSampleCounts);
			if (counts & SampleCount::B64) { return SampleCount::B64; }
			if (counts & SampleCount::B32) { return SampleCount::B32; }
			if (counts & SampleCount::B16) { return SampleCount::B16; }
			if (counts & SampleCount::B8) { return SampleCount::B8; }
			if (counts & SampleCount::B4) { return SampleCount::B4; }
			if (counts & SampleCount::B2) { return SampleCount::B2; }
			return SampleCount::B1;
		}
		MemoryRequirements GetMemoryRequirements(Buffer buffer) {
			VkMemoryRequirements req;
			vkGetBufferMemoryRequirements(s_LogicalDevice, (VkBuffer)buffer, &req); 
			return { req.size, req.alignment };
		}
		MemoryRequirements GetMemoryRequirements(Image image) {
			VkMemoryRequirements req;
			vkGetImageMemoryRequirements(s_LogicalDevice, (VkImage)image, &req); 
			return { req.size, req.alignment };
		}
		void BindMemory(Memory memory, Buffer buffer, size_t offset) {
			if (vkBindBufferMemory(s_LogicalDevice, (VkBuffer)buffer, (VkDeviceMemory)memory, offset) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_BIND_DEVICE_MEMORY);
			}
		}
		void BindMemory(Memory memory, Image image, size_t offset) {
			if (vkBindImageMemory(s_LogicalDevice, (VkImage)image, (VkDeviceMemory)memory, offset) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_BIND_DEVICE_MEMORY);
			}
		}
		void* MapMemory(Memory memory, size_t size, size_t offset) {
			void* data;
			if (vkMapMemory(s_LogicalDevice, (VkDeviceMemory)memory, offset, size, 0, &data) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_MAP_DEVICE_MEMORY);
			}
			return data;
		}

		void UnmapMemory(Memory memory) {
			vkUnmapMemory(s_LogicalDevice, (VkDeviceMemory)memory);
		}

		Memory AllocateMemory(const VkMemoryAllocateInfo& info) {
			Memory mem;
			if (vkAllocateMemory(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, (VkDeviceMemory*)&mem) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_DEVICE_MEM_ALLOCATION);
			}
			return mem;
		}

		Memory AllocateMemory(const VkMemoryRequirements& req, Flags<MemoryProperty> flags) {
			VkMemoryAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.allocationSize = req.size;
			info.memoryTypeIndex = Util::FindMemoryIndex(req.memoryTypeBits, flags);
			return AllocateMemory(info);
		}
		Memory AllocateMemory(Buffer buffer, Flags<MemoryProperty> flags) {
			SGF_ASSERT(buffer != VK_NULL_HANDLE);
			VkMemoryRequirements req;
			vkGetBufferMemoryRequirements(s_LogicalDevice, (VkBuffer)buffer, &req);
			Memory mem = AllocateMemory(req, flags);
			if (vkBindBufferMemory(s_LogicalDevice, (VkBuffer)buffer, (VkDeviceMemory)mem, 0) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_BIND_DEVICE_MEMORY);
			}
			return mem;
		}
		size_t CalcOffset(const VkMemoryRequirements& prevReq, VkMemoryRequirements& objReq) {
			if (prevReq.size % objReq.alignment == 0) {
				return prevReq.size;
			} else {
				return prevReq.size + (objReq.alignment - (prevReq.size % objReq.alignment));
			}
		}
		Memory AllocateMemory(const Buffer* pBuffers, uint32_t bufferCount, Flags<MemoryProperty> flags) {
			SGF_ASSERT(bufferCount != 0);
			SGF_ASSERT(pBuffers != nullptr);
			VkMemoryRequirements req = {};
			VkMemoryRequirements bufReq;
			size_t* offsets = new size_t[bufferCount];
			for (uint32_t i = 0; i < bufferCount; ++i) {
				vkGetBufferMemoryRequirements(s_LogicalDevice, (VkBuffer)pBuffers[i], &bufReq);
				req.memoryTypeBits |= bufReq.memoryTypeBits;
				req.alignment = (bufReq.alignment < req.alignment ? req.alignment : bufReq.alignment);
				offsets[i] = CalcOffset(req, bufReq);
				req.size = offsets[i] + bufReq.size;
				SGF_ASSERT(offsets[i] % bufReq.alignment == 0);
			}
			req.size += ((req.alignment - req.size % req.alignment) ^ req.alignment);
			SGF_ASSERT(req.size % req.alignment == 0);
			Memory mem = AllocateMemory(req, flags);
			for (uint32_t i = 0; i < bufferCount; ++i) {
				if (vkBindBufferMemory(s_LogicalDevice, (VkBuffer)pBuffers[i], (VkDeviceMemory)mem, offsets[i]) != VK_SUCCESS) {
					Log::Fatal("{}", ERROR_BIND_DEVICE_MEMORY);
				}
			}
			delete[] offsets;
			return mem;
		}
		Memory AllocateMemory(Image image, Flags<MemoryProperty> flags) {
			SGF_ASSERT(image != VK_NULL_HANDLE);
			VkMemoryRequirements req;
			vkGetImageMemoryRequirements(s_LogicalDevice, (VkImage)image, &req);
			Memory mem = AllocateMemory(req, flags);
			if (vkBindImageMemory(s_LogicalDevice, (VkImage)image, (VkDeviceMemory)mem, 0) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_BIND_DEVICE_MEMORY);
			}
			return mem;
		}
		Memory AllocateMemory(const Image* pImages, uint32_t imageCount, Flags<MemoryProperty> flags) {
			SGF_ASSERT(imageCount != 0);
			SGF_ASSERT(pImages != nullptr);
			VkMemoryRequirements req = {};
			VkMemoryRequirements imageReq;
			size_t* offsets = new size_t[imageCount];
			for (uint32_t i = 0; i < imageCount; ++i) {
				vkGetImageMemoryRequirements(s_LogicalDevice, (VkImage)pImages[i], &imageReq);
				req.memoryTypeBits |= imageReq.memoryTypeBits;
				req.alignment = (imageReq.alignment < req.alignment ? req.alignment : imageReq.alignment);
				//size_t offset = ((imageReq.alignment - req.size % imageReq.alignment) ^ imageReq.alignment) + req.size;
				offsets[i] = CalcOffset(req, imageReq);
				req.size = offsets[i] + imageReq.size;
				SGF_ASSERT(offsets[i] % imageReq.alignment == 0);
			}
			req.size += ((req.alignment - req.size % req.alignment) ^ req.alignment);
			SGF_ASSERT(req.size % req.alignment == 0);
			Memory mem = AllocateMemory(req, flags);
			for (uint32_t i = 0; i < imageCount; ++i) {
				if (vkBindImageMemory(s_LogicalDevice, (VkImage)pImages[i], (VkDeviceMemory)mem, offsets[i]) != VK_SUCCESS) {
					Log::Fatal("{}", ERROR_BIND_DEVICE_MEMORY);
				}
			}
			delete[] offsets;
			return mem;
		}
		Memory AllocateMemory(const Buffer* pBuffers, uint32_t bufferCount, Image* pImages, uint32_t imageCount, Flags<MemoryProperty> flags) {
			SGF_ASSERT(imageCount != 0);
			SGF_ASSERT(pImages != nullptr);
			SGF_ASSERT(bufferCount != 0);
			SGF_ASSERT(pBuffers != nullptr);
			VkMemoryRequirements req = {};
			VkMemoryRequirements memReq;
			size_t* offsets = new size_t[bufferCount+imageCount];
			for (uint32_t i = 0; i < bufferCount; ++i) {
				vkGetBufferMemoryRequirements(s_LogicalDevice, (VkBuffer)pBuffers[i], &memReq);
				req.memoryTypeBits |= memReq.memoryTypeBits;
				req.alignment = (memReq.alignment < req.alignment ? req.alignment : memReq.alignment);
				offsets[i] = CalcOffset(req, memReq);
				//offsets[i] = ((memReq.alignment - req.size % memReq.alignment) ^ memReq.alignment) + req.size;
				req.size = offsets[i] + memReq.size;
				//req.size += offsets[i] + memReq.size;
				SGF_ASSERT(offsets[i] % memReq.alignment == 0);
			}
			for (uint32_t i = bufferCount; i < imageCount + bufferCount; ++i) {
				vkGetImageMemoryRequirements(s_LogicalDevice, (VkImage)pImages[i], &memReq);
				req.memoryTypeBits |= memReq.memoryTypeBits;
				req.alignment = (memReq.alignment < req.alignment ? req.alignment : memReq.alignment);
				//offsets[i] = ((memReq.alignment - req.size % memReq.alignment) ^ memReq.alignment) + req.size;
				offsets[i] = CalcOffset(req, memReq);
				//req.size += (req.size % req.alignment) + memReq.size;
				req.size = offsets[i] + memReq.size;
				SGF_ASSERT(offsets[i] % memReq.alignment == 0);
			}
			req.size += ((req.alignment - req.size % req.alignment) ^ req.alignment);
			SGF_ASSERT(req.size % req.alignment == 0);
			Memory mem = AllocateMemory(req, flags);
			for (uint32_t i = 0; i < bufferCount; ++i) {
				if (vkBindBufferMemory(s_LogicalDevice, (VkBuffer)pBuffers[i], (VkDeviceMemory)mem, offsets[i]) != VK_SUCCESS) {
					Log::Fatal("{}", ERROR_BIND_DEVICE_MEMORY);
				}
			}
			for (uint32_t i = bufferCount; i < imageCount + bufferCount; ++i) {
				if (vkBindImageMemory(s_LogicalDevice, (VkImage)pImages[i], (VkDeviceMemory)mem, offsets[i]) != VK_SUCCESS) {
					Log::Fatal("{}", ERROR_BIND_DEVICE_MEMORY);
				}
			}
			delete[] offsets;
			return mem;
		}
#pragma endregion DEVICE_MEMORY
#define CREATE_BUFFER_INFO(size, usage, flags) { \
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,\
		nullptr,\
		flags,\
		size,\
		usage,\
		VK_SHARING_MODE_EXCLUSIVE,\
		0,\
		nullptr };
constexpr void CreateDefaultImageInfo(VkImageCreateInfo* pInfo,const VkExtent3D& extent, uint32_t arraySize, Format format, Flags<ImageUsage> usage, uint32_t mipLevelCount, SampleCount samples, Flags<ImageCreate> flags) {
        pInfo->pNext = nullptr;
        pInfo->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        pInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        pInfo->arrayLayers = arraySize;
        pInfo->mipLevels = mipLevelCount;
        pInfo->tiling = VK_IMAGE_TILING_OPTIMAL;
        pInfo->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        pInfo->pQueueFamilyIndices = nullptr;
        pInfo->queueFamilyIndexCount = 0;
        pInfo->samples = (VkSampleCountFlagBits)samples;
        pInfo->extent = extent;
        pInfo->format = (VkFormat)format;
        pInfo->usage = (VkImageUsageFlags)usage.ToUnderlying();
        pInfo->flags = (VkImageCreateFlags)flags.ToUnderlying();
    }

#define SETUP_QUEUE_FAMILY_SHARING(info, indices, familyFlags) do { \
		info.pQueueFamilyIndices = indices;\
        if (familyFlags.Has(QueueType::GRAPHICS)) {\
            SGF_ASSERT(s_GraphicsQueue != VK_NULL_HANDLE);\
            indices[info.queueFamilyIndexCount] = s_GraphicsFamilyIndex;\
            info.queueFamilyIndexCount++;\
        }\
        if (familyFlags.Has(QueueType::COMPUTE)) {\
            SGF_ASSERT(s_ComputeQueue != VK_NULL_HANDLE);\
            indices[info.queueFamilyIndexCount] = s_ComputeFamilyIndex;\
			info.queueFamilyIndexCount++; \
		}\
		if (familyFlags.Has(QueueType::TRANSFER)) {\
			SGF_ASSERT(s_TransferQueue != VK_NULL_HANDLE); \
			indices[info.queueFamilyIndexCount] = s_TransferFamilyIndex; \
			info.queueFamilyIndexCount++;\
        }\
        info.sharingMode = (info.queueFamilyIndexCount > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;\
        } while(0)

		Buffer CreateBuffer(const VkBufferCreateInfo& info) {
			VkBuffer buffer;
			if (vkCreateBuffer(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &buffer) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_BUFFER);
			}
			TRACK_BUFFER(1);
			return (Buffer)buffer;
		}
		Buffer CreateBuffer(size_t size, Flags<BufferUsage> usage, Flags<BufferCreate> flags) {
			VkBufferCreateInfo info = CREATE_BUFFER_INFO(size, usage.ToUnderlying(), flags.ToUnderlying());
			return CreateBuffer(info);
		}
		Buffer CreateBufferShared(size_t size, Flags<BufferUsage> usage, Flags<QueueType> familyFlags, Flags<BufferCreate> flags) {
			uint32_t indices[4] = {};
			VkBufferCreateInfo info = CREATE_BUFFER_INFO(size, usage.ToUnderlying(), flags.ToUnderlying());
			SETUP_QUEUE_FAMILY_SHARING(info, indices, familyFlags);
			return CreateBuffer(info);
		}
#pragma region IMAGE
		Image CreateImage(const VkImageCreateInfo& info) {
			SGF_ASSERT(info.extent.width != 0);
			SGF_ASSERT(info.extent.height != 0);
			SGF_ASSERT(info.extent.depth != 0);
			SGF_ASSERT(info.sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
			Image image;
			if (vkCreateImage(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, (VkImage*)&image) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_IMAGE);
			}
			TRACK_CreateImage(1);
			return image;
		}
		Image CreateImage1D(uint32_t length, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<ImageCreate> flags) {
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, {length, 1, 1}, 1, format, usage, mipLevelCount, samples, flags);
			info.imageType = VK_IMAGE_TYPE_1D;
			return CreateImage(info);
		}
		Image CreateImage2D(uint32_t width, uint32_t height, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<ImageCreate> flags) {
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, {width, height, 1}, 1, format, usage, mipLevelCount, samples, flags);
			info.imageType = VK_IMAGE_TYPE_2D;
			return CreateImage(info);
		}
		Image CreateImage3D(uint32_t width, uint32_t height, uint32_t depth, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<ImageCreate> flags) {
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, {width, height, depth}, 1, format, usage, mipLevelCount, samples, flags);
			info.imageType = VK_IMAGE_TYPE_3D;
			return CreateImage(info);
		}
		Image CreateImageArray1D(uint32_t length, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<ImageCreate> flags) {
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, {length, 1, 1}, arraySize, format, usage, mipLevelCount, samples, flags);
			info.imageType = VK_IMAGE_TYPE_1D;
			return CreateImage(info);
		}
		Image CreateImageArray2D(uint32_t width, uint32_t height, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<ImageCreate> flags) {
			VkImageCreateInfo info{};
			CreateDefaultImageInfo(&info, {width, height, 1}, arraySize, format, usage, mipLevelCount, samples, flags);
			info.imageType = VK_IMAGE_TYPE_2D;
			return CreateImage(info);
		}
		Image CreateImageArray3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<ImageCreate> flags) {
			VkImageCreateInfo info{};
			CreateDefaultImageInfo(&info, { width, height, depth }, arraySize, format, usage, mipLevelCount, samples, flags);
			info.imageType = VK_IMAGE_TYPE_3D;
			return CreateImage(info);
		}
		Image CreateImage1DShared(uint32_t length, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<QueueType> queueFlags, Flags<ImageCreate> flags) {
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, { length, 1, 1 }, 1, format, usage, mipLevelCount, samples, flags);
			uint32_t indices[4] = {};
			info.imageType = VK_IMAGE_TYPE_1D;
			SETUP_QUEUE_FAMILY_SHARING(info, indices, queueFlags);
			return CreateImage(info);
		}
		Image CreateImage2DShared(uint32_t width, uint32_t height, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<QueueType> queueFlags, Flags<ImageCreate> flags) {
			SGF_ASSERT(s_GraphicsQueueCount != 0);
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, { width, height, 1 }, 1, format, usage, mipLevelCount, samples, flags);
			uint32_t indices[4] = {};
			info.imageType = VK_IMAGE_TYPE_2D;
			SETUP_QUEUE_FAMILY_SHARING(info, indices, queueFlags);
			return CreateImage(info);
		}
		Image CreateImage3DShared(uint32_t width, uint32_t height, uint32_t depth, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<QueueType> queueFlags, Flags<ImageCreate> flags) {
			SGF_ASSERT(s_GraphicsQueueCount != 0);
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, { width, height, depth }, 1, format, usage, mipLevelCount, samples, flags);
			uint32_t indices[4] = {};
			info.imageType = VK_IMAGE_TYPE_3D;
			SETUP_QUEUE_FAMILY_SHARING(info, indices, queueFlags);
			return CreateImage(info);
		}
		Image CreateImageArray1DShared(uint32_t length, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<QueueType> queueFlags, Flags<ImageCreate> flags) {
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, { length, 1, 1 }, arraySize, format, usage, mipLevelCount, samples, flags);
			uint32_t indices[4] = {};
			info.imageType = VK_IMAGE_TYPE_1D;
			SETUP_QUEUE_FAMILY_SHARING(info, indices, queueFlags);
			return CreateImage(info);
		}
		Image CreateImageArray2DShared(uint32_t width, uint32_t height, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<QueueType> queueFlags, Flags<ImageCreate> flags) {
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, { width, height, 1 }, arraySize, format, usage, mipLevelCount, samples, flags);
			uint32_t indices[4] = {};
			info.imageType = VK_IMAGE_TYPE_2D;
			SETUP_QUEUE_FAMILY_SHARING(info, indices, queueFlags);
			return CreateImage(info);
		}
		Image CreateImageArray3DShared(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, Format format, Flags<ImageUsage> usage, SampleCount samples, uint32_t mipLevelCount, Flags<QueueType> queueFlags, Flags<ImageCreate> flags) {
			VkImageCreateInfo info;
			CreateDefaultImageInfo(&info, { width, height, depth }, arraySize, format, usage, mipLevelCount, samples, flags);
			uint32_t indices[4] = {};
			info.imageType = VK_IMAGE_TYPE_3D;
			SETUP_QUEUE_FAMILY_SHARING(info, indices, queueFlags);
			return CreateImage(info);
		}
#define IMAGE_VIEW_CREATE_INFO(IMAGE,TYPE,FORMAT,ASPECT,MIP,LEVELS,BASE,COUNT) {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,nullptr,0, (VkImage)IMAGE,TYPE,(VkFormat)FORMAT,\
{VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY},\
{ASPECT.ToUnderlying(),MIP,LEVELS,BASE,COUNT}}
		ImageView CreateImageView(const VkImageViewCreateInfo& info) {
			VkImageView view;
			SGF_ASSERT(info.sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
			if (vkCreateImageView(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &view) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_IMAGE_VIEW);
			}
			TRACK_IMAGE_VIEW(1);
			return (ImageView)view;
		}
		ImageView CreateImageView1D(Image image, Format format, Flags<ImageAspect> imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) {
			VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO((VkImage)image, VK_IMAGE_VIEW_TYPE_1D, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
			return CreateImageView(info);
		}
		ImageView CreateImageView2D(Image image, Format format, Flags<ImageAspect> imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) {
			VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO((VkImage)image, VK_IMAGE_VIEW_TYPE_2D, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
			return CreateImageView(info);
		}

		ImageView CreateImageView3D(Image image, Format format, Flags<ImageAspect> imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) {
			VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_3D, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
			return CreateImageView(info);
		}
		ImageView CreateImageViewCube(Image image, Format format, Flags<ImageAspect> imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer) {
			VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_CUBE, format, imageAspect, mipLevel, levelCount, arrayLayer, 1);
			return CreateImageView(info);
		}
		ImageView CreateImageArrayView1D(Image image, Format format, Flags<ImageAspect> imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer, uint32_t arraySize) {
			VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_1D_ARRAY, format, imageAspect, mipLevel, levelCount, arrayLayer, arraySize);
			return CreateImageView(info);
		}
		ImageView CreateImageArrayView2D(Image image, Format format, Flags<ImageAspect> imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer, uint32_t arraySize) {
			VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_2D_ARRAY, format, imageAspect, mipLevel, levelCount, arrayLayer, arraySize);
			return CreateImageView(info);
		}
		ImageView CreateImageArrayViewCube(Image image, Format format, Flags<ImageAspect> imageAspect, uint32_t mipLevel, uint32_t levelCount, uint32_t arrayLayer, uint32_t arraySize) {
			VkImageViewCreateInfo info = IMAGE_VIEW_CREATE_INFO(image, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, format, imageAspect, mipLevel, levelCount, arrayLayer, arraySize);
			return CreateImageView(info);
		}
#pragma endregion IMAGE_CREATION
		Sampler CreateImageSampler(const VkSamplerCreateInfo& info) {
			VkSampler sampler;
			if (vkCreateSampler(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &sampler) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_SAMPLER);
			}
			TRACK_SAMPLER(1);
			return (Sampler)sampler;
		}
        Sampler CreateImageSampler(FilterType filterType = FilterType::NEAREST, SamplerMipmapMode mipmapMode = SamplerMipmapMode::LINEAR, SamplerAddressMode addressMode = SamplerAddressMode::CLAMP_TO_BORDER,
            float mipLodBias = 0.0f, float maxAnisotropy = 0.0f, CompareOp compareOp = CompareOp::ALWAYS,
            float minLod = 0.0f, float maxLod = 0.0f, BorderColor borderColor = BorderColor::FLOAT_OPAQUE_WHITE) { 
			VkSamplerCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.magFilter = (VkFilter)filterType;
			info.minFilter = (VkFilter)filterType;
			info.mipmapMode = (VkSamplerMipmapMode)mipmapMode;
			info.addressModeU = (VkSamplerAddressMode)addressMode;
			info.addressModeV = (VkSamplerAddressMode)addressMode;
			info.addressModeW = (VkSamplerAddressMode)addressMode;
			info.mipLodBias = mipLodBias;
			info.anisotropyEnable = maxAnisotropy > 0.0f ? VK_TRUE : VK_FALSE;
			info.maxAnisotropy = maxAnisotropy;
			info.compareEnable = compareOp != CompareOp::MAX_ENUM ? VK_TRUE : VK_FALSE;
			info.compareOp = (VkCompareOp)compareOp;
			info.minLod = minLod;
			info.maxLod = maxLod;
			info.borderColor = (VkBorderColor)borderColor;
			info.unnormalizedCoordinates = VK_FALSE; // coordinates are always normalized to [0,1] range
			return CreateImageSampler(info);
		}
		bool Present(Swapchain swapchain, Semaphore waitSemaphore) {
			SGF_ASSERT(swapchain != nullptr);
			VkResult result;
			VkPresentInfoKHR info{};
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			info.pResults = &result;
			info.pWaitSemaphores = (VkSemaphore*)&waitSemaphore;
			info.pWaitSemaphores = (waitSemaphore == nullptr) ? nullptr : (VkSemaphore*)&waitSemaphore;
			info.waitSemaphoreCount = (waitSemaphore == nullptr) ? 0 : 1;
			info.pSwapchains = &swapchain->handle;
			info.pImageIndices = &swapchain->imageIndex;
			info.swapchainCount = 1;
			if (vkQueuePresentKHR(s_PresentQueue, &info) != VK_SUCCESS || result != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_PRESENT_IMAGE);
				swapchain->outOfDate = true;
				return false;
			}
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
				swapchain->outOfDate = true;
			}
			return true;
		}
		void QueueWaitIdle(QueueType queueType, uint64_t timeout) {
			if (vkQueueWaitIdle(Util::GetQueue(queueType)) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_QUEUE_WAIT_IDLE);
			}
		}
		bool Present(const Swapchain* pSwapchains, uint32_t swapchainCount, const Semaphore* pWaitSemaphores, uint32_t waitSemaphoreCount) {
			VkPresentInfoKHR info{};
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			info.pNext = 0;
			std::vector<uint32_t> indices(swapchainCount);
			std::vector<VkResult> results(swapchainCount);
			std::vector<VkSwapchainKHR> swapchains;
			for (uint32_t i = 0; i < swapchainCount; ++i) {
				indices[i] = pSwapchains[i]->imageIndex;
				swapchains[i] = pSwapchains[i]->handle;
			}
			vkQueuePresentKHR(s_PresentQueue, &info);
			if (vkQueuePresentKHR(s_PresentQueue, &info) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_PRESENT_IMAGE);
				return false;
			}
			bool success = true;
			for (uint32_t i = 0; i < swapchainCount; ++i) {
				if (results[i] != VK_SUCCESS) {
					pSwapchains[i]->outOfDate = true;
					if (results[i] == VK_ERROR_OUT_OF_DATE_KHR || results[i] == VK_SUBOPTIMAL_KHR) {
						// do nothing
					}
					else {
						Log::Fatal("{}", ERROR_PRESENT_IMAGE);
					}
					success = false;
				}
			}
			return success;
		}

		Fence CreateFence() {
			SGF_ASSERT(s_LogicalDevice != VK_NULL_HANDLE);
			VkFenceCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			VkFence fence_r;
			if (vkCreateFence(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &fence_r) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_FENCE);
			}
			TRACK_FENCE(1);
			return (Fence)fence_r;
		}
		Fence CreateFenceSignaled() {
			SGF_ASSERT(s_LogicalDevice != VK_NULL_HANDLE);
			VkFenceCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			Fence fence_r;
			if (vkCreateFence(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, (VkFence*)&fence_r) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_FENCE);
			}
			TRACK_FENCE(1);
			return (Fence)fence_r;
		}
		Semaphore CreateSemaphore() {
			VkSemaphoreCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			Semaphore sem;
			if (vkCreateSemaphore(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, (VkSemaphore*)&sem) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_SEMAPHORE);
			}
			TRACK_SEMAPHORE(1);
			return sem;
		}
		void WaitFences(const Fence* pFences, uint32_t count, uint64_t timeout) {
			if (vkWaitForFences(s_LogicalDevice, count, (const VkFence*)pFences, VK_TRUE, timeout) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_WAIT_FENCE);
			}
		}

		QueryPool CreateQueryPool(QueryType queryType, uint32_t queryCount, Flags<QueryPipelineStatistic> pipelineStatistics, Flags<QueryPoolOptions> options) {
			VkQueryPool pool;
			VkQueryPoolCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
			info.pNext = nullptr;
			info.pipelineStatistics = pipelineStatistics;
			info.queryCount = queryCount;
			info.queryType = (VkQueryType)queryType;
			info.flags = options;
			if (vkCreateQueryPool(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &pool) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_QUERY_POOL);
			}
			return (QueryPool)pool;
		}

		ShaderModule CreateShaderModule(const VkShaderModuleCreateInfo& info) {
			VkShaderModule shaderModule;
			if (vkCreateShaderModule(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &shaderModule) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_SHADER_MODULE);
			}
			TRACK_SHADER_MODULE(1);
			return (ShaderModule)shaderModule;
		}
		ShaderModule CreateShaderModule(const void* code, size_t byteSize) {
			SGF_ASSERT(code != nullptr && "data cannot be null!");
			SGF_ASSERT(byteSize != 0 && "data size cannot be zero!");
			VkShaderModuleCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.codeSize = byteSize;
			info.pCode = (uint32_t*)code;
			return CreateShaderModule(info);
		}
		ShaderModule CreateShaderModule(const char* filename) {
			auto code = File::LoadBinary(filename);
			SGF_ASSERT(code.size() > 0 && "failed to open file!");
			return CreateShaderModule(code.data(), code.size());
		}
		ShaderModule CreateShaderModule(const std::string& filename) {
			return CreateShaderModule(filename.c_str());
		}
		PipelineLayout CreatePipelineLayout(const VkPipelineLayoutCreateInfo& info) {
			VkPipelineLayout layout;
			if (vkCreatePipelineLayout(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &layout) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_PIPELINE_LAYOUT);
			}
			TRACK_PIPELINE_LAYOUT(1);
			return (PipelineLayout)layout;
		}
		PipelineLayout CreatePipelineLayout(const DescriptorSetLayout* pLayouts, uint32_t descriptorLayoutCount, const PushConstantRange* pPushConstantRanges, uint32_t pushConstantCount) {
			VkPipelineLayoutCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.setLayoutCount = descriptorLayoutCount;
			info.pSetLayouts = (VkDescriptorSetLayout*)pLayouts;
			info.pushConstantRangeCount = pushConstantCount;
			info.pPushConstantRanges = (VkPushConstantRange*)pPushConstantRanges;
			return CreatePipelineLayout(info);
		}
		GraphicsPipeline CreatePipeline(const VkGraphicsPipelineCreateInfo& info) {
			GraphicsPipeline pipeline;
			if (vkCreateGraphicsPipelines(s_LogicalDevice, VK_NULL_HANDLE, 1, &info, VULKAN_ALLOCATION_CALLBACKS, (VkPipeline*)&pipeline) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_RENDER_PIPELINE);
			}
			TRACK_PIPELINE(1);
			return pipeline;
		}
		ComputePipeline CreatePipeline(const VkComputePipelineCreateInfo& info) {
			ComputePipeline pipeline;
			if (vkCreateComputePipelines(s_LogicalDevice, VK_NULL_HANDLE, 1, &info, VULKAN_ALLOCATION_CALLBACKS, (VkPipeline*)&pipeline) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_COMPUTE_PIPELINE);
			}
			TRACK_PIPELINE(1);
			return pipeline;
		}
		RenderPassBuilder CreateRenderPass() {
			RenderPassBuilder_T* builder = new RenderPassBuilder_T();
			RenderPassBuilder builder = *(RenderPassBuilder*)builder;
			return *(RenderPassBuilder*)&builder;
		}

		Framebuffer CreateFramebuffer(const VkFramebufferCreateInfo& info) {
			VkFramebuffer framebuffer;
			if (vkCreateFramebuffer(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &framebuffer) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_FRAMEBUFFER);
			}
			TRACK_FRAMEBUFFER(1);
			return (Framebuffer)framebuffer;
		}
		Framebuffer CreateFramebuffer(RenderPass renderPass, ImageView* pAttachments, uint32_t attachmentCount, uint32_t width, uint32_t height, uint32_t layerCount) {
			VkFramebufferCreateInfo info{};
			info.attachmentCount = attachmentCount;
			info.renderPass = (VkRenderPass)renderPass;
			info.pAttachments = (VkImageView*)pAttachments;
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.width = width;
			info.height = height;
			info.layers = layerCount;
			return CreateFramebuffer(info);
		}
		RenderPass CreateRenderPass(const VkRenderPassCreateInfo& info) {
			VkRenderPass renderPass;
			if (vkCreateRenderPass(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &renderPass) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_RENDER_PASS);
			}
			TRACK_RENDER_PASS(1);
			return (RenderPass)renderPass;
		}
		RenderPass CreateRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attachmentCount, VkSubpassDescription* pSubpasses, uint32_t subpassCount, VkSubpassDependency* pDependencies, uint32_t dependenfyCount) {
			VkRenderPassCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = attachmentCount;
			info.pAttachments = pAttachments;
			info.subpassCount = subpassCount;
			info.pSubpasses = pSubpasses;
			info.dependencyCount = dependenfyCount;
			info.pDependencies = pDependencies;
			return CreateRenderPass(info);
		}
		RenderPass CreateRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attCount, VkSubpassDescription* pSubpasses, uint32_t subpassCount) {
			SGF_ASSERT(attCount > 0);
			std::vector<VkSubpassDependency> dependencies(subpassCount);
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcAccessMask = 0;
			dependencies[0].dstAccessMask = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].dstStageMask = 0;
				
			if (pSubpasses[0].colorAttachmentCount != 0) {
				dependencies[0].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependencies[0].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependencies[0].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			if (pSubpasses[0].pDepthStencilAttachment != nullptr) {
				dependencies[0].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				dependencies[0].dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				dependencies[0].srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			if (pSubpasses[0].inputAttachmentCount != 0) {
				dependencies[0].dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				dependencies[0].dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependencies[0].srcStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			for (uint32_t i = 1; i < subpassCount; ++i) {
				dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				dependencies[i].srcSubpass = i - 1;
				dependencies[i].dstSubpass = i;
				dependencies[i].srcAccessMask = 0;
				dependencies[i].dstAccessMask = 0;
				dependencies[i].srcStageMask = 0;
				dependencies[i].dstStageMask = 0;
				for (uint32_t j = 0; j < pSubpasses[i].colorAttachmentCount; ++j) {
					const auto& dstAtt = pSubpasses[i].pColorAttachments[j];
					for (uint32_t k = 0; k < pSubpasses[i - 1].colorAttachmentCount; ++k) {
						const auto& srcAtt = pSubpasses[i - 1].pColorAttachments[k];
						if (srcAtt.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						}
						if (pSubpasses[i - 1].pResolveAttachments != nullptr) {
							const auto& srcAttRes = pSubpasses[i - 1].pResolveAttachments[k];
							if (srcAttRes.attachment == dstAtt.attachment) {
								dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
								dependencies[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
								dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
								dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							}
						}
					}
					for (uint32_t k = 0; k < pSubpasses[i - 1].inputAttachmentCount; ++k) {
						const auto& srcAtt = pSubpasses[i - 1].pInputAttachments[k];
						if (srcAtt.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						}
					}
					if (pSubpasses[i - 1].pDepthStencilAttachment != nullptr) {
						const auto& srcAtt = pSubpasses[i - 1].pDepthStencilAttachment[0];
						if (srcAtt.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						}
					}
				}
				for (uint32_t j = 0; j < pSubpasses[i].inputAttachmentCount; ++j) {
					const auto& dstAtt = pSubpasses[i].pInputAttachments[j];
					for (uint32_t k = 0; k < pSubpasses[i - 1].colorAttachmentCount; ++k) {
						const auto& srcAtt = pSubpasses[i - 1].pColorAttachments[k];
						if (srcAtt.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						}
						if (pSubpasses[i - 1].pResolveAttachments != nullptr) {
							const auto& srcAttRes = pSubpasses[i - 1].pResolveAttachments[k];
							if (srcAttRes.attachment == dstAtt.attachment) {
								dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
								dependencies[i].dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
								dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
								dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
							}
						}
					}
					if (pSubpasses[i - 1].pDepthStencilAttachment != nullptr) {
						const auto& srcAtt = pSubpasses[i - 1].pDepthStencilAttachment[0];
						if (srcAtt.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						}
					}
				}
				if (pSubpasses[i].pDepthStencilAttachment != nullptr) {
					const auto& dstAtt = pSubpasses[i].pDepthStencilAttachment[0];
					for (uint32_t k = 0; k < pSubpasses[i - 1].colorAttachmentCount; ++k) {
						const auto& srcAtt = pSubpasses[i - 1].pColorAttachments[k];
						if (srcAtt.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						}
						if (pSubpasses[i - 1].pResolveAttachments != nullptr) {
							const auto& srcAttRes = pSubpasses[i - 1].pResolveAttachments[k];
							if (srcAttRes.attachment == dstAtt.attachment) {
								dependencies[i].srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
								dependencies[i].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
								dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
								dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
							}
						}
					}
					if (pSubpasses[i - 1].pDepthStencilAttachment != nullptr) {
						const auto& srcAtt = pSubpasses[i - 1].pDepthStencilAttachment[0];
						if (srcAtt.attachment == dstAtt.attachment) {
							dependencies[i].srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
							dependencies[i].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
							dependencies[i].srcStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
							dependencies[i].dstStageMask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						}
					}
				}
			}
			return CreateRenderPass(pAttachments, attCount, pSubpasses, subpassCount, dependencies.data(), dependencies.size());
		}
		
		CommandPool CreateCommandPool(const VkCommandPoolCreateInfo& info) {
			SGF_ASSERT(sizeof(VkCommandPool) == sizeof(CommandPool));
			VkCommandPool pool;
			if (vkCreateCommandPool(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &pool) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_COMMAND_POOL);
			}
			return *((CommandPool*)&pool);
		}
		CommandPool CreateCommandPool(QueueType queueFamily, Flags<CommandPoolCreate> flags) {
			VkCommandPoolCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = flags;
			info.queueFamilyIndex = Util::GetQueueTypeIndex(queueFamily);
			TRACK_COMMAND_POOL(1);
			return CreateCommandPool(info);
		}
		VkDescriptorSetLayout CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info) {
			VkDescriptorSetLayout layout;
			if (vkCreateDescriptorSetLayout(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &layout) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_DESCRIPTOR_LAYOUT);
			}
			return layout;
		}
		static_assert(sizeof(DescriptorSetBinding) == sizeof(VkDescriptorSetLayoutBinding));
		static_assert(sizeof(VkDescriptorPoolSize) == sizeof(DescriptorPoolSize));
		VkDescriptorSetLayout CreateDescriptorSetLayout(const DescriptorSetLayout_T& layout) {
			VkDescriptorSetLayoutCreateInfo info;
			static_assert(sizeof(VkDescriptorSetLayoutBinding) == sizeof(DescriptorSetLayoutBinding_T));
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.pNext = nullptr;
			info.pBindings = (const VkDescriptorSetLayoutBinding*)layout.bindings.data();
			info.bindingCount = layout.bindingCount;
			info.flags = layout.createFlags;
			return CreateDescriptorSetLayout(info);
		}
		DescriptorSetLayout GetDescriptorSetLayout(const DescriptorSetBinding* pBindings, uint32_t bindingCount, Flags<DescriptorSetLayoutCreate> flags) {
			SGF_ASSERT(bindingCount <= DescriptorSetLayout::MAX_BINDINGS);
			DescriptorSetLayout_T layout(pBindings, bindingCount, flags);
			uint64_t hash = Util::CreateHash(layout);
			uint64_t index = UINT64_MAX;
			auto it = s_AllocatedDescriptorMap.find(hash);
			if (it != s_AllocatedDescriptorMap.end()) {
				index = static_cast<uint64_t>(it->second);
				while (!s_DescriptorSetLayouts[index].IsEqual(layout)) {
					auto& l = s_DescriptorSetLayouts[index];
					if (l.HasNext()) {
						index = static_cast<uint64_t>(l.hashCollisionIndex);
					}
					else {
						layout.layout = CreateDescriptorSetLayout(layout);
						s_DescriptorSetLayouts.emplace_back(std::move(layout));
						l.hashCollisionIndex = static_cast<uint32_t>(s_DescriptorSetLayouts.size() - 1);
						break;
					}
				}
			} else {
				layout.layout = CreateDescriptorSetLayout(layout);
				s_DescriptorSetLayouts.emplace_back(std::move(layout));
				index = static_cast<uint64_t>(s_DescriptorSetLayouts.size() - 1);
				s_AllocatedDescriptorMap.insert({ hash, static_cast<uint32_t>(index) });
			}
			return *(DescriptorSetLayout*)&index;
		}
		void ClearDescriptorSetLayouts() {
			s_DescriptorSetLayouts.clear();
			s_AllocatedDescriptorMap.clear();
			s_DescriptorSetLayouts.shrink_to_fit();
		}
		DescriptorPool CreateDescriptorPool(const VkDescriptorPoolCreateInfo& info) {
			SGF_ASSERT(info.sType == VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
			SGF_ASSERT(info.pPoolSizes != nullptr);
			SGF_ASSERT(info.poolSizeCount != 0);
			VkDescriptorPool pool;
			if (vkCreateDescriptorPool(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &pool) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_DESCRIPTOR_POOL);
			}
			TRACK_DESCRIPTOR_POOL(1);
			return *(DescriptorPool*)&pool;
		}
		DescriptorPool CreateDescriptorPool(DescriptorSetLayout layout, uint32_t maxSets, Flags<DescriptorPoolCreate> flags) {
			VkDescriptorPoolCreateInfo info;
			info.poolSizeCount = layout.GetBindingCount();
			std::vector<VkDescriptorPoolSize> poolSizes(info.poolSizeCount);
			auto bindings = layout.GetBindings();
			for (uint32_t i = 0; i < info.poolSizeCount; ++i) {
				poolSizes[i].descriptorCount = bindings[i].descriptorCount;
				poolSizes[i].type = (VkDescriptorType)bindings[i].descriptorType;
			}
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			info.flags = flags;
			info.pNext = nullptr;
			info.pPoolSizes = poolSizes.data();
			info.maxSets = maxSets;
			return CreateDescriptorPool(info);
		}
		void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const DescriptorBufferInfo* pBufferInfos, uint32_t descriptorCount) {
			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = (VkDescriptorSet)dstSet;
			write.dstBinding = dstBinding;
			write.dstArrayElement = dstArrayElement;
			write.descriptorCount = descriptorCount;
			write.descriptorType = (VkDescriptorType)descriptorType;
			write.pBufferInfo = (const VkDescriptorBufferInfo*)pBufferInfos;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;
			vkUpdateDescriptorSets(s_LogicalDevice, 1, &write, 0, nullptr);
		}
		void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const DescriptorImageInfo* pImageInfos, uint32_t descriptorCount) {
			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = (VkDescriptorSet)dstSet;
			write.dstBinding = dstBinding;
			write.dstArrayElement = dstArrayElement;
			write.descriptorCount = descriptorCount;
			write.descriptorType = (VkDescriptorType)descriptorType;
			write.pBufferInfo = nullptr;
			write.pImageInfo = (const VkDescriptorImageInfo*)pImageInfos;
			write.pTexelBufferView = nullptr;
			vkUpdateDescriptorSets(s_LogicalDevice, 1, &write, 0, nullptr);
		}
		void UpdateDescriptor(DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, DescriptorType descriptorType, const BufferView* pBufferViews, uint32_t descriptorCount) {
			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = (VkDescriptorSet)dstSet;
			write.dstBinding = dstBinding;
			write.dstArrayElement = dstArrayElement;
			write.descriptorCount = descriptorCount;
			write.descriptorType = (VkDescriptorType)descriptorType;
			write.pBufferInfo = nullptr;
			write.pImageInfo = nullptr;
			write.pTexelBufferView = (const VkBufferView*)pBufferViews;
			vkUpdateDescriptorSets(s_LogicalDevice, 1, &write, 0, nullptr);
		}
		DescriptorPool CreateDescriptorPool(uint32_t maxSets, std::vector<DescriptorPoolSize>& poolSizes, Flags<DescriptorPoolCreate> flags) {
			return CreateDescriptorPool(maxSets, poolSizes.data(), (uint32_t)poolSizes.size(), flags);
		}
		inline bool AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& info, DescriptorSet* pDescriptorSets) {
			VkResult result = vkAllocateDescriptorSets(s_LogicalDevice, &info, (VkDescriptorSet*)pDescriptorSets);
			if (result != VK_SUCCESS) {
				if (result != VK_ERROR_OUT_OF_POOL_MEMORY) {
					Log::Error("{} {}", ERROR_ALLOCATE_DESCRIPTOR_SETS, (uint32_t)result);
				}
				return false;
			}
			return true;
		}
		DescriptorSet AllocateDescriptorSet(DescriptorPool pool, DescriptorSetLayout descriptorSetLayout) {
			DescriptorSet set;
			VkDescriptorSetAllocateInfo info;
			VkDescriptorSetLayout layout = static_cast<VkDescriptorSetLayout>(descriptorSetLayout.GetNativeHandle());
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.descriptorPool = *(VkDescriptorPool*)&pool;
			info.pSetLayouts = &layout;
			info.descriptorSetCount = 1;
			info.pNext = nullptr;
			return (AllocateDescriptorSets(info, &set)) ? set : nullptr;
		}
		bool AllocateDescriptorSets(DescriptorPool pool, DescriptorSetLayout* pSetLayouts, uint32_t setCount, DescriptorSet* pDescriptorSets) {
			std::vector<VkDescriptorSetLayout> layouts(setCount);
			for (uint32_t i = 0; i < setCount; ++i) {
				layouts[i] = static_cast<VkDescriptorSetLayout>(pSetLayouts[i].GetNativeHandle());
			}
			VkDescriptorSetAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.descriptorPool = *(VkDescriptorPool*)&pool;
			info.pSetLayouts = layouts.data();
			info.descriptorSetCount = setCount;
			info.pNext = nullptr;
			return AllocateDescriptorSets(info, pDescriptorSets);
		}
		Format GetSupportedFormat(const Format* pCandidates, uint32_t candidateCount, Flags<FormatFeature> features, ImageTiling tiling) {
			SGF_ASSERT(tiling == VK_IMAGE_TILING_LINEAR || tiling == VK_IMAGE_TILING_OPTIMAL);
			for (uint32_t i = 0; i < candidateCount; i++) {
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(s_PhysicalDevice, (VkFormat)pCandidates[i], &props);

				if ((VkImageTiling)tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
					return pCandidates[i];
				}
				else if ((VkImageTiling)tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
					return pCandidates[i];
				}
			}
			return Format::MAX_ENUM;
		}
		uint32_t AcquireNextImage(Swapchain swapchain, uint64_t timeout, Semaphore signalSemaphore, Fence signalFence) {
			uint32_t imageIndex;
			if (vkAcquireNextImageKHR(s_LogicalDevice, swapchain->handle, timeout, (VkSemaphore)signalSemaphore, (VkFence)signalFence, &imageIndex) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_ACQUIRE_NEXT_IMAGE);
				return UINT32_MAX;
			}
			return imageIndex;
		}
		Swapchain CreateSwapchain(WindowHandle windowHandle, Flags<SwapchainCreate> flags, Flags<ImageUsage> imageUsage) {
			auto size = windowHandle.GetFramebufferSize();
			VkSurfaceKHR surface = Util::CreateSurface(windowHandle);
			if (surface == VK_NULL_HANDLE) {
				Log::Fatal("{}", ERROR_CREATE_SURFACE);
				return nullptr;
			}
			VkSwapchainCreateInfoKHR info;
			Util::CreateSwapchainInfo(surface, windowHandle.GetFramebufferSize(), flags, imageUsage, &info);
			VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
			if (vkCreateSwapchainKHR(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &vkSwapchain) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_SWAPCHAIN);
				return nullptr;
			}
			TRACK_SWAPCHAIN(1);
			Swapchain swapchain = new Swapchain_T;
			swapchain->handle = vkSwapchain;
			Util::FinalizeSwapchain(swapchain, surface, info);
			return swapchain;
		}
		Swapchain RecreateSwapchain(Swapchain oldSwapchain, glm::uvec2 size, Flags<SwapchainCreate> flags, Flags<ImageUsage> imageUsage) {
			Util::PrepareSwapchainRetirement(oldSwapchain);
			VkSurfaceKHR surface = oldSwapchain->surface;
			VkSwapchainCreateInfoKHR info;
			Util::CreateSwapchainInfo(surface, size, flags, imageUsage, &info);
			info.oldSwapchain = oldSwapchain->handle;
			VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
			if (vkCreateSwapchainKHR(s_LogicalDevice, &info, VULKAN_ALLOCATION_CALLBACKS, &vkSwapchain) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_CREATE_SWAPCHAIN);
				vkDestroySwapchainKHR(s_LogicalDevice, info.oldSwapchain, VULKAN_ALLOCATION_CALLBACKS);
				delete[] oldSwapchain;
				return nullptr;
			}
			Swapchain swapchain = oldSwapchain;
			swapchain->handle = vkSwapchain;
			Util::FinalizeSwapchain(swapchain, surface, info);
			return swapchain;
		}
		bool SwapchainOutOfDate(Swapchain swapchain) {
			return swapchain->outOfDate;
		}
		const Image* GetSwapchainImages(Swapchain swapchain, uint32_t* pCount) {
			SGF_ASSERT(swapchain != nullptr);
			*pCount = swapchain->imageCount;
			return (swapchain->imageCount != 0) ? (Image*)swapchain->pImages : nullptr;
		}
		const ImageView* GetSwapchainImageViews(Swapchain swapchain, uint32_t* pCount) {
			SGF_ASSERT(swapchain != nullptr);
			*pCount = swapchain->imageCount;
			return (swapchain->imageCount != 0) ? (ImageView*)swapchain->pImageViews : nullptr;
		}
#pragma region DESTRUCTORS
		void Destroy(Fence fence) {
			SGF_ASSERT(fence != VK_NULL_HANDLE);
			vkDestroyFence(s_LogicalDevice, (VkFence)fence, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_FENCE(-1);
		}
		void Destroy(Semaphore semaphore) {
			SGF_ASSERT(semaphore != VK_NULL_HANDLE);
			vkDestroySemaphore(s_LogicalDevice, (VkSemaphore)semaphore, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_SEMAPHORE(-1);
		}
		void Destroy(Buffer buffer) {
			SGF_ASSERT(buffer != VK_NULL_HANDLE);
			vkDestroyBuffer(s_LogicalDevice, (VkBuffer)buffer, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_BUFFER(-1);
		}
		void Destroy(Image image) {
			SGF_ASSERT(image != VK_NULL_HANDLE);
			vkDestroyImage(s_LogicalDevice, (VkImage)image, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_CreateImage(-1);
		}
		void Destroy(ImageView imageView) {
			SGF_ASSERT(imageView != VK_NULL_HANDLE);
			vkDestroyImageView(s_LogicalDevice, (VkImageView)imageView, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_IMAGE_VIEW(-1);
		}
		void Destroy(Framebuffer framebuffer) {
			SGF_ASSERT(framebuffer != VK_NULL_HANDLE);
			vkDestroyFramebuffer(s_LogicalDevice, (VkFramebuffer)framebuffer, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_FRAMEBUFFER(-1);
		}
		void Destroy(RenderPass renderPass) {
			SGF_ASSERT(renderPass != VK_NULL_HANDLE);
			vkDestroyRenderPass(s_LogicalDevice, (VkRenderPass)renderPass, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_RENDER_PASS(-1);
		}
		inline void Destroy(VkPipeline pipeline) {
			SGF_ASSERT(pipeline != VK_NULL_HANDLE);
			vkDestroyPipeline(s_LogicalDevice, (VkPipeline)pipeline, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_PIPELINE(-1);
		}
		void Destroy(GraphicsPipeline pipeline) {
			Destroy((VkPipeline)pipeline);
		}
		void Destroy(ComputePipeline pipeline) {
			Destroy((VkPipeline)pipeline);
		}
		void Destroy(RayTracingPipeline pipeline) {
			Destroy((VkPipeline)pipeline);
		}
		void Destroy(PipelineLayout pipelineLayout) {
			SGF_ASSERT(pipelineLayout != VK_NULL_HANDLE);
			vkDestroyPipelineLayout(s_LogicalDevice, (VkPipelineLayout)pipelineLayout, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_PIPELINE_LAYOUT(-1);
		}
		/*
		void Destroy(DescriptorSetLayout descriptorSetLayout) {
			SGF_ASSERT(descriptorSetLayout != VK_NULL_HANDLE);
			static_assert(false && "TODO");
			vkDestroyDescriptorSetLayout(s_LogicalDevice, (VkDescriptorSetLayout)descriptorSetLayout, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_DESCRIPTOR_SET_LAYOUT(-1);
		}
		*/
		void Destroy(DescriptorPool descriptorPool) {
			SGF_ASSERT(descriptorPool != VK_NULL_HANDLE);
			vkDestroyDescriptorPool(s_LogicalDevice, *(VkDescriptorPool*)&descriptorPool, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_DESCRIPTOR_POOL(-1);
		}
		void Destroy(Memory memory) {
			SGF_ASSERT(memory != VK_NULL_HANDLE);
			vkFreeMemory(s_LogicalDevice, (VkDeviceMemory)memory, VULKAN_ALLOCATION_CALLBACKS);
			Log::Info("Freed device memory");
			TRACK_DEVICE_MEMORY(-1);
		}
		void Destroy(CommandPool commandPool) {
			SGF_ASSERT(commandPool != VK_NULL_HANDLE);
			vkDestroyCommandPool(s_LogicalDevice, (VkCommandPool)commandPool.GetHandle(), VULKAN_ALLOCATION_CALLBACKS);
			TRACK_COMMAND_POOL(-1);
		}
		void Destroy(Sampler sampler) {
			SGF_ASSERT(sampler != VK_NULL_HANDLE);
			vkDestroySampler(s_LogicalDevice, (VkSampler)sampler, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_SAMPLER(-1);
		}
		void Destroy(Swapchain swapchain) {
			SGF_ASSERT(swapchain != VK_NULL_HANDLE);
			auto& swapchainData = *swapchain;
			Util::PrepareSwapchainRetirement(swapchain);
			vkDestroySwapchainKHR(s_LogicalDevice, swapchainData.handle, VULKAN_ALLOCATION_CALLBACKS);
			vkDestroySurfaceKHR(s_VulkanInstance, swapchainData.surface, VULKAN_ALLOCATION_CALLBACKS);
			delete swapchain;
			TRACK_SWAPCHAIN(-1);
		}
		void Destroy(ShaderModule module) {
			SGF_ASSERT(module != VK_NULL_HANDLE);
			vkDestroyShaderModule(s_LogicalDevice, (VkShaderModule)module, VULKAN_ALLOCATION_CALLBACKS);
			TRACK_SHADER_MODULE(-1);
		}
#pragma endregion DESTRUCTORS
		bool InitializeAPI(Flags<DriverFeature> flags)
		{
			volkInitialize();
			std::vector<const char*> extensions;
			// Vulkan instance:
			if (flags.Has(DriverFeature::PRESENTATION_SUPPORT)) {
				if (!glfwVulkanSupported()) {
					Log::Error("vulkan not supported!");
					return false;
				}
				uint32_t instance_extension_count;
				const char** instance_extensions = glfwGetRequiredInstanceExtensions(&instance_extension_count);
				if (instance_extensions == nullptr) {
					Log::Fatal("missing support for required glfw extensions!");
					return false;
				}
				extensions.assign(instance_extensions, instance_extensions + instance_extension_count);
			}
			VkApplicationInfo app_info;
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pNext = nullptr;
			app_info.apiVersion = VK_API_VERSION_1_0;
			if (flags.Has(DriverFeature::NEWEST_API_VERSION)) {
				auto vkEnumerateInstanceVersion =
					reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
						vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
				if (vkEnumerateInstanceVersion)
				{
					vkEnumerateInstanceVersion(&app_info.apiVersion);
				}
			}
			app_info.pEngineName = SGF_ENGINE_NAME;
			app_info.engineVersion = SGF_ENGINE_VERSION;
			app_info.pApplicationName = SGF_APP_NAME;
			app_info.applicationVersion = SGF_APP_VERSION;
			VkInstanceCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			info.pApplicationInfo = &app_info;
			info.flags = 0; 
			info.pNext = nullptr;
	#ifdef SGF_ENABLE_VALIDATION
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			uint32_t layer_count;
			vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
			std::vector<VkLayerProperties> layers(layer_count);
			vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
			bool debug_support = false;
			Log::Debug("{} vulkan layers available: ", layer_count);
			for (uint32_t i = 0; i < layer_count; ++i) {
				Log::Debug("{}", layers[i].layerName);
				if (strcmp(VULKAN_MESSENGER_NAME, layers[i].layerName) == 0) {
					debug_support = true;
					break;
				}
			}
			if (debug_support) {
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				info.enabledLayerCount = 1;
				info.ppEnabledLayerNames = &VULKAN_MESSENGER_NAME;
				Log::Info("validation enabled");
			} else {
				Log::Info("validation layer requested but not available, continuing without validation!");
				info.enabledLayerCount = 0;
				info.ppEnabledLayerNames = nullptr;
			}
	#else
			info.enabledLayerCount = 0;
			info.ppEnabledLayerNames = nullptr;
	#endif
			info.enabledExtensionCount = (uint32_t)extensions.size();
			info.ppEnabledExtensionNames = extensions.data();
			if (vkCreateInstance(&info, VULKAN_ALLOCATION_CALLBACKS, &s_VulkanInstance) != VK_SUCCESS) {
				Log::Fatal("Failed to create vulkan instance!");
			}
			volkLoadInstance(s_VulkanInstance);
	#ifdef SGF_ENABLE_VALIDATION
			if (debug_support) {
				s_VulkanMessenger = Util::CreateDebugUtilsMessengerEXT(s_VulkanInstance, Util::DebugCallback);
			} else {
				s_VulkanMessenger = nullptr;
			}
	#endif
			return false;
		}
		void TerminateAPI() {
#ifdef SGF_ENABLE_VALIDATION
			Util::DestroyDebugUtilsMessengerEXT(s_VulkanInstance, s_VulkanMessenger);
			s_VulkanMessenger = VK_NULL_HANDLE;
#endif
			vkDestroyInstance(s_VulkanInstance, VULKAN_ALLOCATION_CALLBACKS);
			s_VulkanInstance = VK_NULL_HANDLE;
			volkFinalize();
		}
		bool Initialize(PhysicalDevice device, Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const WindowHandle* pPresentationWindows, uint32_t presentationWindowCount) {
			std::vector<VkSurfaceKHR> surfaces = Util::CreateTemporarySurfaces(pPresentationWindows, presentationWindowCount);
			bool success = Util::InitializeDevice(device, requiredFeatures, requiredQueues, surfaces);
			Util::DestroyTemporarySurfaces(surfaces);
			return success;
		}
		bool Initialize(Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const WindowHandle* pPresentationWindows, uint32_t presentationWindowCount) {
			std::vector<VkSurfaceKHR> surfaces = Util::CreateTemporarySurfaces(pPresentationWindows, presentationWindowCount);
			auto device = Util::PickPhysicalDevice(requiredFeatures, requiredQueues, surfaces);
			bool success = Util::InitializeDevice(device, requiredFeatures, requiredQueues, surfaces);
			Util::DestroyTemporarySurfaces(surfaces);
			return success;
		}
		bool SupportsPresentation(const WindowHandle* pWindows, uint32_t windowCount) {
			std::vector<VkSurfaceKHR> surfaces = Util::CreateTemporarySurfaces(pWindows, windowCount);
			Util::DestroyTemporarySurfaces(surfaces);
		}
		bool HasFeaturesEnabled(Flags<DeviceFeature> features) {
			return s_EnabledFeatures.Has(features);
		}
		void Submit(QueueType queueType, const CommandList* pCommands, uint32_t commandCount, const Semaphore* pWaitSemaphores, const Flags<PipelineStage>* pWaitStages, uint32_t waitSemaphoreCount, const Semaphore* pSignalSemaphores, uint32_t signalSemaphoreCount, Fence fence) {
			VkSubmitInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.commandBufferCount = commandCount;
			info.pCommandBuffers = (VkCommandBuffer*)pCommands;
			info.pWaitSemaphores = (VkSemaphore*)pWaitSemaphores;
			info.pWaitDstStageMask = (VkPipelineStageFlags*)pWaitStages;
			info.waitSemaphoreCount = waitSemaphoreCount;
			info.pSignalSemaphores = (VkSemaphore*)pSignalSemaphores;
			info.signalSemaphoreCount = signalSemaphoreCount;
			vkQueueSubmit(Util::GetQueue(queueType), 1, &info, (VkFence)fence);
		}

		uint32_t GetDeviceCount() {
			uint32_t count;
			if (vkEnumeratePhysicalDevices(s_VulkanInstance, &count, nullptr) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_PHYSICAL_DEVICE_ENUMERATION);
				return 0;
			}
			return count;
		}

		std::vector<PhysicalDevice> GetAvailableDevices() {
			uint32_t count = GetDeviceCount();
			std::vector<PhysicalDevice> devices(count);
			if (vkEnumeratePhysicalDevices(s_VulkanInstance, &count, (VkPhysicalDevice*)devices.data()) != VK_SUCCESS) {
				Log::Fatal("{}", ERROR_PHYSICAL_DEVICE_ENUMERATION);
				return std::vector<PhysicalDevice>();
			}
			return devices;
		}
		std::string GetDeviceName(PhysicalDevice device) {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties((VkPhysicalDevice)device, &properties);
			return std::string(properties.deviceName);
		}
		PhysicalDevice GetCurrentDevice() {
			return (PhysicalDevice)s_PhysicalDevice;
		}
		PhysicalDevice PickPhysicalDevice(Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const WindowHandle* pPresentationWindows, uint32_t presentationWindowCount) {
			auto surfaces = Util::CreateTemporarySurfaces(pPresentationWindows, presentationWindowCount);
			PhysicalDevice picked = Util::PickPhysicalDevice(requiredFeatures, requiredQueues, surfaces);
			Util::DestroyTemporarySurfaces(surfaces);
			return picked;
		}
		bool CheckPhysicalDeviceSupport(PhysicalDevice device, Flags<DeviceFeature> requiredFeatures, Flags<QueueType> requiredQueues, const WindowHandle* pPresentationWindows, uint32_t presentationWindowCount) {
			auto surfaces = Util::CreateTemporarySurfaces(pPresentationWindows, presentationWindowCount);
			Util::QueueSupport queueSupport((VkPhysicalDevice)device, surfaces);
			bool support = Util::CheckPhysicalDeviceSupport(device, requiredFeatures, requiredQueues, queueSupport);
			Util::DestroyTemporarySurfaces(surfaces);
			return support;
		}
}
#pragma endregion GPU_FUNCTIONS

#pragma region BASIC_STRUCTS
namespace SGF::GPU {
	ImageMemoryBarrier::ImageMemoryBarrier() : subresourceRange{} {
		*((VkStructureType*) & fillerData[0]) = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		fillerData[1] = nullptr;
		oldLayout = ImageLayout::UNDEFINED;
		newLayout = ImageLayout::UNDEFINED;
		image = nullptr;
		m_SrcQueueIndex = Util::GetQueueTypeIndex(QueueType::GRAPHICS);
		m_DstQueueIndex = Util::GetQueueTypeIndex(QueueType::GRAPHICS);
	}
	ImageMemoryBarrier::ImageMemoryBarrier(Image image, ImageLayout oldLayout, ImageLayout newLayout, Flags<Access> srcAcc, Flags<Access> dstAcc, QueueType srcQueueType, QueueType dstQueueType, const ImageSubresourceRange& subresourceRange)
		: image(image), srcAccess(srcAcc), dstAccess(dstAcc), oldLayout(oldLayout), newLayout(newLayout), m_SrcQueueIndex(Util::GetQueueTypeIndex(srcQueueType)), m_DstQueueIndex(Util::GetQueueTypeIndex(dstQueueType)), subresourceRange(subresourceRange)
	{
		*((VkStructureType*) & fillerData[0]) = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		fillerData[1] = nullptr;
	}
	void ImageMemoryBarrier::SetDstQueueType(QueueType type) {
		m_DstQueueIndex = Util::GetQueueTypeIndex(type);
	}
	void ImageMemoryBarrier::SetSrcQueueType(QueueType type) {
		m_SrcQueueIndex = Util::GetQueueTypeIndex(type);
	}

	BufferMemoryBarrier::BufferMemoryBarrier() {
		*((VkStructureType*) & fillerData[0]) = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		fillerData[1] = nullptr;
		buffer = nullptr;
		offset = 0;
		size = 0;
		m_SrcQueueIndex = Util::GetQueueTypeIndex(QueueType::GRAPHICS);
		m_DstQueueIndex = Util::GetQueueTypeIndex(QueueType::GRAPHICS);
	}

	BufferMemoryBarrier::BufferMemoryBarrier(Buffer buf, size_t s, size_t o, Flags<Access> srcAccessMask, Flags<Access> dstAccessMask, QueueType srcQueueType, QueueType dstQueueType)
		: buffer(buf), size(s), offset(o), srcAccess(srcAccessMask), dstAccess(dstAccessMask), m_SrcQueueIndex(Util::GetQueueTypeIndex(srcQueueType)), m_DstQueueIndex(Util::GetQueueTypeIndex(dstQueueType))
	{
		*((VkStructureType*) & fillerData[0]) = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		fillerData[1] = nullptr;
	}
	void BufferMemoryBarrier::SetDstQueueType(QueueType type) {
		m_DstQueueIndex = Util::GetQueueTypeIndex(type);
	}
	void BufferMemoryBarrier::SetSrcQueueType(QueueType type) {
		m_SrcQueueIndex = Util::GetQueueTypeIndex(type);
	}

	MemoryBarrier::MemoryBarrier() {
		*((VkStructureType*) & fillerData[0]) = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		fillerData[1] = nullptr;
	}
	MemoryBarrier::MemoryBarrier(Flags<Access> srcAccessMask, Flags<Access> dstAccessMask) : srcAccess(srcAccessMask), dstAccess(dstAccessMask) {
		*((VkStructureType*) & fillerData[0]) = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		fillerData[1] = nullptr;
	}
}
#pragma endregion BASIC_STRUCTS
	// CommandList Functions:

#pragma region COMMAND_LIST_FUNCTIONS
namespace SGF::GPU {
	// Reinterpret a pointer-sized opaque handle as a Vulkan handle.
	// e.g.  VK(Buffer) -> VkBuffer
	template<typename VkT, typename SGFHandle>
	static inline VkT VK(SGFHandle h) {
		return reinterpret_cast<VkT>(h);
	}
	// Reinterpret a Flags<T> bitmask as a plain VkFlags (uint32_t) value.
	template<typename T>
	static inline VkFlags VKF(Flags<T> f) {
		return static_cast<VkFlags>(f);
	}
	// Reinterpret an enum value that is layout-compatible with a Vulkan enum.
	template<typename VkEnum, typename SGFEnum>
	static inline VkEnum VKE(SGFEnum e) {
		return static_cast<VkEnum>(e);
	}
	// Retrieve the raw VkCommandBuffer from a CommandList.
	static inline VkCommandBuffer CMD(const CommandList* cl) {
		return reinterpret_cast<VkCommandBuffer>(cl->GetHandle());
	}
	// -------------------------------------------------------------------------
	// Lifecycle
	// -------------------------------------------------------------------------

	void CommandList::Reset() {
		vkResetCommandBuffer(CMD(this), 0);
	}

	void CommandList::Begin(Flags<CommandBufferUsage> usage) {
		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VKF(usage);
		vkBeginCommandBuffer(CMD(this), &info);
	}

	void CommandList::BeginSecondary(RenderPass renderPass, uint32_t subpassIndex, Framebuffer framebuffer, Flags<CommandBufferUsage> usage) {
		VkCommandBufferBeginInfo info{};
		VkCommandBufferInheritanceInfo inh;
		inh.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inh.pNext = nullptr;
		usage |= CommandBufferUsage::RENDER_PASS_CONTINUE;
		inh.framebuffer = (VkFramebuffer)framebuffer;
		inh.renderPass = (VkRenderPass)renderPass;
		inh.subpass = subpassIndex;
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VKF(usage);
		vkBeginCommandBuffer(CMD(this), &info);
	}

	void CommandList::End() {
		vkEndCommandBuffer(CMD(this));
	}

	// -------------------------------------------------------------------------
	// Render Pass
	// -------------------------------------------------------------------------

	void CommandList::BeginRenderPass(RenderPass renderPass) {
		VkRenderPassBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = VK<VkRenderPass>(renderPass);
		vkCmdBeginRenderPass(CMD(this), &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandList::EndRenderPass() {
		vkCmdEndRenderPass(CMD(this));
	}

	void CommandList::NextSubpass() {
		vkCmdNextSubpass(CMD(this), VK_SUBPASS_CONTENTS_INLINE);
	}

	// -------------------------------------------------------------------------
	// Draw commands
	// -------------------------------------------------------------------------

	void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		vkCmdDraw(CMD(this), vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
		vkCmdDrawIndexed(CMD(this), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandList::DrawIndirect(Buffer indirectBuffer, size_t offset, uint32_t drawCount, uint32_t stride) {
		vkCmdDrawIndirect(CMD(this), VK<VkBuffer>(indirectBuffer), static_cast<VkDeviceSize>(offset), drawCount, stride);
	}

	void CommandList::DrawIndexedIndirect(Buffer indirectBuffer, size_t offset, uint32_t drawCount, uint32_t stride) {
		vkCmdDrawIndexedIndirect(CMD(this), VK<VkBuffer>(indirectBuffer), static_cast<VkDeviceSize>(offset), drawCount, stride);
	}

	void CommandList::DrawIndirectCount(Buffer indirectBuffer, size_t offset, Buffer countBuffer, size_t countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
		vkCmdDrawIndirectCount(CMD(this), VK<VkBuffer>(indirectBuffer), static_cast<VkDeviceSize>(offset), VK<VkBuffer>(countBuffer), static_cast<VkDeviceSize>(countBufferOffset), maxDrawCount, stride);
	}

	void CommandList::DrawIndexedIndirectCount(Buffer indirectBuffer, size_t offset, Buffer countBuffer, size_t countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
		vkCmdDrawIndexedIndirectCount(CMD(this), VK<VkBuffer>(indirectBuffer), static_cast<VkDeviceSize>(offset), VK<VkBuffer>(countBuffer), static_cast<VkDeviceSize>(countBufferOffset), maxDrawCount, stride);
	}

	void CommandList::DrawMeshTasks(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
		vkCmdDrawMeshTasksEXT(CMD(this), groupCountX, groupCountY, groupCountZ);
	}

	void CommandList::DrawMeshTasksIndirect(Buffer indirectBuffer, size_t offset, uint32_t drawCount, uint32_t stride) {
		vkCmdDrawMeshTasksIndirectEXT(CMD(this), VK<VkBuffer>(indirectBuffer), static_cast<VkDeviceSize>(offset), drawCount, stride);
	}

	void CommandList::DrawMeshTasksIndirectCount(Buffer indirectBuffer, size_t offset, Buffer countBuffer, size_t countBufferOffset, uint32_t maxDrawCount, uint32_t stride) {
		vkCmdDrawMeshTasksIndirectCountEXT(CMD(this), VK<VkBuffer>(indirectBuffer), static_cast<VkDeviceSize>(offset), VK<VkBuffer>(countBuffer), static_cast<VkDeviceSize>(countBufferOffset), maxDrawCount, stride);
	}

	// -------------------------------------------------------------------------
	// Compute / Ray Tracing
	// -------------------------------------------------------------------------

	void CommandList::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
		vkCmdDispatch(CMD(this), groupCountX, groupCountY, groupCountZ);
	}

	void CommandList::DispatchIndirect(Buffer indirectBuffer, size_t offset) {
		vkCmdDispatchIndirect(CMD(this), VK<VkBuffer>(indirectBuffer), static_cast<VkDeviceSize>(offset));
	}

	void CommandList::DispatchBase(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
		vkCmdDispatchBase(CMD(this), baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
	}
	void CommandList::TraceRays(const StridedDeviceAddressRegion& raygenSBT, const StridedDeviceAddressRegion& missSBT, const StridedDeviceAddressRegion& hitSBT, const StridedDeviceAddressRegion& callableSBT, uint32_t width, uint32_t height, uint32_t depth) {
		vkCmdTraceRaysKHR(
			CMD(this),
			reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&raygenSBT),
			reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&missSBT),
			reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&hitSBT),
			reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&callableSBT),
			width, height, depth);
	}

	void CommandList::TraceRaysIndirect(const StridedDeviceAddressRegion& raygenSBT, const StridedDeviceAddressRegion& missSBT, const StridedDeviceAddressRegion& hitSBT, const StridedDeviceAddressRegion& callableSBT, uint64_t indirectDeviceAddress) {
		vkCmdTraceRaysIndirectKHR(
			CMD(this),
			reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&raygenSBT),
			reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&missSBT),
			reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&hitSBT),
			reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&callableSBT),
			static_cast<VkDeviceAddress>(indirectDeviceAddress));
	}

	// -------------------------------------------------------------------------
	// Pipeline & descriptor binding
	// -------------------------------------------------------------------------

	void CommandList::BindVertexBuffer(uint32_t binding, Buffer vertexBuffer, size_t offset) {
		VkBuffer     buf = VK<VkBuffer>(vertexBuffer);
		VkDeviceSize off = static_cast<VkDeviceSize>(offset);
		vkCmdBindVertexBuffers(CMD(this), binding, 1, &buf, &off);
	}

	void CommandList::BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const Buffer* vertexBuffers, const size_t* offsets) {
		// Buffer and VkBuffer are the same width; offsets need a cast array.
		// Use a small stack buffer for the common case; heap for large counts.
		constexpr uint32_t kStackMax = 16;
		VkDeviceSize       stackOffsets[kStackMax];
		VkDeviceSize* vkOffsets = stackOffsets;
		if (bindingCount > kStackMax)
			vkOffsets = new VkDeviceSize[bindingCount];

		for (uint32_t i = 0; i < bindingCount; ++i)
			vkOffsets[i] = static_cast<VkDeviceSize>(offsets[i]);

		vkCmdBindVertexBuffers(CMD(this), firstBinding, bindingCount, reinterpret_cast<const VkBuffer*>(vertexBuffers), vkOffsets);

		if (bindingCount > kStackMax)
			delete[] vkOffsets;
	}
	void CommandList::BindIndexBuffer32Bit(Buffer indexBuffer, size_t offset) {
		vkCmdBindIndexBuffer(CMD(this), VK<VkBuffer>(indexBuffer), static_cast<VkDeviceSize>(offset), VK_INDEX_TYPE_UINT32);
	}
	void CommandList::BindIndexBuffer16Bit(Buffer indexBuffer, size_t offset) {
		vkCmdBindIndexBuffer(CMD(this), VK<VkBuffer>(indexBuffer), static_cast<VkDeviceSize>(offset), VK_INDEX_TYPE_UINT16);
	}
	void CommandList::BindIndexBuffer8Bit(Buffer indexBuffer, size_t offset) {
		vkCmdBindIndexBuffer(CMD(this), VK<VkBuffer>(indexBuffer), static_cast<VkDeviceSize>(offset), VK_INDEX_TYPE_UINT8_KHR);
	}
	void CommandList::BindPipeline(ComputePipeline pipeline) {
		vkCmdBindPipeline(CMD(this), VK_PIPELINE_BIND_POINT_COMPUTE, VK<VkPipeline>(pipeline));
	}
	void CommandList::BindPipeline(GraphicsPipeline pipeline) {
		vkCmdBindPipeline(CMD(this), VK_PIPELINE_BIND_POINT_GRAPHICS, VK<VkPipeline>(pipeline));
	}
	void CommandList::BindPipeline(RayTracingPipeline pipeline) {
		vkCmdBindPipeline(CMD(this), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, VK<VkPipeline>(pipeline));
	}

	void CommandList::PushConstants(PipelineLayout layout, Flags<ShaderStage> stageFlags, uint32_t offset, uint32_t size, const void* pValues) {
		vkCmdPushConstants(CMD(this), VK<VkPipelineLayout>(layout), VKF(stageFlags), offset, size, pValues);
	}
#ifdef SGF_GPU_EXTENDED_FUNCTIONS
	void CommandList::PushDescriptorSet(PipelineBindPoint bindPoint, PipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const WriteDescriptorSet* descriptorWrites) {
		vkCmdPushDescriptorSetKHR(
			CMD(this),
			VKE<VkPipelineBindPoint>(bindPoint),
			VK<VkPipelineLayout>(layout),
			set,
			descriptorWriteCount,
			reinterpret_cast<const VkWriteDescriptorSet*>(descriptorWrites));
	}
#endif

	void CommandList::BindDescriptorSet(PipelineType pipelineType, PipelineLayout layout, uint32_t setIndex, DescriptorSet set) {
		// Bind point is unknown here without pipeline context; graphics is the
		// common default. Callers that need compute/RT should use BindDescriptorSets
		// with an explicit bind point or extend the API accordingly.
		VkDescriptorSet vkSet = VK<VkDescriptorSet>(set);
		vkCmdBindDescriptorSets(CMD(this), VKE<VkPipelineBindPoint>(pipelineType), VK<VkPipelineLayout>(layout), setIndex, 1, &vkSet, 0, nullptr);
	}

	void CommandList::BindDescriptorSets(PipelineType pipelineType, PipelineLayout layout, const DescriptorSet* sets, uint32_t count, uint32_t firstSet) {
		vkCmdBindDescriptorSets(
			CMD(this),
			VKE<VkPipelineBindPoint>(pipelineType),
			VK<VkPipelineLayout>(layout),
			firstSet,
			count,
			reinterpret_cast<const VkDescriptorSet*>(sets),
			0, nullptr);
	}

	// -------------------------------------------------------------------------
	// Dynamic state
	// -------------------------------------------------------------------------

	void CommandList::SetViewport(SGF::GPU::Viewport viewport) {
		vkCmdSetViewport(CMD(this), 0, 1, reinterpret_cast<const VkViewport*>(&viewport));
	}

	void CommandList::SetViewport(float width, float height, float xoffset, float yoffset, float minDepth, float maxDepth) {
		VkViewport vp{ xoffset, yoffset, width, height, minDepth, maxDepth };
		vkCmdSetViewport(CMD(this), 0, 1, &vp);
	}

	void CommandList::SetViewports(const Viewport* viewports, uint32_t count) {
		vkCmdSetViewport(CMD(this), 0, count, reinterpret_cast<const VkViewport*>(viewports));
	}

	void CommandList::SetScissor(const Rect2D& scissor) {
		vkCmdSetScissor(CMD(this), 0, 1, reinterpret_cast<const VkRect2D*>(&scissor));
	}

	void CommandList::SetScissors(const Rect2D* scissors, uint32_t count) {
		vkCmdSetScissor(CMD(this), 0, count, reinterpret_cast<const VkRect2D*>(scissors));
	}

	void CommandList::SetLineWidth(float lineWidth) {
		vkCmdSetLineWidth(CMD(this), lineWidth);
	}

	void CommandList::SetDepthBias(float constantFactor, float clamp, float slopeFactor) {
		vkCmdSetDepthBias(CMD(this), constantFactor, clamp, slopeFactor);
	}

	void CommandList::SetDepthBiasEnable(bool enable) {
		vkCmdSetDepthBiasEnableEXT(CMD(this), static_cast<VkBool32>(enable));
	}

	void CommandList::SetDepthBounds(float minDepthBounds, float maxDepthBounds) {
		vkCmdSetDepthBounds(CMD(this), minDepthBounds, maxDepthBounds);
	}

	void CommandList::SetDepthBoundsTestEnable(bool enable) {
		vkCmdSetDepthBoundsTestEnableEXT(CMD(this), static_cast<VkBool32>(enable));
	}

	void CommandList::SetDepthTestEnable(bool enable) {
		vkCmdSetDepthTestEnableEXT(CMD(this), static_cast<VkBool32>(enable));
	}

	void CommandList::SetDepthWriteEnable(bool enable) {
		vkCmdSetDepthWriteEnableEXT(CMD(this), static_cast<VkBool32>(enable));
	}

	void CommandList::SetDepthCompareOp(CompareOp compareOp) {
		vkCmdSetDepthCompareOpEXT(CMD(this), VKE<VkCompareOp>(compareOp));
	}

	void CommandList::SetStencilTestEnable(bool enable) {
		vkCmdSetStencilTestEnableEXT(CMD(this), static_cast<VkBool32>(enable));
	}

	void CommandList::SetStencilOp(Flags<StencilFace> faceMask, StencilOp failOp, StencilOp passOp, StencilOp depthFailOp, CompareOp compareOp) {
		vkCmdSetStencilOpEXT(
			CMD(this),
			VKF(faceMask),
			VKE<VkStencilOp>(failOp),
			VKE<VkStencilOp>(passOp),
			VKE<VkStencilOp>(depthFailOp),
			VKE<VkCompareOp>(compareOp));
	}

	void CommandList::SetStencilCompareMask(Flags<StencilFace> faceMask, uint32_t compareMask) {
		vkCmdSetStencilCompareMask(CMD(this), VKF(faceMask), compareMask);
	}

	void CommandList::SetStencilWriteMask(Flags<StencilFace> faceMask, uint32_t writeMask) {
		vkCmdSetStencilWriteMask(CMD(this), VKF(faceMask), writeMask);
	}

	void CommandList::SetStencilReference(Flags<StencilFace> faceMask, uint32_t reference) {
		vkCmdSetStencilReference(CMD(this), VKF(faceMask), reference);
	}

	void CommandList::SetBlendConstants(float r, float g, float b, float a) {
		float blendConstants[4] = { r, g, b, a };
		vkCmdSetBlendConstants(CMD(this), blendConstants);
	}

#ifdef SGF_GPU_EXTENDED_FUNCTIONS
	void CommandList::SetCullMode(Flags<CullMode> cullMode) {
		vkCmdSetCullModeEXT(CMD(this), VKF(cullMode));
	}

	void CommandList::SetFrontFace(FrontFace frontFace) {
		vkCmdSetFrontFaceEXT(CMD(this), VKE<VkFrontFace>(frontFace));
	}

	void CommandList::SetPrimitiveTopology(PrimitiveTopology topology) {
		vkCmdSetPrimitiveTopologyEXT(CMD(this), VKE<VkPrimitiveTopology>(topology));
	}

	void CommandList::SetPrimitiveRestartEnable(bool enable) {
		vkCmdSetPrimitiveRestartEnableEXT(CMD(this), static_cast<VkBool32>(enable));
	}

	void CommandList::SetRasterizerDiscardEnable(bool enable) {
		vkCmdSetRasterizerDiscardEnableEXT(CMD(this), static_cast<VkBool32>(enable));
	}

	void CommandList::SetVertexInputBindingStride(uint32_t firstBinding, uint32_t bindingCount, const uint32_t* strides) {
		vkCmdSetVertexInputEXT(CMD(this), 0, nullptr, 0, nullptr);
		// Stride-only dynamic state is applied via vkCmdBindVertexBuffers2EXT.
		// A null sizes array means the sizes are ignored (whole buffer used).
		vkCmdBindVertexBuffers2EXT(CMD(this), firstBinding, bindingCount, nullptr, nullptr, nullptr, reinterpret_cast<const VkDeviceSize*>(strides));
	}

	void CommandList::SetSampleLocations(const SampleLocationsInfo& sampleLocationsInfo) {
		vkCmdSetSampleLocationsEXT(CMD(this), reinterpret_cast<const VkSampleLocationsInfoEXT*>(&sampleLocationsInfo));
	}

	void CommandList::SetFragmentShadingRate(Extent2D fragmentSize, FragmentShadingRateCombinerOp combinerOps[2]) {
		vkCmdSetFragmentShadingRateKHR(
			CMD(this),
			reinterpret_cast<const VkExtent2D*>(&fragmentSize),
			reinterpret_cast<const VkFragmentShadingRateCombinerOpKHR*>(combinerOps));
	}
#endif

	// -------------------------------------------------------------------------
	// Copy / Blit / Resolve
	// -------------------------------------------------------------------------

	void CommandList::CopyBuffer(Buffer srcBuffer, Buffer dstBuffer, uint32_t regionCount, const BufferCopy* regions) {
		vkCmdCopyBuffer(CMD(this), VK<VkBuffer>(srcBuffer), VK<VkBuffer>(dstBuffer), regionCount, reinterpret_cast<const VkBufferCopy*>(regions));
	}

	void CommandList::FillBuffer(Buffer dstBuffer, size_t dstOffset, size_t size, uint32_t data) {
		vkCmdFillBuffer(CMD(this), VK<VkBuffer>(dstBuffer), static_cast<VkDeviceSize>(dstOffset), static_cast<VkDeviceSize>(size), data);
	}

	void CommandList::UpdateBuffer(Buffer dstBuffer, size_t dstOffset, size_t dataSize, const void* pData) {
		vkCmdUpdateBuffer(CMD(this), VK<VkBuffer>(dstBuffer), static_cast<VkDeviceSize>(dstOffset), static_cast<VkDeviceSize>(dataSize), pData);
	}

	void CommandList::CopyImage(Image srcImage, ImageLayout srcLayout, Image dstImage, ImageLayout dstLayout, uint32_t regionCount, const ImageCopy* regions) {
		vkCmdCopyImage(
			CMD(this),
			VK<VkImage>(srcImage), VKE<VkImageLayout>(srcLayout),
			VK<VkImage>(dstImage), VKE<VkImageLayout>(dstLayout),
			regionCount, reinterpret_cast<const VkImageCopy*>(regions));
	}

	void CommandList::BlitImage(Image srcImage, ImageLayout srcLayout, Image dstImage, ImageLayout dstLayout, uint32_t regionCount, const ImageBlit* regions, FilterType filter) {
		vkCmdBlitImage(
			CMD(this),
			VK<VkImage>(srcImage), VKE<VkImageLayout>(srcLayout),
			VK<VkImage>(dstImage), VKE<VkImageLayout>(dstLayout),
			regionCount, reinterpret_cast<const VkImageBlit*>(regions),
			VKE<VkFilter>(filter));
	}

	void CommandList::ResolveImage(Image srcImage, ImageLayout srcLayout, Image dstImage, ImageLayout dstLayout, uint32_t regionCount, const ImageResolve* regions) {
		vkCmdResolveImage(
			CMD(this),
			VK<VkImage>(srcImage), VKE<VkImageLayout>(srcLayout),
			VK<VkImage>(dstImage), VKE<VkImageLayout>(dstLayout),
			regionCount, reinterpret_cast<const VkImageResolve*>(regions));
	}

	void CommandList::CopyBufferToImage(Buffer srcBuffer, Image dstImage, ImageLayout dstImageLayout, uint32_t regionCount, const BufferImageCopy* regions) {
		vkCmdCopyBufferToImage(
			CMD(this),
			VK<VkBuffer>(srcBuffer),
			VK<VkImage>(dstImage),
			VKE<VkImageLayout>(dstImageLayout),
			regionCount, reinterpret_cast<const VkBufferImageCopy*>(regions));
	}

	void CommandList::CopyImageToBuffer(Image srcImage, ImageLayout srcImageLayout, Buffer dstBuffer, uint32_t regionCount, const BufferImageCopy* regions) {
		vkCmdCopyImageToBuffer(
			CMD(this),
			VK<VkImage>(srcImage),
			VKE<VkImageLayout>(srcImageLayout),
			VK<VkBuffer>(dstBuffer),
			regionCount, reinterpret_cast<const VkBufferImageCopy*>(regions));
	}

	void CommandList::ClearColorImage(Image image, ImageLayout imageLayout, const ClearColorValue& color, uint32_t rangeCount, const ImageSubresourceRange* ranges) {
		vkCmdClearColorImage(
			CMD(this),
			VK<VkImage>(image),
			VKE<VkImageLayout>(imageLayout),
			reinterpret_cast<const VkClearColorValue*>(&color),
			rangeCount,
			reinterpret_cast<const VkImageSubresourceRange*>(ranges));
	}

	void CommandList::ClearDepthStencilImage(Image image, ImageLayout imageLayout, float depth, uint32_t stencil, uint32_t rangeCount, const ImageSubresourceRange* ranges) {
		VkClearDepthStencilValue ds{ depth, stencil };
		vkCmdClearDepthStencilImage(
			CMD(this),
			VK<VkImage>(image),
			VKE<VkImageLayout>(imageLayout),
			&ds,
			rangeCount,
			reinterpret_cast<const VkImageSubresourceRange*>(ranges));
	}

	void CommandList::ClearAttachments(uint32_t attachmentCount, const ClearAttachment* attachments, uint32_t rectCount, const ClearRect* rects) {
		vkCmdClearAttachments(
			CMD(this),
			attachmentCount, reinterpret_cast<const VkClearAttachment*>(attachments),
			rectCount, reinterpret_cast<const VkClearRect*>(rects));
	}

	// -------------------------------------------------------------------------
	// Synchronisation — Barriers & Events
	// -------------------------------------------------------------------------
	void CommandList::PipelineBarrier(Flags<PipelineStage> srcStageMask, Flags<PipelineStage> dstStageMask, Flags<Dependency> dependencyFlags, uint32_t memoryBarrierCount, const GPU::MemoryBarrier* memoryBarriers, uint32_t bufferMemoryBarrierCount, const BufferMemoryBarrier* bufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const ImageMemoryBarrier* imageMemoryBarriers) {
		vkCmdPipelineBarrier(
			CMD(this),
			VKF(srcStageMask), VKF(dstStageMask), VKF(dependencyFlags),
			memoryBarrierCount, reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers),
			bufferMemoryBarrierCount, reinterpret_cast<const VkBufferMemoryBarrier*>(bufferMemoryBarriers),
			imageMemoryBarrierCount, reinterpret_cast<const VkImageMemoryBarrier*>(imageMemoryBarriers));
	}

	void CommandList::MemoryBarrier(Flags<PipelineStage> srcStage, Flags<PipelineStage> dstStage, Flags<Access> srcAccess, Flags<Access> dstAccess) {
		VkMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VKF(srcAccess);
		barrier.dstAccessMask = VKF(dstAccess);
		vkCmdPipelineBarrier(CMD(this), VKF(srcStage), VKF(dstStage), 0, 1, &barrier, 0, nullptr, 0, nullptr);
	}

	void CommandList::BufferBarrier(Buffer buffer, size_t offset, size_t size, Flags<PipelineStage> srcStage, Flags<PipelineStage> dstStage, Flags<Access> srcAccess, Flags<Access> dstAccess, QueueType srcQueueType, QueueType dstQueueType) {
		VkBufferMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.srcAccessMask = VKF(srcAccess);
		barrier.dstAccessMask = VKF(dstAccess);
		barrier.srcQueueFamilyIndex = Util::GetQueueTypeIndex(srcQueueType);
		barrier.dstQueueFamilyIndex = Util::GetQueueTypeIndex(dstQueueType);
		barrier.buffer = VK<VkBuffer>(buffer);
		barrier.offset = static_cast<VkDeviceSize>(offset);
		barrier.size = static_cast<VkDeviceSize>(size);
		vkCmdPipelineBarrier(CMD(this), VKF(srcStage), VKF(dstStage), 0, 0, nullptr, 1, &barrier, 0, nullptr);
	}

	void CommandList::ImageBarrier(Image image, const ImageSubresourceRange& subresourceRange, ImageLayout oldLayout, ImageLayout newLayout, Flags<PipelineStage> srcStage, Flags<PipelineStage> dstStage, Flags<Access> srcAccess, Flags<Access> dstAccess, QueueType srcQueueType, QueueType dstQueueType) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = VKF(srcAccess);
		barrier.dstAccessMask = VKF(dstAccess);
		barrier.oldLayout = VKE<VkImageLayout>(oldLayout);
		barrier.newLayout = VKE<VkImageLayout>(newLayout);
		barrier.srcQueueFamilyIndex = Util::GetQueueTypeIndex(srcQueueType);
		barrier.dstQueueFamilyIndex = Util::GetQueueTypeIndex(dstQueueType);
		barrier.image = VK<VkImage>(image);
		barrier.subresourceRange = *reinterpret_cast<const VkImageSubresourceRange*>(&subresourceRange);
		vkCmdPipelineBarrier(CMD(this), VKF(srcStage), VKF(dstStage), 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

#ifdef SGF_GPU_EXTENDED_FUNCTIONS
	void CommandList::SetEvent(Event event, Flags<PipelineStage> stageMask) {
		vkCmdSetEvent(CMD(this), VK<VkEvent>(event), VKF(stageMask));
	}

	void CommandList::SetEvent2(Event event, const DependencyInfo & dependencyInfo) {
		vkCmdSetEvent2(CMD(this), VK<VkEvent>(event), reinterpret_cast<const VkDependencyInfo*>(&dependencyInfo));
	}

	void CommandList::ResetEvent(Event event, Flags<PipelineStage> stageMask) {
		vkCmdResetEvent(CMD(this), VK<VkEvent>(event), VKF(stageMask));
	}

	void CommandList::ResetEvent2(Event event, Flags<PipelineStage2> stageMask) {
		vkCmdResetEvent2(CMD(this), VK<VkEvent>(event), static_cast<VkPipelineStageFlags2>(stageMask));
	}

	void CommandList::WaitEvents(uint32_t eventCount, const Event * events, Flags<PipelineStage> srcStageMask, Flags<PipelineStage> dstStageMask, uint32_t memoryBarrierCount, const MemoryBarrier * memoryBarriers, uint32_t bufferMemoryBarrierCount, const BufferMemoryBarrier * bufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const ImageMemoryBarrier * imageMemoryBarriers) {
		vkCmdWaitEvents(
			CMD(this),
			eventCount, reinterpret_cast<const VkEvent*>(events),
			VKF(srcStageMask), VKF(dstStageMask),
			memoryBarrierCount, reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers),
			bufferMemoryBarrierCount, reinterpret_cast<const VkBufferMemoryBarrier*>(bufferMemoryBarriers),
			imageMemoryBarrierCount, reinterpret_cast<const VkImageMemoryBarrier*>(imageMemoryBarriers));
	}

	void CommandList::WaitEvents2(uint32_t eventCount, const Event * events, const DependencyInfo * dependencyInfos) {
		vkCmdWaitEvents2(
			CMD(this),
			eventCount,
			reinterpret_cast<const VkEvent*>(events),
			reinterpret_cast<const VkDependencyInfo*>(dependencyInfos));
	}
#endif

	// -------------------------------------------------------------------------
	// Queries
	// -------------------------------------------------------------------------

	void CommandList::BeginQuery(QueryPool queryPool, uint32_t query, Flags<QueryControl> flags) {
		vkCmdBeginQuery(CMD(this), VK<VkQueryPool>(queryPool), query, VKF(flags));
	}

	void CommandList::EndQuery(QueryPool queryPool, uint32_t query) {
		vkCmdEndQuery(CMD(this), VK<VkQueryPool>(queryPool), query);
	}

	void CommandList::ResetQueryPool(QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
		vkCmdResetQueryPool(CMD(this), VK<VkQueryPool>(queryPool), firstQuery, queryCount);
	}

	void CommandList::WriteTimestamp(Flags<PipelineStage> pipelineStage, QueryPool queryPool, uint32_t query) {
		vkCmdWriteTimestamp(CMD(this), static_cast<VkPipelineStageFlagBits>(VKF(pipelineStage)), VK<VkQueryPool>(queryPool), query);
	}

	void CommandList::CopyQueryPoolResults(QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, Buffer dstBuffer, size_t dstOffset, size_t stride, Flags<QueryResult> flags) {
		vkCmdCopyQueryPoolResults(
			CMD(this),
			VK<VkQueryPool>(queryPool),
			firstQuery, queryCount,
			VK<VkBuffer>(dstBuffer),
			static_cast<VkDeviceSize>(dstOffset),
			static_cast<VkDeviceSize>(stride),
			VKF(flags));
	}
#ifdef SGF_GPU_EXTENDED_FUNCTIONS

	void CommandList::BeginConditionalRendering(Buffer buffer, size_t offset, Flags<ConditionalRenderingFlag> flags) {
		VkConditionalRenderingBeginInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
		info.buffer = VK<VkBuffer>(buffer);
		info.offset = static_cast<VkDeviceSize>(offset);
		info.flags = VKF(flags);
		vkCmdBeginConditionalRenderingEXT(CMD(this), &info);
	}

	void CommandList::EndConditionalRendering() {
		vkCmdEndConditionalRenderingEXT(CMD(this));
	}

	// -------------------------------------------------------------------------
	// Acceleration Structures
	// -------------------------------------------------------------------------

	void CommandList::BuildAccelerationStructures(uint32_t infoCount, const AccelerationStructureBuildGeometryInfo * infos, const AccelerationStructureBuildRangeInfo* const* rangeInfos) {
		vkCmdBuildAccelerationStructuresKHR(
			CMD(this),
			infoCount,
			reinterpret_cast<const VkAccelerationStructureBuildGeometryInfoKHR*>(infos),
			reinterpret_cast<const VkAccelerationStructureBuildRangeInfoKHR* const*>(rangeInfos));
	}

	void CommandList::BuildAccelerationStructuresIndirect(uint32_t infoCount, const AccelerationStructureBuildGeometryInfo * infos, const uint64_t * indirectDeviceAddresses, const uint32_t * indirectStrides, const uint32_t* const* maxPrimitiveCounts) {
		vkCmdBuildAccelerationStructuresIndirectKHR(
			CMD(this),
			infoCount,
			reinterpret_cast<const VkAccelerationStructureBuildGeometryInfoKHR*>(infos),
			reinterpret_cast<const VkDeviceAddress*>(indirectDeviceAddresses),
			indirectStrides,
			maxPrimitiveCounts);
	}

	void CommandList::CopyAccelerationStructure(const CopyAccelerationStructureInfo & info) {
		vkCmdCopyAccelerationStructureKHR(CMD(this), reinterpret_cast<const VkCopyAccelerationStructureInfoKHR*>(&info));
	}

	void CommandList::CopyAccelerationStructureToMemory(const CopyAccelerationStructureToMemoryInfo & info) {
		vkCmdCopyAccelerationStructureToMemoryKHR(CMD(this), reinterpret_cast<const VkCopyAccelerationStructureToMemoryInfoKHR*>(&info));
	}

	void CommandList::CopyMemoryToAccelerationStructure(const CopyMemoryToAccelerationStructureInfo & info) {
		vkCmdCopyMemoryToAccelerationStructureKHR(CMD(this), reinterpret_cast<const VkCopyMemoryToAccelerationStructureInfoKHR*>(&info));
	}

	void CommandList::WriteAccelerationStructuresProperties(uint32_t accelerationStructureCount, const AccelerationStructure * accelerationStructures, QueryType queryType, QueryPool queryPool, uint32_t firstQuery) {
		vkCmdWriteAccelerationStructuresPropertiesKHR(
			CMD(this),
			accelerationStructureCount,
			reinterpret_cast<const VkAccelerationStructureKHR*>(accelerationStructures),
			VKE<VkQueryType>(queryType),
			VK<VkQueryPool>(queryPool),
			firstQuery);
	}

	// -------------------------------------------------------------------------
	// Debug utilities
	// -------------------------------------------------------------------------

	void CommandList::BeginDebugLabel(const char* labelName, float r, float g, float b, float a) {
		VkDebugUtilsLabelEXT label{};
		label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.pLabelName = labelName;
		label.color[0] = r;
		label.color[1] = g;
		label.color[2] = b;
		label.color[3] = a;
		vkCmdBeginDebugUtilsLabelEXT(CMD(this), &label);
	}

	void CommandList::EndDebugLabel() {
		vkCmdEndDebugUtilsLabelEXT(CMD(this));
	}

	void CommandList::InsertDebugLabel(const char* labelName, float r, float g, float b, float a) {
		VkDebugUtilsLabelEXT label{};
		label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.pLabelName = labelName;
		label.color[0] = r;
		label.color[1] = g;
		label.color[2] = b;
		label.color[3] = a;
		vkCmdInsertDebugUtilsLabelEXT(CMD(this), &label);
	}
#endif

	// -------------------------------------------------------------------------
	// Secondary command buffer execution
	// -------------------------------------------------------------------------

	void CommandList::ExecuteCommands(uint32_t commandBufferCount, const CommandList * commandLists) {
		// The VkCommandBuffer handles are stored as the first member of each
		// CommandList (m_Handle), so the array of CommandLists is NOT directly
		// reinterpretable as an array of VkCommandBuffer.  Collect them first.
		constexpr uint32_t kStackMax = 16;
		VkCommandBuffer    stackBufs[kStackMax];
		VkCommandBuffer* bufs = stackBufs;
		if (commandBufferCount > kStackMax)
			bufs = new VkCommandBuffer[commandBufferCount];

		for (uint32_t i = 0; i < commandBufferCount; ++i)
			bufs[i] = CMD(&commandLists[i]);

		vkCmdExecuteCommands(CMD(this), commandBufferCount, bufs);

		if (commandBufferCount > kStackMax)
			delete[] bufs;
	}

	// -------------------------------------------------------------------------
	// Transform feedback
	// -------------------------------------------------------------------------

	void CommandList::BeginTransformFeedback(uint32_t firstCounterBuffer, uint32_t counterBufferCount, const Buffer * counterBuffers, const size_t * counterBufferOffsets) {
		vkCmdBeginTransformFeedbackEXT(
			CMD(this),
			firstCounterBuffer,
			counterBufferCount,
			reinterpret_cast<const VkBuffer*>(counterBuffers),
			reinterpret_cast<const VkDeviceSize*>(counterBufferOffsets));
	}

	void CommandList::EndTransformFeedback(uint32_t firstCounterBuffer, uint32_t counterBufferCount, const Buffer * counterBuffers, const size_t * counterBufferOffsets) {
		vkCmdEndTransformFeedbackEXT(
			CMD(this),
			firstCounterBuffer,
			counterBufferCount,
			reinterpret_cast<const VkBuffer*>(counterBuffers),
			reinterpret_cast<const VkDeviceSize*>(counterBufferOffsets));
	}

	void CommandList::BeginQueryIndexed(QueryPool queryPool, uint32_t query, Flags<QueryControl> flags, uint32_t index) {
		vkCmdBeginQueryIndexedEXT(CMD(this), VK<VkQueryPool>(queryPool), query, VKF(flags), index);
	}

	void CommandList::EndQueryIndexed(QueryPool queryPool, uint32_t query, uint32_t index) {
		vkCmdEndQueryIndexedEXT(CMD(this), VK<VkQueryPool>(queryPool), query, index);
	}

	void CommandList::BindTransformFeedbackBuffers(uint32_t firstBinding, uint32_t bindingCount, const Buffer * buffers, const size_t * offsets, const size_t * sizes) {
		vkCmdBindTransformFeedbackBuffersEXT(
			CMD(this),
			firstBinding,
			bindingCount,
			reinterpret_cast<const VkBuffer*>(buffers),
			reinterpret_cast<const VkDeviceSize*>(offsets),
			reinterpret_cast<const VkDeviceSize*>(sizes));
	}

	void CommandList::DrawIndirectByteCount(uint32_t instanceCount, uint32_t firstInstance, Buffer counterBuffer, size_t counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) {
		vkCmdDrawIndirectByteCountEXT(
			CMD(this),
			instanceCount,
			firstInstance,
			VK<VkBuffer>(counterBuffer),
			static_cast<VkDeviceSize>(counterBufferOffset),
			counterOffset,
			vertexStride);
	}
}
#pragma endregion COMMAND_LIST_FUNCTIONS

#pragma region RENDER_PASS_BUILDER
namespace SGF::GPU {
	void RenderPassBuilder::FinalizeBuildData() {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		builder->createInfo.pAttachments = builder->descriptions.data();
		builder->createInfo.pSubpasses = builder->subpasses.data();
		builder->createInfo.pDependencies = builder->dependencies.data();
		builder->createInfo.pNext = nullptr;
		SGF_ASSERT(builder->subpassData.size() == builder->subpasses.size());
		for (size_t i = 0; i < builder->subpassData.size(); ++i) {
			auto& subpassData = builder->subpassData[i];
			auto& subpass = builder->subpasses[i];
			subpass.pInputAttachments = subpassData.inputReferences.empty() ? nullptr : subpassData.inputReferences.data();
			subpass.inputAttachmentCount = static_cast<uint32_t>(subpassData.inputReferences.size());

			subpass.pColorAttachments = subpassData.colorReferences.empty() ? nullptr : subpassData.colorReferences.data();
			subpass.colorAttachmentCount = static_cast<uint32_t>(subpassData.colorReferences.size());

			subpass.pResolveAttachments = subpassData.resolveReferences.empty() ? nullptr : subpassData.resolveReferences.data();
			SGF_ASSERT(subpassData.resolveReferences.size() == subpassData.colorReferences.size() || subpassData.resolveReferences.empty());

			subpass.pPreserveAttachments = subpassData.preserveReferences.empty() ? nullptr : subpassData.preserveReferences.data();
			subpass.preserveAttachmentCount = static_cast<uint32_t>(subpassData.preserveReferences.size());

			subpass.pDepthStencilAttachment = subpassData.depthStencilReference.attachment == VK_ATTACHMENT_UNUSED ? nullptr : &subpassData.depthStencilReference;
		}
	}
	RenderPass RenderPassBuilder::Build() {
		FinalizeBuildData();
		VkRenderPass renderPass = VK_NULL_HANDLE;
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		if (!vkCreateRenderPass(VK<VkDevice>(s_LogicalDevice), reinterpret_cast<const VkRenderPassCreateInfo*>(&builder->createInfo), VULKAN_ALLOCATION_CALLBACKS, &renderPass)) {
			Log::Fatal("{}", ERROR_CREATE_RENDER_PASS);
			return nullptr;
		}
		return (RenderPass)renderPass;
	}

	RenderPassBuilder& RenderPassBuilder::AddAttachment(
		Format format,
		SampleCount samples,
		ImageLayout initialLayout,
		ImageLayout finalLayout,
		AttachmentLoad loadOp,
		AttachmentStore storeOp,
		AttachmentLoad stencilLoadOp,
		AttachmentStore stencilStoreOp) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		builder->descriptions.emplace_back();
		builder->createInfo.attachmentCount = static_cast<uint32_t>(builder->descriptions.size());
		auto& att = builder->descriptions.back();
		att.format = VKE<VkFormat>(format);
		att.samples = VKE<VkSampleCountFlagBits>(samples);
		att.initialLayout = VKE<VkImageLayout>(initialLayout);
		att.finalLayout = VKE<VkImageLayout>(finalLayout);
		att.loadOp = VKE<VkAttachmentLoadOp>(loadOp);
		att.storeOp = VKE<VkAttachmentStoreOp>(storeOp);
		att.stencilLoadOp = VKE<VkAttachmentLoadOp>(stencilLoadOp);
		att.stencilStoreOp = VKE<VkAttachmentStoreOp>(stencilStoreOp);
		att.flags = 0;
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::AddColorAttachment(
		Format format,
		SampleCount samples,
		ImageLayout initialLayout,
		ImageLayout finalLayout,
		AttachmentLoad loadOp,
		AttachmentStore storeOp) {
		return AddAttachment(format, samples, initialLayout, finalLayout, loadOp, storeOp, AttachmentLoad::DONT_CARE, AttachmentStore::DONT_CARE);
	}

	RenderPassBuilder& RenderPassBuilder::AddDepthAttachment(
		Format format,
		SampleCount samples,
		ImageLayout initialLayout,
		ImageLayout finalLayout,
		AttachmentLoad loadOp,
		AttachmentStore storeOp,
		AttachmentLoad stencilLoadOp,
		AttachmentStore stencilStoreOp) {
		return AddAttachment(format, samples, initialLayout, finalLayout, loadOp, storeOp, stencilLoadOp, stencilStoreOp);
	}

	RenderPassBuilder& RenderPassBuilder::AddSubpass() {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		builder->subpasses.emplace_back(VkSubpassDescription{});
		builder->createInfo.subpassCount = static_cast<uint32_t>(builder->subpasses.size());
		auto& subpass = builder->subpasses.back();
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		builder->subpassData.emplace_back();
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::AddSubpass(
		Flags<PipelineStage> pipelineStages,
		Flags<Access> accessTypes,
		Flags<PipelineStage> nextPipelineStages,
		Flags<Access> nextAccessTypes,
		Flags<Dependency> dependencyFlags = Dependency::NONE) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		SGF_ASSERT(builder->createInfo.subpassCount > 0);
		uint32_t srcSubpass = builder->createInfo.subpassCount - 1;
		AddSubpassDependency(srcSubpass, srcSubpass + 1, pipelineStages, nextPipelineStages, accessTypes, nextAccessTypes, dependencyFlags);
		return AddSubpass();
	}
	RenderPassBuilder& RenderPassBuilder::AddColorReference(uint32_t attachmentIndex, ImageLayout layout) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData.back();
		data.colorReferences.push_back(VkAttachmentReference{ attachmentIndex, VKE<VkImageLayout>(layout) });
		builder->subpasses.back().colorAttachmentCount = static_cast<uint32_t>(data.colorReferences.size());
		builder->subpasses.back().pColorAttachments = data.colorReferences.data();
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::SetColorReference(uint32_t referenceIndex, uint32_t attachmentIndex, ImageLayout layout) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData.back();
		data.colorReferences[referenceIndex] = VkAttachmentReference{ attachmentIndex, VKE<VkImageLayout>(layout) };
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::ClearColorReferences() {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return ClearColorReferences(builder->createInfo.subpassCount - 1);
	}
	RenderPassBuilder& RenderPassBuilder::ClearColorReferences(uint32_t subpassIndex) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData[subpassIndex];
		data.colorReferences.clear();
		auto& subpass = builder->subpasses[subpassIndex];
		subpass.colorAttachmentCount = 0;
		subpass.pColorAttachments = nullptr;
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::AddInputReference(uint32_t attachmentIndex, ImageLayout layout) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData.back();
		data.inputReferences.push_back(VkAttachmentReference{ attachmentIndex, VKE<VkImageLayout>(layout) });
		builder->subpasses.back().inputAttachmentCount = static_cast<uint32_t>(data.inputReferences.size());
		builder->subpasses.back().pInputAttachments = data.inputReferences.data();
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::SetInputReference(uint32_t referenceIndex, uint32_t attachmentIndex, ImageLayout layout) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData.back();
		data.inputReferences[referenceIndex] = VkAttachmentReference{ attachmentIndex, VKE<VkImageLayout>(layout) };
		return*this;
	}
	RenderPassBuilder& RenderPassBuilder::ClearInputReferences() {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return ClearInputReferences(builder->createInfo.subpassCount - 1);
	}
	RenderPassBuilder& RenderPassBuilder::ClearInputReferences(uint32_t subpassIndex) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData[subpassIndex];
		data.inputReferences.clear();
		auto& subpass = builder->subpasses[subpassIndex];
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::DepthStencilReference(uint32_t attachmentIndex, ImageLayout layout) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& subpass = builder->subpasses.back();
		auto& data = builder->subpassData.back();
		data.depthStencilReference = VkAttachmentReference{ attachmentIndex, VKE<VkImageLayout>(layout) };
		subpass.pDepthStencilAttachment = &data.depthStencilReference;
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::SetDepthStencilReference(uint32_t referenceIndex, uint32_t attachmentIndex, ImageLayout layout) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData.back();
		data.depthStencilReference = VkAttachmentReference{ attachmentIndex, VKE<VkImageLayout>(layout) };
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::ClearDepthStencilReference() {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return ClearDepthStencilReference(builder->createInfo.subpassCount - 1);
	}
	RenderPassBuilder& RenderPassBuilder::ClearDepthStencilReference(uint32_t subpassIndex) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData[subpassIndex];
		auto& subpass = builder->subpasses[subpassIndex];
		subpass.pDepthStencilAttachment = nullptr;
		data.depthStencilReference = VkAttachmentReference{ VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED };
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::AddResolveReference(uint32_t attachmentIndex, ImageLayout layout) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData.back();
		data.resolveReferences.push_back(VkAttachmentReference{ attachmentIndex, VKE<VkImageLayout>(layout) });
		builder->subpasses.back().pResolveAttachments = data.resolveReferences.data();
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::SetResolveReference(uint32_t referenceIndex, uint32_t attachmentIndex, ImageLayout layout) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData.back();
		data.resolveReferences[referenceIndex] = VkAttachmentReference{ attachmentIndex, VKE<VkImageLayout>(layout) };
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::ClearResolveReferences() {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return ClearResolveReferences(builder->createInfo.subpassCount - 1);
	}
	RenderPassBuilder& RenderPassBuilder::ClearResolveReferences(uint32_t subpassIndex) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData[subpassIndex];
		data.resolveReferences.clear();
		auto& subpass = builder->subpasses[subpassIndex];
		subpass.pResolveAttachments = nullptr;
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::AddPreserveReference(uint32_t attachmentIndex) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		auto& data = builder->subpassData.back();
		data.preserveReferences.push_back(attachmentIndex);
		builder->subpasses.back().preserveAttachmentCount = static_cast<uint32_t>(data.preserveReferences.size());
		builder->subpasses.back().pPreserveAttachments = data.preserveReferences.data();
		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::ClearAttachments() {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		builder->descriptions.clear();
		builder->createInfo.attachmentCount = 0;
		SGF_ASSERT(builder->subpasses.size() == builder->subpassData.size());
		for (size_t i = 0; i < builder->subpassData.size(); ++i) {
			builder->subpassData[i].colorReferences.clear();
			builder->subpassData[i].inputReferences.clear();
			builder->subpassData[i].resolveReferences.clear();
			builder->subpassData[i].preserveReferences.clear();
			builder->subpassData[i].depthStencilReference = VkAttachmentReference{ VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED };
			auto& subpass = builder->subpasses[i];
			subpass.colorAttachmentCount = 0;
			subpass.pColorAttachments = nullptr;
			subpass.inputAttachmentCount = 0;
			subpass.pInputAttachments = nullptr;
			subpass.pDepthStencilAttachment = nullptr;
			subpass.pResolveAttachments = nullptr;
			subpass.preserveAttachmentCount = 0;
			subpass.pPreserveAttachments = nullptr;
		}
		return *this;
	}
	RenderPassBuilder& RenderPassBuilder::ClearSubpasses() {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		builder->subpassData.clear();
		builder->subpassData.shrink_to_fit();
		builder->subpasses.clear();
		builder->subpasses.shrink_to_fit();
		return AddSubpass();
	}
	/**
	* Subpass Dependencies:
	*  - Used for synchronization between subpasses and external operations.
	*  - Each dependency specifies a source and destination subpass, along with the stages and access types involved.
	*  - For external dependencies, use SUBPASS_EXTERNAL as the source or destination subpass index.
	*/
	RenderPassBuilder& RenderPassBuilder::AddSubpassDependency(
		uint32_t srcSubpass,
		uint32_t dstSubpass,
		Flags<PipelineStage> srcStages,
		Flags<PipelineStage> dstStages,
		Flags<Access> srcAccesses,
		Flags<Access> dstAccesses,
		Flags<Dependency> dependencyFlags) {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		builder->dependencies.emplace_back(VkSubpassDependency{});
		auto& dep = builder->dependencies.back();
		dep.dependencyFlags = VKF(dependencyFlags);
		dep.dstAccessMask = VKF(dstAccesses);
		dep.srcAccessMask = VKF(srcAccesses);
		dep.dstStageMask = VKF(dstStages);
		dep.srcStageMask = VKF(srcStages);
		dep.dstSubpass = dstSubpass;
		dep.srcSubpass = srcSubpass;
		return *this;
	}

	/**
	* Getters:
	*/
	uint32_t RenderPassBuilder::GetAttachmentCount() const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return static_cast<uint32_t>(builder->descriptions.size());
	}
	uint32_t RenderPassBuilder::GetSubpassCount() const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return static_cast<uint32_t>(builder->subpasses.size());
	}
	uint32_t RenderPassBuilder::GetDependencyCount() const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return static_cast<uint32_t>(builder->dependencies.size());
	}
	uint32_t RenderPassBuilder::GetColorReferenceCount() const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return GetColorReferenceCount(builder->createInfo.subpassCount - 1);
	}
	uint32_t RenderPassBuilder::GetColorReferenceCount(uint32_t subpassIndex) const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return static_cast<uint32_t>(builder->subpassData[subpassIndex].colorReferences.size());
	}
	uint32_t RenderPassBuilder::GetInputReferenceCount() const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return GetInputReferenceCount(builder->createInfo.subpassCount - 1);
	}
	uint32_t RenderPassBuilder::GetInputReferenceCount(uint32_t subpassIndex) const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return static_cast<uint32_t>(builder->subpassData[subpassIndex].inputReferences.size());
	}
	uint32_t RenderPassBuilder::GetResolveReferenceCount() const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return GetResolveReferenceCount(builder->createInfo.subpassCount - 1);
	}
	uint32_t RenderPassBuilder::GetResolveReferenceCount(uint32_t subpassIndex) const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return static_cast<uint32_t>(builder->subpassData[subpassIndex].resolveReferences.size());
	}
	bool RenderPassBuilder::HasDepthStencilReference() const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return HasDepthStencilReference(builder->createInfo.subpassCount - 1);
	}
	bool RenderPassBuilder::HasDepthStencilReference(uint32_t subpassIndex) const {
		RenderPassBuilder_T* builder = reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
		return builder->subpasses[subpassIndex].pDepthStencilAttachment != nullptr;
	}
	RenderPassBuilder::RenderPassBuilder(const RenderPassBuilder& other) {
		RenderPassBuilder_T* otherBuilder = reinterpret_cast<RenderPassBuilder_T*>(other.m_Handle);
		RenderPassBuilder_T* newBuilder = new RenderPassBuilder_T();
		*newBuilder = *otherBuilder;
		m_Handle = newBuilder;
	}
	RenderPassBuilder::RenderPassBuilder(RenderPassBuilder&& other) {
		m_Handle = other.m_Handle;
		other.m_Handle = nullptr;
	}
	RenderPassBuilder::~RenderPassBuilder() {
		if (m_Handle)
			delete reinterpret_cast<RenderPassBuilder_T*>(m_Handle);
	}
}
#pragma endregion RENDER_PASS_BUILDER


#pragma region GRAPHICS_PIPELINE_BUILDER
namespace SGF::GPU {
	GraphicsPipeline GraphicsPipelineBuilder::Build() {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		VkPipeline pipeline = VK_NULL_HANDLE;
		if (vkCreateGraphicsPipelines(s_LogicalDevice, nullptr, 1, &builder->createInfo, VULKAN_ALLOCATION_CALLBACKS, &pipeline) != VK_SUCCESS) {
			Log::Error("{}", ERROR_CREATE_GRAPHICS_PIPELINE);
			return nullptr;
		}
		return (GraphicsPipeline)pipeline;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Layout(PipelineLayout layout) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->createInfo.layout = VK<VkPipelineLayout>(layout);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::RenderPass(SGF::GPU::RenderPass renderPass, uint32_t subpass = 0) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->createInfo.renderPass = VK<VkRenderPass>(renderPass);
		return *this;
	}

	//=========================================================
	// Shader Stages
	//=========================================================

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::VertexShader(const char* filename) {
		AddShaderStage(filename, ShaderStage::VERTEX);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::FragmentShader(const char* filename) {
			AddShaderStage(filename, ShaderStage::FRAGMENT);
			return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::GeometryShader(const char* filename) {
		AddShaderStage(filename, ShaderStage::GEOMETRY);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::TessellationControlShader(const char* filename) {
		AddShaderStage(filename, ShaderStage::GEOMETRY);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::TessellationEvaluationShader(const char* filename) {
		AddShaderStage(filename, ShaderStage::TESSELLATION_EVALUATION);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::VertexShader(ShaderModule module, const char* entry) {
		AddShaderStage(module, ShaderStage::VERTEX, entry);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::FragmentShader(ShaderModule module, const char* entry) {
		AddShaderStage(module, ShaderStage::FRAGMENT, entry);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::GeometryShader(ShaderModule module, const char* entry) {
		AddShaderStage(module, ShaderStage::GEOMETRY, entry);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::TessellationControlShader(ShaderModule module, const char* entrypoint) {
		AddShaderStage(module, ShaderStage::TESSELLATION_CONTROL, entrypoint);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::TessellationEvaluationShader(ShaderModule module, const char* entrypoint) {
		AddShaderStage(module, ShaderStage::TESSELLATION_EVALUATION, entrypoint);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::ClearVertexInput() {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->vertexBindingDescriptions.clear();
		builder->vertexAttributeDescriptions.clear();
		builder->vertexInputState.vertexBindingDescriptionCount = 0;
		builder->vertexInputState.pVertexBindingDescriptions = nullptr;
		builder->vertexInputState.vertexAttributeDescriptionCount = 0;
		builder->vertexInputState.pVertexAttributeDescriptions = nullptr;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddVertexBinding(uint32_t stride, VertexInputRate rate) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->vertexBindingDescriptions.emplace_back();
		auto& bindingDesc = builder->vertexBindingDescriptions.back();
		bindingDesc.binding = static_cast<uint32_t>(builder->vertexBindingDescriptions.size() - 1);
		bindingDesc.stride = stride;
		bindingDesc.inputRate = VKE<VkVertexInputRate>(rate);
		builder->vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(builder->vertexBindingDescriptions.size());
		builder->vertexInputState.pVertexBindingDescriptions = builder->vertexBindingDescriptions.data();
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexBinding(uint32_t stride, VertexInputRate rate) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		auto& bindingDesc = builder->vertexBindingDescriptions.back();
		bindingDesc.stride = stride;
		bindingDesc.inputRate = VKE<VkVertexInputRate>(rate);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddVertexAttribute(Format format, uint32_t offset) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->createInfo.pVertexInputState = &builder->vertexInputState;
		builder->vertexAttributeDescriptions.emplace_back();
		auto& attributeDesc = builder->vertexAttributeDescriptions.back();
		attributeDesc.location = static_cast<uint32_t>(builder->vertexInputState.vertexAttributeDescriptionCount);
		attributeDesc.binding = static_cast<uint32_t>(builder->vertexInputState.vertexBindingDescriptionCount - 1);
		attributeDesc.format = VKE<VkFormat>(format);
		attributeDesc.offset = offset;
		builder->vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(builder->vertexAttributeDescriptions.size());
		builder->vertexInputState.pVertexAttributeDescriptions = builder->vertexAttributeDescriptions.data();
		return *this;
	}
	//=========================================================
	// Input Assembly
	//=========================================================

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Topology(PrimitiveTopology topology) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->inputAssemblyState.topology = VKE<VkPrimitiveTopology>(topology);
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::PrimitiveRestart(bool enable) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->inputAssemblyState.primitiveRestartEnable = (VkBool32)enable;
		return *this;
	}

	//=========================================================
	// Tessellation
	//=========================================================

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::PatchControlPoints(uint32_t count) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->tessellationState.patchControlPoints = count;
		return *this;
	}

	//=========================================================
	// Viewport
	//=========================================================
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Viewport(
		float width,
		float height,
		float x,
		float y,
		float minDepth,
		float maxDepth) {
		return Viewport(SGF::GPU::Viewport{ x, y, width, height, minDepth, maxDepth });
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Viewport(const SGF::GPU::Viewport& viewport) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->viewports.push_back(reinterpret_cast<const VkViewport&>(viewport));
		builder->viewportState.viewportCount = static_cast<uint32_t>(builder->viewports.size());
		builder->viewportState.pViewports = builder->viewports.data();
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Scissor(
		uint32_t width,
		uint32_t height,
		int32_t x,
		int32_t y) {
		return Scissor(Rect2D{ x, y, width, height });
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Scissor(const Rect2D& scissor) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->scissors.push_back(reinterpret_cast<const VkRect2D&>(scissor));
		builder->viewportState.scissorCount = static_cast<uint32_t>(builder->scissors.size());
		builder->viewportState.pScissors = builder->scissors.data();
		return *this;
	}
	//=========================================================
	// Rasterizer
	//=========================================================
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::PolygonMode(SGF::GPU::PolygonMode mode) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->rasterizationState.polygonMode = VKE<VkPolygonMode>(mode);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::CullMode(SGF::GPU::CullMode mode) {
		auto* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->rasterizationState.cullMode = VKE<VkCullModeFlags>(mode);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::FrontFace(SGF::GPU::FrontFace face) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->rasterizationState.frontFace = VKE<VkFrontFace>(face);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::LineWidth(float width) {
		auto* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->rasterizationState.lineWidth = width;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::DepthClamp(bool enable) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->rasterizationState.depthClampEnable = (VkBool32)enable;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::RasterizerDiscard(bool enable) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->rasterizationState.rasterizerDiscardEnable = (VkBool32)enable;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::DepthBias(
		bool enable,
		float constantFactor,
		float clamp,
		float slopeFactor) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->rasterizationState.depthBiasEnable = (VkBool32)enable;
		builder->rasterizationState.depthBiasConstantFactor = constantFactor;
		builder->rasterizationState.depthBiasClamp = clamp;
		builder->rasterizationState.depthBiasSlopeFactor = slopeFactor;
		return *this;
	}
	//=========================================================
	// Multisampling
	//=========================================================
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::SampleCount(SGF::GPU::SampleCount samples) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->multisampleState.rasterizationSamples = VKE<VkSampleCountFlagBits>(samples);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::SampleShading(bool enable, float minimum) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->multisampleState.sampleShadingEnable = (VkBool32)enable;
		builder->multisampleState.minSampleShading = minimum;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::AlphaToCoverage(bool enable) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->multisampleState.alphaToCoverageEnable = (VkBool32)enable;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::AlphaToOne(bool enable) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->multisampleState.alphaToOneEnable = (VkBool32)enable;
		return *this;
	}
	//=========================================================
	// Depth / Stencil
	//=========================================================
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Depth(bool test, bool write, CompareOp compare) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->depthStencilState.depthTestEnable = (VkBool32)test;
		builder->depthStencilState.depthWriteEnable = (VkBool32)write;
		builder->depthStencilState.depthCompareOp = VKE<VkCompareOp>(compare);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::DepthBounds(bool enable, float minimum, float maximum) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->depthStencilState.depthBoundsTestEnable = (VkBool32)enable;
		builder->depthStencilState.minDepthBounds = minimum;
		builder->depthStencilState.maxDepthBounds = maximum;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::StencilTest(bool enable) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->depthStencilState.stencilTestEnable = (VkBool32)enable;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::FrontStencil(const StencilOpState& state) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->depthStencilState.front = VK<VkStencilOpState>(state);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::FrontStencil(
		StencilOp failOp,
		StencilOp passOp,
		StencilOp depthFailOp,
		CompareOp compareOp,
		uint32_t compareMask,
		uint32_t writeMask,
		uint32_t reference) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->depthStencilState.front = VkStencilOpState{
			VKE<VkStencilOp>(failOp),
			VKE<VkStencilOp>(passOp),
			VKE<VkStencilOp>(depthFailOp),
			VKE<VkCompareOp>(compareOp),
			compareMask,
			writeMask,
			reference
			};
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::BackStencil(const StencilOpState& state) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->depthStencilState.back = VK<VkStencilOpState>(state);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::BackStencil(
		StencilOp failOp,
		StencilOp passOp,
		StencilOp depthFailOp,
		CompareOp compareOp,
		uint32_t compareMask,
		uint32_t writeMask,
		uint32_t reference) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->depthStencilState.back = VkStencilOpState{
			VKE<VkStencilOp>(failOp),
			VKE<VkStencilOp>(passOp),
			VKE<VkStencilOp>(depthFailOp),
			VKE<VkCompareOp>(compareOp),
			compareMask,
			writeMask,
			reference
			};
		return *this;
	}
	//=========================================================
	// Color Blend
	//=========================================================
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddColorBlendAttachment(
		bool blendEnable,
		Flags<ColorComponent> colorMask,
		BlendOp alphaBlendOp,
		BlendFactor srcAlpha,
		BlendFactor dstAlpha,
		BlendOp colorBlendOp,
		BlendFactor srcColor,
		BlendFactor dstColor) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->colorBlendAttachments.push_back(VkPipelineColorBlendAttachmentState{
			(VkBool32)blendEnable,
			VKE<VkBlendFactor>(srcColor),
			VKE<VkBlendFactor>(dstColor),
			VKE<VkBlendOp>(colorBlendOp),
			VKE<VkBlendFactor>(srcAlpha),
			VKE<VkBlendFactor>(dstAlpha),
			VKE<VkBlendOp>(alphaBlendOp),
			VKF(colorMask)
		});
		builder->colorBlendState.pAttachments = builder->colorBlendAttachments.data();
		builder->colorBlendState.attachmentCount = static_cast<uint32_t>(builder->colorBlendAttachments.size());
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetColorBlendAttachment(
		uint32_t index,
		bool blendEnable,
		Flags<ColorComponent> colorMask,
		BlendOp alphaBlendOp,
		BlendFactor srcAlpha,
		BlendFactor dstAlpha,
		BlendOp colorBlendOp,
		BlendFactor srcColor,
		BlendFactor dstColor) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->colorBlendAttachments[index] = VkPipelineColorBlendAttachmentState{
			(VkBool32)blendEnable,
			VKE<VkBlendFactor>(srcColor),
			VKE<VkBlendFactor>(dstColor),
			VKE<VkBlendOp>(colorBlendOp),
			VKE<VkBlendFactor>(srcAlpha),
			VKE<VkBlendFactor>(dstAlpha),
			VKE<VkBlendOp>(alphaBlendOp),
			VKF(colorMask)
		};
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::LogicOp(bool enable, SGF::GPU::LogicOp op) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->colorBlendState.logicOpEnable = (VkBool32)enable;
		builder->colorBlendState.logicOp = VKE<VkLogicOp>(op);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::BlendConstants(float r, float g, float b, float a) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->colorBlendState.blendConstants[0] = r;
		builder->colorBlendState.blendConstants[1] = g;
		builder->colorBlendState.blendConstants[2] = b;
		builder->colorBlendState.blendConstants[3] = a;
		return *this;
	}
	//=========================================================
	// Dynamic State
	//=========================================================
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::DynamicState(SGF::GPU::DynamicState state) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->dynamicStates.push_back(VKE<VkDynamicState>(state));
		builder->dynamicState.dynamicStateCount = static_cast<uint32_t>(builder->dynamicStates.size());
		builder->dynamicState.pDynamicStates = builder->dynamicStates.data();
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::ClearDynamicStates() {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->dynamicStates.clear();
		builder->dynamicState.dynamicStateCount = 0;
		builder->dynamicState.pDynamicStates = nullptr;
		return *this;
	}
	//=========================================================
	// Convenience Helpers
	//=========================================================
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Opaque() {
		return AddColorBlendAttachment(
			false,
			ColorComponent::RGBA,
			BlendOp::ADD,
			BlendFactor::ONE,
			BlendFactor::ZERO,
			BlendOp::ADD,
			BlendFactor::ONE,
			BlendFactor::ZERO);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::AlphaBlending() {
		return AddColorBlendAttachment(
			true,
			ColorComponent::RGBA,
			BlendOp::ADD,
			BlendFactor::SRC_ALPHA,
			BlendFactor::ONE_MINUS_SRC_ALPHA,
			BlendOp::ADD,
			BlendFactor::ONE,
			BlendFactor::ZERO);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::AdditiveBlending() {
		return AddColorBlendAttachment(
			true,
			ColorComponent::RGBA,
			BlendOp::ADD,
			BlendFactor::SRC_ALPHA,
			BlendFactor::ONE,
			BlendOp::ADD,
			BlendFactor::ONE,
			BlendFactor::ONE);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::PremultipliedAlpha() {
		return AddColorBlendAttachment(
			true,
			ColorComponent::RGBA,
			BlendOp::ADD,
			BlendFactor::ONE,
			BlendFactor::ONE_MINUS_SRC_ALPHA,
			BlendOp::ADD,
			BlendFactor::ONE,
			BlendFactor::ONE_MINUS_SRC_ALPHA);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::NoCulling() {
		return CullMode(CullMode::NONE);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::BackFaceCulling() {
		return CullMode(CullMode::BACK);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::FrontFaceCulling() {
		return CullMode(CullMode::FRONT);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Wireframe() {
		return PolygonMode(PolygonMode::LINE);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::Fill() {
		return PolygonMode(PolygonMode::FILL);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::DisableDepth() {
		return Depth(false, false, CompareOp::LESS_OR_EQUAL);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::DepthReadOnly() {
		return Depth(true, false, CompareOp::LESS_OR_EQUAL);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::DynamicViewport() {
		return DynamicState(DynamicState::VIEWPORT);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::DynamicScissor() {
		return DynamicState(DynamicState::SCISSOR);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::DynamicViewportScissor() {
		return DynamicViewport().DynamicScissor();
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::TriangleList() {
		return Topology(PrimitiveTopology::TRIANGLE_LIST);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::TriangleStrip() {
		return Topology(PrimitiveTopology::TRIANGLE_STRIP);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::LineList() {
		return Topology(PrimitiveTopology::LINE_LIST);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::LineStrip() {
		return Topology(PrimitiveTopology::LINE_STRIP);
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::PointList() {
		return Topology(PrimitiveTopology::POINT_LIST);
	}
	GraphicsPipelineBuilder::~GraphicsPipelineBuilder() {
		if (m_Handle) 
			delete reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
	}
	void GraphicsPipelineBuilder::AddShaderStage(const char* filename, ShaderStage stage) {
		ShaderModule module = CreateShaderModule(filename);
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->shaderModules.push_back(module);
		AddShaderStage(module, stage, "main");
	}
	void GraphicsPipelineBuilder::AddShaderStage(ShaderModule module, ShaderStage stage, const char* entry) {
		GraphicsPipelineBuilder_T* builder = reinterpret_cast<GraphicsPipelineBuilder_T*>(m_Handle);
		builder->shaderStages.emplace_back();
		auto& shaderStage = builder->shaderStages.back();
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStage.module = VK<VkShaderModule>(module);
		shaderStage.pName = entry;
		shaderStage.pSpecializationInfo = nullptr;
		shaderStage.flags = 0;
	}
}
#pragma endregion GRAPHICS_PIPELINE_BUILDER

#pragma region COMMAND_POOL
namespace SGF::GPU {
	static_assert(sizeof(CommandList) == sizeof(VkCommandBuffer));
	constexpr CommandList SGFType(VkCommandBuffer buffer) {
		CommandList list = *(CommandList*)&buffer;
		return list;
	}
	CommandList CommandPool::Allocate() const {
		VkCommandBuffer buffer;
		Util::AllocateCommandBuffers((VkCommandPool)m_Handle, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &buffer);
		return SGFType(buffer);
	}
	CommandList CommandPool::AllocateSecondary() const {
		VkCommandBuffer buffer;
		Util::AllocateCommandBuffers((VkCommandPool)m_Handle, VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1, &buffer);
		return SGFType(buffer);
	}
	std::vector<CommandList> CommandPool::Allocate(uint32_t count) const {
		std::vector<CommandList> commands(count);
		Util::AllocateCommandBuffers((VkCommandPool)m_Handle, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(commands.size()), (VkCommandBuffer*)commands.data());
		return commands;
	}
	std::vector<CommandList> CommandPool::AllocateSecondary(uint32_t count) const {
		std::vector<CommandList> commands(count);
		Util::AllocateCommandBuffers((VkCommandPool)m_Handle, VK_COMMAND_BUFFER_LEVEL_SECONDARY, static_cast<uint32_t>(commands.size()), (VkCommandBuffer*)commands.data());
		return commands;
	}
	void CommandPool::AllocateToBuffer(CommandList* pCommandLists, uint32_t count) const {
		Util::AllocateCommandBuffers((VkCommandPool)m_Handle, VK_COMMAND_BUFFER_LEVEL_PRIMARY, count, (VkCommandBuffer*)pCommandLists);
	}
	void CommandPool::AllocateSecondaryToBuffer(CommandList* pCommandLists, uint32_t count) const {
		Util::AllocateCommandBuffers((VkCommandPool)m_Handle, VK_COMMAND_BUFFER_LEVEL_SECONDARY, count, (VkCommandBuffer*)pCommandLists);
	}	
	void CommandPool::Free(const CommandList& commandList) const {
		vkFreeCommandBuffers(s_LogicalDevice, static_cast<VkCommandPool>(m_Handle), 1, reinterpret_cast<const VkCommandBuffer*>(&commandList));
	}
	void CommandPool::Free(const CommandList* pCommandLists, uint32_t count) const {
		vkFreeCommandBuffers(s_LogicalDevice, static_cast<VkCommandPool>(m_Handle), count, reinterpret_cast<const VkCommandBuffer*>(pCommandLists));
	}	
	void CommandPool::Reset() {
		if (vkResetCommandPool(s_LogicalDevice, static_cast<VkCommandPool>(m_Handle), 0) != VK_SUCCESS) {
			Log::Fatal("{}", ERROR_RESET_COMMAND_POOL);
		}
	}
	void CommandPool::ResetRelease() {
		if (vkResetCommandPool(s_LogicalDevice, static_cast<VkCommandPool>(m_Handle), VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT) != VK_SUCCESS) {
			Log::Fatal("{}", ERROR_RESET_COMMAND_POOL);
		}
	}
}
#pragma endregion COMMAND_POOL


#pragma region DESCRIPTOR_SET_LAYOUT
namespace SGF::GPU {
	DescriptorType DescriptorSetLayout::GetDescriptorType(uint32_t index) const {
		SGF_ASSERT(m_Index < s_DescriptorSetLayouts.size());
		return (DescriptorType)s_DescriptorSetLayouts[m_Index].bindings[index].vkBinding.descriptorType;
	}
	DescriptorSetBinding DescriptorSetLayout::GetBinding(uint32_t index) const {
		SGF_ASSERT(m_Index < s_DescriptorSetLayouts.size());
		return *(const DescriptorSetBinding*)&(s_DescriptorSetLayouts[m_Index].bindings[index].vkBinding);
	}
	const DescriptorSetBinding* DescriptorSetLayout::GetBindings() const {
		static_assert(sizeof(DescriptorSetBinding) == sizeof(DescriptorSetLayoutBinding_T));
		return (const DescriptorSetBinding*)(s_DescriptorSetLayouts[m_Index].bindings.data());
	}
	uint32_t DescriptorSetLayout::GetBindingCount() const {
		SGF_ASSERT(m_Index < s_DescriptorSetLayouts.size());
		return s_DescriptorSetLayouts[m_Index].bindingCount;
	}
	void* DescriptorSetLayout::GetNativeHandle() const {
		SGF_ASSERT(m_Index < s_DescriptorSetLayouts.size());
		return s_DescriptorSetLayouts[m_Index].layout;
	}
}
#pragma endregion DESCRIPTOR_SET_LAYOUT
