// SimpleVulkanEngine.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#include "SimpleVulkanEditor.h"
#include "SVE_Backend.h"
#include "render/UiHandler.h"
#include "render/SVE_SceneRenderer.h"

#include <thread>

void SimpleVulkanEditor::loadModel(const char* filename) {
	models.emplace_back(filename);
	//models[0].loadFromGLTF();
	shl::logDebug("loading model finished!");
	//renderer = new SveSceneRenderer();
	sceneRenderer.addModel(models.back());
}

void SimpleVulkanEditor::handleInput() {
	glfwPollEvents();
	static float xpos, ypos;
	auto pos = SVE::getCursorPos();
	if (SVE::isMouseClicked(GLFW_MOUSE_BUTTON_2)) {
		SVE::hideCursor();
		const float scale_factor = 0.001f;
		float x_amount = (float)pos.x - xpos;
		float y_amount = (float)pos.y - ypos;
		float rotation_amount = glm::sqrt(x_amount * x_amount + y_amount * y_amount) * scale_factor;
		if (rotation_amount != 0.0f) {
			ViewCamera.rotate(y_amount * scale_factor, x_amount * scale_factor);
		}
		shl::logDebug("rotation amount: ", rotation_amount, " x amount: ", x_amount, " y amount: ", y_amount);
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

SimpleVulkanEditor::SimpleVulkanEditor()
{
	models.emplace_back("resources/assets/models/test_car.gltf");
	
	sceneRenderer.addModel(models[0]);
}

void SimpleVulkanEditor::run() {
	VkCommandPool pools[] = {
		vkl::createCommandPool(SVE::getDevice(), SVE::getGraphicsFamily(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT),
		vkl::createCommandPool(SVE::getDevice(), SVE::getGraphicsFamily(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)
	};
	VkCommandBuffer secondary[]{
		vkl::createCommandBuffer(SVE::getDevice(), pools[0], VK_COMMAND_BUFFER_LEVEL_SECONDARY),
		vkl::createCommandBuffer(SVE::getDevice(), pools[1], VK_COMMAND_BUFFER_LEVEL_SECONDARY)
	};
	static_assert(ARRAY_SIZE(pools) == ARRAY_SIZE(secondary));
	while (!SVE::shouldClose()) {
		handleInput();
		auto primary_commands = SVE::newFrame();

		BuildUi();
		SVE::resetCommandPool(pools[SVE::getInFlightIndex()]);
		SVE::beginRenderCommands(secondary[SVE::getInFlightIndex()]);
		sceneRenderer.draw(secondary[SVE::getInFlightIndex()], ViewCamera.getViewProj());
		vkEndCommandBuffer(secondary[SVE::getInFlightIndex()]);

		SVE::renderFrame(1, &secondary[SVE::getInFlightIndex()]);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	SVE::deviceWaitIdle();
	for (size_t i = 0; i < ARRAY_SIZE(secondary); ++i) {
		SVE::destroyCommandBuffer(pools[i], secondary[i]);
		SVE::destroyCommandPool(pools[i]);
	}
	shl::logInfo("closing application...");
}

SimpleVulkanEditor::~SimpleVulkanEditor()
{
	SVE::deviceWaitIdle();
}
