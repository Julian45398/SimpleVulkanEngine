#include "SGF_Core.hpp"

#include <volk.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <cstring>

#include <cstdlib>

#include "Render/Device.hpp"
#include "Render/RenderPass.hpp"
#include "Render/CommandList.hpp"
#include "Layers/LayerStack.hpp"
#include "Layers/ImGuiLayer.hpp"
#include "Layers/ViewportLayer.hpp"
#include "Window.hpp"

namespace SGF {

    VkInstance VulkanInstance = VK_NULL_HANDLE;
#ifdef SGF_LOG_VULKAN_ALLOCATIONS
	const char COMMAND_SCOPE[] = "Command Scope!";
	const char OBJECT_SCOPE[] = "Object Scope!";
	const char SYSTEM_SCOPE[] = "System Scope!";
	const char CACHE_SCOPE[] = "Cache Scope!";
	const char DEVICE_SCOPE[] = "Device Scope!";
	const char INSTANCE_SCOPE[] = "Instance Scope!";
	void vulkanCallbackMessage(const char* operation, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
		char const* scope;
		switch (allocationScope)
		{
		case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND:
			scope = COMMAND_SCOPE;
			break;
		case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT:
			scope = OBJECT_SCOPE;
			break;
		case VK_SYSTEM_ALLOCATION_SCOPE_CACHE:
			scope = SYSTEM_SCOPE;
			break;
		case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE:
			scope = DEVICE_SCOPE;
			break;
		case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE:
			scope = INSTANCE_SCOPE;
			break;
		case VK_SYSTEM_ALLOCATION_SCOPE_MAX_ENUM:
			scope = "NO VALID SCOPE!";
			break;
		default:
			scope = "ERROR!!!!!!!!!";
			break;
		}
		SGF::debug(operation, scope, " size: ", size, " alignment: ", alignment);
	}
	void* vulkanAllocationFunction(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope scope) {
		vulkanCallbackMessage("Allocation: ", size, alignment, scope);
		void* result = _aligned_malloc(size, alignment);
		return result;
	}
	void* vulkanReallocationFunction(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope scope) {
		void* result = nullptr;
		if (pOriginal != nullptr) {
			vulkanCallbackMessage("Reallocation: ", size, alignment, scope);
			result = _aligned_realloc(pOriginal, size, alignment);
		}
		return result;
	}
	void vulkanFreeFunction(void* pUserData, void* pMemory) {
		if (pMemory != nullptr) {
			SGF::debug("Freeing vulkan memory: ", pMemory);
			_aligned_free(pMemory);
		}
	}
	void vulkanInternalAllocationFunction(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope) {
		if (allocationType == VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE) {
			vulkanCallbackMessage("Internal Executable Allocation: ", size, 0, scope);
		}
		else {
			vulkanCallbackMessage("Internal Allocation: ", size, 0, scope);
		}
	}
	void vulkanInternalFreeFunction(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope) {
		if (allocationType == VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE) {
			vulkanCallbackMessage("Internal Executable Free: ", size, 0, scope);
		}
		else {
			vulkanCallbackMessage("Internal Free: ", size, 0, scope);
		}
	}
	
	VkAllocationCallbacks ALLOCATION_LOGGER = { nullptr, vulkanAllocationFunction, vulkanReallocationFunction, vulkanFreeFunction, vulkanInternalAllocationFunction, vulkanInternalFreeFunction };
    VkAllocationCallbacks* VulkanAllocator = &ALLOCATION_LOGGER;
#else
    VkAllocationCallbacks* VulkanAllocator = nullptr;
#endif
#ifdef SGF_ENABLE_VALIDATION
	const char* VULKAN_MESSENGER_NAME = "VK_LAYER_KHRONOS_validation";
    VkDebugUtilsMessengerEXT VulkanMessenger = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT createDebugUtilsMessengerEXT(const VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback) {
		VkDebugUtilsMessengerCreateInfoEXT create_info = {
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, nullptr, 0,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			debugCallback, nullptr
		};
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		VkDebugUtilsMessengerEXT messenger;
		if (func != nullptr && func(instance, &create_info, VulkanAllocator, &messenger) == VK_SUCCESS) {
			return messenger;
		}
		else {
            warn("Failed to create vulkan messenger!");
			return VK_NULL_HANDLE;
		}
	}
	void destroyDebugUtilsMessengerEXT(const VkInstance instance, VkDebugUtilsMessengerEXT messenger) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, messenger, VulkanAllocator);
		}
	}
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		switch (message_severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			//Log.info(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			warn(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			error(pCallbackData->pMessage);
			break;
		}
		return VK_FALSE;
	}
#endif
    
    void initVulkan() {
        volkInitialize();
        // Vulkan instance:
		uint32_t instance_extension_count;
		const char** instance_extensions = glfwGetRequiredInstanceExtensions(&instance_extension_count);
		if (instance_extensions == nullptr) {
			fatal("missing support for required glfw extensions!");
		}
		std::vector<const char*> extensions(instance_extensions, instance_extensions + instance_extension_count);
        VkApplicationInfo app_info;
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr;
        app_info.apiVersion = VK_API_VERSION_1_0;
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
        uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		std::vector<VkLayerProperties> layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
		bool debug_support = false;
		debug("available layers: ");
		for (uint32_t i = 0; i < layer_count; ++i) {
			debug(layers[i].layerName);
			if (strcmp(VULKAN_MESSENGER_NAME, layers[i].layerName) == 0) {
				debug_support = true;
				break;
			}
		}
		if (debug_support) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            info.enabledLayerCount = 1;
            info.ppEnabledLayerNames = &VULKAN_MESSENGER_NAME;
        } else {
            info.enabledLayerCount = 0;
            info.ppEnabledLayerNames = nullptr;
        }
#else
        info.enabledLayerCount = 0;
        info.ppEnabledLayerNames = nullptr;
#endif
        info.enabledExtensionCount = (uint32_t)extensions.size();
        info.ppEnabledExtensionNames = extensions.data();
        if (vkCreateInstance(&info, VulkanAllocator, &VulkanInstance) != VK_SUCCESS) {
            fatal("Failed to create vulkan instance!");
        }
		volkLoadInstance(VulkanInstance);
#ifdef SGF_ENABLE_VALIDATION
        if (debug_support) {
			VulkanMessenger = createDebugUtilsMessengerEXT(VulkanInstance, debugCallback);
		} else {
			VulkanMessenger = nullptr;
		}
#endif
    }
    void terminateVulkan() {
#ifdef SGF_ENABLE_VALIDATION
        destroyDebugUtilsMessengerEXT(VulkanInstance, VulkanMessenger);
        VulkanMessenger = VK_NULL_HANDLE;
#endif
        vkDestroyInstance(VulkanInstance, VulkanAllocator);
        VulkanInstance = VK_NULL_HANDLE;
        volkFinalize();
    }

	uint32_t s_InFlightIndex = 0;
    uint32_t GetFrameIndex() {
		return s_InFlightIndex;
	}
    void Init() {
#ifdef SGF_LOG_FILE
		_LogFile.open(SGF_LOG_FILE);
#endif
        glfwInit();
        initVulkan();
		Device::PickNew();
		Window::Open();
    }
    void Terminate() {
		Window::Close();
		Device::Shutdown();
        glfwTerminate();
        terminateVulkan();
#ifdef SGF_LOG_FILE
		_LogFile.close();
#endif

    }
	bool IsInitialized() {
		return VulkanInstance != VK_NULL_HANDLE;
	}
	void Run() {
		auto& window = Window::Get();
		auto& device = Device::Get();
		
		struct PerFrame {
			VkSemaphore imageAvailable;
			VkSemaphore renderFinished;
			CommandList commands;
		};
		PerFrame perFrame[FRAMES_IN_FLIGHT] = {
			{ device.semaphore(), device.semaphore(), CommandList(device, QUEUE_TYPE_GRAPHICS, 0, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)},
			{ device.semaphore(), device.semaphore(), CommandList(device, QUEUE_TYPE_GRAPHICS, 0, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)}
		};
		Timer timer;

		double deltaTime = 0.0;
		uint32_t index = 0;
		while (!window.shouldClose()) {
			Input::PollEvents();
			if (Input::IsKeyPressed(KEY_G)) {
				info("g is pressed!");
			}
			UpdateEvent updateEvent(deltaTime);
			LayerStack::OnEvent(updateEvent);
			perFrame[index].commands.begin();
			window.nextFrame(perFrame[index].imageAvailable);
			RenderEvent renderEvent(deltaTime, perFrame[index].commands, window.getFramebufferSize());
			renderEvent.addWait(perFrame[index].imageAvailable, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			renderEvent.addSignal(perFrame[index].renderFinished);
			LayerStack::OnEvent(renderEvent);
			perFrame[index].commands.end();
			perFrame[index].commands.submit(renderEvent.getWait().data(), renderEvent.getWaitStages().data(), renderEvent.getWait().size(), renderEvent.getSignal().data(), renderEvent.getSignal().size());
			window.presentFrame(perFrame[index].renderFinished);
			deltaTime = timer.ellapsedMillis();
			index = (index + 1) % FRAMES_IN_FLIGHT;
		}
		device.waitIdle();
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
			device.destroy(perFrame[i].imageAvailable, perFrame[i].renderFinished);
		}
		LayerStack::Clear();
	}
}

int main() {
	SGF::PreInit();
	SGF::Init();
	SGF::Setup();
	SGF::Run();
	SGF::Cleanup();
	SGF::Terminate();
	return 0;
}