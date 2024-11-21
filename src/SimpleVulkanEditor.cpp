// SimpleVulkanEngine.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#include "SimpleVulkanEditor.h"

void SimpleVulkanEditor::init()
{
	initBackend();
	Core.initialize(600, 400);
	Renderer.init();
	sceneBuffer.init();
	models.emplace_back("resources/assets/models/test_car.gltf");
	shl::logDebug("loading model finished!");
	sceneBuffer.addModel(models[0]);
}

void SimpleVulkanEditor::handleInput() {
	glfwPollEvents();
	static float xpos, ypos;
	auto pos = Core.getMousePos();
	if (Core.isMouseClicked(GLFW_MOUSE_BUTTON_2)) {
		Core.hideCursor();
		ViewCamera.rotate(0.005f * (pos.x - xpos), 0.005f * (pos.y - ypos));
		if (Core.isKeyPressed(GLFW_KEY_W)) {
			ViewCamera.moveForward(0.5f * Core.getFrameTime());
		}
		if (Core.isKeyPressed(GLFW_KEY_S)) {
			ViewCamera.moveForward(-0.5f * Core.getFrameTime());
		}
		if (Core.isKeyPressed(GLFW_KEY_A)) {
			ViewCamera.moveRight(-0.5f * Core.getFrameTime());
		}
		if (Core.isKeyPressed(GLFW_KEY_D)) {
			ViewCamera.moveRight(0.5f * Core.getFrameTime());
		}
	}
	else {
		Core.showCursor();
	}
	xpos = pos.x;
	ypos = pos.y;
}

void SimpleVulkanEditor::run() {
	shl::logDebug("starting loop:");
	while (Core.shouldRun()) {
		handleInput();
		auto primary_commands = Core.beginRendering();
		{
			UniformData data = {};
			data.Transform = ViewCamera.getViewProj();
			Renderer.updateUniformBuffer(data);
		}
		if (sceneBuffer.hasChanges()) {
			shl::logDebug("scene has changes!");
			sceneBuffer.uploadChanges(primary_commands);
		}
		controlUI(ViewCamera);
		auto secondary = Core.getNextSecondaryCommands();
		Renderer.getRenderCommands(secondary, primary_commands);
		sceneBuffer.render(secondary);
		vkEndCommandBuffer(secondary);

		Core.finalizeRendering();
	}
}

void SimpleVulkanEditor::terminate()
{
}
