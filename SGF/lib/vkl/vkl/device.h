#pragma once

#include "vkl_core.h"

namespace vkl {
	inline VkPhysicalDeviceFeatures getPhysicalDeviceFeatures(const VkPhysicalDevice physicalDevice) {
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);
		return features;
	}
	inline VkPhysicalDeviceProperties getPhysicalDeviceProperties(const VkPhysicalDevice physicalDevice) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		return properties;
	}
	inline VkDeviceQueueCreateInfo createDeviceQueueInfo(VkDeviceQueueCreateFlags flags, uint32_t queueFamilyIndex, const float& priority, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, pNext, flags, queueFamilyIndex, 1, &priority
		};
	}
	inline VkDeviceQueueCreateInfo createDeviceQueueInfo(VkDeviceQueueCreateFlags flags, uint32_t queueFamilyIndex, uint32_t queueCount, const float* pQueuePriorities, const void* pNext = nullptr) {
		return {
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, pNext, flags, queueFamilyIndex, queueCount, pQueuePriorities
		};
	}
	inline std::vector<VkQueueFamilyProperties> getQueueFamilyProperties(VkPhysicalDevice physicalDevice) {
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_families(count);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queue_families.data());
		return queue_families;
	}
	inline bool getPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t queueFamilyIndex) {
			VkBool32 support = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &support);
			return support == VK_TRUE;
	}
	inline uint32_t getQueueIndex(VkPhysicalDevice physicalDevice, VkQueueFlags flagBits) {
		std::vector<VkQueueFamilyProperties> queue_properties = getQueueFamilyProperties(physicalDevice);
		for (uint32_t i = 0; i < queue_properties.size(); ++i) {
			if (queue_properties[i].queueFlags & flagBits) {
				return i;
			}
		}
		return UINT32_MAX;
	}
	inline uint32_t getPresentQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
		std::vector<VkQueueFamilyProperties> queue_properties = getQueueFamilyProperties(physicalDevice);
		for (uint32_t i = 0; i < queue_properties.size(); ++i) {
			VkBool32 support = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &support);
			if (support) {
				return i;
			}
		}
		return UINT32_MAX;
	}
	inline bool checkPhysicalDeviceExtensionSupport(const VkInstance instance, const VkPhysicalDevice device, uint32_t extensionCount, const char* const* pExtensions) {
		bool support = true;
		uint32_t available_extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &available_extension_count, nullptr);
		VkExtensionProperties* available_extensions = new VkExtensionProperties[available_extension_count];
		vkEnumerateDeviceExtensionProperties(device, nullptr, &available_extension_count, available_extensions);
		for (uint32_t i = 0; i < extensionCount; ++i) {
			bool extension_supported = false;
			for (uint32_t j = 0; j < available_extension_count; ++j) {
				if (strcmp(available_extensions[j].extensionName, pExtensions[i]) == 0) {
					extension_supported = true;
					break;
				}
			}
			if (!extension_supported) {
				support = false;
				break;
			};
		}
		delete[] available_extensions;
		return support;
	}
	inline bool checkPhysicalDeviceSurfaceSupport(VkPhysicalDevice device, const VkSurfaceKHR surface) {
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
		if (count == 0) {
			surface_support = false;
		}
		return surface_support;
	}
	inline VkPhysicalDevice getPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surface, uint32_t extensionCount, const char* const* pExtensions) {
		uint32_t device_count;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
		VkPhysicalDevice* devices = new VkPhysicalDevice[device_count];
		vkEnumeratePhysicalDevices(instance, &device_count, devices);
		VkPhysicalDevice suitable_device = VK_NULL_HANDLE;
		for (uint32_t k = 0; k < device_count; ++k) {
			VkPhysicalDevice& device = devices[k];
			bool suitable = checkPhysicalDeviceExtensionSupport(instance, device, extensionCount, pExtensions);
			if (surface != nullptr) {
				suitable = suitable && checkPhysicalDeviceSurfaceSupport(device, surface);
			}
			if (suitable) {
				suitable_device = device;
				break;
			}
		}
		delete[] devices;
		return suitable_device;
	}
	inline std::vector<VkPhysicalDevice> getPhysicalDevices(const VkInstance instance, const VkSurfaceKHR surface, uint32_t extensionCount, const char* const* pExtensions) {
		uint32_t device_count;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
		VkPhysicalDevice* devices = new VkPhysicalDevice[device_count];
		std::vector<VkPhysicalDevice> suitable_devices;
		suitable_devices.reserve(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices);
		for (uint32_t k = 0; k < device_count; ++k) {
			VkPhysicalDevice& device = devices[k];
			bool suitable = checkPhysicalDeviceExtensionSupport(instance, device, extensionCount, pExtensions);
			if (surface != nullptr) {
				suitable = suitable && checkPhysicalDeviceSurfaceSupport(device, surface);
			}
			if (suitable) {
				suitable_devices.push_back(device);
				break;
			}
		}
		delete[] devices;
		suitable_devices.shrink_to_fit();
		return suitable_devices;
	}

	inline VkDevice createDevice(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface, const VkPhysicalDeviceFeatures& enabledFeatures, 
		uint32_t extensionCount, const char* const* pExtensions, uint32_t queueInfoCount, VkDeviceQueueCreateInfo* pQueueCreateInfos, bool validationEnabled = false) {
		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = queueInfoCount;
		create_info.pQueueCreateInfos = pQueueCreateInfos;
		create_info.pEnabledFeatures = &enabledFeatures;
		create_info.enabledExtensionCount = extensionCount;
		create_info.ppEnabledExtensionNames = pExtensions;

		if (validationEnabled) {
			create_info.enabledLayerCount = 1;
			create_info.ppEnabledLayerNames = &VKL_VALIDATION_LAYER_NAME;
		} else {
			create_info.enabledLayerCount = 0;
		}

		VkDevice device;
		VKL_CHECK(vkCreateDevice(physical_device, &create_info, VKL_Callbacks, &device), VKL_ERROR_DEVICE_CREATION_FAILED);
		return device;
	}
	inline void destroyDevice(VkDevice device) {
		vkDestroyDevice(device, VKL_Callbacks);
	}
}