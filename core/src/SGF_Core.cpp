#include "SGF_Core.hpp"

#include <volk.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <cstring>

#define SGF_ENGINE_NAME "SGF"
#define SGF_ENGINE_VERSION VK_MAKE_API_VERSION(0, 0, 0, 1)

#ifndef SGF_APP_VERSION
#define SGF_APP_VERSION SGF_ENGINE_VERSION
#endif
#ifndef SGF_APP_NAME
#define SGF_APP_NAME SGF_ENGINE_NAME 
#endif

namespace SGF {
    VkInstance VulkanInstance = VK_NULL_HANDLE;
    VkAllocationCallbacks* VulkanAllocator = nullptr;
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

    void init() {
#ifdef SGF_LOG_FILE
		_LogFile.open(SGF_LOG_FILE);
#endif
        glfwInit();
        initVulkan();
    }
    void terminate() {
        glfwTerminate();
        terminateVulkan();
#ifdef SGF_LOG_FILE
		_LogFile.close();
#endif
    }
	bool isInitialized() {
		return VulkanInstance != VK_NULL_HANDLE;
	}
}