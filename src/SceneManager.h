#pragma once

#include "render/SVE_SceneRenderer.h"
#include "render/SVE_GridRenderer.h"
#include "render/CameraController.h"

class SveSceneManager {
private:
	std::vector<SveModel> models;
	SveSceneRenderer renderer;
	CameraController cameraController;


	inline void loadModel(const char* filename) {
		models.emplace_back(filename);
		renderer.addModel(models.back());
	}
};