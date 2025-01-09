// SimpleVulkanEngine.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#include "SimpleVulkanEditor.h"
#include "SVE_Backend.h"
#include "render/UiHandler.h"

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
		const float scale_factor = 0.005f;
		ViewCamera.rotate(0.005f * ((float)pos.x - xpos), 0.005f * ((float)pos.y - ypos));
		if (SVE::isKeyPressed(GLFW_KEY_W)) {
			ViewCamera.moveForward((float)SVE::getFrameTime() * scale_factor);
		}
		if (SVE::isKeyPressed(GLFW_KEY_S)) {
			ViewCamera.moveForward(-(float)SVE::getFrameTime() * scale_factor);
		}
		if (SVE::isKeyPressed(GLFW_KEY_A)) {
			ViewCamera.moveRight(-(float)SVE::getFrameTime() * scale_factor);
		}
		if (SVE::isKeyPressed(GLFW_KEY_D)) {
			ViewCamera.moveRight((float)SVE::getFrameTime() * scale_factor);
		}
	}
	else {
		SVE::showCursor();
	}
	xpos = (float)pos.x;
	ypos = (float)pos.y;
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
		Menu::BuildUi();
		//controlUI(ViewCamera);
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
