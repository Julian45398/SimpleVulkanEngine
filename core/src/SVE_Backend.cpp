#ifdef SVE_WINDOWS 
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(SVE_LINUX)
#ifdef SVE_USE_X11
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(SVE_USE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#elif define(SVE_APPLE)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include "SVE_Backend.h"

#include <nfd_glfw3.h>


namespace SVE {
	using namespace _private;
	inline const VkFormat POSSIBLE_DEPTH_FORMATS[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	inline const VkPresentModeKHR PRESENT_MODES[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
	inline const VkFormat SURFACE_FORMATS[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };

	VkDescriptorPool ImGuiDescriptorPool = VK_NULL_HANDLE;

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

	void createPresentResources() {
		const VkColorSpaceKHR requested_colorspace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		VkSurfaceFormatKHR surface_format = vkl::pickSurfaceFormat(_Physical, _Surface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR });
		VkPresentModeKHR present_mode = vkl::pickPresentMode(_Physical, _Surface, ARRAY_SIZE(PRESENT_MODES), PRESENT_MODES);

		_Swapchain = vkl::createSwapchain(_Logical, _Physical, _Surface, _PresentIndex, _GraphicsIndex, { _WindowWidth, _WindowHeight }, surface_format, present_mode);

		VkFormat depth_format = vkl::findSupportedFormat(_Physical, ARRAY_SIZE(POSSIBLE_DEPTH_FORMATS), POSSIBLE_DEPTH_FORMATS, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		// render pass:
		{
			shl::logInfo("creating render passes: ");
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
			_RenderPass = vkl::createRenderPass(_Logical, ARRAY_SIZE(attachments), attachments, ARRAY_SIZE(subpasses), subpasses, ARRAY_SIZE(dependencies), dependencies);
#ifdef SVE_RENDER_IN_VIEWPORT
			// change attachments: 
			attachments[0].format = VK_FORMAT_R8G8B8A8_SRGB;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			shl::logInfo("TODO: change method to size viewport height and width!");
			_ViewportWidth = _WindowWidth - 100;
			_ViewportHeight = _WindowHeight - 100;
			_ViewportSampler = vkl::createSampler(_Logical, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
				
			VkImage attachment_images[FRAMES_IN_FLIGHT + 2];
			for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
				_ViewportImages[i] = vkl::createImage2D(_Logical, attachments[0].format, _ViewportWidth, _ViewportHeight,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
				attachment_images[i] = _ViewportImages[i];
			}
			_ViewportRenderPass = vkl::createRenderPass(_Logical, ARRAY_SIZE(attachments), attachments, ARRAY_SIZE(subpasses), subpasses, ARRAY_SIZE(dependencies), dependencies);
			_DepthImage = vkl::createImage2D(_Logical, depth_format, _WindowWidth, _WindowHeight, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
			_ViewportDepthImage = vkl::createImage2D(_Logical, depth_format, _ViewportWidth, _ViewportHeight, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
			attachment_images[FRAMES_IN_FLIGHT] = _DepthImage;
			attachment_images[FRAMES_IN_FLIGHT+1] = _ViewportDepthImage;

			_AttachmentMemory = vkl::allocateAndBind(_Logical, _Physical, ARRAY_SIZE(attachment_images), attachment_images, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			_DepthImageView = vkl::createImageView(_Logical, _DepthImage, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
			_ViewportDepthImageView = vkl::createImageView(_Logical, _ViewportDepthImage, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
			
			for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
				_ViewportImageViews[i] = vkl::createImageView(_Logical, _ViewportImages[i], attachments[0].format);
				VkImageView viewport_attachments[] = { _ViewportImageViews[i], _ViewportDepthImageView };
				_ViewportFramebuffers[i] = vkl::createFramebuffer(_Logical, _ViewportRenderPass, ARRAY_SIZE(viewport_attachments), viewport_attachments, _ViewportWidth, _ViewportHeight, 1);
			}
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			attachments[0].format = surface_format.format;
#else
			_DepthImage = vkl::createImage2D(_Logical, depth_format, _WindowWidth, _WindowHeight, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
			_AttachmentMemory = vkl::allocateForImage(_Logical, _Physical, _DepthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			_DepthImageView = vkl::createImageView2D(_Logical, _DepthImage, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
#endif

			auto images = vkl::getSwapchainImages(_Logical, _Swapchain);
			_ImageResources.resize(images.size());
			for (size_t i = 0; i < images.size(); ++i) {
				_ImageResources[i].image = images[i];
				_ImageResources[i].imageView = vkl::createImageView2D(_Logical, images[i], surface_format.format);
				_ImageResources[i].commandPool = vkl::createCommandPool(_Logical, _GraphicsIndex);
				_ImageResources[i].primaryCommands = vkl::createCommandBuffer(_Logical, _ImageResources[i].commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
#ifdef SVE_RENDER_IN_VIEWPORT
				_ImageResources[i].imGuiCommands = vkl::createCommandBuffer(_Logical, _ImageResources[i].commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
#else
				_ImageResources[i].imGuiCommands = vkl::createCommandBuffer(_Logical, _ImageResources[i].commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
#endif
				VkImageView attachments[] = { _ImageResources[i].imageView, _DepthImageView };
				_ImageResources[i].framebuffer = vkl::createFramebuffer(_Logical, _RenderPass, ARRAY_SIZE(attachments), attachments, _WindowWidth, _WindowHeight, 1);
			}
		}
	}
 
	void destroyPresentResources() {
		vkl::destroySwapchain(_Logical, _Swapchain);
		vkl::destroyRenderPass(_Logical, _RenderPass);
		vkl::destroyImageView(_Logical, _DepthImageView);
		vkl::destroyImage(_Logical, _DepthImage);
		vkl::freeMemory(_Logical, _AttachmentMemory);
		for (size_t i = 0; i < _ImageResources.size(); ++i) {
			vkl::destroyCommandPool(_Logical, _ImageResources[i].commandPool);
			vkl::destroyFramebuffer(_Logical, _ImageResources[i].framebuffer);
			vkl::destroyImageView(_Logical, _ImageResources[i].imageView);
		}
	}
	void setupSynchronization() {
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
			_Synchronization[i].fence = vkl::createFence(_Logical, VK_FENCE_CREATE_SIGNALED_BIT);
			_Synchronization[i].imageAvailable = vkl::createSemaphore(_Logical);
			_Synchronization[i].renderFinished = vkl::createSemaphore(_Logical);
		}
	}
	void destroySynchronization() {
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
			vkl::destroyFence(_Logical, _Synchronization[i].fence);
			vkl::destroySemaphore(_Logical, _Synchronization[i].imageAvailable);
			vkl::destroySemaphore(_Logical, _Synchronization[i].renderFinished);
#ifdef SVE_RENDER_IN_VIEWPORT
			vkl::destroySemaphore(_Logical, _Synchronization[i].viewportRenderFinished);
#endif
		}
	}
	void onFramebufferResize() {
		for (size_t i = 0; i < _FramebufferResizeCallbackListeners.size(); ++i) {
			auto& listener = _FramebufferResizeCallbackListeners[i];
			auto function = _FramebufferResizeCallbackFunctions[listener.callbackIndex];
			function(listener.data);
		}
	}
	// recreates the swapchain and framebuffers
	void onWindowResize(uint32_t width, uint32_t height) {
		_WindowWidth = width;
		_WindowHeight = height;
		vkDeviceWaitIdle(_Logical);
		destroyPresentResources();
		createPresentResources();
		onFramebufferResize();
		//displayResizeCallback(onDisplayResizePtr);
	}
	void vulkanCheckResult(VkResult result) {
		VKL_CHECK(result, "ImGui error");
	}

	void setupImGui() {
		shl::logDebug("setting up imgui!");
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = SVE::getImageCount();
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		vkCreateDescriptorPool(_Logical, &pool_info, nullptr, &ImGuiDescriptorPool);

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan(_Window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = _Instance;
		init_info.PhysicalDevice = _Physical;
		init_info.Device = _Logical;
		init_info.QueueFamily = _GraphicsIndex;
		init_info.Queue = _PresentQueue;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = ImGuiDescriptorPool;
		init_info.RenderPass = _RenderPass;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = (uint32_t)_ImageResources.size();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = vkl::VKL_Callbacks;
		init_info.CheckVkResultFn = vulkanCheckResult;
		ImGui_ImplVulkan_Init(&init_info);

#ifdef SVE_RENDER_IN_VIEWPORT
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i) {
			shl::logInfo("adding textures: ", _ViewportSampler, ", ", _ViewportImageViews[i]);
			_ViewportTextureIDs[i] = ImGui_ImplVulkan_AddTexture(_ViewportSampler, _ViewportImageViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
#endif
		shl::logInfo("ImGui initialized!");
	}
	void terminateImGui() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		destroyDescriptorPool(ImGuiDescriptorPool);
	}

	void setupVulkanInstance() {
		uint32_t instance_extension_count;
		const char** instance_extensions = glfwGetRequiredInstanceExtensions(&instance_extension_count);
		if (instance_extensions == nullptr) {
			shl::logFatal("missing support for required glfw extensions!");
		}
		std::vector<const char*> extensions(instance_extensions, instance_extensions + instance_extension_count);
	#ifdef VKL_ENABLE_VALIDATION
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		std::vector<VkLayerProperties> layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
		bool debug_support = false;
		shl::logDebug("available layers: ");
		for (uint32_t i = 0; i < layer_count; ++i) {
			shl::logDebug(layers[i].layerName);
			if (strcmp(vkl::VKL_VALIDATION_LAYER_NAME, layers[i].layerName) == 0) {
				debug_support = true;
				break;
			}
		}
		if (debug_support) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			_Instance = vkl::createInstance(VK_VERSION_1_0, (uint32_t)extensions.size(), extensions.data(), debugCallback);
			volkLoadInstance(_Instance);
			DebugUtilsMessenger = vkl::createDebugUtilsMessengerEXT(_Instance, debugCallback);
			if (!DebugUtilsMessenger) {
				shl::logError("failed to create debug messenger!");
			} else {
				shl::logInfo("debug messenger created!");
			}
		} else {
			shl::logWarn("Validation layers requested but not available!");
			_Instance = vkl::createInstance(VK_VERSION_1_0, (uint32_t)extensions.size(), extensions.data());
			volkLoadInstanceOnly(_Instance);
			DebugUtilsMessenger = nullptr;
		}
		shl::logInfo("Vulkan instance created!");
	#else
		_Instance = vkl::createInstance(VK_VERSION_1_0, extensions.size(), extensions.data());
		volkLoadInstance(_Instance);
	#endif
		assert(_Instance != VK_NULL_HANDLE);
	}
	void setupVulkanDevice() {
		const char* device_extensions[]{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		// picking physical device:
		std::vector<VkPhysicalDevice> physical_devices = vkl::getPhysicalDevices(_Instance, _Surface, ARRAY_SIZE(device_extensions), device_extensions);
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
				_Physical = device;
				max_score = score;
			}
		}
		if (_Physical == VK_NULL_HANDLE) {
			shl::logFatal("failed to find suitable device!");
		}
		shl::logInfo("Physical device picked!");
		// creating logical device:
		auto features = vkl::getPhysicalDeviceFeatures(_Physical);
		_GraphicsIndex = vkl::getQueueIndex(_Physical, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
		uint32_t queue_count = 1;
		VkDeviceQueueCreateInfo queue_infos[2] = {};
		float prio = 1.0f;
		queue_infos[0] = vkl::createDeviceQueueInfo(VKL_FLAG_NONE, _GraphicsIndex, 1, &prio);
		if (!vkl::getPhysicalDeviceSurfaceSupportKHR(_Physical, _Surface, _GraphicsIndex)) {
			_PresentIndex = vkl::getPresentQueueFamilyIndex(_Physical, _Surface);
			queue_infos[1] = vkl::createDeviceQueueInfo(VKL_FLAG_NONE, _PresentIndex, 1, &prio);
			queue_count++;
		}
		if (_GraphicsIndex == UINT32_MAX || _PresentIndex == UINT32_MAX) {
			shl::logFatal("no sufficient queue family found!");
		}
#ifdef VKL_ENABLE_VALIDATION
		_Logical = vkl::createDevice(_Physical, _Surface, features, ARRAY_SIZE(device_extensions), device_extensions, queue_count, queue_infos, DebugUtilsMessenger != nullptr);
#else
		_Logical = vkl::createDevice(_Physical, _Surface, features, ARRAY_SIZE(device_extensions), device_extensions, queue_count, queue_infos, false);
#endif 
		shl::logInfo("logical device created!");
		volkLoadDevice(_Logical);

		vkGetDeviceQueue(_Logical, _GraphicsIndex, 0, &_GraphicsQueue);
		vkGetDeviceQueue(_Logical, _PresentIndex, 0, &_PresentQueue);
	}
	void windowResizeCallback(GLFWwindow* window, int width, int height) {
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		//RenderCore& user = *(RenderCore*)glfwGetWindowUserPointer(window);
		onWindowResize(width, height);
	}
	void setupWindow(uint32_t width, uint32_t height) {
		// GLFW: 
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		_Window = glfwCreateWindow(width, height, PROJECT_NAME, nullptr, nullptr);
		if (_Window == nullptr) {
			shl::logFatal("failed to create window!");
		}
		_WindowWidth = width;
		_WindowHeight = height;
		glfwSetWindowSizeCallback(_Window, windowResizeCallback);
		//glfwSetWindowUserPointer(Window, this);
		// Vulkan instance:
		if (glfwCreateWindowSurface(_Instance, _Window, vkl::VKL_Callbacks, &_Surface) != VK_SUCCESS || _Surface == VK_NULL_HANDLE) {
			shl::logFatal("failed to create window surface!");
		}
		else {
			shl::logDebug("surface created!");
		}
	}


	
	void init(uint32_t windowWidth, uint32_t windowHeight) {
		if (volkInitialize() != VK_SUCCESS) {
			shl::logFatal("failed to initialize volk!");
		} else {
			shl::logInfo("volk initialized");
		}
		glfwInit();
		shl::logInfo("glfw initialized");
		setupVulkanInstance();
		shl::logDebug("vulkan instance created!");
		setupWindow(windowWidth, windowHeight);
		shl::logDebug("window created!");
		setupVulkanDevice();
		shl::logDebug("device created!");
		createPresentResources();
		shl::logDebug("swapchain created!");
		setupImGui();
		shl::logDebug("imgui initialized!");
		setupSynchronization();
		_FrameTimer.reset();
		_FrameTime = 0;
		shl::logInfo("backend initialized!");
	}
	void terminate() {
		deviceWaitIdle();
		shl::logDebug("terminating application");
		//ImGui_ImplVulkan_Shutdown();
		terminateImGui();
		shl::logDebug("imgui vulkan deinitialized");
		destroyPresentResources();
		destroySynchronization();
		shl::logInfo("destroying device...");
		vkl::destroyDevice(_Logical);
		shl::logInfo("Destroyed vulkan device");
#ifdef VKL_ENABLE_VALIDATION
		vkl::destroyDebugUtilsMessengerEXT(_Instance, DebugUtilsMessenger);
#endif
		vkl::destroySurface(_Instance, _Surface);
		vkl::destroyInstance(_Instance);
		shl::logInfo("Destroyed vulkan instance");
		glfwDestroyWindow(_Window);
		shl::logInfo("destroyed glfw window");
	}
	uint32_t addFramebufferResizeCallbackFunction(CallbackFunction callback) {
		uint32_t index = (uint32_t)_FramebufferResizeCallbackFunctions.size();
		_FramebufferResizeCallbackFunctions.push_back(callback);
		return index;
	}

	uint32_t addFramebufferResizeCallbackListener(uint32_t callbackFunctionIndex, void* listener) {
		uint32_t index = (uint32_t)_FramebufferResizeCallbackListeners.size();
		_FramebufferResizeCallbackListeners.push_back({ listener, callbackFunctionIndex });
		return index;
	}
	void setViewport(uint32_t width, uint32_t height, uint32_t xOffset, uint32_t yOffset) {
		_private::_ViewportWidth = (uint32_t)width;
		_private::_ViewportHeight = (uint32_t)height;
		_private::_ViewportOffsetX = (uint32_t)xOffset;
		_private::_ViewportOffsetY = (uint32_t)yOffset;
		//onFramebufferResize();
	}


	std::string openFileDialog(uint32_t filterCount, const nfdu8filteritem_t* filters) {
		NFD_Init();

		nfdu8char_t* outPath;
		nfdopendialogu8args_t args = { 0 };

		NFD_GetNativeWindowFromGLFWWindow(_Window, &args.parentWindow);
		args.filterList = static_cast<const nfdu8filteritem_t*>(filters);
		args.filterCount = filterCount;
		shl::logDebug("filter count: ", filterCount);
		nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

		std::string filepath;
		if (result == NFD_OKAY) {
			filepath = outPath;
			shl::logInfo("Picked file: ", filepath);
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL) {
			shl::logInfo("User pressed cancel.");
		}
		else {
			shl::logError(NFD_GetError());
		}

		NFD_Quit();

		return filepath;
	}
	std::string saveFileDialog(uint32_t filterCount, const nfdu8filteritem_t* filters) {
		NFD_Init();

		nfdu8char_t* outPath;
		nfdsavedialogu8args_t args = {};
		NFD_GetNativeWindowFromGLFWWindow(_Window, &args.parentWindow);
		args.filterList = static_cast<const nfdu8filteritem_t*>(filters);
		args.filterCount = filterCount;
		nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);

		std::string filepath;
		if (result == NFD_OKAY)
		{
			filepath = outPath;
			shl::logInfo("User picked savefile: ", filepath);
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL)
		{
			shl::logInfo("User pressed cancel.");
		}
		else
		{

			shl::logError(NFD_GetError());
		}

		NFD_Quit();

		return filepath;
	}
}
