#include "engine_core.h"

#include "render/util.h"

#ifdef VKL_ENABLE_VALIDATION
VkDebugUtilsMessengerEXT DebugUtilsMessenger = VK_NULL_HANDLE;
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

void windowResizeCallback(GLFWwindow * window, int width, int height) {
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
	RenderCore& user = *(RenderCore*)glfwGetWindowUserPointer(window);
	user.onWindowResize(width, height);
}

void initBackend() {
	glfwInit();
}

void RenderCore::initialize(uint32_t windowWidth, uint32_t windowHeight) {
	setupVulkanInstance();
	setupWindow(windowWidth, windowHeight);
	setupVulkanDevice();
	createPresentResources();
	setupImGui();
	setupSynchronization();
	FrameTimer.reset();
	FrameTime = 0;
}
void RenderCore::terminate()
{
	glfwDestroyWindow(Window);
}
void RenderCore::createPresentResources() {
		const VkColorSpaceKHR requested_colorspace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		VkSurfaceFormatKHR surface_format = vkl::pickSurfaceFormat(PhysicalDevice, Surface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR });
		VkPresentModeKHR present_mode = vkl::pickPresentMode(PhysicalDevice, Surface, ARRAY_SIZE(PRESENT_MODES), PRESENT_MODES);

		Swapchain = vkl::createSwapchain(LogicalDevice, PhysicalDevice, Surface, PresentIndex, GraphicsIndex, {WindowWidth, WindowHeight}, surface_format, present_mode);
		
		VkFormat depth_format = vkl::findSupportedFormat(PhysicalDevice, ARRAY_SIZE(POSSIBLE_DEPTH_FORMATS), POSSIBLE_DEPTH_FORMATS, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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
			RenderPass = vkl::createRenderPass(LogicalDevice, ARRAY_SIZE(attachments), attachments, ARRAY_SIZE(subpasses), subpasses, ARRAY_SIZE(dependencies), dependencies);
			DepthImage = vkl::createImage2D(LogicalDevice, depth_format, WindowWidth, WindowHeight, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
			DepthMemory = vkl::allocateForImage(LogicalDevice, PhysicalDevice, DepthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			vkBindImageMemory(LogicalDevice, DepthImage, DepthMemory, 0);
			DepthImageView = vkl::createImageView(LogicalDevice, DepthImage, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

			auto images = vkl::getSwapchainImages(LogicalDevice, Swapchain);
			ImageRes.resize(images.size());
			for (size_t i = 0; i < images.size(); ++i) {
				ImageRes[i].image = images[i];
				ImageRes[i].imageView = vkl::createImageView(LogicalDevice, images[i], surface_format.format);
				ImageRes[i].commandPool = vkl::createCommandPool(LogicalDevice, GraphicsIndex);
				ImageRes[i].commandBuffer = vkl::createCommandBuffer(LogicalDevice, ImageRes[i].commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

				for (uint32_t j = 0; j < COMMAND_BUFFER_COUNT; ++j) {
					ImageRes[i].secondaryPools[j] = vkl::createCommandPool(LogicalDevice, GraphicsIndex);
					ImageRes[i].secondaryCommands[j] = vkl::createCommandBuffer(LogicalDevice, ImageRes[i].secondaryPools[j], VK_COMMAND_BUFFER_LEVEL_SECONDARY);
				}
				VkImageView attachments[] = { ImageRes[i].imageView, DepthImageView };
				ImageRes[i].framebuffer = vkl::createFramebuffer(LogicalDevice, RenderPass, ARRAY_SIZE(attachments), attachments, WindowWidth, WindowHeight, 1);
			}
		}

	}
void RenderCore::destroyPresentResources() {
	vkl::destroySwapchain(LogicalDevice, Swapchain);
	vkl::destroyRenderPass(LogicalDevice, RenderPass);
	vkl::destroyImageView(LogicalDevice, DepthImageView);
	vkl::destroyImage(LogicalDevice, DepthImage);
	vkl::freeMemory(LogicalDevice, DepthMemory);
	for (size_t i = 0; i < ImageRes.size(); ++i) {
		vkl::destroyCommandPool(LogicalDevice, ImageRes[i].commandPool);
		for (uint32_t j = 0; j < COMMAND_BUFFER_COUNT; ++j) {
			vkl::destroyCommandPool(LogicalDevice, ImageRes[i].secondaryPools[j]);
		}
		vkl::destroyFramebuffer(LogicalDevice, ImageRes[i].framebuffer);
		vkl::destroyImageView(LogicalDevice, ImageRes[i].imageView);
	}
}
void RenderCore::setupSynchronization() {
	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
		Sync[i].Fence = vkl::createFence(LogicalDevice, VK_FENCE_CREATE_SIGNALED_BIT);
		Sync[i].ImageAvailable = vkl::createSemaphore(LogicalDevice);
		Sync[i].RenderFinished = vkl::createSemaphore(LogicalDevice);
	}
}
void RenderCore::destroySynchronization() {
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
			vkl::destroyFence(LogicalDevice, Sync[i].Fence);
			vkl::destroySemaphore(LogicalDevice, Sync[i].ImageAvailable);
			vkl::destroySemaphore(LogicalDevice, Sync[i].RenderFinished);
		}
	}
// recreates the swapchain and framebuffers
void RenderCore::onWindowResize(uint32_t width, uint32_t height) {
	WindowWidth = width;
	WindowHeight = height;
	vkDeviceWaitIdle(LogicalDevice);
	destroyPresentResources();
	createPresentResources();
}
void vulkanCheckResult(VkResult result) {
	VKL_CHECK(result, "");
}

void RenderCore::setupImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	VkDescriptorPool pool;
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1;
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	vkCreateDescriptorPool(LogicalDevice, &pool_info, nullptr, &pool);

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(Window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = Instance;
	init_info.PhysicalDevice = PhysicalDevice;
	init_info.Device = LogicalDevice;
	init_info.QueueFamily = GraphicsIndex;
	init_info.Queue = PresentQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = pool;
	init_info.RenderPass = RenderPass;
	init_info.Subpass = 0;
	init_info.MinImageCount = 2;
	init_info.ImageCount = ImageRes.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = vkl::VKL_Callbacks;
	init_info.CheckVkResultFn = vulkanCheckResult;
	ImGui_ImplVulkan_Init(&init_info);
	shl::logInfo("ImGui initialized!");
}


void RenderCore::setupVulkanInstance() {
	uint32_t instance_extension_count;
	const char** instance_extensions = glfwGetRequiredInstanceExtensions(&instance_extension_count);
	if (instance_extensions == nullptr) {
		shl::logFatal("missing support for required glfw extensions!");
	}
	std::vector<const char*> extensions(instance_extensions, instance_extensions + instance_extension_count);
#ifdef VKL_ENABLE_VALIDATION
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	Instance = vkl::createInstance(VK_VERSION_1_0, extensions.size(), extensions.data(), debugCallback);
	shl::logInfo("Vulkan instance created!");
	DebugUtilsMessenger = vkl::createDebugUtilsMessengerEXT(Instance, debugCallback);
	shl::logInfo("debug messenger created!");
#else
	Instance = vkl::createInstance(VK_VERSION_1_0, extensions.size(), extensions.data());
#endif
	assert(Instance != VK_NULL_HANDLE);

}
void RenderCore::setupVulkanDevice() {
	const char* device_extensions[]{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	// picking physical device:
	std::vector<VkPhysicalDevice> physical_devices = vkl::getPhysicalDevices(Instance, Surface, ARRAY_SIZE(device_extensions), device_extensions);
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
			PhysicalDevice = device;
			max_score = score;
		}
	}
	if (PhysicalDevice == VK_NULL_HANDLE) {
		shl::logFatal("failed to find suitable device!");
	}
	shl::logInfo("Physical device picked!");
	// creating logical device:
	auto features = vkl::getPhysicalDeviceFeatures(PhysicalDevice);
	GraphicsIndex = vkl::getQueueIndex(PhysicalDevice, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
	uint32_t queue_count = 1;
	VkDeviceQueueCreateInfo queue_infos[2] = {};
	float prio = 1.0f;
	queue_infos[0] = vkl::createDeviceQueueInfo(VKL_FLAG_NONE, GraphicsIndex, 1, &prio);
	if (!vkl::getPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, Surface, GraphicsIndex)) {
		PresentIndex = vkl::getPresentQueueFamilyIndex(PhysicalDevice, Surface);
		queue_infos[1] = vkl::createDeviceQueueInfo(VKL_FLAG_NONE, PresentIndex, 1, &prio);
		queue_count++;
	}
	if (GraphicsIndex == UINT32_MAX || PresentIndex == UINT32_MAX) {
		shl::logFatal("no sufficient queue family found!");
	}
	LogicalDevice = vkl::createDevice(PhysicalDevice, Surface, features, ARRAY_SIZE(device_extensions), device_extensions, queue_count, queue_infos);
	shl::logInfo("logical device created!");

	vkGetDeviceQueue(LogicalDevice, GraphicsIndex, 0, &GraphicsQueue);
	vkGetDeviceQueue(LogicalDevice, PresentIndex, 0, &PresentQueue);
}

void RenderCore::setupWindow(uint32_t width, uint32_t height) {
	// GLFW: 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	Window = glfwCreateWindow(width, height, PROJECT_NAME, nullptr, nullptr);
	if (Window == nullptr) {
		shl::logFatal("failed to create window!");
	}
	WindowWidth = width;
	WindowHeight = height;
	glfwSetWindowSizeCallback(Window, windowResizeCallback);
	glfwSetWindowUserPointer(Window, this);
	// Vulkan instance:
	if (glfwCreateWindowSurface(Instance, Window, vkl::VKL_Callbacks, &Surface) != VK_SUCCESS || Surface == VK_NULL_HANDLE) {
		shl::logFatal("failed to create window surface!");
	}
	else {
		shl::logDebug("surface created!");
	}
}
