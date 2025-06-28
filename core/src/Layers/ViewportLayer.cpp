#include "Layers/ViewportLayer.hpp"
#include "SGF.hpp"
#include "Layers/ImGuiLayer.hpp"
#include "Render/CameraController.hpp"

namespace SGF {
	ViewportLayer::ViewportLayer(VkFormat colorFormat) : Layer("Viewport"), imageFormat(colorFormat), 
		cursor("assets/textures/zombie.png"), uniformBuffer(SGF_FRAMES_IN_FLIGHT) {
		auto& device = Device::Get();
		const std::vector<VkAttachmentDescription> attachments = {
			SGF::Vk::CreateAttachmentDescription(imageFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR),
			SGF::Vk::CreateAttachmentDescription(VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
		};
		auto colorRef = SGF::Vk::CreateAttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		auto depthRef = SGF::Vk::CreateAttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		const std::vector<VkSubpassDescription> subpasses = {
			SGF::Vk::CreateSubpassDescription(&colorRef, 1, nullptr, &depthRef)
		};
		const std::vector<VkSubpassDependency> dependencies = {
			{ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT }
		};
		renderPass = device.CreateRenderPass(attachments, subpasses, dependencies);
		pipelineLayout = device.CreatePipelineLayout(nullptr, 0, nullptr, 0);
		graphicsPipeline = device.CreateGraphicsPipeline(pipelineLayout, renderPass, 0).DynamicState(VK_DYNAMIC_STATE_SCISSOR).DynamicState(VK_DYNAMIC_STATE_VIEWPORT)
				.VertexShader("shaders/test_triangle.vert").FragmentShader("shaders/test_triangle.frag").SampleCount(VK_SAMPLE_COUNT_1_BIT).Build();
		sampler = device.CreateImageSampler(VK_FILTER_NEAREST);
		signalSemaphore = device.CreateSemaphore();

		VkDescriptorPoolSize poolSizes[] = {
			Vk::CreateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SGF_MAX_FRAMES_IN_FLIGHT),
			Vk::CreateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, SGF_MAX_FRAMES_IN_FLIGHT * 128),
			Vk::CreateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, SGF_MAX_FRAMES_IN_FLIGHT)
		};
		descriptorPool = device.CreateDescriptorPool(20, poolSizes);
		VkDescriptorSetLayoutBinding layoutBindings[] = {
			Vk::CreateDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
		};
		uniformLayout = device.CreateDescriptorSetLayout(layoutBindings);


		VkDescriptorSetLayout layouts[SGF_MAX_FRAMES_IN_FLIGHT];
		for (uint32_t i = 0; i <  SGF_MAX_FRAMES_IN_FLIGHT; ++i) {
			commands[i].Init(QUEUE_FAMILY_GRAPHICS, 0, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
			layouts[i] = uniformLayout;
		}
		device.AllocateDescriptorSets(descriptorPool, layouts, uniformDescriptors);
		// Writing Descriptor Sets:
		//std::vector<VkWriteDescriptorSet> descriptor_writes(uniformDescriptors.size());
		//VkWriteDescriptorSet descriptorWrites[SGF_MAX_FRAMES_IN_FLIGHT];
		VkDescriptorBufferInfo uniform_info = {VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
		for (uint32_t i = 0; i < SGF_MAX_FRAMES_IN_FLIGHT; ++i) {
			uniform_info.buffer = uniformBuffer.GetBuffer(i);
			VkWriteDescriptorSet writes[] = {
				Vk::CreateDescriptorWrite(uniformDescriptors[i], 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &uniform_info, 1)
			};
			device.UpdateDescriptors(writes);
		}
		modelRenderer.Initialize(renderPass, 0, descriptorPool, uniformLayout, SGF_MAX_FRAMES_IN_FLIGHT);
	}
	ViewportLayer::~ViewportLayer() {
		auto& device = Device::Get();
		DestroyFramebuffer();
		device.Destroy(renderPass, graphicsPipeline, pipelineLayout, sampler, signalSemaphore, descriptorPool, uniformLayout);
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
	void ViewportLayer::OnEvent(const UpdateEvent& event) {
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
		UpdateViewport(event);
		ImGui::Begin("Other Window");
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
				modelRenderer.AddModel(models.back());
			}
		}

		ImGui::Text("Cursor Pos: { %.4f, %.4f }", cursorpos.x, cursorpos.y);
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
		/*
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
		*/
		glm::vec3 up = camera.GetUp();
		glm::vec3 right = camera.GetRight();
		float yaw = camera.GetYaw();
		float pitch = camera.GetPitch();
		ImGui::ColorButton("ColorButton", ImVec4(0.2, 0.8, 0.1, 1.0), 0, ImVec2(20.f, 20.f));
		//ImGui::Text("Frame time: {d}", event.get);
		ImGui::End();
	}
	void ViewportLayer::RenderViewport(RenderEvent& event) {
		VkClearValue clearValues[] = {
			SGF::Vk::CreateColorClearValue(0.f, 0.f, 1.f, 1.f),
			SGF::Vk::CreateDepthClearValue(1.f, 0)
		};
		if (isOrthographic) {
			uniformBuffer.SetValueAt(imageIndex, cameraController.GetOrthoViewMatrix(viewSize, aspectRatio));
		} else {
			uniformBuffer.SetValueAt(imageIndex, cameraController.GetViewProjMatrix(aspectRatio));
		}
		VkRect2D renderArea;
		renderArea.extent.width = width;
		renderArea.extent.height = height;
		renderArea.offset.x = 0;
		renderArea.offset.y = 0;
		auto& c = commands[imageIndex];
		c.Begin();
		//auto& c = event.getCommands();
		c.BeginRenderPass(renderPass,framebuffer, renderArea, clearValues, ARRAY_SIZE(clearValues), VK_SUBPASS_CONTENTS_INLINE);
		c.BindGraphicsPipeline(graphicsPipeline);
		c.SetRenderArea(width, height);
		//c.Draw(3);
		modelRenderer.Draw(c, uniformDescriptors[imageIndex], width, height, imageIndex);
		c.EndRenderPass();
		c.End();
		c.Submit(nullptr, FLAG_NONE, signalSemaphore);
		event.AddWait(signalSemaphore, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		imageIndex = (imageIndex + 1) % SGF_MAX_FRAMES_IN_FLIGHT;
	}
	void ViewportLayer::UpdateViewport(const UpdateEvent& event) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		ImVec2 size = ImGui::GetContentRegionAvail();
		if ((uint32_t)size.x != width || (uint32_t)size.y != height) {
			ResizeFramebuffer((uint32_t)size.x, (uint32_t)size.y);
		}

		if (Input::HasFocus() && ImGui::IsWindowHovered()) {
			if (Input::IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				ImGui::SetWindowFocus();
			}
			cameraController.UpdateCamera(event);
		}
		ImGui::Image(imGuiImageID, size);
		ImGui::End();
		ImGui::PopStyleVar();
	}
	class SceneManager {

	};
	void ViewportLayer::ResizeFramebuffer(uint32_t w, uint32_t h) {
		width = w;
		height = h;
		info("Resizing framebuffer");
		if (colorImage != VK_NULL_HANDLE) {
			DestroyFramebuffer();
			info("framebuffer destroyed");
		}
		CreateFramebuffer();
	}
	void ViewportLayer::CreateFramebuffer() {
		auto& device = Device::Get();

		colorImage = device.CreateImage2D((uint32_t)width, (uint32_t)height, imageFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		depthImage = device.CreateImage2D((uint32_t)width, (uint32_t)height, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		//depthImage = CreateImage(info);

		device.WaitIdle();
		VkImage images[] = { colorImage, depthImage };
		deviceMemory = device.AllocateMemory(images, ARRAY_SIZE(images));
		colorImageView = device.CreateImageView2D(colorImage, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		depthImageView = device.CreateImageView2D(depthImage, VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT);
		VkImageView imageViews[] = { colorImageView, depthImageView };
		framebuffer = device.CreateFramebuffer(renderPass, imageViews, ARRAY_SIZE(imageViews), (uint32_t)width, (uint32_t)height, 1);
		if (descriptorSet != VK_NULL_HANDLE) {
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler = sampler;
			desc_image[0].imageView = colorImageView;
			desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = descriptorSet;
			write_desc[0].descriptorCount = 1;
			write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc[0].pImageInfo = desc_image;
			//vkUpdateDescriptorSets(device.getLogical(), 1, write_desc, 0, nullptr);
			device.UpdateDescriptors(write_desc);
		}
		else {
			descriptorSet = ImGui_ImplVulkan_AddTexture(sampler, colorImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			imGuiImageID = *(ImTextureID*)&descriptorSet;
		}
		aspectRatio = ((float)width/(float)height);
	}
	void ViewportLayer::DestroyFramebuffer() {
		auto& device = Device::Get();
		device.WaitIdle();
		device.Destroy(framebuffer, colorImageView, colorImage, depthImageView, depthImage, deviceMemory);
	}
}