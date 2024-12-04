#pragma once

#include "core.h"
#include "SVE_Backend.h"

class Camera {
public:
	glm::vec3 Pos = { 0.f, 0.f, 0.f };
	glm::vec3 ViewDir = { 1.f, 0.f, 0.f };
	glm::vec3 RightDir = { 0.f, 1.f, 0.f };
	glm::mat3 Transform = glm::mat3(1.f);
	float Yaw = 0.0f;
	float Pitch = 0.0f;
	float Roll = 0.0f;
	float Fov = 45.0f;
public:
	Camera() = default;
	Camera(const glm::vec3& position, float Yaw, float Pitch, float Roll, float Fov)
		: Pos(position), Yaw(Yaw), Pitch(Pitch), Roll(Roll), Fov(Fov)
	{
		recalculateTransform();
	}
	Camera(float xpos, float ypos, float zpos, float Yaw, float Pitch, float Roll, float Fov)
		: Pos(glm::vec3(xpos, ypos, zpos)), Yaw(Yaw), Pitch(Pitch), Roll(Roll), Fov(Fov)
	{
		recalculateTransform();
	}
	inline void setPos(float xpos, float ypos, float zpos) {
		Pos = { xpos, ypos, zpos };
	}
	inline const glm::vec3& getPos() {
		return Pos;
	}
	inline void rotate(float yaw, float pitch) {
		constexpr float MAX_PITCH = glm::pi<float>() * 0.49;
		this->Yaw -= yaw;
		this->Pitch += pitch;

		if (MAX_PITCH < Pitch) {
			Pitch = MAX_PITCH;
		}
		else if (Pitch < -MAX_PITCH) {
			Pitch = -MAX_PITCH;
		}
		if (Yaw < -glm::pi<float>()) {
			Yaw += 2 * glm::pi<float>();
		}
		else if (glm::pi<float>() < Yaw) {
			Yaw -= 2 * glm::pi<float>();
		}
		recalculateTransform();
	}
	inline void roll(float radians) {
		Roll += radians;
		if (Roll < -glm::pi<float>()) {
			Roll += 2 * glm::pi<float>();
		}
		else if (glm::pi<float>() < Roll) {
			Roll -= 2 * glm::pi<float>();
		}
		recalculateTransform();
	}
	inline void moveForward(float amount) {
		Pos += Transform[0] * amount;
	}
	inline void moveUp(float amount) {
		Pos += Transform[2] * amount;
	}
	inline void moveRight(float amount) {
		Pos -= Transform[1] * amount;
	}
	inline void moveBack(float amount) {
		Pos -= Transform[0] * amount;
	}
	inline void moveLeft(float amount) {
		Pos += Transform[1] * amount;
	}
	inline void moveDown(float amount) {
		Pos -= Transform[2] * amount;
	}
	inline const glm::vec3& getForward() const {
		return -Transform[0];
	}
	inline const glm::vec3& getRight() const {
		return Transform[1];
	}
	inline const glm::vec3& getUp() const {
		return Transform[2];
	}
	inline glm::mat4 getViewProj() const {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		//float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		float aspect_ratio = SVE::getAspectRatio();
		const glm::vec3 forward = getForward();
		const glm::vec3 upwards = getUp();
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), 2 * Roll, -Transform[0]);
		glm::mat4 view = glm::lookAt(Pos, Pos - forward, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 proj = glm::perspective(glm::radians(Fov), aspect_ratio, 0.1f, 10000.0f);
		proj[1][1] *= -1;


		//glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//glm::mat4 view =  glm::lookAt(Pos, Pos + forward, upwards);
		//glm::mat4 proj =  glm::perspective(glm::radians(Fov), aspect_ratio, 0.1f, 10.0f);
		return proj * view * model;
	}
private:
	void recalculateTransform() {
		float sin_yaw = glm::sin(Yaw);
		float cos_yaw = glm::cos(Yaw);
		float sin_pitch = glm::sin(Pitch);
		float cos_pitch = glm::cos(Pitch);
		float sin_roll = glm::sin(Roll);
		float cos_roll = glm::cos(Roll);
		// forward:
		Transform[0][0] = cos_pitch * cos_yaw;
		Transform[0][1] = cos_pitch * sin_yaw;
		Transform[0][2] = -sin_pitch;
		// right:
		Transform[1][0] = sin_roll * sin_pitch * cos_yaw - cos_roll * sin_yaw;
		Transform[1][1] = sin_roll * sin_pitch * sin_yaw + cos_roll * cos_yaw;
		Transform[1][2] = sin_roll * cos_pitch;
		// up:
		Transform[2][0] = cos_roll * sin_pitch * cos_yaw + sin_roll * sin_yaw;
		Transform[2][1] = cos_roll * sin_pitch * sin_yaw - sin_roll * cos_yaw;
		Transform[2][2] = cos_roll * cos_pitch;
	}
};