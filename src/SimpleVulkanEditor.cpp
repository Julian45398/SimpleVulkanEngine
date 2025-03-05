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
	viewCameraController.updateCamera();
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
		glm::mat4 matrix;
		if (isOrthographic) {
			matrix = viewCameraController.getOrthoViewMatrix(viewSize);
		}
		else {
			matrix = viewCameraController.getViewProjMatrix();
		}
		sceneRenderer.draw(secondary[SVE::getInFlightIndex()], matrix);
		vkEndCommandBuffer(secondary[SVE::getInFlightIndex()]);

		SVE::renderFrame(1, &secondary[SVE::getInFlightIndex()]);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
