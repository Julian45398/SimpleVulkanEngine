#pragma once

#include "ImGuiLayer.hpp"

namespace SGF {
	class DockspaceLayer : public Layer {
		DockspaceLayer();
		~DockspaceLayer();
        virtual void onRender(RenderEvent& event) override;
        virtual void onUpdate(const UpdateEvent& event) override;
        virtual void onAttach() override;
        virtual void onDetach() override;
	};
}