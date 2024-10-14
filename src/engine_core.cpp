#include "engine_core.h"

#include "render/util.h"

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


void destroyFrameResources() {
	vkl::destroyImage(g_Device, g_DepthImage);
	vkl::destroyImageView(g_Device, g_DepthImageView);
	vkl::freeMemory(g_Device, g_DepthMemory);
	for (size_t i = 0; i < g_FrameResources.size(); ++i) {
		FrameResources& res = g_FrameResources[i];
		vkl::destroyFramebuffer;
	}
}

void onWindowResize(GLFWwindow * window, int width, int height) {
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
	g_Swapchain = vkl::createSwapchain(g_Device, g_PhysicalDevice, g_Surface, {(uint32_t)width, (uint32_t)height}, {});

}

void initWindow() {
	// GLFW: 
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	uint32_t window_width = 600;
	uint32_t window_height = 400;
	g_Window = glfwCreateWindow(window_width, window_height, PROJECT_NAME, nullptr, nullptr);
	glfwSetWindowSizeCallback(g_Window, onWindowResize);
	shl::logInfo("window created!");
	// Vulkan instance:
	uint32_t instance_extension_count;
	const char** instance_extensions = glfwGetRequiredInstanceExtensions(&instance_extension_count);
	if (instance_extensions == nullptr) {
		shl::logFatal("missing support for required glfw extensions!");
	}
	std::vector<const char*> extensions(instance_extensions, instance_extensions + instance_extension_count);
#ifdef VKL_ENABLE_VALIDATION
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	g_Instance = vkl::createInstance(VK_VERSION_1_0, extensions.size(), extensions.data(), debugCallback);
	shl::logInfo("Vulkan instance created!");
	g_DebugUtilsMessenger = vkl::createDebugUtilsMessengerEXT(g_Instance, debugCallback);
	shl::logInfo("debug messenger created!");
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
	shl::logInfo("Physical device picked!");
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
	shl::logInfo("logical device created!");

	vkGetDeviceQueue(g_Device, g_GraphicsIndex, 0, &g_GraphicsQueue);
	vkGetDeviceQueue(g_Device, g_PresentIndex, 0, &g_PresentQueue);


	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
	VkSurfaceFormatKHR surface_format = vkl::pickSurfaceFormat(g_PhysicalDevice, g_Surface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR });
	g_Swapchain = vkl::createSwapchain(g_Device, g_PhysicalDevice, g_Surface, { window_width, window_height }, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR }, g_PresentIndex, g_GraphicsIndex, ARRAY_SIZE(present_modes), present_modes);
	VkFormat possible_depth_formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkFormat depth_format = vkl::findSupportedFormat(g_PhysicalDevice, ARRAY_SIZE(possible_depth_formats), possible_depth_formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	// render pass:
	{
		VkAttachmentDescription attachments[] = {
			vkl::createAttachmentDescription(surface_format.format, VK_SAMPLE_COUNT_1_BIT, 
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),
			vkl::createAttachmentDescription(depth_format, VK_SAMPLE_COUNT_1_BIT, 
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		};
		VkAttachmentReference color_ref = vkl::createAttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkAttachmentReference depth_ref = vkl::createAttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		VkSubpassDescription subpasses[] = {
			vkl::createSubpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &color_ref, nullptr, depth_ref, 0, nullptr) 
		};
		VkSubpassDependency dependencies[] = {
			vkl::createSubpassDependency(VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
		};
		g_RenderPass = vkl::createRenderPass(g_Device, ARRAY_SIZE(attachments), attachments, ARRAY_SIZE(subpasses), subpasses, ARRAY_SIZE(dependencies), dependencies);
		g_DepthImage = util::createImage2D(window_width, window_height, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		g_DepthMemory = vkl::allocateForImage(g_Device, g_PhysicalDevice, g_DepthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkBindImageMemory(g_Device, g_DepthImage, g_DepthMemory, 0);
		g_DepthImageView = util::createImageView(g_DepthImage, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
		auto swapchain_images = vkl::getSwapchainImages(g_Device, g_Swapchain);
		g_FrameResources.resize(swapchain_images.size());
		for (size_t i = 0; i < swapchain_images.size(); ++i) {
			g_FrameResources[i].image = swapchain_images[i];
			g_FrameResources[i].imageView = util::createImageView(g_FrameResources[i].image, surface_format.format);
			g_FrameResources[i].commandPool = vkl::createCommandPool(g_Device, g_GraphicsIndex);
			g_FrameResources[i].commandBuffer = vkl::createCommandBuffer(g_Device, g_FrameResources[i].commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			VkImageView attachments[] = { g_FrameResources[i].imageView, g_DepthImageView };
			g_FrameResources[i].framebuffer = vkl::createFramebuffer(g_Device, g_RenderPass, ARRAY_SIZE(attachments), attachments, window_width, window_height, 1);
		}
	}
}
