﻿// SimpleVulkanEngine.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#include "SimpleVulkanEditor.h"
#include "SVE_Backend.h"

void SimpleVulkanEditor::init()
{
	//SVE::init(600, 400);
	//sceneBuffer.init();
	//renderPipeline.create("resources/shaders/model.vert", "resources/shaders/model.frag", SVE_MODEL_VERTEX_INPUT_INFO, uniformBuffer, sceneBuffer.imageBuffer.getSampler(), sceneBuffer.imageBuffer.getImageView());
	models.emplace_back("resources/assets/models/test_car.gltf");
	shl::logDebug("loading model finished!");
	sceneBuffer.addModel(models[0]);
}

void SimpleVulkanEditor::handleInput() {
	glfwPollEvents();
	static float xpos, ypos;
	auto pos = SVE::getCursorPos();
	if (SVE::isMouseClicked(GLFW_MOUSE_BUTTON_2)) {
		SVE::hideCursor();
		ViewCamera.rotate(0.005f * (pos.x - xpos), 0.005f * (pos.y - ypos));
		if (SVE::isKeyPressed(GLFW_KEY_W)) {
			ViewCamera.moveForward(0.5f * SVE::getFrameTime());
		}
		if (SVE::isKeyPressed(GLFW_KEY_S)) {
			ViewCamera.moveForward(-0.5f * SVE::getFrameTime());
		}
		if (SVE::isKeyPressed(GLFW_KEY_A)) {
			ViewCamera.moveRight(-0.5f * SVE::getFrameTime());
		}
		if (SVE::isKeyPressed(GLFW_KEY_D)) {
			ViewCamera.moveRight(0.5f * SVE::getFrameTime());
		}
	}
	else {
		SVE::showCursor();
	}
	xpos = pos.x;
	ypos = pos.y;
}

void SimpleVulkanEditor::run() {
	shl::logDebug("starting loop:");
	VkCommandPool pools[] = {
		vkl::createCommandPool(SVE::getDevice(), SVE::getGraphicsFamily(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT),
		vkl::createCommandPool(SVE::getDevice(), SVE::getGraphicsFamily(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)
	};
	VkCommandBuffer secondary[]{
		vkl::createCommandBuffer(SVE::getDevice(), pools[0], VK_COMMAND_BUFFER_LEVEL_SECONDARY),
		vkl::createCommandBuffer(SVE::getDevice(), pools[1], VK_COMMAND_BUFFER_LEVEL_SECONDARY)
	};
	while (!SVE::shouldClose()) {
		handleInput();
		auto primary_commands = SVE::newFrame();
		vkl::resetCommandPool(SVE::getDevice(), pools[SVE::getInFlightIndex()]);
		SVE::beginRenderCommands(secondary[SVE::getInFlightIndex()]);
		renderPipeline.bindPipeline(secondary[SVE::getInFlightIndex()]);
		//auto primary_commands = SVE::beginRendering();
		{
			UniformData data = {};
			data.transformMatrix = ViewCamera.getViewProj();
			uniformBuffer.update(data);
		}
		if (sceneBuffer.hasChanges()) {
			shl::logDebug("scene has changes!");
			sceneBuffer.uploadChanges(primary_commands);
		}
		controlUI(ViewCamera);
		//auto secondary = SVE::getNextSecondaryCommands();
		//Renderer.getRenderCommands(secondary, primary_commands);
		sceneBuffer.render(secondary[SVE::getInFlightIndex()]);
		vkEndCommandBuffer(secondary[SVE::getInFlightIndex()]);

		SVE::renderFrame(1, &secondary[SVE::getInFlightIndex()]);
	}
}

void SimpleVulkanEditor::terminate()
{
	vkDeviceWaitIdle(SVE::getDevice());
}
