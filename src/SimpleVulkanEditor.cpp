// SimpleVulkanEngine.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#include "SimpleVulkanEditor.h"
#include "SVE_Backend.h"
#include "render/UiHandler.h"
#include "render/SVE_SceneRenderer.h"

void SimpleVulkanEditor::init()
{
	//SVE::init(600, 400);
	//sceneBuffer.init();
	//renderPipeline.create("resources/shaders/model.vert", "resources/shaders/model.frag", SVE_MODEL_VERTEX_INPUT_INFO, uniformBuffer, sceneBuffer.imageBuffer.getSampler(), sceneBuffer.imageBuffer.getImageView());

	models.emplace_back("resources/assets/models/test_car.gltf");
	//models[0].loadFromGLTF();
	//shl::logDebug("loading model finished!");
	renderer = new SveSceneRenderer();
	renderer->addModel(models[0]);


	Image image;
	image.width = models.back().images[0].width;
	image.height = models.back().images[0].height;
	image.pixels = std::vector(models.back().images[0].pixels);
	Mesh mesh;
	mesh.imageIndex = 0;
	mesh.indices = std::vector(models.back().indices);
	mesh.vertices = std::vector(models.back().vertices);
	mesh.instanceTransforms.push_back(glm::mat4(1.f));
	Model model;
	model.images.push_back(image);
	model.meshes.push_back(mesh);
	sceneRenderer.addModel(model);
	//sceneBuffer.addModel(models[0]);
}

void SimpleVulkanEditor::loadModel(const char* filename) {
	models.emplace_back(filename);
	//models[0].loadFromGLTF();
	shl::logDebug("loading model finished!");
	//renderer = new SveSceneRenderer();
	renderer->addModel(models.back());
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

		BuildUi();
		vkl::resetCommandPool(SVE::getDevice(), pools[SVE::getInFlightIndex()]);
		SVE::beginRenderCommands(secondary[SVE::getInFlightIndex()]);
		//renderer->renderScene(secondary[SVE::getInFlightIndex()], ViewCamera);
		sceneRenderer.draw(secondary[SVE::getInFlightIndex()], ViewCamera.getViewProj());
		//renderPipeline.bindPipeline(secondary[SVE::getInFlightIndex()]);
		//auto primary_commands = SVE::beginRendering();
		{
			UniformData data = {};
			data.transformMatrix = ViewCamera.getViewProj();
			//uniformBuffer.update(data);
		}
		/*
		if (sceneBuffer.hasChanges()) {
			shl::logDebug("scene has changes!");
			sceneBuffer.uploadChanges(primary_commands);
		}*/

		//controlUI(ViewCamera);
		//auto secondary = SVE::getNextSecondaryCommands();
		//Renderer.getRenderCommands(secondary, primary_commands);
		//sceneBuffer.render(secondary[SVE::getInFlightIndex()]);
		vkEndCommandBuffer(secondary[SVE::getInFlightIndex()]);

		SVE::renderFrame(1, &secondary[SVE::getInFlightIndex()]);
	}
}

void SimpleVulkanEditor::terminate()
{
	vkDeviceWaitIdle(SVE::getDevice());
}