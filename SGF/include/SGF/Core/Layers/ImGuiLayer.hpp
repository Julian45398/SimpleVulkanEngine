#pragma once

#include "Layer.hpp"
#include <SGF/Core/Flags.hpp>
#include <SGF/Core/GPU.hpp>
#include <imgui.h>

namespace SGF {
	class ImGuiLayer : public Layer {
	public:
		static ImTextureID AddTexture(GPU::DescriptorPool descriptorPool, GPU::Sampler sampler, GPU::ImageView imageView, GPU::ImageLayout imageLayout = GPU::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		static void UpdateTexture(ImTextureID textureID, GPU::Sampler sampler, GPU::ImageView imageView, GPU::ImageLayout imageLayout = GPU::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		static void RemoveTexture(GPU::DescriptorPool pool, ImTextureID textureID);

		ImGuiLayer(Flags<GPU::SampleCount> sampleCount);
		virtual void OnAttach() override;
		virtual bool OnEvent(const UpdateEvent& event) override;
		virtual bool OnEvent(const RenderEvent& event) override;
		//virtual void OnEvent(const DeviceDestroyEvent& event) override;
		//virtual void OnEvent(const DeviceCreateEvent& event) override;
		~ImGuiLayer();
	private:
		GPU::DescriptorPool descriptorPool;
	};
}
