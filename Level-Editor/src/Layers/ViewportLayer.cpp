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

		modelPickBuffer = device.CreateBuffer(sizeof(uint32_t) * SGF_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		modelPickMemory = device.AllocateMemory(modelPickBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		modelPickMapped = (CursorHover*)device.MapMemory(modelPickMemory);

		gridRenderer.Init(viewport.GetRenderPass(), 0, uniformLayout);
		VkDescriptorSetLayout descriptorLayouts[] = {
			uniformLayout, modelRenderer.GetDescriptorSetLayout()
		};
		// Pipeline:
        {
            // Layout:
            VkDescriptorSetLayout descriptor_layouts[] = {
                uniformLayout,
				modelRenderer.GetDescriptorSetLayout()
            };
            VkPushConstantRange colorOverlayRange;
            colorOverlayRange.offset = 0;
            colorOverlayRange.size = sizeof(glm::vec4);
            colorOverlayRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            VkPushConstantRange nodeIndexRange;
            nodeIndexRange.offset = sizeof(glm::vec4);
            nodeIndexRange.size = sizeof(uint32_t);
            nodeIndexRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            VkPushConstantRange transparency;
            transparency.offset = sizeof(glm::vec4) + sizeof(uint32_t);
            transparency.size = sizeof(float);
            transparency.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            VkPushConstantRange push_constant_ranges[] = {
               colorOverlayRange, nodeIndexRange, transparency 
            };
            pipelineLayout = device.CreatePipelineLayout(descriptor_layouts, push_constant_ranges);
			outlineLayout = device.CreatePipelineLayout(descriptor_layouts);
        }
		renderPipeline = device.CreateGraphicsPipeline(pipelineLayout, viewport.GetRenderPass(), 0)
            .FragmentShader("shaders/model.frag").VertexShader("shaders/model.vert").VertexInput(modelRenderer.GetPipelineVertexInput())
            .DynamicState(VK_DYNAMIC_STATE_VIEWPORT).DynamicState(VK_DYNAMIC_STATE_SCISSOR).Depth(true, true).AddColorBlendAttachment(false, VK_COLOR_COMPONENT_R_BIT).Build();
		outlinePipeline = device.CreateGraphicsPipeline(outlineLayout, viewport.GetRenderPass(), 0)
            .FragmentShader("shaders/outline.frag").VertexShader("shaders/outline.vert").VertexInput(modelRenderer.GetPipelineVertexInput())
            .DynamicState(VK_DYNAMIC_STATE_VIEWPORT).DynamicState(VK_DYNAMIC_STATE_SCISSOR).FrontFace(VK_FRONT_FACE_CLOCKWISE).Depth(true, false, VK_COMPARE_OP_LESS_OR_EQUAL).Build();


		models.emplace_back("assets/models/Low-Poly-Car.gltf");
        modelBindOffsets.push_back(modelRenderer.UploadModel(models.back()));
	}

	ViewportLayer::~ViewportLayer() {
		auto& device = Device::Get();
		device.Destroy(sampler, signalSemaphore, descriptorPool, uniformLayout);
	}
	void ViewportLayer::OnAttach() {}
	void ViewportLayer::OnDetach() {}
	void ViewportLayer::OnEvent(RenderEvent& event) {
		VkClearValue clearValues[] = {
			SGF::Vk::CreateColorClearValue(0.f, 0.f, 1.f, 1.f),
			SGF::Vk::CreateColorClearValue(UINT32_MAX, 0U, 0U, 0U),
			SGF::Vk::CreateDepthClearValue(1.f, 0),
		};
		VkRect2D renderArea;
		renderArea.extent = viewport.GetExtent();
		renderArea.offset.x = 0;
		renderArea.offset.y = 0;
		auto& c = commands[imageIndex];
		c.Begin();
		c.BeginRenderPass(viewport.GetRenderPass(), viewport.GetFramebuffer(), renderArea, clearValues, ARRAY_SIZE(clearValues), VK_SUBPASS_CONTENTS_INLINE);
		modelRenderer.PrepareDrawing(imageIndex);

		RenderViewport(event);

		c.EndRenderPass();
		// Increment image index before buffer copy
		VkBufferImageCopy region;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;
		region.bufferOffset = sizeof(uint32_t) * imageIndex;
		region.imageExtent = { 1, 1, 1 }; 
		region.imageOffset = { glm::clamp((int32_t)relativeCursor.x, 0, (int32_t)viewport.GetWidth()), glm::clamp((int32_t)relativeCursor.y, 0, (int32_t)viewport.GetHeight()), 0 };
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		vkCmdCopyImageToBuffer(c, viewport.GetPickImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, modelPickBuffer, 1, &region);
		c.End();
		c.Submit(nullptr, FLAG_NONE, signalSemaphore);
		event.AddWait(signalSemaphore, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		imageIndex = (imageIndex + 1) % SGF_FRAMES_IN_FLIGHT;
	}
	void ViewportLayer::OnEvent(const WindowResizeEvent& event) {}
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
		UpdateDebugWindow(event);
		UpdateModelWindow(event);
	}

	void ViewportLayer::DrawModelNodeExcludeSelectedHierarchy(const GenericModel& model, const GenericModel::Node& node) const {
		auto& c = commands[imageIndex];
		if (&model == &models[selectedModelIndex] && selectedNodeIndex == node.index) return;
		vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4), sizeof(uint32_t), &node.index);
		modelRenderer.DrawNode(c, model, node);
		for (auto& n : node.children) {
			DrawModelNodeExcludeSelectedHierarchy(model, model.nodes[n]);
		}
	}
	void ViewportLayer::DrawModelNodeRecursive(const GenericModel& model, const GenericModel::Node& node) const {
		auto& c = commands[imageIndex];
		if (&model == &models[selectedModelIndex] && selectedNodeIndex == node.index) return;
		vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4), sizeof(uint32_t), &node.index);
		modelRenderer.DrawNode(c, model, node);
		for (auto& n : node.children) {
			DrawModelNodeExcludeSelectedHierarchy(model, model.nodes[n]);
		}
	}

	void ViewportLayer::RenderWireframe(RenderEvent& event) {

	}
	const glm::vec4 NO_COLOR_MODIFIER(1.f, 1.f, 1.f, 0.f);
	const glm::vec4 SELECTED_COLOR(.7f, .4f, .2f, .6f);
	const glm::vec4 SELECTED_HOVERED_COLOR(.7f, .2f, .4f, .7f);
	const glm::vec4 HOVER_COLOR(.7f, .4f, .2f, .8f);
	const float MESH_TRANSPARENCY = .6f;
	const float NO_TRANSPARENCY = 1.f;

	void ViewportLayer::RenderModel(RenderEvent& event, uint32_t modelIndex) {
		CursorHover currentID(modelIndex, 0);
		const glm::vec4* selectionColor;
		float transparency = 1.f;
		if (selectedModelIndex == UINT32_MAX) {
			selectionColor = (modelIndex == hoverValue.model) ? &HOVER_COLOR : &NO_COLOR_MODIFIER;
		} else if (modelIndex == selectedModelIndex) {
			selectionColor = &NO_COLOR_MODIFIER;
		} else {
			selectionColor = (modelIndex == hoverValue.model) ? &HOVER_COLOR : &NO_COLOR_MODIFIER;
			transparency = (modelIndex == hoverValue.model) ? 1.f : MESH_TRANSPARENCY;
		}
		auto& c = commands[imageIndex];
		modelRenderer.BindBuffersToModel(c, modelBindOffsets[modelIndex]);
		vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec4), selectionColor);
		vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4) + sizeof(uint32_t), sizeof(float), &transparency);
		for (size_t j = 0; j < models[modelIndex].nodes.size(); ++j) {
			currentID.node = j;
			vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4), sizeof(uint32_t), &currentID);
			modelRenderer.DrawNode(c, models[modelIndex], models[modelIndex].nodes[j]);
		}
	}

	// Model Selection:
	void ViewportLayer::RenderModelSelection(RenderEvent& event) {
		BindPipeline(renderPipeline, pipelineLayout);
		assert(modelBindOffsets.size() == models.size());
		assert(selectedModelIndex == UINT32_MAX || selectedNodeIndex == models[selectedModelIndex].GetRoot().index);
		
		for (size_t i = 0; i < modelBindOffsets.size(); ++i) {
			RenderModel(event, i);
		}
	}
	
	void ViewportLayer::RenderNodeSelection(RenderEvent& event) {
		auto& c = commands[imageIndex];
		BindPipeline(renderPipeline, pipelineLayout);
		assert(modelBindOffsets.size() == models.size());
		CursorHover currentID(0, 0);
		for (size_t i = 0; i < modelBindOffsets.size(); ++i) {
			currentID.model = i;
			const glm::vec4* selectionColor = &NO_COLOR_MODIFIER;
			float transparency = 1.f;
			if (selectedModelIndex != UINT32_MAX) {
				transparency = MESH_TRANSPARENCY;
			}
			modelRenderer.BindBuffersToModel(c, modelBindOffsets[i]);
			vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec4), selectionColor);
			vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4) + sizeof(uint32_t), sizeof(float), &transparency);
			for (size_t j = 0; j < models[i].nodes.size(); ++j) {
				currentID.node = j;
				if (currentID == hoverValue) continue;
				if (selectedModelIndex == i && selectedNodeIndex == j) {
					uint8_t tempArray[sizeof(currentID) + sizeof(transparency)];
					memcpy(tempArray, &currentID, sizeof(currentID));
					memcpy(tempArray + sizeof(currentID), &NO_TRANSPARENCY, sizeof(transparency));
					vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4), sizeof(tempArray), tempArray);
					modelRenderer.DrawNode(c, models[i], models[i].nodes[j]);
					vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4) + sizeof(uint32_t), sizeof(transparency), &transparency);
				} else {
					vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4), sizeof(uint32_t), &currentID);
					modelRenderer.DrawNode(c, models[i], models[i].nodes[j]);
				}
			}
			if (hoverValue.IsValid()) {
				selectionColor = (selectedModelIndex == hoverValue.model && selectedNodeIndex == hoverValue.node) ? &NO_COLOR_MODIFIER : &HOVER_COLOR;
				uint8_t tempArray[sizeof(glm::vec4) + sizeof(uint32_t) + sizeof(float)];
				memcpy(tempArray, selectionColor, sizeof(glm::vec4));
				memcpy(tempArray + sizeof(glm::vec4), &hoverValue, sizeof(uint32_t));
				memcpy(tempArray + sizeof(glm::vec4) + sizeof(uint32_t), &NO_TRANSPARENCY, sizeof(float));
				vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(tempArray), tempArray);
				modelRenderer.DrawNode(c, models[i], models[i].nodes[hoverValue.node]);
			}
		}
	}
	void ViewportLayer::BindPipeline(VkPipeline pipeline, VkPipelineLayout layout) {
		auto& c = commands[imageIndex];
		VkDescriptorSet sets[] = {
			uniformDescriptors[imageIndex],
			modelRenderer.GetDescriptorSet(imageIndex),
		};
		vkCmdBindPipeline(c, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindDescriptorSets(c, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, ARRAY_SIZE(sets), sets, 0, nullptr);
		glm::uvec2 viewportSize;
		viewportSize.x = viewport.GetWidth();
		viewportSize.y = viewport.GetHeight();
        VkViewport view = { (float)0.0f, (float)0.0f, (float)viewport.GetWidth(), (float)viewport.GetHeight(), 0.0f, 1.0f };
        VkRect2D scissor = { {0, 0}, viewport.GetExtent() };
        vkCmdSetViewport(c, 0, 1, &view);
        vkCmdSetScissor(c, 0, 1, &scissor);
	}
	void ViewportLayer::RenderViewport(RenderEvent& event) {
		auto& c = commands[imageIndex];
		if (isOrthographic) {
			uniformBuffer.SetValueAt(imageIndex, cameraController.GetOrthoViewMatrix(viewSize, viewport.GetAspectRatio()));
		} else {
			uniformBuffer.SetValueAt(imageIndex, cameraController.GetViewProjMatrix(viewport.GetAspectRatio()));
		}
		if (selectionMode == SelectionMode::MODEL) {
			RenderModelSelection(event);
		} else {
			RenderNodeSelection(event);
		}

		if (selectedModelIndex != UINT32_MAX) {
			BindPipeline(outlinePipeline, pipelineLayout);
			modelRenderer.BindBuffersToModel(c, modelBindOffsets[selectedModelIndex]);
			if (selectionMode == SelectionMode::MODEL) {
				modelRenderer.DrawModel(c, models[selectedModelIndex]);
			} else {
				modelRenderer.DrawNode(c, models[selectedModelIndex], models[selectedModelIndex].nodes[selectedNodeIndex]);
			}
		}
		gridRenderer.Draw(c, uniformDescriptors[imageIndex], viewport.GetWidth(), viewport.GetHeight());
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
		// Get hover value
		if (ImGui::IsItemHovered()) {
			ImVec2 mouse = ImGui::GetIO().MousePos;
			ImVec2 imageMin = ImGui::GetItemRectMin();
			relativeCursor = ImVec2(mouse.x - imageMin.x, mouse.y - imageMin.y);
			hoverValue = modelPickMapped[imageIndex];
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				if (hoverValue.IsValid()) {
					selectedModelIndex = hoverValue.model;
					selectedNodeIndex = (selectionMode == SelectionMode::NODE) ? hoverValue.node : models[selectedModelIndex].GetRoot().index;
				} else {
					selectedModelIndex = UINT32_MAX;
					selectedModelIndex = UINT32_MAX;
				}
			}
		} else {
			hoverValue = CursorHover(UINT32_MAX);
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ViewportLayer::DrawTreeNode(const GenericModel& model, const GenericModel::Node& node) {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DrawLinesToNodes | ImGuiTreeNodeFlags_OpenOnArrow;
		if (node.children.size() == 0 && node.meshes.size() == 0) {
			flags |= ImGuiTreeNodeFlags_Leaf;
		}
		bool open = ImGui::TreeNodeEx(&node, flags, "%s", node.name.c_str());
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			//ImGui::IsItemToggledSelection
			info("Node clicked!");
			//selectedModelIndex = UINT32_MAX;
		}
		if (open) {
			// Draw Subnodes:
			for (uint32_t child : node.children)
				DrawTreeNode(model, model.nodes[child]);
			// Draw Meshes:
			for (auto& m : node.meshes) {
				ImGuiTreeNodeFlags mflags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				ImGui::TreeNodeEx(&m, mflags, "Mesh %d", m);
				if (ImGui::IsItemClicked()) {
					info("Mesh clicked");
				}
			}
			ImGui::TreePop();
		}
	}

    void ViewportLayer::ClearSelection() {
		selectedModelIndex = UINT32_MAX;
		selectedNodeIndex = UINT32_MAX;
	}

	void ViewportLayer::UpdateModelWindow(const UpdateEvent& event) {
		ImGui::Begin("Models",nullptr, (inputMode & INPUT_CAPTURED) ? (ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs) : ImGuiWindowFlags_None);
		if (models.size() == 0) ImGui::Text("No Models Loaded!");
		for (size_t i = 0; i < models.size(); ++i) {
			auto& model = models[i];
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DrawLinesFull;
			if (i == selectedModelIndex) flags |= ImGuiTreeNodeFlags_Selected;
			bool open = ImGui::TreeNodeEx(&models[i], flags, "Model: %s", models[i].name.c_str());
			if (ImGui::IsItemClicked()) {
				info("Model Clicked!");
			}
			if (open) {
				DrawTreeNode(model, model.nodes[0]);
				ImGui::TreePop();
			}
		}
		ImGui::Separator();
		if (ImGui::Button("Import Model")) {
			WindowHandle handle(ImGui::GetWindowViewport());
			auto filename = handle.OpenFileDialog("Model files", "gltf,glb,fbx,obj,usdz");
			if (!filename.empty()) {
				models.emplace_back(filename.c_str());
				modelBindOffsets.push_back(modelRenderer.UploadModel(models.back()));
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
		ImGui::Separator();
		if (selectedModelIndex != UINT32_MAX) {
			auto& selectedModel = models[selectedModelIndex];
			ImGui::Text("Model: %s", selectedModel.GetName().c_str());
			auto& node = selectedModel.GetNodes()[selectedNodeIndex];
			ImGui::Separator();
			ImGui::Text("Selected Node: %s", node.name.c_str());
			ImGui::Text("Mesh Count: %ld", node.meshes.size());
			ImGui::Text("Children Count: %ld", node.children.size());
			{
				auto& t = node.globalTransform[0];
				float floats[4] = { t[0], t[1], t[2], t[3] };
				ImGui::InputFloat4("0", floats, "%.4f", ImGuiInputTextFlags_ReadOnly);
			} {
				auto& t = node.globalTransform[1];
				float floats[4] = { t[0], t[1], t[2], t[3] };
				ImGui::InputFloat4("1", floats, "%.4f", ImGuiInputTextFlags_ReadOnly);
			} {
				auto& t = node.globalTransform[2];
				float floats[4] = { t[0], t[1], t[2], t[3] };
				ImGui::InputFloat4("2", floats, "%.4f", ImGuiInputTextFlags_ReadOnly);
			} {
				auto& t = node.globalTransform[3];
				float floats[4] = { t[0], t[1], t[2], t[3] };
				ImGui::InputFloat4("3", floats, "%.4f", ImGuiInputTextFlags_ReadOnly);
			}
		}
		
		
		ImGui::Separator();
		ImGui::End();
	}
	
	void ViewportLayer::UpdateDebugWindow(const UpdateEvent& event) {
		ImGui::Begin("Debug Window",nullptr, (inputMode & INPUT_CAPTURED) ? (ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs) : ImGuiWindowFlags_None);
		ImGui::Text("Application average %.3f ms/frame", event.GetDeltaTime());

		ImGui::Text("Relative Pos: (%.3f, %.3f)", relativeCursor.x, relativeCursor.y);
		ImGui::Text("Model: %d, Node: %d", hoverValue.model, hoverValue.node);

		ImGui::Text("Total Indices: %d,\nTotal Vertices: %d,\nTotal Textures: %ld\nMemory Used: %ld,\nMemory Allocated: %ld",
			modelRenderer.GetTotalIndexCount(), modelRenderer.GetTotalVertexCount(),
			modelRenderer.GetTextureCount(), modelRenderer.GetTotalDeviceMemoryUsed(), modelRenderer.GetTotalDeviceMemoryAllocated());

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