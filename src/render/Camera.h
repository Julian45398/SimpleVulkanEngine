#pragma once

#include "core.h"
#include "SVE_Backend.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Camera {
public:
	glm::vec3 pos = { 10.f, 0.f, 0.f };
	float fov = 45.0f;
	//glm::vec3 forward = glm::vec3(1.f, 0.f, 0.f);
	//float roll = 0.0f;
	glm::quat rotation = glm::quatLookAt(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));
public:
	Camera() = default;
	Camera(const glm::vec3& position, const glm::vec3& target, float fieldOfView)
	 : pos(position), fov(fieldOfView), rotation(glm::quatLookAt(position - target, glm::vec3(0.0f, 0.0f, 1.0f)))
	{}
	Camera(const glm::vec3& position, float yaw, float pitch, float roll, float fieldOfView)
	 : pos(position), fov(fieldOfView), rotation(glm::vec3(yaw, pitch, roll))
	{}
	Camera(float xpos, float ypos, float zpos, float yaw, float pitch, float roll, float fieldOfView)
	 : pos(glm::vec3(xpos, ypos, zpos)), fov(fieldOfView), rotation(glm::vec3(yaw, pitch, roll))
	{}

	inline void setPos(float xpos, float ypos, float zpos) {
		pos = { xpos, ypos, zpos };
	}
	inline const glm::vec3& getPos() {
		return pos;
	}
	inline glm::vec3 getForward() const {
		return glm::normalize(glm::rotate(rotation, glm::vec3(1.f, 0.f, 0.f)));
	}
	inline glm::vec3 getRight() const {
		return glm::normalize(glm::rotate(rotation, glm::vec3(0.f, 1.f, 0.f)));
	}
	inline glm::vec3 getUp() const {
		return glm::normalize(glm::rotate(rotation, glm::vec3(0.f, 0.f, 1.f)));
	}
	inline glm::vec3 getEuler() const {
		return glm::eulerAngles(rotation);
	}
	inline float getRoll() const {
		return glm::roll(rotation);
	}
	inline float getYaw() const {
		return glm::yaw(rotation);
	}
	inline float getPitch() const {
		return glm::pitch(rotation);
	}
	inline glm::mat3 getRotationMatrix() const {
		return glm::mat3_cast(rotation);
	}
	inline void rotate(float angle, const glm::vec3& axis) {
		//constexpr float MAX_PITCH = glm::pi<float>() * 0.499f;
		//float angleRad = angle;
    	//const auto& axisNorm = glm::normalize(axis);

    	//float w = glm::cos(angleRad * 0.5f);
    	//float v = glm::sin(angleRad * 0.5f);
    	//glm::vec3 qv = axisNorm * v;

    	//rotation = glm::normalize(glm::quat(w, qv));
		rotation = glm::normalize(glm::rotate(rotation, angle, axis));
	}
	inline void rotate(float pitch, float yaw) {
		const auto up = getUp();
		const auto right = getRight();
		glm::quat pitchQuat = glm::angleAxis(pitch, right);
		glm::quat yawQuat = glm::angleAxis(pitch, up);
		//rotate(pitch, right);
		rotation = yawQuat * pitchQuat * rotation;
		rotation = glm::normalize(rotation);
		//rotate(yaw, up);
	}
	inline void setForward(const glm::vec3& forward) {
		rotation = glm::quatLookAt(forward, getUp());
	}
	inline void roll(float radians) {
		/*
		Roll += radians;
		if (Roll < -glm::pi<float>()) {
			Roll += 2 * glm::pi<float>();
		}
		else if (glm::pi<float>() < Roll) {
			Roll -= 2 * glm::pi<float>();
		}
		recalculateTransform();
		*/
	}
	inline void moveForward(float amount) {
		pos += getForward() * amount;
	}
	inline void moveUp(float amount) {
		pos -= getUp() * amount;
	}
	inline void moveRight(float amount) {
		pos -= getRight() * amount;
	}
	inline void moveBack(float amount) {
		moveForward(-amount);
	}
	inline void moveLeft(float amount) {
		moveRight(-amount);
	}
	inline void moveDown(float amount) {
		moveUp(-amount);
	}
	inline glm::mat4 getViewProj() const {
		float aspect_ratio = SVE::getAspectRatio();
		//glm::mat4 model = glm::mat4(1.f);
		glm::vec3 forward = getForward();
		glm::mat4 view = glm::lookAt(pos, pos + forward, getUp());
		//glm::mat4 view = glm::lookAt(pos, Pos + Transform[0], glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 proj = glm::perspective(glm::radians(fov), aspect_ratio, 0.1f, 10000.0f);

		//proj[1][1] *= -1;
		//glm::mat4 rot = glm::toMat4(rotation);
		glm::mat4 res = proj * view;
		return res;
	}
	//inline Ray createRay(float xCenter, float yCenter) {
	//}
/*
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
*/
};