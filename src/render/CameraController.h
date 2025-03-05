#pragma once


#include "Camera.h"


class CameraController {
	inline static constexpr float BASE_ZOOM_FACTOR = 0.5f;
	inline static constexpr float BASE_SENSITIVITY = 0.002f;
	inline static constexpr float BASE_MOVEMENT_SPEED = 0.008f;
	Camera camera;
	float fieldOfView = BASE_ZOOM_FACTOR;
	float xSensitivity = BASE_SENSITIVITY;
	float ySensitivity = BASE_SENSITIVITY;
	float movementSpeed = BASE_MOVEMENT_SPEED;
	glm::vec2 mousePos = glm::vec2(0.0f, 0.0f);

public:
	inline void setZoom(float zoomFactor) {
		float inv_zoom = 1.f / (BASE_ZOOM_FACTOR + zoomFactor);
		fieldOfView = glm::atan(inv_zoom);
		//fieldOfView = BASE_FOV * zoomFactor;
		xSensitivity = BASE_SENSITIVITY * inv_zoom;
		ySensitivity = BASE_SENSITIVITY * inv_zoom;
	}
	inline void updateCamera() {
		auto pos = SVE::getCursorPos();
		static bool isMouseClicked = false;
		if (SVE::isMouseClicked(GLFW_MOUSE_BUTTON_2)) {
			isMouseClicked = true;
			SVE::hideCursor();
			//const float scale_factor = 0.001f;
			float x_amount = (mousePos.x - (float)pos.x) * xSensitivity;
			float y_amount = ((float)pos.y - mousePos.y) * ySensitivity;
			float w_amount = 0.0f;
			if (SVE::isKeyPressed(GLFW_KEY_E)) {
				w_amount = (float)SVE::getFrameTime() * xSensitivity;
			}
			if (SVE::isKeyPressed(GLFW_KEY_Q)) {
				w_amount -= (float)SVE::getFrameTime() * xSensitivity;
			}
			if (glm::abs(x_amount) > 0.005f || glm::abs(y_amount) > 0.005f) {
				shl::logInfo("X_amount: ", x_amount, ", Y_Amount: ", y_amount);
			}
			if (SVE::isKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
				movementSpeed = 8.f * BASE_MOVEMENT_SPEED;
			}
			else {
				movementSpeed = BASE_MOVEMENT_SPEED;
			}
			camera.rotate(x_amount, y_amount, w_amount);
			if (SVE::isKeyPressed(GLFW_KEY_W)) {
				camera.moveForward((float)SVE::getFrameTime() * movementSpeed);
			}
			if (SVE::isKeyPressed(GLFW_KEY_S)) {
				camera.moveBack((float)SVE::getFrameTime() * movementSpeed);
			}
			if (SVE::isKeyPressed(GLFW_KEY_A)) {
				camera.moveLeft((float)SVE::getFrameTime() * movementSpeed);

			}
			if (SVE::isKeyPressed(GLFW_KEY_D)) {
				camera.moveRight((float)SVE::getFrameTime() * movementSpeed);
			}
		}
		else {
			if (isMouseClicked) {
				shl::logInfo("mouse is freed!");
			}
			isMouseClicked = false;
			SVE::showCursor();
		}
		mousePos.x = (float)pos.x;
		mousePos.y = (float)pos.y;
	}
	inline glm::mat4 getViewProjMatrix() const {
		return camera.getViewProj(fieldOfView, SVE::getAspectRatio());
	}
	inline glm::mat4 getViewMatrix() const {
		return camera.getView();
	}
	inline glm::mat4 getProjMatrix() const {
		return camera.getProj(fieldOfView, SVE::getAspectRatio());
	}
	inline glm::mat4 getOrthoViewMatrix(float viewSize) const {
		return camera.getOrthoView(viewSize, SVE::getAspectRatio());
	}
	inline const Camera& getCamera() const {
		return camera;
	}
};