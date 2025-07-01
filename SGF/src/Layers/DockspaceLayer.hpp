#pragma once

#include "ImGuiLayer.hpp"

namespace SGF {
	class DockspaceLayer : public Layer {
		DockspaceLayer();
		~DockspaceLayer();
        virtual void OnEvent(RenderEvent& event) override;
        virtual void OnEvent(const UpdateEvent& event) override;
        virtual void OnAttach() override;
        virtual void OnDetach() override;
	};
}