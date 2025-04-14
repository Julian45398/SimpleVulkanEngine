#pragma once

//#include "engine_core.h"

#include "ui_data.h"
#include "SVE_Backend.h"
#include "render/SVE_RenderPipeline.h"
#include "render/SVE_StagingBuffer.h"
#include "render/SVE_UniformBuffer.h"
#include "render/SVE_VertexBuffer.h"
#include "render/SVE_SceneRenderer.h"
#include "render/CameraController.h"

struct UniformData {
	glm::mat4 transformMatrix;
};

class SimpleVulkanEditor {
private:
	//ModelRenderer Renderer;
	//StagingBuffer StagingBuffer;
	//SveUniformBuffer<UniformData> uniformBuffer;
	//SceneBuffer sceneBuffer;
	//SveImageBuffer imageBuffer;
	//SveRenderPipeline renderPipeline;
	std::vector<SveModel> models;
	//SveSceneRenderer* renderer = nullptr;
	SveSceneRenderer sceneRenderer;
	float LeftPanelWidth = 200.f;
	float RightPanelWidth = 200.f;
	float BottomPanelHeight = 200.f;
	float ViewportHeight = 0.0f;
	float ViewportWidth = 0.0f;
	float ViewportOffsetX = 0.0f;
	float ViewportOffsetY = 0.0f;
	//bool ViewportResized = true;
	bool isOrthographic = false;

	float viewSize = 10.f;
	float cameraZoom = 1.f;

	//Camera ViewCamera;
	CameraController viewCameraController;
public:
	SimpleVulkanEditor();
		//uniformBuffer(), sceneBuffer(), renderPipeline("resources/shaders/model.vert", "resources/shaders/model.frag", SVE_MODEL_VERTEX_INPUT_INFO, uniformBuffer, sceneBuffer.imageBuffer.getSampler(), sceneBuffer.imageBuffer.getImageView())
	void run();
	~SimpleVulkanEditor();
private:
	void loadModel(const char* filename);
	inline void BuildUi() {
		auto& io = ImGui::GetIO();
		ImGui::BeginMainMenuBar();
		float height_offset = ImGui::GetWindowHeight();
		ImGui::EndMainMenuBar();
		ImGui::SetNextWindowPos(ImVec2(0.f, height_offset));
		ImGui::SetNextWindowSize(ImVec2(LeftPanelWidth, io.DisplaySize.y - height_offset));
		ImGui::SetNextWindowSizeConstraints(ImVec2(50.f, io.DisplaySize.y), ImVec2(io.DisplaySize.x * 0.5f - 50.f, io.DisplaySize.y));
		ImGui::Begin("Left Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		LeftPanelWidth = ImGui::GetWindowWidth();
		
		ImGui::Text("Application average %.3f ms/frame", SVE::getFrameTime());
		const auto cursor = SVE::getCursorPos();
		ImGui::Text("Cursor Pos: { %.4f, %.4f }", cursor.x, cursor.y);
		if (isOrthographic) {
			if (ImGui::Button("Set Perspective")) {
				isOrthographic = false;
			}
			else {
				ImGui::DragFloat("Orthographic View Size: ", &viewSize, 1.f, 0.f, 2000.f);
			}
		}
		else {
			if (ImGui::Button("Set Orthographic")) {
				isOrthographic = true;
			}
			else {
				ImGui::DragFloat("Zoom: ", &cameraZoom, 0.5f, 0.f, 2000.f);
				viewCameraController.setZoom(cameraZoom);
			}
		}
		const auto& camera = viewCameraController.getCamera();
		glm::vec3 pos = camera.getPos();
		glm::vec3 forward = camera.getForward();
		Ray center_ray(pos, forward);
		for (size_t i = 0; i < models.size(); ++i) {
			shl::Timer time;
			time.reset();
			float t = models[i].getIntersection(center_ray);
			double ellapsed = time.ellapsedMillis();
			if (t > 0 && t != std::numeric_limits<float>::infinity()) {
				ImGui::Text("collision at %.4f distance. Time: %.4fms", t, ellapsed);
			} else {
				ImGui::Text("no collision. Time: %.4fms", ellapsed);
			}
		}
		glm::vec3 up = camera.getUp();
		glm::vec3 right = camera.getRight();
		float yaw = camera.getYaw();
		float pitch = camera.getPitch();
		float roll = camera.getRoll();
		ImGui::Text("Camera Forward: { %.4f, %.4f, %.4f }", forward.x, forward.y, forward.z);
		ImGui::Text("Camera Right: { %.4f, %.4f, %.4f }", right.x, right.y, right.z);
		ImGui::Text("Camera Up: { %.4f, %.4f, %.4f }", up.x, up.y, up.z);
		ImGui::Text("Camera Yaw: %.4f, Pitch: %.4f, Roll: %.4f ", yaw, pitch, roll);

		if (ImGui::Button("Load GLTF file")) {
			nfdu8filteritem_t filters[] = { {"GLTF files: ", "gltf,glb"} };
			std::string filepath = SVE::openFileDialog(ARRAY_SIZE(filters), filters);
			if (!filepath.empty()) {
				loadModel(filepath.c_str());
			}
		}
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - RightPanelWidth, height_offset));
		ImGui::SetNextWindowSize(ImVec2(RightPanelWidth, io.DisplaySize.y - height_offset));
		ImGui::SetNextWindowSizeConstraints(ImVec2(50.f, io.DisplaySize.y), ImVec2(io.DisplaySize.x * 0.5f - 50.f, io.DisplaySize.y));
		ImGui::Begin("Right Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		RightPanelWidth = ImGui::GetWindowWidth();
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(LeftPanelWidth, io.DisplaySize.y - BottomPanelHeight));
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - LeftPanelWidth - RightPanelWidth, BottomPanelHeight));
		ImGui::SetNextWindowSizeConstraints(ImVec2(50.f, 50.f), ImVec2(io.DisplaySize.x - 50.f, io.DisplaySize.y * 0.7f));
		ImGui::Begin("Bottom Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
		BottomPanelHeight = ImGui::GetWindowHeight();
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
				
		SVE::setViewport((uint32_t)viewport_width, (uint32_t)viewport_height, (uint32_t)LeftPanelWidth, (uint32_t)height_offset);
	}

	void handleInput();
};