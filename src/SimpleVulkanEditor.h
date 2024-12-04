#pragma once

//#include "engine_core.h"

#include "ui_data.h"
#include "SceneBuffer.h"
#include "SVE_Backend.h"
#include "render/SVE_RenderPipeline.h"
#include "render/SVE_StagingBuffer.h"
#include "render/SVE_UniformBuffer.h"
#include "render/SVE_VertexBuffer.h"
#include "render/Camera.h"

struct UniformData {
	glm::mat4 transformMatrix;
};

class SimpleVulkanEditor {
	//ModelRenderer Renderer;
	//StagingBuffer StagingBuffer;
	SveUniformBuffer<UniformData> uniformBuffer;
	SceneBuffer sceneBuffer;
	//SveImageBuffer imageBuffer;
	SveRenderPipeline renderPipeline;
	std::vector<SveModel> models;

	Camera ViewCamera;
public:
	inline SimpleVulkanEditor() : 
		uniformBuffer(), sceneBuffer(), renderPipeline("resources/shaders/model.vert", "resources/shaders/model.frag", SVE_MODEL_VERTEX_INPUT_INFO, uniformBuffer, sceneBuffer.imageBuffer.getSampler(), sceneBuffer.imageBuffer.getImageView())
	{}
	void init();
	void run();
	void terminate();
private:
	void handleInput();
};