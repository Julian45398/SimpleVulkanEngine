#include "engine_core.h"


#ifdef VKL_ENABLE_VALIDATION
VkDebugUtilsMessengerEXT g_DebugUtilsMessenger = VK_NULL_HANDLE;
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
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
#endif

inline void initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	g_Window = glfwCreateWindow(600, 400, PROJECT_NAME, nullptr, nullptr);
	uint32_t instance_extension_count;
	const char** instance_extensions = glfwGetRequiredInstanceExtensions(&instance_extension_count);
	if (instance_extensions == nullptr) {
		shl::logFatal("missing support for required glfw extensions!");
	}
	std::vector<const char*> extensions(instance_extensions, instance_extensions + instance_extension_count);
#ifdef VKL_ENABLE_VALIDATION
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	g_Instance = vkl::createInstance(VK_VERSION_1_0, extensions.size(), extensions.data(), debugCallback);
	g_DebugUtilsMessenger = vkl::createDebugUtilsMessengerEXT(g_Instance, debugCallback);
#else
	g_Instance = vkl::createInstance(VK_VERSION_1_0, extensions.size(), extensions.data());
#endif
	assert(g_Instance != VK_NULL_HANDLE);
	if (glfwCreateWindowSurface(g_Instance, g_Window, vkl::VKL_Callbacks, &g_Surface) != VK_SUCCESS) {
		shl::logFatal("failed to create window surface!");
	}
	const char* device_extensions[]{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	// picking physical device:
	std::vector<VkPhysicalDevice> physical_devices = vkl::getPhysicalDevices(g_Instance, g_Surface, ARRAY_SIZE(device_extensions), device_extensions);
	uint32_t max_score = 0;
	for (auto device : physical_devices) {
		uint32_t score = 0;
		auto properties = vkl::getPhysicalDeviceProperties(device);
		auto features = vkl::getPhysicalDeviceFeatures(device);
		if (features.geometryShader == VK_FALSE) {
			continue;
		}
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}
		if (max_score <= score) {
			g_PhysicalDevice = device;
			max_score = score;
		}
	}
	if (g_PhysicalDevice == VK_NULL_HANDLE) {
		shl::logFatal("failed to find suitable device!");
	}
	// creating logical device:
	auto features = vkl::getPhysicalDeviceFeatures(g_PhysicalDevice);
	g_GraphicsIndex = vkl::getQueueIndex(g_PhysicalDevice, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
	uint32_t queue_count = 1;
	VkDeviceQueueCreateInfo queue_infos[2] = {};
	float prio = 1.0f;
	queue_infos[0] = vkl::createDeviceQueueInfo(VKL_FLAG_NONE, g_GraphicsIndex, 1, &prio);
	if (!vkl::getPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_Surface, g_GraphicsIndex)) {
		g_PresentIndex = vkl::getPresentQueueFamilyIndex(g_PhysicalDevice, g_Surface);
		queue_infos[1] = vkl::createDeviceQueueInfo(VKL_FLAG_NONE, g_PresentIndex, 1, &prio);
		queue_count++;
	}
	if (g_GraphicsIndex == UINT32_MAX || g_PresentIndex == UINT32_MAX) {
		shl::logFatal("no sufficient queue family found!");
	}
	g_Device = vkl::createDevice(g_PhysicalDevice, g_Surface, features, ARRAY_SIZE(device_extensions), device_extensions, queue_count, queue_infos);


	uint32_t width, height;
	glfwGetFramebufferSize(g_Window, (int*) &width, (int*) &height);
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
	g_Swapchain = vkl::createSwapchain(g_Device, g_PhysicalDevice, g_Surface, { width, height }, g_PresentIndex, g_GraphicsIndex, ARRAY_SIZE(present_modes), present_modes);
	
}