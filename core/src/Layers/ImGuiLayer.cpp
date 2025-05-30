#pragma once

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

	ImGuiLayer::ImGuiLayer(const Window& window, VkSampleCountFlagBits sampleCount) : Layer("ImGuiLayer") {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		auto& device = Device::Get();
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
		};
		descriptorPool = device.descriptorPool(window.getImageCount(), pool_sizes, ARRAY_SIZE(pool_sizes), VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)window.getNativeWindow(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = VulkanInstance;
		init_info.PhysicalDevice = device;
		init_info.Device = device;
		init_info.QueueFamily = device.graphicsFamily();
		init_info.Queue = device.graphicsQueue(0);
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = descriptorPool;
		init_info.RenderPass = window.getRenderPass();
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = (uint32_t)window.getImageCount();
		init_info.MSAASamples = sampleCount;
		init_info.Allocator = VulkanAllocator;
		init_info.CheckVkResultFn = imguiVulkanCheckResult;
		ImGui_ImplVulkan_Init(&init_info);
		VkFormat requestedFormats[] = { Swapchain::DEFAULT_SURFACE_FORMAT.format, VK_FORMAT_B8G8R8A8_UNORM };
		ImGui_ImplVulkanH_SelectSurfaceFormat(device.getPhysical(), window.getSurface(), requestedFormats, 
			ARRAY_SIZE(requestedFormats), Swapchain::DEFAULT_SURFACE_FORMAT.colorSpace);
	}
	void ImGuiLayer::onUpdate(const UpdateEvent& event) {
		static bool showDemo = true;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow(&showDemo);
	}
	void ImGuiLayer::onRender(RenderEvent& event) {
		ImGui::Render();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		ImDrawData* draw_data = ImGui::GetDrawData();
		auto& commands = event.getCommands();
		const std::vector<VkClearValue> clearValues = {
				SGF::createColorClearValue(0.3f, 0.3f, 0.3f, 1.0f),
				//SGF::createDepthClearValue(1.0f, 0)
				//SGF::createColorClearValue(0.3f, 0.3f, 0.3f, 1.0f),
			};
		commands.beginRenderPass(event.getWindow(), clearValues);
		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(draw_data, commands.getCommands());
		commands.endRenderPass();
	}
	ImGuiLayer::~ImGuiLayer() {
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplVulkan_Shutdown();
		auto& device = Device::Get();
		device.destroy(descriptorPool);
	}
	void ImGuiLayer::onAttach() {
		//ImGui_ImplGlfw_InstallCallbacks();
	}
}
