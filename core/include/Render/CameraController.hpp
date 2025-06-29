#pragma once


#include "Window.hpp"
#include "Camera.hpp"
#include "Layers/LayerStack.hpp"


namespace SGF {
	class CameraController {
		inline static constexpr float BASE_ZOOM_FACTOR = 0.5f;
		inline static constexpr float BASE_SENSITIVITY = 0.002f;
		inline static constexpr float BASE_MOVEMENT_SPEED = 0.008f;
		Camera camera;
		float fieldOfView = BASE_ZOOM_FACTOR;
		float xSensitivity = BASE_SENSITIVITY;
		float ySensitivity = BASE_SENSITIVITY;
		float movementSpeed = BASE_MOVEMENT_SPEED;
		//glm::vec2 mousePos = glm::vec2(0.0f, 0.0f);
	public:
		inline void SetZoom(float zoomFactor) {
			float inv_zoom = 1.f / (BASE_ZOOM_FACTOR + zoomFactor);
			fieldOfView = glm::atan(inv_zoom);
			//fieldOfView = BASE_FOV * zoomFactor;
			xSensitivity = BASE_SENSITIVITY * inv_zoom;
			ySensitivity = BASE_SENSITIVITY * inv_zoom;
		}
		inline void UpdateCamera(const glm::dvec2& cursorMove, double frameTime) {
			assert(Input::IsMouseButtonPressed(MOUSE_BUTTON_RIGHT));
			//const float scale_factor = 0.001f;
			float x_amount = -(cursorMove.x) * xSensitivity;
			float y_amount = (cursorMove.y) * ySensitivity;
			float w_amount = 0.0f;
			if (Input::IsKeyPressed(KEY_E)) {
				w_amount = frameTime * xSensitivity;
			}
			if (Input::IsKeyPressed(KEY_Q)) {
				w_amount -= frameTime * xSensitivity;
			}
			if (Input::IsKeyPressed(KEY_LEFT_CONTROL)) {
				movementSpeed = 8.f * BASE_MOVEMENT_SPEED;
			}
			else {
				movementSpeed = BASE_MOVEMENT_SPEED;
			}
			camera.Rotate(x_amount, y_amount, w_amount);
			if (Input::IsKeyPressed(KEY_W)) {
				camera.MoveForward(frameTime * movementSpeed);
			}
			if (Input::IsKeyPressed(KEY_S)) {
				camera.MoveBack(frameTime * movementSpeed);
			}
			if (Input::IsKeyPressed(KEY_A)) {
				camera.MoveLeft(frameTime * movementSpeed);

			}
			if (Input::IsKeyPressed(KEY_D)) {
				camera.MoveRight(frameTime * movementSpeed);
			}
		}
		inline glm::mat4 GetViewProjMatrix(float aspectRatio) const { return camera.GetViewProj(fieldOfView, aspectRatio); }
		inline glm::mat4 GetViewMatrix() const { return camera.GetView(); }
		inline glm::mat4 GetProjMatrix(float aspectRatio) const { return camera.GetProj(fieldOfView, aspectRatio); }
		inline glm::mat4 GetOrthoViewMatrix(float viewSize, float aspectRatio) const { return camera.GetOrthoView(viewSize, aspectRatio); }
		inline Camera& GetCamera() { return camera; }
		inline const Camera& GetCamera() const { return camera; }
	};
}
