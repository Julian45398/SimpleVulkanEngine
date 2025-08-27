#pragma once

#include "SGF_Core.hpp"
#include "Layers/Layer.hpp"
#include "imgui.h"

namespace SGF {
	class ImGuiLayer : public Layer {
	public:
		static ImTextureID AddVulkanTexture(VkDescriptorPool descriptorPool, VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		static void UpdateVulkanTexture(ImTextureID textureID, VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		static void RemoveVulkanTexture(VkDescriptorPool pool, ImTextureID textureID);

		ImGuiLayer(VkSampleCountFlagBits sampleCount);
		virtual void OnAttach() override;
		virtual void OnEvent(const UpdateEvent& event) override;
		virtual void OnEvent(RenderEvent& event) override;
		virtual void OnEvent(const DeviceDestroyEvent& event) override;
		virtual void OnEvent(const DeviceCreateEvent& event) override;
		~ImGuiLayer();
	private:
		VkDescriptorPool descriptorPool;
	};
}
