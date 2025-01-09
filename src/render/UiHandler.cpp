#include "UiHandler.h"
#include "SVE_Backend.h"


namespace Menu {
	float LeftPanelWidth = 200.f;
	float RightPanelWidth = 200.f;
	float BottomPanelHeight = 200.f;
	float ViewportHeight = 0.0f;
	float ViewportWidth = 0.0f;
	float ViewportOffsetX = 0.0f;
	float ViewportOffsetY = 0.0f;
	bool ViewportResized = true;

	void BuildUi() {
		auto& io = ImGui::GetIO();
		ImGui::BeginMainMenuBar();
		float height_offset = ImGui::GetWindowHeight();
		ImGui::EndMainMenuBar();
		ImGui::SetNextWindowPos(ImVec2(0.f, height_offset));
		ImGui::SetNextWindowSize(ImVec2(LeftPanelWidth, io.DisplaySize.y - height_offset));
		ImGui::SetNextWindowSizeConstraints(ImVec2(50.f, io.DisplaySize.y), ImVec2(io.DisplaySize.x * 0.5f - 50.f, io.DisplaySize.y));
		ImGui::Begin("Left Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		if (ImGui::GetWindowWidth() != LeftPanelWidth) {
			ViewportResized = true;
			LeftPanelWidth = ImGui::GetWindowWidth();
		}
		ImGui::Text("Application average %.3f ms/frame", SVE::getFrameTime());
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - RightPanelWidth, height_offset));
		ImGui::SetNextWindowSize(ImVec2(RightPanelWidth, io.DisplaySize.y - height_offset));
		ImGui::SetNextWindowSizeConstraints(ImVec2(50.f, io.DisplaySize.y), ImVec2(io.DisplaySize.x * 0.5f - 50.f, io.DisplaySize.y));
		ImGui::Begin("Right Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		if (ImGui::GetWindowWidth() != RightPanelWidth) {
			ViewportResized = true;
			RightPanelWidth = ImGui::GetWindowWidth();
		}
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(LeftPanelWidth, io.DisplaySize.y - BottomPanelHeight));
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - LeftPanelWidth - RightPanelWidth, BottomPanelHeight));
		ImGui::SetNextWindowSizeConstraints(ImVec2(50.f, 50.f), ImVec2(io.DisplaySize.x - 50.f, io.DisplaySize.y * 0.7f));
		ImGui::Begin("Bottom Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		if (ImGui::GetWindowHeight() != BottomPanelHeight || ViewportResized) {
			ViewportResized = true;
			BottomPanelHeight = ImGui::GetWindowHeight();
		}
		ImGui::End();
		float viewport_height = io.DisplaySize.y - BottomPanelHeight - height_offset;
		float viewport_width = io.DisplaySize.x - LeftPanelWidth - RightPanelWidth;
		if (viewport_width < 0.0f) {
			viewport_width = 0.0f;
		}
		if (viewport_height < 0.0f) {
			viewport_height = 0.0f;
		}
		ImGui::SetNextWindowPos(ImVec2(LeftPanelWidth, height_offset));
		ImGui::SetNextWindowSize(ImVec2(viewport_width, viewport_height));
		//ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
		ImGui::End();
				
		if (ViewportResized) {
			ViewportResized = false;
			
			SVE::setViewport((uint32_t)viewport_width, (uint32_t)viewport_height, (uint32_t)LeftPanelWidth, (uint32_t)height_offset);
		}
	}
}