#include "ViewportLayer.hpp"

namespace SGF {
	ViewportLayer::ViewportLayer(VkFormat colorFormat) : Layer("Viewport"), viewport(colorFormat, VK_FORMAT_D16_UNORM), 
		cursor("assets/textures/zombie.png"), uniformBuffer(SGF_FRAMES_IN_FLIGHT) {
		auto& device = Device::Get();
		sampler = device.CreateImageSampler(VK_FILTER_NEAREST);
		signalSemaphore = device.CreateSemaphore();

		VkDescriptorPoolSize poolSizes[] = {
			Vk::CreateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SGF_FRAMES_IN_FLIGHT),
			Vk::CreateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, SGF_FRAMES_IN_FLIGHT * 128),
			Vk::CreateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, SGF_FRAMES_IN_FLIGHT),
			Vk::CreateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
		};
		descriptorPool = device.CreateDescriptorPool(20, poolSizes);
		VkDescriptorSetLayoutBinding layoutBindings[] = {
			Vk::CreateDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
		};
		uniformLayout = device.CreateDescriptorSetLayout(layoutBindings);

		VkDescriptorSetLayout layouts[SGF_FRAMES_IN_FLIGHT];
		for (uint32_t i = 0; i <  SGF_FRAMES_IN_FLIGHT; ++i) {
			commands[i].Init(QUEUE_FAMILY_GRAPHICS, 0, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
			layouts[i] = uniformLayout;
		}
		device.AllocateDescriptorSets(descriptorPool, layouts, uniformDescriptors);
		// Writing Descriptor Sets:
		VkDescriptorBufferInfo uniform_info = {VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
		for (uint32_t i = 0; i < SGF_FRAMES_IN_FLIGHT; ++i) {
			uniform_info.buffer = uniformBuffer.GetBuffer(i);
			VkWriteDescriptorSet writes[] = {
				Vk::CreateDescriptorWrite(uniformDescriptors[i], 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &uniform_info, 1)
			};
			device.UpdateDescriptors(writes);
		}
		modelRenderer.Initialize(viewport.GetRenderPass(), 0, descriptorPool, uniformLayout);
		gridRenderer.Init(viewport.GetRenderPass(), 0, uniformLayout);
		renderPipeline = Device::Get().CreateGraphicsPipeline(modelRenderer.GetPipelineLayout(), viewport.GetRenderPass(), 0)
            .FragmentShader("shaders/model.frag").VertexShader("shaders/model.vert").VertexInput(modelRenderer.GetPipelineVertexInput())
            .DynamicState(VK_DYNAMIC_STATE_VIEWPORT).DynamicState(VK_DYNAMIC_STATE_SCISSOR).Depth(true, true).Build();
	}
	ViewportLayer::~ViewportLayer() {
		auto& device = Device::Get();
		device.Destroy(sampler, signalSemaphore, descriptorPool, uniformLayout);
	}
	void ViewportLayer::OnAttach() {
	}
	void ViewportLayer::OnDetach() {
	}
	void ViewportLayer::OnEvent(RenderEvent& event) {
		RenderViewport(event);
	}
	void ViewportLayer::OnEvent(const WindowResizeEvent& event) {
		SGF::info("WindowResized: width: ", event.GetWidth(), " height: ", event.GetHeight());
	}
	bool ViewportLayer::OnEvent(const KeyPressedEvent& event) {
		if (inputMode == INPUT_CAPTURED && event.GetKey() == SGF::KEY_ESCAPE) {
			event.GetWindow().FreeCursor();
			inputMode == INPUT_SELECTED;
			return true;
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
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
		UpdateViewport(event);
		UpdateStatusWindow(event);
	}

	void ViewportLayer::RenderViewport(RenderEvent& event) {
		VkClearValue clearValues[] = {
			SGF::Vk::CreateColorClearValue(0.f, 0.f, 1.f, 1.f),
			SGF::Vk::CreateDepthClearValue(1.f, 0)
		};
		if (isOrthographic) {
			uniformBuffer.SetValueAt(imageIndex, cameraController.GetOrthoViewMatrix(viewSize, viewport.GetAspectRatio()));
		} else {
			uniformBuffer.SetValueAt(imageIndex, cameraController.GetViewProjMatrix(viewport.GetAspectRatio()));
		}
		VkRect2D renderArea;
		renderArea.extent = viewport.GetExtent();
		renderArea.offset.x = 0;
		renderArea.offset.y = 0;
		auto& c = commands[imageIndex];
		c.Begin();
		//auto& c = event.getCommands();
		c.BeginRenderPass(viewport.GetRenderPass(), viewport.GetFramebuffer(), renderArea, clearValues, ARRAY_SIZE(clearValues), VK_SUBPASS_CONTENTS_INLINE);
		//c.Draw(3);
		glm::uvec2 viewportSize;
		viewportSize.x = viewport.GetWidth();
		viewportSize.y = viewport.GetHeight();
		modelRenderer.PrepareDrawing(c, renderPipeline, uniformDescriptors[imageIndex], viewportSize, imageIndex);
		if (selectedModelIndex != UINT32_MAX) {
			for (size_t i = 0; i < modelDrawData.size(); ++i) {
				if (i == selectedModelIndex) continue;
				modelRenderer.DrawModel(c, modelDrawData[i]);
			}
			if (selectedMeshIndex != UINT32_MAX) {
				for (size_t i = 0; i < modelDrawData[selectedModelIndex].meshes.size(); ++i) {
					if (i == selectedMeshIndex) continue;
					modelRenderer.DrawMesh(c, modelDrawData[selectedModelIndex].meshes[i]);
				}
				modelRenderer.SetColorModifier(c, glm::vec4(0.7f, 0.4f, 0.2f, 0.8f));
				modelRenderer.DrawMesh(c, modelDrawData[selectedModelIndex].meshes[selectedMeshIndex]);
			} else {
				modelRenderer.SetColorModifier(c, glm::vec4(0.7f, 0.4f, 0.2f, 0.8f));
				modelRenderer.DrawModel(c, modelDrawData[selectedModelIndex]);
			}
		} else {
			for (size_t i = 0; i < modelDrawData.size(); ++i) {
				modelRenderer.DrawModel(c, modelDrawData[i]);
			}
		}
		//modelRenderer.Draw(c, uniformDescriptors[imageIndex], viewport.GetWidth(), viewport.GetHeight(), imageIndex);
		gridRenderer.Draw(c, uniformDescriptors[imageIndex], viewport.GetWidth(), viewport.GetHeight());
		c.EndRenderPass();
		c.End();
		c.Submit(nullptr, FLAG_NONE, signalSemaphore);
		event.AddWait(signalSemaphore, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		imageIndex = (imageIndex + 1) % SGF_FRAMES_IN_FLIGHT;
	}

	void ViewportLayer::UpdateViewport(const UpdateEvent& event) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport", nullptr, (inputMode & INPUT_CAPTURED) ? (ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs) : ImGuiWindowFlags_None);
		ImVec2 size = ImGui::GetContentRegionAvail();
		if ((uint32_t)size.x != viewport.GetWidth() || (uint32_t)size.y != viewport.GetHeight()) {
			ResizeFramebuffer((uint32_t)size.x, (uint32_t)size.y);
		}
		if (ImGui::IsWindowHovered()) {
			SET_FLAG(inputMode, INPUT_HOVERED);
		} else {
			UNSET_FLAG(inputMode, INPUT_HOVERED);
		}

		if (!HAS_FLAG(inputMode, INPUT_CAPTURED) && Input::HasFocus() && ImGui::IsWindowHovered()) {
			/*
			if (Input::IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				ImGui::SetWindowFocus();
				Input::CaptureCursor();
				SET_FLAG(inputMode, INPUT_CAPTURED | INPUT_HOVERED | INPUT_SELECTED);
			}
			*/
		}
		if (HAS_FLAG(inputMode, INPUT_CAPTURED)) {
			cameraController.UpdateCamera(cursorMove, event.GetDeltaTime());
		}
		ImGui::Image(imGuiImageID, size);
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ViewportLayer::UpdateModelWindow(const UpdateEvent& event) {
		ImGui::Separator();
		ImGui::Text("Models");
		for (size_t i = 0; i < models.size(); ++i) {
			auto& model = models[i];
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DrawLinesFull;
			if (i == selectedModelIndex) flags |= ImGuiTreeNodeFlags_Selected;
			bool open = ImGui::TreeNodeEx(&models[i], flags, "Model: %s", models[i].debugName.c_str());
			if (ImGui::IsItemClicked()) {
				if (!open) {
					selectedModelIndex = UINT32_MAX; 
					selectedMeshIndex = UINT32_MAX;
					SGF::info("Model unselected!");
				} else {
					SGF::info("Model Node selected!");
					selectedModelIndex = i;
				}
			}
			if (open) {
				for (size_t j = 0; j < model.meshInfos.size(); ++j) {
					ImGuiTreeNodeFlags subFlags = ImGuiTreeNodeFlags_Leaf;
					if (j == selectedMeshIndex) subFlags |= ImGuiTreeNodeFlags_Selected;
					bool subOpen = ImGui::TreeNodeEx(&model.meshInfos[j], subFlags, "Mesh: %ld", j);
					if (ImGui::IsItemClicked()) {
						if (!subOpen) {
							selectedMeshIndex = UINT32_MAX;
							SGF::info("Mesh unselected");
						} else {
							SGF::info("Mesh Node clicked!");
							selectedMeshIndex = j;
							selectedModelIndex = i;
						}
					}
					if (subOpen) {
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::Separator();
	}
	
	void ViewportLayer::UpdateStatusWindow(const UpdateEvent& event) {
		ImGui::Begin("Debug Window",nullptr, (inputMode & INPUT_CAPTURED) ? (ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs) : ImGuiWindowFlags_None);
		ImGui::Text("Application average %.3f ms/frame", event.GetDeltaTime());
		glm::vec2 cursorpos(0.0f, 0.0f);
		if (Input::HasFocus()) {
			cursorpos = Input::GetCursorPos();
		}
		if (ImGui::Button("Import GLTF-Model")) {
			WindowHandle handle(ImGui::GetWindowViewport());
			auto filename = handle.OpenFileDialog("GLTF files", "gltf, gdb");
			if (!filename.empty()) {
				models.emplace_back(filename.c_str());
				modelDrawData.push_back(modelRenderer.AddModel(models.back()));
			}
		}
		if (models.size() != 0) {
			UpdateModelWindow(event);
		}

		ImGui::Text("Cursor Pos: { %.4f, %.4f }", cursorpos.x, cursorpos.y);
		ImGui::Text("Cursor Pos: { %.4f, %.4f }", cursorPos.x, cursorPos.y);
		ImGui::Text("Cursor Pos: { %.4f, %.4f }", cursorMove.x, cursorMove.y);
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
		const auto& camera = cameraController.GetCamera();
		glm::vec3 pos = camera.GetPos();
		glm::vec3 forward = camera.GetForward();

		float p[] = {pos.x, pos.y, pos.z};
		ImGui::InputFloat3("Camera Position: ", p, "%.3f", ImGuiInputTextFlags_ReadOnly);
		p[0] = forward.x;
		p[1] = forward.y;
		p[2] = forward.z;
		ImGui::InputFloat3("Camera Forward: ", p, "%.3f", ImGuiInputTextFlags_ReadOnly);
		p[0] = camera.GetYaw();
		p[1] = camera.GetPitch();
		p[2] = camera.GetRoll();
		ImGui::InputFloat3("Camera Rotation: ", p, "%.3f", ImGuiInputTextFlags_ReadOnly);
		float m[4] = {};
		glm::mat4 mat = cameraController.GetViewProjMatrix(viewport.GetAspectRatio());
		m[0] = mat[0][0];
		m[1] = mat[0][1];
		m[2] = mat[0][2];
		m[3] = mat[0][3];
		ImGui::InputFloat4("1", m, "%.3f", ImGuiInputTextFlags_ReadOnly);
		m[0] = mat[1][0];
		m[1] = mat[1][1];
		m[2] = mat[1][2];
		m[3] = mat[1][3];
		ImGui::InputFloat4("2", m, "%.3f", ImGuiInputTextFlags_ReadOnly);
		m[0] = mat[2][0];
		m[1] = mat[2][1];
		m[2] = mat[2][2];
		m[3] = mat[2][3];
		ImGui::InputFloat4("3", m, "%.3f", ImGuiInputTextFlags_ReadOnly);
		m[0] = mat[3][0];
		m[1] = mat[3][1];
		m[2] = mat[3][2];
		m[3] = mat[3][3];
		ImGui::InputFloat4("4", m, "%.3f", ImGuiInputTextFlags_ReadOnly);




		/*
		Ray center_ray(pos, forward);
		// getting closest intersection;
		float closest = std::numeric_limits<float>::infinity();
		uint32_t modelIndex = models.size();

		SGF::Timer time;
		time.reset();
		for (size_t i = 0; i < models.size(); ++i) {
			float t = models[i].getIntersection(center_ray);
			closest = std::min(t, closest);
		}
		double ellapsed = time.ellapsedMillis();
		if (closest > 0 && closest != std::numeric_limits<float>::infinity()) {
			ImGui::Text("colliding with model %i at %.4f distance. Time: %.4fms", (int)modelIndex, closest, ellapsed);
		} else {
			ImGui::Text("no collision. Time: %.4fms", ellapsed);
		}
		*/

		glm::vec3 up = camera.GetUp();
		glm::vec3 right = camera.GetRight();
		float yaw = camera.GetYaw();
		float pitch = camera.GetPitch();
		ImGui::ColorButton("ColorButton", ImVec4(0.2, 0.8, 0.1, 1.0), 0, ImVec2(20.f, 20.f));
		//ImGui::Text("Frame time: {d}", event.get);
		ImGui::End();
	}
	void ViewportLayer::ResizeFramebuffer(uint32_t w, uint32_t h) {
		viewport.Resize(w, h);
		auto& device = Device::Get();
		device.WaitIdle();
		if (imGuiImageID != 0) {
			ImGuiLayer::UpdateVulkanTexture(imGuiImageID, sampler, viewport.GetColorView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		else {
			imGuiImageID = ImGuiLayer::AddVulkanTexture(descriptorPool, sampler, viewport.GetColorView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			//imGuiImageID = *(ImTextureID*)&descriptorSet;
		}
	}
}