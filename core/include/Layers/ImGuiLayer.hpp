#pragma once

#include "SGF_Core.hpp"
#include "Layers/Layer.hpp"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

namespace SGF {
	class ImGuiLayer : public Layer {
	public:
		ImGuiLayer(VkSampleCountFlagBits sampleCount);
		virtual void OnAttach() override;
		virtual void OnEvent(const UpdateEvent& event) override;
		virtual void OnEvent(RenderEvent& event) override;
		~ImGuiLayer();
	private:
		VkDescriptorPool descriptorPool;
	};
}
