#include "Layers/ImGuiLayer.hpp"
#include "Render/Device.hpp"
#include "Window.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "Render/CommandList.hpp"

namespace SGF {
	extern VkInstance VulkanInstance;
	extern VkAllocationCallbacks* VulkanAllocator;
	void imguiVulkanCheckResult(VkResult result) {
		if (result != VK_SUCCESS) {
			fatal(ERROR_VULKAN_IMGUI);
		}
	}

	ImGuiLayer::ImGuiLayer(VkSampleCountFlagBits sampleCount) : Layer("ImGuiLayer") {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		auto& device = Device::Get();
		auto& window = Window::Get();
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
		};
		descriptorPool = device.CreateDescriptorPool(window.GetImageCount(), pool_sizes, ARRAY_SIZE(pool_sizes), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)window.GetNativeWindow().GetHandle(), false);
		ImGui_ImplGlfw_SetCallbacksChainForAllWindows(true);
		ImGui_ImplGlfw_InstallCallbacks((GLFWwindow*)window.GetNativeWindow().GetHandle());
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = VulkanInstance;
		init_info.PhysicalDevice = device;
		init_info.Device = device;
		init_info.QueueFamily = device.GetGraphicsFamily();
		init_info.Queue = device.GetGraphicsQueue(0);
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = descriptorPool;
		init_info.RenderPass = window.GetRenderPass();
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = (uint32_t)window.GetImageCount();
		init_info.MSAASamples = sampleCount;
		init_info.Allocator = VulkanAllocator;
		init_info.CheckVkResultFn = imguiVulkanCheckResult;
		ImGui_ImplVulkan_Init(&init_info);
		VkFormat requestedFormats[] = { Window::DEFAULT_SURFACE_FORMAT.format, VK_FORMAT_B8G8R8A8_UNORM };
		ImGui_ImplVulkanH_SelectSurfaceFormat(device.GetPhysical(), window.GetSurface(), requestedFormats, 
			ARRAY_SIZE(requestedFormats), Window::DEFAULT_SURFACE_FORMAT.colorSpace);
	}
	void ImGuiLayer::OnEvent(const UpdateEvent& event) {
		static bool showDemo = true;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	void ImGuiLayer::OnEvent(RenderEvent& event) {
		ImGui::Render();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		ImDrawData* draw_data = ImGui::GetDrawData();
		auto& commands = event.getCommands();
		const std::vector<VkClearValue> clearValues = {
				SGF::Vk::CreateColorClearValue(0.3f, 0.3f, 0.3f, 1.0f),
				//SGF::createDepthClearValue(1.0f, 0)
				//SGF::createColorClearValue(0.3f, 0.3f, 0.3f, 1.0f),
			};
		commands.BeginRenderPass(Window::Get());
		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(draw_data, commands.GetCommands());
		commands.EndRenderPass();
	}
	ImGuiLayer::~ImGuiLayer() {
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplVulkan_Shutdown();
		auto& device = Device::Get();
		device.Destroy(descriptorPool);
	}
	void ImGuiLayer::OnAttach() {
		//ImGui_ImplGlfw_InstallCallbacks();
	}
}
