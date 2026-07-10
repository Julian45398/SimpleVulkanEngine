#pragma once

#include <SGF/Core/Math/Math.hpp>
#include <SGF/Core/Macros.hpp>
#include "Ray.hpp"

namespace SGF {
    class AABB {
    private:
        glm::vec3 m_Min;
        glm::vec3 m_Max;
    public:

        AABB(const glm::vec3& m_Min, const glm::vec3& m_Max);
        inline AABB() {
            m_Min = glm::vec3(0.f);
            m_Max = m_Min;
        }
        inline void Set(const glm::vec3& newMin, const glm::vec3& newMax) {
            SGF_ASSERT(newMin.x <= newMax.x);
            SGF_ASSERT(newMin.y <= newMax.y);
            SGF_ASSERT(newMin.z <= newMax.z);
            m_Min = newMin;
            m_Max = newMax;
        }
        inline void Set(const glm::vec3& startPoint) {
            m_Min = startPoint;
            m_Max = startPoint;
        }
        inline void Set(float x, float y, float z) {
            Set(glm::vec3(x, y, z));
        }
        void SetMin(const glm::vec3& newMin) {
            SGF_ASSERT(newMin.x < m_Max.x);
            SGF_ASSERT(newMin.y < m_Max.y);
            SGF_ASSERT(newMin.z < m_Max.z);
            m_Min = newMin;
        }
        void SetMax(const glm::vec3& newMax) {
            SGF_ASSERT(m_Min.x < newMax.x);
            SGF_ASSERT(m_Min.y < newMax.y);
            SGF_ASSERT(m_Min.z < newMax.z);
            m_Max = newMax;
        }
        void AddPoint(const glm::vec3& p);
        void Move(const glm::vec3& move);

        bool HasIntersection(const Ray& ray) const;
        float GetIntersection(const Ray& ray) const;
        const glm::vec3& GetMin() const { return m_Min; }
        const glm::vec3& GetMax() const { return m_Max; }
    };
}