#include "SVE_AABB.h"

SveAABB::SveAABB(const glm::vec3& newMin, const glm::vec3& newMax) {
    assert(newMin.x <= newMax.x);
    assert(newMin.y <= newMax.y);
    assert(newMin.z <= newMax.z);
    min = newMin;
    max = newMax;
}

void SveAABB::addPoint(const glm::vec3& point) {
	max.x = std::max(point.x, max.x);
	max.y = std::max(point.y, max.y);
	max.z = std::max(point.z, max.z);

	min.x = std::min(point.x, min.x);
	min.y = std::min(point.y, min.y);
	min.z = std::min(point.z, min.z);
}

float SveAABB::getIntersection(const SveRay& ray) const {
	glm::vec3 tMin = (min - ray.getOrigin()) * ray.getInvDir();
	glm::vec3 tMax = (max - ray.getOrigin()) * ray.getInvDir();
	glm::vec3 t1 = glm::min(tMin, tMax);
	glm::vec3 t2 = glm::max(tMin, tMax);
	float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
	float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);
    if (tFar < tNear || tFar < 0.0f) {
        return -1.f;
    }
	return tNear < 0.0f ? 0.0f : tNear;
}

bool SveAABB::hasIntersection(const SveRay& ray) const {
    glm::vec3 tMin = (min - ray.getOrigin()) * ray.getInvDir();
	glm::vec3 tMax = (max - ray.getOrigin()) * ray.getInvDir();
	glm::vec3 t1 = glm::min(tMin, tMax);
	glm::vec3 t2 = glm::max(tMin, tMax);
	float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
	float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);
    return !(tFar < tNear || tFar < 0.0f);
}
