#pragma once

#include "SGF_Core.hpp"
#include <glm/gtc/quaternion.hpp>

namespace SGF {
	inline void DecomposeTransformationMatrix(const glm::mat4& matrix, glm::vec3* pTranslation, glm::quat* pRotation, glm::vec3* pScale) {
		if (pTranslation) {
			*pTranslation = glm::vec3(matrix[3]);
		}
		if (pRotation || pScale) {
			glm::vec3 col0 = glm::vec3(matrix[0]);
			glm::vec3 col1 = glm::vec3(matrix[1]);
			glm::vec3 col2 = glm::vec3(matrix[2]);

			glm::vec3 scale = glm::vec3(glm::length(col0), glm::length(col1), glm::length(col2));
			if (pScale) {
				*pScale = scale;
			}
			if (pRotation) {
				glm::mat3 rotMat(
					col0 / (scale).x,
					col1 / (scale).y,
					col2 / (scale).z
				);
				*pRotation = glm::quat_cast(rotMat);
			}
		}
	}
}