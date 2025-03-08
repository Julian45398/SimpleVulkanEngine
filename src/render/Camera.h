#pragma once

#include "core.h"
#include "SVE_Backend.h"

//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>


inline constexpr glm::vec3 X_AXIS = glm::vec3(1.f, 0.f, 0.f);
inline constexpr glm::vec3 Y_AXIS = glm::vec3(0.f, 1.f, 0.f);
inline constexpr glm::vec3 Z_AXIS = glm::vec3(0.f, 0.f, 1.f);

class Camera {
public:
	glm::vec3 pos = { 0.f, 0.f, 0.f };
	float yaw = 0.0f;
	float pitch = 0.0f;
	float roll = 0.0f;
	glm::mat3 transform = glm::mat3(X_AXIS, Y_AXIS, Z_AXIS);
	//float fov = 45.0f;
	//glm::vec3 forward = glm::vec3(1.f, 0.f, 0.f);
	//float roll = 0.0f;
	//glm::quat rotation = glm::quatLookAt(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));
public:
	Camera() = default;
	Camera(const glm::vec3& position, const glm::vec3& target)
	 : pos(position), transform(glm::lookAt(position, target, Z_AXIS))
	{}
	Camera(const glm::vec3& position, float yaw, float pitch, float roll)
	 : pos(position), yaw(yaw), pitch(pitch), roll(roll)
	{}
	Camera(float xpos, float ypos, float zpos, float yaw, float pitch, float roll, float fieldOfView)
	 : pos(glm::vec3(xpos, ypos, zpos)), yaw(yaw), pitch(pitch), roll(roll)
	{}

	inline void setPos(float xpos, float ypos, float zpos) {
		pos = { xpos, ypos, zpos };
	}
	inline const glm::vec3& getPos() const {
		return pos;
	}
	inline void moveForward(float amount) {
		pos += transform[0] * amount;
	}
	inline void moveUp(float amount) {
		pos += transform[2] * amount;
	}
	inline void moveRight(float amount) {
		pos += transform[1] * amount;
	}
	inline void moveBack(float amount) {
		pos -= transform[0] * amount;
	}
	inline void moveLeft(float amount) {
		pos -= transform[1] * amount;
	}
	inline void moveDown(float amount) {
		pos -= transform[2] * amount;
	}
	inline const glm::vec3& getForward() const {
		return transform[0];
	}
	inline const glm::vec3& getRight() const {
		return transform[1];
	}
	inline const glm::vec3 getUp() const {
		return transform[2];
	}
	inline glm::vec3 getEuler() const {
		return glm::vec3(yaw, pitch, roll);
	}
	inline float getRoll() const {
		return roll;
	}
	inline float getYaw() const {
		return yaw;
	}
	inline float getPitch() const {
		return pitch;
	}
	inline const glm::mat3& getRotationMatrix() const {
		transform;
	}
	inline void setRotation(float yawRadians, float pitchRadians, float rollRadians) {
		this->yaw = yawRadians;
		this->pitch = pitchRadians;
		this->roll = rollRadians;
		updateRotation();
	}
	inline void rotate(float yawRadians, float pitchRadians, float rollRadians) {
		this->yaw += yawRadians;
		this->pitch += pitchRadians;
		this->roll += rollRadians;
		updateRotation();
	}
	
	inline void setForward(const glm::vec3& forward) {
		shl::logWarn("TODO: implement Camera::setForward()");
		//rotation = glm::quatLookAt(forward, getUp());
	}
	inline glm::mat4 getView() const {
		glm::vec3 forward = getForward();
		return glm::lookAt(pos, pos + forward, getUp());
	}
	inline glm::mat4 getProj(float fov, float aspectRatio, float nearClip = 0.01f, float farClip = 100000.f) const {
		glm::mat4 proj = glm::perspective(fov, aspectRatio, 0.1f, 10000.0f);
		proj[1][1] *= -1;
		return proj;
	}
	inline glm::mat4 getOrtho(float viewSize, float aspectRatio, float nearClip = 0.01f, float farClip = 100000.f) const {
		glm::mat4 ortho = glm::ortho(-viewSize * 0.5f * aspectRatio, viewSize * 0.5f * aspectRatio,  viewSize * 0.5f, -viewSize * 0.5f, nearClip, farClip);
		return ortho;
	}
	inline glm::mat4 getOrthoView(float viewSize, float aspectRatio, float nearClip = 0.01f, float farClip = 100000.f) const {
		//glm::mat4 model = glm::rotate(glm::mat4(1.0f), 2.0f * roll, -forward);
		glm::mat4 view = getView();
		//glm::mat4 view = glm::lookAt(pos, Pos + Transform[0], glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 proj = getOrtho(viewSize, aspectRatio, nearClip, farClip);

		return proj * view;
	}
	inline glm::mat4 getViewProj(float fovRadians, float aspectRatio, float nearClip = 0.01f, float farClip = 100000.f) const {
		//glm::mat4 model = glm::rotate(glm::mat4(1.0f), 2.0f * roll, -forward);
		glm::mat4 view = getView();
		//glm::mat4 view = glm::lookAt(pos, Pos + Transform[0], glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 proj = getProj(fovRadians, aspectRatio, nearClip, farClip);

		//glm::mat4 rot = glm::toMat4(rotation);
		glm::mat4 res = proj * view;
		return res;
	}
	//inline Ray createRay(float xCenter, float yCenter) {
	//}
	void recalculateTransform() {
		float sin_yaw = glm::sin(yaw);
		float cos_yaw = glm::cos(yaw);
		float sin_pitch = glm::sin(pitch);
		float cos_pitch = glm::cos(pitch);
		float sin_roll = glm::sin(roll);
		float cos_roll = glm::cos(roll);
		// forward:
		transform[0][0] = cos_pitch * cos_yaw;
		transform[0][1] = cos_pitch * sin_yaw;
		transform[0][2] = -sin_pitch;
		// right:
		transform[1][0] = sin_roll * sin_pitch * cos_yaw - cos_roll * sin_yaw;
		transform[1][1] = sin_roll * sin_pitch * sin_yaw + cos_roll * cos_yaw;
		transform[1][2] = sin_roll * cos_pitch;
		// up:
		transform[2][0] = cos_roll * sin_pitch * cos_yaw + sin_roll * sin_yaw;
		transform[2][1] = cos_roll * sin_pitch * sin_yaw - sin_roll * cos_yaw;
		transform[2][2] = cos_roll * cos_pitch;
	}
	void updateRotation() {
		constexpr float MAX_PITCH = glm::pi<float>() * 0.499f;
		if (this->roll < -glm::pi<float>()) {
			this->roll += 2 * glm::pi<float>();
		}
		else if (glm::pi<float>() < this->roll) {
			this->roll -= 2.f * glm::pi<float>();
		}
		if (MAX_PITCH < this->pitch) {
			this->pitch = MAX_PITCH;
		}
		else if (this->pitch < -MAX_PITCH) {
			this->pitch = -MAX_PITCH;
		}
		if (this->yaw < -glm::pi<float>()) {
			this->yaw += 2.f * glm::pi<float>();
		}
		else if (glm::pi<float>() < this->yaw) {
			this->yaw -= 2.f * glm::pi<float>();
		}
		recalculateTransform();
	}
};