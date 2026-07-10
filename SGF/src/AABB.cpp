#include <SGF/Core/Math/Geometry/AABB.hpp>
#include <SGF/Core/Macros.hpp>

namespace SGF {
	AABB::AABB(const glm::vec3& newMin, const glm::vec3& newMax) {
		SGF_ASSERT(newMin.x <= newMax.x);
		SGF_ASSERT(newMin.y <= newMax.y);
		SGF_ASSERT(newMin.z <= newMax.z);
		m_Min = newMin;
		m_Max = newMax;
	}
	void AABB::AddPoint(const glm::vec3& point) {
		m_Max.x = std::max(point.x, m_Max.x);
		m_Max.y = std::max(point.y, m_Max.y);
		m_Max.z = std::max(point.z, m_Max.z);

		m_Min.x = std::min(point.x, m_Min.x);
		m_Min.y = std::min(point.y, m_Min.y);
		m_Min.z = std::min(point.z, m_Min.z);
	}
	float AABB::GetIntersection(const Ray& ray) const {
		glm::vec3 tMin = (m_Min - ray.GetOrigin()) * ray.GetInvDirection();
		glm::vec3 tMax = (m_Max - ray.GetOrigin()) * ray.GetInvDirection();
		glm::vec3 t1 = glm::min(tMin, tMax);
		glm::vec3 t2 = glm::max(tMin, tMax);
		float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
		float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);
		if (tFar < tNear || tFar < 0.0f) {
			return -1.f;
		}
		return tNear < 0.0f ? 0.0f : tNear;
	}
	bool AABB::HasIntersection(const Ray& ray) const {
		glm::vec3 tMin = (m_Min - ray.GetOrigin()) * ray.GetInvDirection();
		glm::vec3 tMax = (m_Max - ray.GetOrigin()) * ray.GetInvDirection();
		glm::vec3 t1 = glm::min(tMin, tMax);
		glm::vec3 t2 = glm::max(tMin, tMax);
		float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
		float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);
		return !(tFar < tNear || tFar < 0.0f);
	}
}