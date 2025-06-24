#include "Layers/ViewportLayer.hpp"
#include "SGF.hpp"
#include "Layers/ImGuiLayer.hpp"

namespace SGF {
	ViewportLayer::ViewportLayer(VkFormat colorFormat) : Layer("Viewport"), imageFormat(colorFormat), 
		commands(Device::Get(), QUEUE_TYPE_GRAPHICS, 0, VK_COMMAND_BUFFER_LEVEL_PRIMARY, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) {
		auto& device = Device::Get();
		const std::vector<VkAttachmentDescription> attachments = {
			createAttachmentDescription(imageFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR),
			createAttachmentDescription(VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
		};
		auto colorRef = createAttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		auto depthRef = createAttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		const std::vector<VkSubpassDescription> subpasses = {
			createSubpassDescription(&colorRef, 1, nullptr, &depthRef)
		};
		const std::vector<VkSubpassDependency> dependencies = {
			{ VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT }
		};
		renderPass = device.renderPass(attachments, subpasses, dependencies);
		pipelineLayout = device.pipelineLayout(nullptr, 0, nullptr, 0);
		graphicsPipeline = device.graphicsPipeline(pipelineLayout, renderPass, 0).dynamicState(VK_DYNAMIC_STATE_SCISSOR).dynamicState(VK_DYNAMIC_STATE_VIEWPORT)
				.vertexShader("shaders/test_triangle.vert").fragmentShader("shaders/test_triangle.frag").sampleCount(VK_SAMPLE_COUNT_1_BIT).build();
		sampler = device.imageSampler(VK_FILTER_NEAREST);
		signalSemaphore = device.semaphore();
	}
	ViewportLayer::~ViewportLayer() {
		auto& device = Device::Get();
		destroyFramebuffer();
		device.destroy(renderPass, graphicsPipeline, pipelineLayout, sampler, signalSemaphore);
	}
	void ViewportLayer::onAttach() {
	}
	void ViewportLayer::onDetach() {

	}
	void ViewportLayer::onRender(RenderEvent& event) {
		renderViewport(event);
	}

	
	void ViewportLayer::onUpdate(const UpdateEvent& event) {
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
		updateViewport(event);
		ImGui::Begin("Other Window");
		ImGui::Text("Application average %.3f ms/frame", event.getDeltaTime());
		ImGui::ColorButton("ColorButton", ImVec4(0.2, 0.8, 0.1, 1.0), 0, ImVec2(20.f, 20.f));
		//ImGui::Text("Frame time: {d}", event.get);
		ImGui::End();
	}
	void ViewportLayer::renderViewport(RenderEvent& event) {
		VkClearValue clearValues[] = {
			SGF::createColorClearValue(0.f, 0.f, 1.f, 1.f),
			SGF::createDepthClearValue(1.f, 0)
		};
		VkRect2D renderArea;
		renderArea.extent.width = width;
		renderArea.extent.height = height;
		renderArea.offset.x = 0;
		renderArea.offset.y = 0;
		commands.begin();
		//auto& c = event.getCommands();
		commands.beginRenderPass(renderPass,framebuffer, renderArea, clearValues, ARRAY_SIZE(clearValues), VK_SUBPASS_CONTENTS_INLINE);
		commands.bindGraphicsPipeline(graphicsPipeline);
		commands.setRenderArea(width, height);
		commands.draw(3);
		commands.endRenderPass();
		commands.end();
		commands.submit(nullptr, FLAG_NONE, signalSemaphore);
		event.addWait(signalSemaphore, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}
	void ViewportLayer::updateViewport(const UpdateEvent& event) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		ImVec2 size = ImGui::GetContentRegionAvail();
		if ((uint32_t)size.x != width || (uint32_t)size.y != height) {
			resizeFramebuffer((uint32_t)size.x, (uint32_t)size.y);
		}
		if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered()) {
			if (Input::IsKeyPressed(KEY_T)) {
				info("is key pressed!");
			}
		}
		ImGui::Image(imGuiImageID, size);
		ImGui::End();
		ImGui::PopStyleVar();
	}
	void ViewportLayer::resizeFramebuffer(uint32_t w, uint32_t h) {
		width = w;
		height = h;
		info("Resizing framebuffer");
		if (colorImage != VK_NULL_HANDLE) {
			destroyFramebuffer();
			info("framebuffer destroyed");
		}
		createFramebuffer();
	}
	void ViewportLayer::createFramebuffer() {
		auto& device = Device::Get();

		colorImage = device.image2D((uint32_t)width, (uint32_t)height, imageFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		depthImage = device.image2D((uint32_t)width, (uint32_t)height, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		//depthImage = device.image(info);

		device.waitIdle();
		VkImage images[] = { colorImage, depthImage };
		deviceMemory = device.allocate(images, ARRAY_SIZE(images));
		colorImageView = device.imageView2D(colorImage, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		depthImageView = device.imageView2D(depthImage, VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT);
		VkImageView imageViews[] = { colorImageView, depthImageView };
		framebuffer = device.framebuffer(renderPass, imageViews, ARRAY_SIZE(imageViews), (uint32_t)width, (uint32_t)height, 1);
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
			vkUpdateDescriptorSets(device.getLogical(), 1, write_desc, 0, nullptr);
		}
		else {
			descriptorSet = ImGui_ImplVulkan_AddTexture(sampler, colorImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			imGuiImageID = *(ImTextureID*)&descriptorSet;
		}
	}
	void ViewportLayer::destroyFramebuffer() {
		auto& device = Device::Get();
		device.waitIdle();
		device.destroy(framebuffer, colorImageView, colorImage, depthImageView, depthImage, deviceMemory);
	}
}