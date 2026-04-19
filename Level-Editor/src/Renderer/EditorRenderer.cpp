#include "EditorRenderer.hpp"

namespace SGF {
	EditorRenderer::EditorRenderer(VkFormat imageFormat) : viewport(imageFormat, VK_FORMAT_D16_UNORM), uniformBuffer(SGF_FRAMES_IN_FLIGHT), hoverValue(UINT32_MAX) {
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
            colorOverlayRange.size = sizeof(glm::vec4) + sizeof(uint32_t) + sizeof(float); // color overlay (vec4), node index (uint32), transparency (f32)
            colorOverlayRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            //VkPushConstantRange nodeIndexRange;
            //nodeIndexRange.offset = sizeof(glm::vec4);
            //nodeIndexRange.size = sizeof(uint32_t);
            //nodeIndexRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            //VkPushConstantRange transparency;
            //transparency.offset = sizeof(glm::vec4) + sizeof(uint32_t);
            //transparency.size = sizeof(float);
            //transparency.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            VkPushConstantRange push_constant_ranges[] = {
               colorOverlayRange//, nodeIndexRange, transparency 
            };
            pipelineLayout = device.CreatePipelineLayout(descriptor_layouts, push_constant_ranges);
			outlineLayout = device.CreatePipelineLayout(descriptor_layouts);
        }
		staticRenderPipeline = device.CreateGraphicsPipeline(pipelineLayout, viewport.GetRenderPass(), 0)
            .FragmentShader("shaders/model.frag").VertexShader("shaders/model.vert").VertexInput(modelRenderer.GetPipelineVertexInput())
            .DynamicState(VK_DYNAMIC_STATE_VIEWPORT).DynamicState(VK_DYNAMIC_STATE_SCISSOR).Depth(true, true).AddColorBlendAttachment(false, VK_COLOR_COMPONENT_R_BIT).Build();
		outlinePipeline = device.CreateGraphicsPipeline(outlineLayout, viewport.GetRenderPass(), 0)
            .FragmentShader("shaders/outline.frag").VertexShader("shaders/outline.vert").VertexInput(modelRenderer.GetPipelineVertexInput())
            .DynamicState(VK_DYNAMIC_STATE_VIEWPORT).DynamicState(VK_DYNAMIC_STATE_SCISSOR).Depth(true, false, VK_COMPARE_OP_LESS_OR_EQUAL).AddColorBlendAttachment(false, 0)
			.FrontFace(VK_FRONT_FACE_CLOCKWISE).Build();
	}
	EditorRenderer::~EditorRenderer() {
		auto& device = Device::Get();
		device.Destroy(sampler, signalSemaphore, descriptorPool, uniformLayout, pipelineLayout, outlineLayout, staticRenderPipeline, outlinePipeline);
	}


	void EditorRenderer::BeginFrame(RenderEvent& event, const glm::mat4& viewProj) {
		VkClearValue clearValues[] = {
			Vk::CreateColorClearValue(0.1f, 0.1f, 0.1f, 1.f),
			Vk::CreateColorClearValue(UINT32_MAX, 0U, 0U, 0U),
			Vk::CreateDepthClearValue(1.f, 0),
		};
		VkRect2D renderArea;
		renderArea.extent = viewport.GetExtent();
		renderArea.offset.x = 0;
		renderArea.offset.y = 0;
		auto& c = commands[imageIndex];
		c.Begin();
		hoverValue = modelPickMapped[imageIndex];
		uniformBuffer.SetValueAt(imageIndex, viewProj);
		c.BeginRenderPass(viewport.GetRenderPass(), viewport.GetFramebuffer(), renderArea, clearValues, ARRAY_SIZE(clearValues), VK_SUBPASS_CONTENTS_INLINE);
		modelRenderer.PrepareDrawing(imageIndex);

	}

	void EditorRenderer::EndFrame(RenderEvent& event, glm::uvec2 pixelPos) {
		auto& c = commands[imageIndex];
		c.EndRenderPass();

		// Transition pick image from COLOR_ATTACHMENT_OPTIMAL -> TRANSFER_SRC_OPTIMAL
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = viewport.GetPickImage();
			barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			vkCmdPipelineBarrier(
				c,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}

		// Increment image index before buffer copy
		VkBufferImageCopy region;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;
		region.bufferOffset = sizeof(uint32_t) * imageIndex;
		region.imageExtent = { 1, 1, 1 }; 
		region.imageOffset = { glm::clamp((int32_t)pixelPos.x, 0, (int32_t)viewport.GetWidth() - 1), glm::clamp((int32_t)pixelPos.y, 0, (int32_t)viewport.GetHeight() - 1), 0 };
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

	void EditorRenderer::SetColorModifier(const glm::vec4& colorModifier) const {
		VkCommandBuffer c = commands[imageIndex];
		vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec4), &colorModifier);
	}
	void EditorRenderer::SetModelTransparency(float transparency) const {
		VkCommandBuffer c = commands[imageIndex];
		vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4) + sizeof(uint32_t), sizeof(float), &transparency);
	}
	void EditorRenderer::SetModifiers(const glm::vec4& colorModifier, float transparency) const {
		VkCommandBuffer c = commands[imageIndex];
		vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec4), &colorModifier);
		vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4) + sizeof(uint32_t), sizeof(float), &transparency);
	}
	void EditorRenderer::SetCurrentID(CursorHover currentID) const {
		VkCommandBuffer c = commands[imageIndex];
		vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4), sizeof(uint32_t), &currentID);
	}
	void EditorRenderer::DrawModelExcludeNode(const GenericModel& model, uint32_t modelIndex, const GenericModel::Node& excludedNode) const {
		VkCommandBuffer c = commands[imageIndex];
		modelRenderer.BindBuffersToModel(c, model);
		DrawNodeRecursiveExcludeNodePrivate(model, modelIndex, model.GetRoot(), excludedNode);
	}
	void EditorRenderer::DrawNodeRecursiveExcludeNode(const GenericModel& model, uint32_t modelIndex, const GenericModel::Node& currentNode, const GenericModel::Node& excludedNode) const {
		VkCommandBuffer c = commands[imageIndex];
		modelRenderer.BindBuffersToModel(c, model);
		DrawNodeRecursiveExcludeNodePrivate(model, modelIndex, currentNode, excludedNode);
	}
	void EditorRenderer::DrawNodeRecursiveExcludeNodePrivate(const GenericModel& model, uint32_t modelIndex, const GenericModel::Node& currentNode, const GenericModel::Node& excludedNode) const {
		if (currentNode.index == excludedNode.index) { return; }
		if (currentNode.meshes.size() != 0) {
			CursorHover currentID(modelIndex, currentNode.index);
			VkCommandBuffer c = commands[imageIndex];
			SetCurrentID(currentID);
			modelRenderer.DrawNode(c, model, currentNode);
		}
		for (auto& n : currentNode.children) {
			DrawNodeRecursiveExcludeNodePrivate(model, modelIndex, model.GetNode(n), excludedNode);
		}
	}
	void EditorRenderer::DrawModelNodeRecursive(const GenericModel& model, uint32_t modelIndex, const GenericModel::Node& node) const {
		VkCommandBuffer c = commands[imageIndex];
		if (node.meshes.size() != 0) {
			CursorHover currentID(modelIndex, node.index);
			SetCurrentID(currentID);
			modelRenderer.DrawNode(c, model, node);
		}
		for (uint32_t n : node.children) {
			DrawModelNodeRecursive(model, modelIndex, model.GetNode(n));
		}
	}
	void EditorRenderer::DrawModel(const GenericModel& model, uint32_t modelIndex) {
		CursorHover currentID(modelIndex, 0);
		const glm::vec4* selectionColor;
		auto& c = commands[imageIndex];
		modelRenderer.BindBuffersToModel(c, model);
		for (size_t j = 0; j < model.nodes.size(); ++j) {
			currentID.node = j;
			SetCurrentID(currentID);
			modelRenderer.DrawNode(c, model, model.nodes[j]);
		}
	}
	void EditorRenderer::DrawNodeOutline(const GenericModel& model, const GenericModel::Node& selectedNode) {
		VkCommandBuffer c = commands[imageIndex];
		modelRenderer.BindBuffersToModel(c, model);
		modelRenderer.DrawNodeRecursive(c, model, selectedNode);
	}
	void EditorRenderer::DrawModelOutline(const GenericModel& model) {
		VkCommandBuffer c = commands[imageIndex];
		modelRenderer.BindBuffersToModel(c, model);
		modelRenderer.DrawModel(c, model);
	}
	/*
	void EditorRenderer::DrawSelectedNode(const GenericModel& selectedModel, const GenericModel::Node& selectedNode) {
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
			auto& model = *models[i];
			for (size_t j = 0; j < model.nodes.size(); ++j) {
				currentID.node = j;
				if (currentID == hoverValue) continue;
				if (selectedModelIndex == i && selectedNodeIndex == j) {
					uint8_t tempArray[sizeof(currentID) + sizeof(transparency)];
					memcpy(tempArray, &currentID, sizeof(currentID));
					memcpy(tempArray + sizeof(currentID), &NO_TRANSPARENCY, sizeof(transparency));
					vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4), sizeof(tempArray), tempArray);
					modelRenderer.DrawNode(c, model, model.nodes[j]);
					vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4) + sizeof(uint32_t), sizeof(transparency), &transparency);
				} else {
					vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4), sizeof(uint32_t), &currentID);
					modelRenderer.DrawNode(c, model, model.nodes[j]);
				}
			}
			if (hoverValue.IsValid()) {
				selectionColor = (selectedModelIndex == hoverValue.model && selectedNodeIndex == hoverValue.node) ? &NO_COLOR_MODIFIER : &HOVER_COLOR;
				uint8_t tempArray[sizeof(glm::vec4) + sizeof(uint32_t) + sizeof(float)];
				memcpy(tempArray, selectionColor, sizeof(glm::vec4));
				memcpy(tempArray + sizeof(glm::vec4), &hoverValue, sizeof(uint32_t));
				memcpy(tempArray + sizeof(glm::vec4) + sizeof(uint32_t), &NO_TRANSPARENCY, sizeof(float));
				vkCmdPushConstants(c, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(tempArray), tempArray);
				modelRenderer.DrawNode(c, model, model.nodes[hoverValue.node]);
			}
		}
	}
	*/
    void EditorRenderer::DrawGrid() {
		gridRenderer.Draw(commands[imageIndex], uniformDescriptors[imageIndex], viewport.GetWidth(), viewport.GetHeight());
    }

	void EditorRenderer::ResizeFramebuffer(uint32_t w, uint32_t h) {
		auto& device = Device::Get();
		device.WaitIdle();
		viewport.Resize(w, h);
		if (imGuiImageID != 0) {
			ImGuiLayer::UpdateVulkanTexture(imGuiImageID, sampler, viewport.GetColorView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		else {
			imGuiImageID = ImGuiLayer::AddVulkanTexture(descriptorPool, sampler, viewport.GetColorView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	
	void EditorRenderer::BindPipeline(VkPipeline pipeline, VkPipelineLayout layout) {
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
}