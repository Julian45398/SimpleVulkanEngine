#pragma once

#include "vkl_core.h"
#include <assert.h>
#include <iostream>

namespace vkl {
	inline VkDebugUtilsMessengerEXT createDebugUtilsMessengerEXT(const VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback) {
		VkDebugUtilsMessengerCreateInfoEXT create_info = {
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, nullptr, 0,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			debugCallback, nullptr
		};
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		VkDebugUtilsMessengerEXT messenger;
		if (func != nullptr && func(instance, &create_info, VKL_Callbacks, &messenger) == VK_SUCCESS) {
			return messenger;
		}
		else {
			return VK_NULL_HANDLE;
		}
	}
	inline void destroyDebugUtilsMessengerEXT(const VkInstance instance, VkDebugUtilsMessengerEXT messenger) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, messenger, VKL_Callbacks);
		}
	}
	inline void destroySurface(const VkInstance instance, VkSurfaceKHR surface) {
		vkDestroySurfaceKHR(instance, surface, VKL_Callbacks);
	}

	inline VkInstance createInstance(uint32_t apiVersion, uint32_t extensionCount, const char* const* pExtensions, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback = nullptr) {
		assert(!(pExtensions == nullptr && extensionCount != 0));
		VkApplicationInfo app_info {
			VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, VKL_APPLICATION_NAME, VKL_APPLICATION_VERSION, VKL_ENGINE_NAME, VKL_ENGINE_VERSION, apiVersion 
		};
		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.enabledExtensionCount = extensionCount;
		create_info.ppEnabledExtensionNames = pExtensions;
		if (debugCallback) {
			VkDebugUtilsMessengerCreateInfoEXT debug_info = {
				VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, nullptr, 0,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				debugCallback, nullptr
			};
			assert(extensionCount != 0);
			assert(pExtensions != nullptr);

			create_info.pNext = &debug_info;
			create_info.enabledLayerCount = 1;
			create_info.ppEnabledLayerNames = &VKL_VALIDATION_LAYER_NAME;
		} else {
			create_info.pNext = nullptr;
			create_info.enabledLayerCount = 0;
			create_info.ppEnabledLayerNames = nullptr;
		
		}
		VkInstance instance;
		VKL_CHECK(vkCreateInstance(&create_info, VKL_Callbacks, &instance), VKL_ERROR_INSTANCE_CREATION_FAILED);
		return instance;
	}
	inline void destroyInstance(VkInstance instance) {
		vkDestroyInstance(instance, VKL_Callbacks);
	}
}
