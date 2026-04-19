#include "ViewportLayer.hpp"
#include <glm/gtc/quaternion.hpp>

#include "ImGuizmo.h"
#include "ModelSelectionCPU.hpp"
#include <glm/gtx/matrix_decompose.hpp>

namespace SGF {
	const glm::vec4 NO_COLOR_MODIFIER(1.f, 1.f, 1.f, 0.f);
	const glm::vec4 SELECTED_COLOR(.7f, .4f, .2f, .6f);
	const glm::vec4 SELECTED_HOVERED_COLOR(.7f, .2f, .4f, .7f);
	const glm::vec4 HOVER_COLOR(.7f, .4f, .2f, .8f);
	const float MESH_TRANSPARENCY = .6f;
	const float NO_TRANSPARENCY = 1.f;

	ViewportLayer::ViewportLayer(VkFormat colorFormat) : Layer("Viewport"), editorRenderer(colorFormat), debugPanel("Debug Panel"),
			debugRenderer(editorRenderer.GetRenderPass(), editorRenderer.GetSubpass()) {
		VkDescriptorBufferInfo uniform_info = {VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
		// Pipeline:
		ImportModel("assets/models/Low-Poly-Car.gltf");
	}
	ViewportLayer::~ViewportLayer() {}
	void ViewportLayer::OnAttach() {}
	void ViewportLayer::OnDetach() {}
	void ViewportLayer::OnEvent(RenderEvent& event) {
		auto s = profiler.ProfileScope("Render Viewport");
		glm::mat4 viewProj;
		if (isOrthographic) {
			viewProj = cameraController.GetOrthoViewMatrix(viewSize, editorRenderer.GetViewport().GetAspectRatio());
		} else {
			viewProj = cameraController.GetViewProjMatrix(editorRenderer.GetViewport().GetAspectRatio());
		}
		editorRenderer.BeginFrame(event, viewProj);

		// Draw all static Models
		editorRenderer.BindRenderPipeline();
		for (size_t i = 0; i < models.size(); ++i) {
			auto& model = *models[i];
			if (!ImGuizmo::IsUsing() && (editorRenderer.IsCursorHoveringItem() && editorRenderer.GetHoveredModelIndex() == i && selectionMode != SelectionMode::NO_SELECTION)) {
				if (selectionMode == SelectionMode::NODE) {
					editorRenderer.SetModifiers(NO_COLOR_MODIFIER, 1.f);
					editorRenderer.DrawModelExcludeNode(model, i, model.GetNode(editorRenderer.GetHoveredNodeIndex()));
					editorRenderer.SetModifiers(HOVER_COLOR, 1.f);
					editorRenderer.DrawModelNodeRecursive(model, i, model.GetNode(editorRenderer.GetHoveredNodeIndex()));
				}
				else {
					editorRenderer.SetModifiers(HOVER_COLOR, 1.f);
					editorRenderer.DrawModel(model, i);
				}
			}
			else {
				editorRenderer.SetModifiers(NO_COLOR_MODIFIER, 1.f);
				editorRenderer.DrawModel(model, i);
			}
		}
		// Selection Outline
		if (selectedModelIndex != UINT32_MAX) {
			assert(selectedModelIndex < models.size());
			editorRenderer.BindOutlinePipeline();
			auto& model = *models[selectedModelIndex];
			editorRenderer.DrawNodeOutline(model, model.GetNode(selectedNodeIndex));
		}
		editorRenderer.DrawGrid();
		debugRenderer.Draw(editorRenderer.GetCurrentCommandBuffer(), cameraController.GetViewProjMatrix(editorRenderer.GetAspectRatio()), editorRenderer.GetWidth(), editorRenderer.GetHeight());
		if (animationControllers.size() > 0) {
			debugRenderer.Clear();
			debugPanel.AddMessage("Animations active");
		}
		editorRenderer.EndFrame(event, glm::uvec2(relativeCursor.x, relativeCursor.y));
	}

	void ViewportLayer::OnEvent(const WindowResizeEvent& event) {}
	bool ViewportLayer::OnEvent(const KeyPressedEvent& event) {
		if (inputMode == INPUT_CAPTURED) {
			if (event.GetKey() == SGF::KEY_ESCAPE) {
				event.GetWindow().FreeCursor();
				inputMode = INPUT_SELECTED;
				return true;
			}
		}
		else {
			if (event.GetKey() == SGF::KEY_T) {
				auto& camera = cameraController.GetCamera();
				auto invProj = glm::inverse(cameraController.GetProjMatrix(editorRenderer.GetAspectRatio()));
				auto invView = glm::inverse(cameraController.GetViewMatrix());
				debugRenderer.AddFrustum(invProj, invView, SGF::Color::RGBA8::Green());
				if (hitInfo.meshIndex != UINT32_MAX) {
					debugRenderer.AddLine(camera.GetPos(), hitInfo.position, SGF::Color::RGBA8::Red());
				}
				else {
					Ray ray = SGF::CreateRayFromPixel(relativeCursor.x, relativeCursor.y, editorRenderer.GetWidth(), editorRenderer.GetHeight(), cameraController.GetViewMatrix(), cameraController.GetProjMatrix(editorRenderer.GetAspectRatio()));
					debugRenderer.AddLine(camera.GetPos(), ray.GetOrigin() + ray.GetDirection() * 1000.f, SGF::Color::RGBA8::Blue());
				}
			}
			if (event.GetKey() == SGF::KEY_B) {
				std::vector<glm::vec3> vertices;
				std::vector<uint32_t> indices;
				for (size_t l = 0; l < models.size(); ++l) {
					auto& model = *models[l];
					size_t startNode = 0;
					size_t endNode = 0;
					if (selectedModelIndex == UINT32_MAX) {
						startNode = 0;
						endNode = model.nodes.size();
					} else if (selectedModelIndex != l) {
						continue;
					} else if (selectedNodeIndex == model.GetRoot().index) {
						startNode = 0;
						endNode = model.nodes.size();
					} else if (selectedNodeIndex != UINT32_MAX) {
						startNode = selectedNodeIndex;
						endNode = startNode + 1;
					}

					for (size_t k = startNode; k < endNode; ++k) {
						auto& node = model.GetNode(k);
						for (size_t i = 0; i < node.meshes.size(); ++i) {
							auto& mesh = model.meshes[node.meshes[i]];
							for (size_t j = 0; j < mesh.indexCount; ++j) {
								auto& index = model.indices[mesh.indexOffset + j];
								indices.push_back(index);
							}
							for (size_t j = 0; j < mesh.vertexCount; ++j) {
								auto& vertex = model.vertices[mesh.vertexOffset + j];
								vertices.push_back(vertex.position);
							}
						}
						debugRenderer.AddMesh(vertices, indices, node.globalTransform, SGF::Color::RGBA8(.5f, .5f, .2f));
						vertices.clear();
						indices.clear();
					}
				}
			}
			if (event.GetKey() == SGF::KEY_C) {
				debugRenderer.Clear();
			}
		}
		return false;
	}
	bool ViewportLayer::OnEvent(const MouseReleasedEvent& event) {
		if (HAS_FLAG(inputMode, INPUT_CAPTURED) && event.GetButton() == SGF::MOUSE_BUTTON_RIGHT) {
			event.GetWindow().FreeCursor();
			UNSET_FLAG(inputMode, INPUT_CAPTURED);
			cursorMove.x = 0;
			cursorMove.y = 0;
			return true;
		}
		return false;
	}
	bool ViewportLayer::OnEvent(const MouseMovedEvent& event) {
		if ((inputMode & INPUT_CAPTURED)) {
			auto& newpos = event.GetPos();
			cursorMove = newpos - cursorPos;
			cursorPos = newpos; 
			return true;
		}
		return false;
	}
	bool ViewportLayer::OnEvent(const MousePressedEvent& event) {
		if ((inputMode & INPUT_HOVERED) && event.GetButton() == SGF::MOUSE_BUTTON_RIGHT) {
			event.GetWindow().CaptureCursor();
			cursorPos = event.GetWindow().GetCursorPos();
			ImGui::SetWindowFocus("Viewport");
			SET_FLAG(inputMode, INPUT_SELECTED | INPUT_CAPTURED);
			return true;
		}
		return false;
	}
    
	void ViewportLayer::OnEvent(const UpdateEvent& event) {
		ImGuizmo::BeginFrame();
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
		UpdateAnimations(event);
		UpdateViewport(event);
		UpdateDebugWindow(event);
		UpdateModelWindow(event);
		profiler.DisplayResults();
	}


	void ViewportLayer::UpdateAnimations(const UpdateEvent& event) {
		for (size_t i = 0; i < animationControllers.size(); ++i) {
			auto& controller = animationControllers[i];
			controller.Update(event.GetDeltaTime()/1000.f);
			std::vector<glm::mat4> boneMatrices;
			controller.GetBonePalette(boneMatrices);
			for (size_t j = 0; j < boneMatrices.size(); ++j) {
				glm::vec3 start, end;
				start = glm::vec3(boneMatrices[j] * glm::vec4(0, 0, 0, 1));
				end = glm::vec3(boneMatrices[j] * glm::vec4(0, 1, 0, 1));
				debugRenderer.AddLine(start, end, SGF::Color::RGBA8::Red());
			}
		}
		// 
		if (!animationControllers.empty()) {
			for (size_t i = 0; i < models.size(); ++i) {
				//modelRenderer.UpdateInstanceTransforms(modelBindOffsets[i], *models[i]);
			}
		}
	}

	void ViewportLayer::UpdateViewport(const UpdateEvent& event) {
		auto s = profiler.ProfileScope("Update Viewport");
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport", nullptr, (inputMode & INPUT_CAPTURED) ? (ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs) : ImGuiWindowFlags_None);
		ImVec2 size = ImGui::GetContentRegionAvail();
		if ((uint32_t)size.x != editorRenderer.GetWidth() || (uint32_t)size.y != editorRenderer.GetHeight()) {
			editorRenderer.ResizeFramebuffer((uint32_t)size.x, (uint32_t)size.y);
		}
		if (ImGui::IsWindowHovered()) {
			SET_FLAG(inputMode, INPUT_HOVERED);
		} else {
			UNSET_FLAG(inputMode, INPUT_HOVERED);
		}
		if (HAS_FLAG(inputMode, INPUT_CAPTURED)) {
			cameraController.UpdateCamera(cursorMove, event.GetDeltaTime());
		}
		ImGui::Image(editorRenderer.GetImGuiTextureID(), size);
		hitInfo.meshIndex = UINT32_MAX;
		hitInfo.nodeIndex = UINT32_MAX;
		hitInfo.triangleIndex = UINT32_MAX;
		hitInfo.position = glm::vec3(0.f);
		hitInfo.t = std::numeric_limits<float>::max();
		// Get hover value
		if (ImGui::IsItemHovered()) {
			auto hv = profiler.ProfileScope("CPU Model Selection");
			//TestSelectionAlgorithms();
			uint32_t modelHover = editorRenderer.GetHoveredModelIndex();
			uint32_t nodeHover = editorRenderer.GetHoveredNodeIndex();
			debugPanel.AddMessage(fmt::format("GPU Hover - Model: {} Node: {}", modelHover, nodeHover));
			Ray ray;
			{
				ImVec2 mouse = ImGui::GetIO().MousePos;
				ImVec2 imageMin = ImGui::GetItemRectMin();
				relativeCursor = ImVec2(mouse.x - imageMin.x, mouse.y - imageMin.y);
				ray = SGF::CreateRayFromPixel(relativeCursor.x, relativeCursor.y, editorRenderer.GetWidth(), editorRenderer.GetHeight(), cameraController.GetViewMatrix(), cameraController.GetProjMatrix(editorRenderer.GetAspectRatio()));
				for (size_t i = 0; i < models.size(); ++i) {
					auto& model = *models[i];
					if (SGF::GetModelIntersection(ray, model, hitInfo)) {

					}
				}
				if (editorRenderer.IsCursorHoveringItem()) {
					if (hitInfo.nodeIndex == nodeHover) {
						debugPanel.AddMessage("CPU is the same as GPU hover value");
					}
					else {
						debugPanel.AddMessage(fmt::format("CPU and GPU hover value dont match! CPU: {} - GPU: {}", hitInfo.nodeIndex, nodeHover));
					}
				}
				else if (hitInfo.t < std::numeric_limits<float>::max()) {
					debugPanel.AddMessage("CPU Hit detected, where no hit should be!");
				}
				else {
					debugPanel.AddMessage("No hit with both methods");
				}
			}
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { 
				if (ImGuizmo::IsOver()) { // Do nothing, gizmo is being used
				} else if (editorRenderer.IsCursorHoveringItem()) {
					selectedModelIndex = modelHover;
					selectedNodeIndex = (selectionMode == SelectionMode::NODE) ? nodeHover : models[selectedModelIndex]->GetRoot().index;
				} else {
					selectedModelIndex = UINT32_MAX;
					selectedNodeIndex = UINT32_MAX;
				}
			}
		} else {
		}
		if (selectedModelIndex != UINT32_MAX) {
			ImGuizmo::SetOrthographic(isOrthographic);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y, editorRenderer.GetWidth(), editorRenderer.GetHeight());
			auto view = cameraController.GetViewMatrix();
			glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
			// Apply the flip
			view = flipY * view;

			auto& model = *models[selectedModelIndex];
			auto& node = model.GetNode(selectedNodeIndex);
			auto proj = cameraController.GetProjMatrix(editorRenderer.GetAspectRatio());
			auto globalTransform = model.GetNode(selectedNodeIndex).globalTransform;
			glm::mat4 delta(1.f);
			ImGuizmo::Manipulate((float*)&view, (float*)&proj, ImGuizmo::OPERATION::UNIVERSAL, ImGuizmo::MODE::LOCAL, (float*)&globalTransform, (float*)&delta);
			if (ImGuizmo::IsUsing()) {
				if (selectionMode == SelectionMode::MODEL) {
					// Apply to root node
					model.TransformNodeRecursive(node, delta);
				} else if (selectionMode == SelectionMode::NODE) {
					model.TransformNode(node, delta);
				}
				editorRenderer.UpdateInstanceTransforms(model);
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ViewportLayer::DrawTreeNode(uint32_t modelIndex, const GenericModel::Node& node) {
		assert(modelIndex < models.size());
		const auto& model = *models[modelIndex];
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DrawLinesToNodes | ImGuiTreeNodeFlags_OpenOnArrow;
		if (node.children.size() == 0 && node.meshes.size() == 0) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}
		if (modelIndex == selectedModelIndex && node.index == selectedNodeIndex) flags |= ImGuiTreeNodeFlags_Selected;
		bool open = ImGui::TreeNodeEx(&node, flags, "%s", node.name.c_str());
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			//ImGui::IsItemToggledSelection
			if (flags & ImGuiTreeNodeFlags_Selected) {
				ClearSelection();
			} else {
				selectedModelIndex = modelIndex;
				selectedNodeIndex = node.index;
			}
		}
		if (open) {
			// Draw Subnodes:
			for (uint32_t child : node.children)
				DrawTreeNode(modelIndex, model.nodes[child]);
			// Draw Meshes:
			for (auto& m : node.meshes) {
				ImGuiTreeNodeFlags mflags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				ImGui::TreeNodeEx(&m, mflags, "Mesh %d", m);
				if (ImGui::IsItemClicked()) {
					Log::Debug("Mesh clicked");
				}
			}
			ImGui::TreePop();
		}
	}

    void ViewportLayer::ClearSelection() {
		selectedModelIndex = UINT32_MAX;
		selectedNodeIndex = UINT32_MAX;
	}

	void ViewportLayer::TestSelectionAlgorithms() {
		if (editorRenderer.IsCursorHoveringItem() && editorRenderer.GetHoveredNodeIndex() == 12) {
			debugPanel.AddMessage("Testing selection algorithms on node 12");
			uint32_t hoveredModelIndex = editorRenderer.GetHoveredModelIndex();
			uint32_t hoveredNodeIndex = editorRenderer.GetHoveredNodeIndex();
			auto& model = *models[hoveredModelIndex];
			auto& node12 = model.GetNode(12);
			Ray ray = SGF::CreateRayFromPixel(relativeCursor.x, relativeCursor.y, editorRenderer.GetWidth(), editorRenderer.GetHeight(), cameraController.GetViewMatrix(), cameraController.GetProjMatrix(editorRenderer.GetAspectRatio()));
			HitInfo cpuHitInfo;
			std::vector<uint32_t> debugCheckedNodes;
			bool cpuHit = SGF::GetNodeIntersection(ray, model, node12, cpuHitInfo);
			const auto& vertices = model.GetVertices();
			const auto& indices = model.GetIndices();
			const auto& transform = node12.globalTransform;

			std::vector<uint32_t> meshIndices;
			std::vector<glm::vec3> meshVertices;
			for (size_t j = 0; j < node12.meshes.size(); ++j) {
				auto& mesh = model.GetMesh(node12, j);
				meshIndices.reserve(mesh.indexCount);
				meshVertices.reserve(mesh.vertexCount);
				for (size_t i = 0; i < mesh.indexCount / 3; ++i) {
					uint32_t i0 = indices[mesh.indexOffset + i * 3 + 0];
					uint32_t i1 = indices[mesh.indexOffset + i * 3 + 1];
					uint32_t i2 = indices[mesh.indexOffset + i * 3 + 2];

					glm::vec3 v0 = vertices[mesh.vertexOffset].position;
					glm::vec3 v1 = vertices[mesh.vertexOffset].position;
					glm::vec3 v2 = vertices[mesh.vertexOffset].position;

					v0 = glm::vec3(transform * glm::vec4(v0, 1.f));
					v1 = glm::vec3(transform * glm::vec4(v1, 1.f));
					v2 = glm::vec3(transform * glm::vec4(v2, 1.f));
					meshIndices.push_back(i0);
					meshIndices.push_back(i1);
					meshIndices.push_back(i2);
					meshVertices.push_back(v0);
					meshVertices.push_back(v1);
					meshVertices.push_back(v2);
					debugRenderer.AddMesh(meshVertices, meshIndices, transform, SGF::Color::RGBA8::Red());
					meshIndices.clear();
					meshVertices.clear();
				}
			}

			if (cpuHit) {
				debugPanel.AddMessage(fmt::format("CPU Hit on Node 12! Hit Position: [ {}, {}, {} ]\nHit Normal: [ {}, {}, {} ]", cpuHitInfo.position.x, cpuHitInfo.position.y, cpuHitInfo.position.z, cpuHitInfo.normal.x, cpuHitInfo.normal.y, cpuHitInfo.normal.z));
			}
			else {
				debugPanel.AddMessage("No CPU Hit on Node 12");
			}
		}
	}

	void ViewportLayer::ImportModel(const char* filename) {
		models.emplace_back(new GenericModel(filename));
		editorRenderer.AddModel(*models.back());
		//modelBindOffsets.push_back(modelRenderer.UploadModel(*models.back()));
		if (models.back()->HasAnimations()) {
			Log::Debug("Model has animations!");
			//animationControllers.emplace_back(models.back());
		}
		else {
			Log::Debug("Model has no animations!");
		}
	}

	void ViewportLayer::UpdateModelWindow(const UpdateEvent& event) {
		ImGui::Begin("Models",nullptr, (inputMode & INPUT_CAPTURED) ? (ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs) : ImGuiWindowFlags_None);
		if (ImGui::Button("Import Model")) {
			Log::Debug("Import Model Button Clicked");
			WindowHandle handle(ImGui::GetWindowViewport());
			auto filename = handle.OpenFileDialog("Model files", "gltf,glb,fbx,obj,usdz");
			if (!filename.empty()) {
				ImportModel(filename.c_str());
			}
			ClearSelection();
		}
		if (selectionMode == SelectionMode::MODEL) {
			if (ImGui::Button("Selection Mode: Model")) {
				selectionMode = SelectionMode::NODE; 
				ClearSelection();
			}
		} else if (selectionMode == SelectionMode::NODE) {
			if (ImGui::Button("Selection Mode: Node ")) {
				selectionMode = SelectionMode::MODEL;
				ClearSelection();
			}
		}
		if (selectedModelIndex != UINT32_MAX) {
			auto& model = *models[selectedModelIndex];
			if (model.HasSkeletalAnimation()) {
				if (ImGui::Button("Show Bones")) {
					for (size_t i = 0; i < model.bones.size(); ++i) {
						auto& bone = model.bones[i];
						ImGui::Text("Bone %d: %s", i, bone.name.c_str());
					}
					auto& bones = model.bones;
					for (size_t i = 0; i < bones.size(); ++i) {
						auto& bone = bones[i];
						glm::vec3 p1 = glm::vec3(bone.offsetMatrix * glm::vec4(0.f,0.f,0.f,1.f));
						glm::vec3 p2 = glm::vec3(bone.offsetMatrix * glm::vec4(1.f,0.f,0.f,1.f));
						debugRenderer.AddLine(p1, p2, SGF::Color::RGBA8::Green())	;
					}
				}
			}

			if (ImGui::Button("Clear Selection")) {
				ClearSelection();
			}
		}
		ImGui::Separator();
		ShowModelHierarchy();
		ImGui::Separator();
		ShowSelectionInformation();
		ImGui::End();
	}

	void ViewportLayer::ShowModelHierarchy() {
		if (models.size() == 0) ImGui::Text("No Models Loaded!");
		for (size_t i = 0; i < models.size(); ++i) {
			auto& model = *models[i];
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DrawLinesFull;
			if (i == selectedModelIndex) flags |= ImGuiTreeNodeFlags_Selected;

			bool open = ImGui::TreeNodeEx(models[i].get(), flags, "Model: %s", model.GetName().c_str());
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
				if ((flags & ImGuiTreeNodeFlags_Selected) && (selectedNodeIndex == model.GetRoot().index)) {
					ClearSelection();
				}
				else {
					selectedNodeIndex = model.GetRoot().index;
					selectedModelIndex = i;
				}
				Log::Debug("Model Clicked!");
			}
			if (open) {
				DrawTreeNode(i, model.nodes[0]);
				ImGui::TreePop();
			}
		}
	}

	void DecomposeTransformationMatrix(const glm::mat4& matrix, glm::vec3* pTranslation, glm::quat* pRotation, glm::vec3* pScale) {
		if (pTranslation) {
			*pTranslation = glm::vec3(matrix[3]);
		}
		if (pRotation || pScale) {
			glm::vec3 col0 = glm::vec3(matrix[0]);
			glm::vec3 col1 = glm::vec3(matrix[1]);
			glm::vec3 col2 = glm::vec3(matrix[2]);

			glm::vec3 scale = glm::vec3(glm::length(col0), glm::length(col1), glm::length(col2));
			if (pScale) {
				*pScale = scale;
			}
			if (pRotation) {
				glm::mat3 rotMat(
					col0 / (scale).x,
					col1 / (scale).y,
					col2 / (scale).z
				);
				*pRotation = glm::quat_cast(rotMat);
			}
		}
	}

	void ViewportLayer::ShowSelectionInformation() {
		if (selectedModelIndex == UINT32_MAX) {
			return;
		}
		// Model Information:
		auto& selectedModel = *models[selectedModelIndex];
		ImGui::Text("Model: %s", selectedModel.GetName().c_str());
		auto& node = selectedModel.GetNodes()[selectedNodeIndex];
		ImGui::Separator();
		ImGui::Text("Selected Node: %s", node.name.c_str());
		ImGui::Text("Mesh Count: %ld", node.meshes.size());
		ImGui::Text("Children Count: %ld", node.children.size());
		ImGui::Text("Has Animation: %s", selectedModel.HasAnimations() ? "True" : "False");

		// Animations
		if (selectedModel.HasAnimations()) {
			ImGui::Text("Animation Count: %ld", selectedModel.animations.size());
			ImGui::Separator();


			static int selectedAnimationIndex = -1;
			if (selectedAnimationIndex >= (int)selectedModel.animations.size()) {
				selectedAnimationIndex = -1;
			}

			const char* previewText = (selectedAnimationIndex >= 0 && selectedAnimationIndex < (int)selectedModel.animations.size())
				? selectedModel.animations[selectedAnimationIndex].name.c_str()
				: "Select Animation";

			if (ImGui::BeginCombo("##AnimationCombo", previewText)) {
				for (int i = 0; i < (int)selectedModel.animations.size(); ++i) {
					const auto& anim = selectedModel.animations[i];
					bool isSelected = (selectedAnimationIndex == i);
					if (ImGui::Selectable(anim.name.c_str(), isSelected)) {
						selectedAnimationIndex = i;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			// Play/Stop Button
			ImGui::SameLine();
			AnimationController* pController = nullptr;
			for (size_t i = 0; i < animationControllers.size(); ++i) {
				if (animationControllers[i].GetModel() == &selectedModel) {
					pController = &animationControllers[i];
					break;
				}
			}
			bool isPlaying = (pController != nullptr && pController->IsPlaying());

			if (isPlaying) {
				if (ImGui::Button("Stop##AnimationStop", ImVec2(50, 0))) {
					pController->Pause();
				}
			}
			else {
				ImGui::BeginDisabled(selectedAnimationIndex < 0);
				if (ImGui::Button("Play##AnimationPlay", ImVec2(50, 0))) {
					if (pController == nullptr) {
						animationControllers.emplace_back(&selectedModel);
						pController = &animationControllers.back();
					}
					if (selectedAnimationIndex >= 0 && selectedAnimationIndex < (int)selectedModel.animations.size()) {
						pController->PlayAnimation(selectedModel.animations[selectedAnimationIndex].name);
					}
				}
				ImGui::EndDisabled();
			}

			// Animation Details
			if (selectedAnimationIndex >= 0 && selectedAnimationIndex < (int)selectedModel.animations.size()) {
				ImGui::Separator();
				const auto& selectedAnimation = selectedModel.animations[selectedAnimationIndex];
				ImGui::Text("Animation: %s", selectedAnimation.name.c_str());
				ImGui::Text("Duration: %.2f seconds", selectedAnimation.duration);
				ImGui::Text("Ticks Per Second: %.2f", selectedAnimation.ticksPerSecond);
				ImGui::Text("Channels: %ld", selectedAnimation.channels.size());

				// Timeline slider
				float currentTime = (pController != nullptr) ? pController->GetCurrentTime() : 0.0f;
				currentTime = currentTime / 1.000f;
				ImGui::SliderFloat("##AnimationTimeline", &currentTime, 0.0f, selectedAnimation.duration, "%.2f s");
				if (ImGui::IsItemEdited() && pController != nullptr) {
					pController->SetCurrentTime(currentTime);
				}

				// Display channel information
				if (ImGui::TreeNode("Channels")) {
					for (size_t i = 0; i < selectedAnimation.channels.size(); ++i) {
						const auto& channel = selectedAnimation.channels[i];
						ImGui::Text("Channel %ld - Bone Index: %u", i, channel.boneIndex);
						ImGui::Indent();
						ImGui::Text("Position Keys: %ld", channel.positionKeys.size());
						ImGui::Text("Rotation Keys: %ld", channel.rotationKeys.size());
						ImGui::Text("Scale Keys: %ld", channel.scaleKeys.size());
						ImGui::Unindent();
					}
					ImGui::TreePop();
				}
			}

			// List all animations with details
			if (ImGui::TreeNode("All Animations")) {
				for (size_t i = 0; i < selectedModel.animations.size(); ++i) {
					const auto& anim = selectedModel.animations[i];
					ImGui::Text("Animation %ld: %s", i, anim.name.c_str());
					ImGui::Indent();
					ImGui::Text("Duration: %.2f s | Ticks/Sec: %.2f | Channels: %ld",
						anim.duration, anim.ticksPerSecond, anim.channels.size());
					ImGui::Unindent();
				}
				ImGui::TreePop();
			}
		}
		// Node Information:
		// Decompose Matrix
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;
		DecomposeTransformationMatrix(node.globalTransform, &translation, &rotation, &scale);
		glm::vec3 eulerDegrees = glm::degrees(glm::eulerAngles(rotation));

		ImGui::Text("Translation:");
		float transValues[3] = { translation.x, translation.y, translation.z };
		ImGui::InputFloat3("Position##trans", transValues, "%.3f", ImGuiInputTextFlags_ReadOnly);

		ImGui::Text("Rotation (Degrees):");
		float rotValues[3] = { eulerDegrees.x, eulerDegrees.y, eulerDegrees.z };
		ImGui::InputFloat3("Rotation##euler", rotValues, "%.3f", ImGuiInputTextFlags_ReadOnly);

		ImGui::Text("Scale:");
		float scaleValues[3] = { scale.x, scale.y, scale.z };
		ImGui::InputFloat3("Scale##comp", scaleValues, "%.3f", ImGuiInputTextFlags_ReadOnly);
		ImGui::Separator();
	}

	void ViewportLayer::UpdateDebugWindow(const UpdateEvent& event) {
		ImGui::Begin("Debug Window", nullptr, (inputMode & INPUT_CAPTURED) ? (ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs) : ImGuiWindowFlags_None);
		ImGui::Text("Application average %.3f ms/frame", event.GetDeltaTime());

		ImGui::Text("Relative Pos: (%.3f, %.3f)", relativeCursor.x, relativeCursor.y);
		ImGui::Text("Model: %d, Node: %d", editorRenderer.GetHoveredModelIndex(), editorRenderer.GetHoveredNodeIndex());

		//ImGui::Text("Total Indices: %d,\nTotal Vertices: %d,\nTotal Textures: %ld\nMemory Used: %ld,\nMemory Allocated: %ld",
			//editorRenderer.GetTotalIndexCount(), editorRenderer.GetTotalVertexCount(),
			//editorRenderer.GetTextureCount(), editorRenderer.GetTotalDeviceMemoryUsed(), editorRenderer.GetTotalDeviceMemoryAllocated());

		ImGui::Text("Selected ModelIndex: %d", selectedModelIndex);
		ImGui::Separator();

		cursorMove.x = 0; cursorMove.y = 0;
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
				cameraController.SetZoom(cameraZoom);
			}
		}
		float cameraPos[3] = { cameraController.GetCamera().GetPos().x, cameraController.GetCamera().GetPos().y, cameraController.GetCamera().GetPos().z };
		float cameraForward[3] = { cameraController.GetCamera().GetForward().x, cameraController.GetCamera().GetForward().y, cameraController.GetCamera().GetForward().z };
		if (ImGui::InputFloat3("Camera Position: ", cameraPos, "%.3f")) {
			cameraController.GetCamera().SetPos(cameraPos[0], cameraPos[1], cameraPos[2]);
		}
		if (ImGui::InputFloat3("Camera Forward: ", cameraForward, "%.3f")) {
			cameraController.GetCamera().SetForward(cameraForward[0], cameraForward[1], cameraForward[2]);
		}
		const auto& camera = cameraController.GetCamera();
		auto forward = camera.GetForward();
		auto position = camera.GetPos();
		debugPanel.AddMessage(std::move(fmt::format("Input has Focus: {}", (Input::HasFocus()) ? "True" : "False")));
		debugPanel.AddMessage(std::move(fmt::format("Camera Position: [{}, {}, {}]", position.x, position.y, position.z)));
		debugPanel.AddMessage(std::move(fmt::format("Camera Forward: [{}, {}, {}]", forward.x, forward.y, forward.z)));
		ImGui::Text("Input has Focus %s", (Input::HasFocus()) ? "True" : "False");
		ImGui::End();

		debugPanel.Draw();
	}
}