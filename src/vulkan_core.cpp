#include "vulkan_core.h"

namespace sve {

#ifndef NDEBUG
	inline VkDebugUtilsMessengerEXT G_VkValidation = VK_NULL_HANDLE;
	inline const char VK_VALIDATION_LAYER[] = "VK_LAYER_KHRONOS_validation";
	inline VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		switch (message_severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			//shl::logDebug(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			shl::logWarn(pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			shl::logError(pCallbackData->pMessage);
			break;
		}
		return VK_FALSE;
	}
	inline VkDebugUtilsMessengerCreateInfoEXT setupDebugMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT debug_callback)
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = debug_callback;
		return create_info;
	}
	inline void createDebugUtilsMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info = setupDebugMessengerCreateInfo(debugCallback);
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VKL_Instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr && func(VKL_Instance, &create_info, VKL_Callbacks, &VKL_DebugMessenger) == VK_SUCCESS) {
			shl::logInfo("Validation layers enabled!");
		}
		else {
			shl::logWarn("failed to create vulkan debug messenger!");
		}
	}
	inline void destroyDebugUtilsMessenger() {
		if (VKL_DebugMessenger == VK_NULL_HANDLE) {
			return;
		}
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VKL_Instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(VKL_Instance, VKL_DebugMessenger, VKL_Callbacks);
			VKL_DebugMessenger = VK_NULL_HANDLE;
		}
		else {
			shl::logWarn("failed to destroy debug messenger!");
		}
	}
#endif
	namespace util {
		struct QueueIndices {
			uint32_t present;
			uint32_t graphics;
			uint32_t compute;
			uint32_t transfer;
			bool presentSupport;
			bool graphicSupport;
			bool computeSupport;
			bool transferSupport;
		};
		VkQueueFamilyProperties* getQueueFamilyProperties(VkPhysicalDevice physical_device, uint32_t* queue_family_count) {
			uint32_t count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
			VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)malloc(count * sizeof(VkQueueFamilyProperties));

			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, queue_families);
			*queue_family_count = count;
			return queue_families;
		}
		QueueIndices getQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface) {
			uint32_t count = 0;
			VkQueueFamilyProperties* queue_families = getQueueFamilyProperties(device, &count);
			QueueIndices indices{};
			for (uint32_t i = 0; i < count; i++) {
				VkBool32 support = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &support);
				if (!indices.presentSupport && support == VK_TRUE) {
					indices.present = i;
					indices.presentSupport = true;
				}
				bool compute_support = queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
				bool graphic_support = queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
				bool transfer_support = queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT;
				if (graphic_support && transfer_support && compute_support) {
					if (!indices.graphicSupport) {
						indices.graphics = i;
						indices.graphicSupport = true;
					}
				}
				else if (transfer_support && compute_support) {
					if (!indices.computeSupport) {
						indices.compute = i;
						indices.computeSupport = true;
					}
				}
				else if (transfer_support) {
					if (!indices.transferSupport) {
						indices.transfer = i;
						indices.transferSupport = true;
					}
				}
				if (indices.transferSupport && indices.presentSupport && indices.graphicSupport && indices.computeSupport) {
					break;
				}
			}
			free(queue_families);
			if (!indices.computeSupport) {
				indices.compute = indices.graphics;
			}
			if (!indices.transferSupport) {
				indices.transfer = indices.graphics;
			}
			return indices;
		}

		bool checkRequiredExtensionSupport(VkPhysicalDevice physical_device, uint32_t extension_count, const char* const* extensions)
		{
			uint32_t available_extension_count;
			vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extension_count, nullptr);
			std::vector<VkExtensionProperties> availableExtensions(available_extension_count);
			vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extension_count, availableExtensions.data());
			bool extension_present = false;
			for (uint32_t i = 0; i < extension_count; ++i) {
				for (uint32_t j = 0; j < available_extension_count; ++j) {
					if (strcmp(availableExtensions[j].extensionName, extensions[i]) == 0) {
						extension_present = true;
					}
				}
				if (!extension_present) return false;
				extension_present = false;
			}
			return true;
		}
		uint32_t chooseSwapchainImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
		{
			uint32_t image_count = capabilities.minImageCount + 1;
			if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
				image_count = capabilities.maxImageCount;
			}
			return image_count;
		}
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkPhysicalDevice physical_device)
		{
			uint32_t format_count;
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, VKL_Surface, &format_count, nullptr);
			std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
			if (format_count != 0) {
				vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, VKL_Surface, &format_count, surface_formats.data());
			}
			else {
				shl::logFatal("no surface format available!");
				return { VK_FORMAT_MAX_ENUM, VK_COLOR_SPACE_MAX_ENUM_KHR };
			}

			for (const auto& available_format : surface_formats) {
				if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return available_format;
				}
			}
			return surface_formats[0];
		}
		VkPresentModeKHR choosePresentMode(VkPhysicalDevice physical_device)
		{
			uint32_t present_mode_count;
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, VKL_Surface, &present_mode_count, nullptr);
			std::vector<VkPresentModeKHR> present_modes(present_mode_count);
			if (present_mode_count != 0) {
				vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, VKL_Surface, &present_mode_count, present_modes.data());
			}
			else {
				shl::logWarn("no present mode available!");
				return VK_PRESENT_MODE_MAX_ENUM_KHR;
			}

			for (const auto& available_present_mode : present_modes) {
				if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
					return available_present_mode;
				}
			}
			return VK_PRESENT_MODE_FIFO_KHR;
		}
		bool checkDeviceWindowSupport(VkPhysicalDevice physical_device)
		{
			QueueIndices indices = getQueueFamilyIndices(physical_device, VKL_Surface);

			VkSurfaceFormatKHR format = chooseSwapSurfaceFormat(physical_device);
			VkPresentModeKHR present_mode = choosePresentMode(physical_device);
			bool swapChainAdequate = format.format != VK_FORMAT_MAX_ENUM && present_mode != VK_PRESENT_MODE_MAX_ENUM_KHR;

			return indices.presentSupport && swapChainAdequate;
		}
		uint32_t rateDeviceSuitability(VkPhysicalDevice physical_device, uint32_t extension_count, const char* const* required_device_extensions)
		{
			if (!checkRequiredExtensionSupport(physical_device, extension_count, required_device_extensions)
				|| !checkDeviceWindowSupport(physical_device)) {
				return 0;
			}
			VkPhysicalDeviceProperties device_properties;
			vkGetPhysicalDeviceProperties(physical_device, &device_properties);

			VkPhysicalDeviceFeatures device_features;
			vkGetPhysicalDeviceFeatures(physical_device, &device_features);

			// Application can't function without geometry shaders
			if (!device_features.geometryShader) {
				return 0;
			}

			uint32_t score = 0;

			// Discrete GPUs have a significant performance advantage
			if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				score += 10000u;
			}
			// Maximum possible size of textures affects graphics quality
			score += device_properties.limits.maxImageDimension2D;

			return score;
		}
	}

	inline void setupVulkan() {
		// Instance:
		{
			VkApplicationInfo app_info{
				VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, PROJECT_NAME,
				VK_MAKE_API_VERSION(0, PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_MINOR),
				PROJECT_NAME,
				VK_MAKE_API_VERSION(0, PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_MINOR),
				VK_VERSION_1_0
			};
			int ext_count;
			const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&ext_count);
			if (glfw_extensions == nullptr) {
				shl::logFatal("missing support for required glfw extensions");
			}

			std::vector<const char*> extensions(glfw_extensions, glfw_extensions + ext_count);
#ifndef NDEBUG
			auto debug_info = setupDebugMessengerCreateInfo(debugCallback);
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
			VkInstanceCreateInfo info{
				VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#ifndef NDEBUG
				& debug_info, VK_FLAG_NONE, &app_info,
				1, &VK_VALIDATION_LAYER,
#else
				nullptr, VK_FLAG_NONE, &app_info,
				0, nullptr,
#endif
				extensions.size(), extensions.data()
			};
			if (vkCreateInstance(&info, &G_Callbacks, &G_VkInstance) != VK_SUCCESS) {
				shl::logFatal("failed to create vulkan instance!");
			}
#ifndef NDEBUG
			createDebugUtilsMessenger();
#endif
		}
		// Surface:
		{
			if (glfwCreateWindowSurface(G_VkInstance, G_Window, &G_Callbacks, &G_VkSurface) != VK_SUCCESS) {
				shl::logFatal("failed to create window surface!");
			}
		}
		// Device:
		{
			uint32_t physical_device_count;
			vkEnumeratePhysicalDevices(G_VkInstance, &physical_device_count, nullptr);
			std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
			vkEnumeratePhysicalDevices(G_VkInstance, &physical_device_count, physical_devices.data());
			uint32_t max_score = 0;
			const char const* required_device_extensions[] = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
			for (VkPhysicalDevice device : physical_devices) {
				uint32_t score = util::rateDeviceSuitability(device, ARRAY_SIZE(required_device_extensions), required_device_extensions);
				if (max_score < score) {
					max_score = score;
					G_VkPhysicalDevice = device;
				}
			}
			if (G_VkPhysicalDevice == VK_NULL_HANDLE) {
				shl::logFatal("no suitable gpu found!");
			}
			util::QueueIndices indices = util::getQueueFamilyIndices(VKL_PhysicalDevice, VKL_Surface);

			VKL_PresentIndex = indices.present;
			VKL_GraphicsIndex = indices.graphics;

			std::vector<VkDeviceQueueCreateInfo> queue_infos;
			std::set<uint32_t> unique_queue_families;
			unique_queue_families.insert(indices.graphics);
			unique_queue_families.insert(indices.present);

			float queuePriority = 1.0f;
			for (uint32_t queue_family : unique_queue_families) {
				VkDeviceQueueCreateInfo queue_info{};
				queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queue_info.queueFamilyIndex = queue_family;
				queue_info.queueCount = 1;
				queue_info.pQueuePriorities = &queuePriority;
				queue_infos.push_back(queue_info);
			}

			VkPhysicalDeviceFeatures device_features;
			vkGetPhysicalDeviceFeatures(VKL_PhysicalDevice, &device_features);

			VkDeviceCreateInfo create_info{
				VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, VK_FLAG_NONE, (uint32_t)queue_infos.size(), queue_infos.data(), 
#ifndef NDEBUG	
				1, &VK_VALIDATION_LAYER, 
#else
				0, nullptr, 
#endif
				ARRAY_SIZE(required_device_extensions), required_device_extensions.data(), &device_features
			};
			if (vkCreateDevice(G_VkPhysicalDevice, &create_info, &G_Callbacks, &G_VkDevice) != VK_SUCCESS) {
				shl::logFatal("failed to create logical device!");
			}
			vkGetDeviceQueue(G_VkDevice, indices.graphics, 0, &G_VkGraphicsQueue);
			vkGetDeviceQueue(G_VkDevice, indices.present, 0, &G_VkPresentQueue);
		}
	}
}