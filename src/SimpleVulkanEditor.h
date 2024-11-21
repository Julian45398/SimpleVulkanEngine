#pragma once

#include "engine_core.h"

#include "ui_data.h"
#include "render/ModelRenderer.h"
#include "render/ModelVertexBuffer.h"
#include "render/StagingBuffer.h"
#include "SceneBuffer.h"


class SimpleVulkanEditor {
	ModelRenderer Renderer;
	//StagingBuffer StagingBuffer;
	SceneBuffer sceneBuffer;
	std::vector<SveModel> models;

	Camera ViewCamera;
public:
	void init();
	void run();
	void terminate();
private:
	void handleInput();
};