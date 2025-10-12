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

		models.emplace_back("assets/models/Low-Poly-Car.gltf");
        modelBindOffsets.push_back(modelRenderer.UploadModel(models.back()));
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

	void ViewportLayer::DrawModelNodeExcludeSelected(VkCommandBuffer commands, const GenericModel& model, const GenericModel::Node& node) const {
		if (selectedNode == &node) {
			if (nullptr == selectedMesh) return;
			if (node.meshes.size() != 0) modelRenderer.SetMeshTransform(commands, node.globalTransform);
			for (auto& m : node.meshes) {
				if (&m == selectedMesh) continue;
				modelRenderer.DrawMesh(commands, model.meshes[m]);
			}
		} else {
			modelRenderer.DrawNode(commands, model, node);
		}
		for (auto& n : node.children) {
			DrawModelNodeExcludeSelected(commands, model, model.nodes[n]);
		}
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
		c.BeginRenderPass(viewport.GetRenderPass(), viewport.GetFramebuffer(), renderArea, clearValues, ARRAY_SIZE(clearValues), VK_SUBPASS_CONTENTS_INLINE);
		glm::uvec2 viewportSize;
		viewportSize.x = viewport.GetWidth();
		viewportSize.y = viewport.GetHeight();
		modelRenderer.PrepareDrawing(c, renderPipeline, uniformDescriptors[imageIndex], viewportSize, imageIndex);
		if (selectedModel != nullptr) {
			auto selectedModelHandle = SIZE_MAX;
			for (size_t i = 0; i < modelBindOffsets.size(); ++i) {
				if (&models[i] == selectedModel) {
					selectedModelHandle = i;
					continue;
				};
				modelRenderer.BindBuffersToModel(c, modelBindOffsets[i]);
				modelRenderer.DrawModel(c, models[i]);
			}
			assert(selectedModelHandle != SIZE_MAX);
			if (selectedModelHandle >= modelBindOffsets.size()) {
				selectedModel = nullptr;
				selectedMesh = nullptr;
				selectedNode = nullptr;
			} else {
				modelRenderer.BindBuffersToModel(c, modelBindOffsets[selectedModelHandle]);
				auto& model = models[selectedModelHandle];
				if (nullptr == selectedNode) {
					assert(selectedMesh == nullptr);
					modelRenderer.SetColorModifier(c, glm::vec4(0.2, 0.4, 0.7, 0.8));
					modelRenderer.DrawModel(c, model);
				} else { // node selected
					if (nullptr == selectedMesh) {
						DrawModelNodeExcludeSelected(c, model, model.GetRoot());
						modelRenderer.SetColorModifier(c, glm::vec4(0.8, 0.45, 0.23, 0.8));
						modelRenderer.DrawNodeRecursive(c, model, *selectedNode);
					} else {
						// Mesh Selected
						for (auto& n : model.nodes) {
							if (&n == selectedNode) continue;
							modelRenderer.DrawNode(c, model, n);
						}
						modelRenderer.SetMeshTransform(c, selectedNode->globalTransform);
						for (auto& mi : selectedNode->children) {
							if (&mi == selectedMesh) continue;
							modelRenderer.DrawMesh(c, model.meshes[mi]);
						}
						modelRenderer.SetColorModifier(c, glm::vec4(0.4, 0.7, 0.23, 0.8));
						modelRenderer.DrawMesh(c, model.meshes[*selectedMesh]);
					}
				}
			}
		} else {
			for (size_t i = 0; i < modelBindOffsets.size(); ++i) {
				modelRenderer.BindBuffersToModel(c, modelBindOffsets[i]);
				modelRenderer.DrawModel(c, models[i]);
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

		if (HAS_FLAG(inputMode, INPUT_CAPTURED)) {
			cameraController.UpdateCamera(cursorMove, event.GetDeltaTime());
		}
		ImGui::Image(imGuiImageID, size);
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ViewportLayer::DrawNode(const GenericModel& model, const GenericModel::Node& node) {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DrawLinesToNodes | ImGuiTreeNodeFlags_OpenOnArrow;
		if (&node == selectedNode) {
			flags |= ImGuiTreeNodeFlags_Selected;
		}
		if (node.children.size() == 0 && node.meshes.size() == 0) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}
		bool open = ImGui::TreeNodeEx(&node, flags, "%s", node.name.c_str());
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			//ImGui::IsItemToggledSelection
			selectedMesh = nullptr;
			if (selectedNode == &node) {
				selectedNode = nullptr;
			} else {
				selectedModel = &model;
				selectedNode = &node;
			}
		}
		if (open) {
			// Draw Subnodes:
			for (uint32_t child : node.children)
				DrawNode(model, model.nodes[child]);
			// Draw Meshes:
			for (auto& m : node.meshes) {
				ImGuiTreeNodeFlags mflags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				if (&m == selectedMesh) {
					mflags |= ImGuiTreeNodeFlags_Selected;
				}
				ImGui::TreeNodeEx(&m, mflags, "Mesh %d", m);
				if (ImGui::IsItemClicked()) {
					if (&m == selectedMesh) {
						selectedMesh = nullptr;
					} else {
						selectedNode = &node;
						selectedModel = &model;
						selectedMesh = &m;
					}
				}
			}
			ImGui::TreePop();
		}
		
	}

	void ViewportLayer::BuildNodeTree(const GenericModel& model, const GenericModel::Node& node) {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DrawLinesFull;
		bool open = ImGui::TreeNodeEx(&node, flags, "%s", node.name.c_str());
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			selectedNode = nullptr;
			selectedMesh = nullptr;
			if (selectedModel == &model) {
				selectedModel = nullptr;
			} else {
				selectedModel = &model;
			}
		}
		if (open) {
			for (size_t i = 0; i < node.children.size(); ++i) {
				DrawNode(model, model.nodes[node.children[i]]);
			}
			ImGui::TreePop();
		}
	}

	void ViewportLayer::UpdateModelWindow(const UpdateEvent& event) {
		ImGui::Separator();
		ImGui::Text("Models");
		for (size_t i = 0; i < models.size(); ++i) {
			auto& model = models[i];
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DrawLinesFull;
			if (&models[i] == selectedModel) flags |= ImGuiTreeNodeFlags_Selected;
			bool open = ImGui::TreeNodeEx(&models[i], flags, "Model: %s", models[i].name.c_str());
			if (ImGui::IsItemClicked()) {
				selectedNode = nullptr;
				selectedMesh = nullptr;
				if (selectedModel == &models[i]) {
					selectedModel = nullptr;
				} else {
					selectedModel = &models[i];
				}
			}
			if (open) {
				DrawNode(model, model.nodes[0]);
				ImGui::TreePop();
			}
		}
		ImGui::Separator();
		if (selectedNode) {
			ImGui::Separator();
			ImGui::Text("Selected Node: %s", selectedNode->name.c_str());
			ImGui::Text("Mesh Count: %ld", selectedNode->meshes.size());
			ImGui::Text("Children Count: %ld", selectedNode->children.size());
			{
				auto& t = selectedNode->globalTransform[0];
				float floats[4] = { t[0], t[1], t[2], t[3] };
				ImGui::InputFloat4("0", floats, "%.4f", ImGuiInputTextFlags_ReadOnly);
			} {
				auto& t = selectedNode->globalTransform[1];
				float floats[4] = { t[0], t[1], t[2], t[3] };
				ImGui::InputFloat4("1", floats, "%.4f", ImGuiInputTextFlags_ReadOnly);
			} {
				auto& t = selectedNode->globalTransform[2];
				float floats[4] = { t[0], t[1], t[2], t[3] };
				ImGui::InputFloat4("2", floats, "%.4f", ImGuiInputTextFlags_ReadOnly);
			} {
				auto& t = selectedNode->globalTransform[3];
				float floats[4] = { t[0], t[1], t[2], t[3] };
				ImGui::InputFloat4("3", floats, "%.4f", ImGuiInputTextFlags_ReadOnly);
			}
		}
	}
	
	void ViewportLayer::UpdateStatusWindow(const UpdateEvent& event) {
		ImGui::Begin("Debug Window",nullptr, (inputMode & INPUT_CAPTURED) ? (ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs) : ImGuiWindowFlags_None);
		ImGui::Text("Application average %.3f ms/frame", event.GetDeltaTime());
		glm::vec2 cursorpos(0.0f, 0.0f);
		if (Input::HasFocus()) {
			cursorpos = Input::GetCursorPos();
		}
		if (ImGui::Button("Import Model")) {
			WindowHandle handle(ImGui::GetWindowViewport());
			auto filename = handle.OpenFileDialog("Model files", "gltf,glb,fbx,obj,usdz");
			if (!filename.empty()) {
				models.emplace_back(filename.c_str());
				modelBindOffsets.push_back(modelRenderer.UploadModel(models.back()));
			}
			selectedModel = nullptr;
			selectedNode = nullptr;
			selectedMesh = nullptr;
		}
		ImGui::Separator();
		ImGui::Text("Total Indices: %d,\nTotal Vertices: %d,\nTotal Textures: %ld\nMemory Used: %ld,\nMemory Allocated: %ld",
			modelRenderer.GetTotalIndexCount(), modelRenderer.GetTotalVertexCount(),
			modelRenderer.GetTextureCount(), modelRenderer.GetTotalDeviceMemoryUsed(), modelRenderer.GetTotalDeviceMemoryAllocated());

		ImGui::Separator();
		if (models.size() != 0) {
			UpdateModelWindow(event);
		}

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
		}
	}
}