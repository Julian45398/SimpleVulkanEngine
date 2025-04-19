#pragma once

#include "core.h"
#include "SVE_Ray.h"

class SveAABB {
public:
    glm::vec3 min;
    glm::vec3 max;

    SveAABB(const glm::vec3& min, const glm::vec3& max);
    inline SveAABB() {
        min = glm::vec3(0.f);
        max = min;
    }
    inline void set(const glm::vec3& newMin, const glm::vec3& newMax) {
        assert(newMin.x <= newMax.x);
        assert(newMin.y <= newMax.y);
        assert(newMin.z <= newMax.z);
        min = newMin;
        max = newMax;
    }
    inline void set(const glm::vec3& startPoint) {
        min = startPoint;
        max = startPoint;
    }
    inline void set(float x, float y, float z) {
        set(glm::vec3(x, y, z));
    }
    void setMin(const glm::vec3& newMin) {
        assert(newMin.x < max.x);
        assert(newMin.y < max.y);
        assert(newMin.z < max.z);
        min = newMin;
    }
    void setMax(const glm::vec3& newMax) {
        assert(min.x < newMax.x);
        assert(min.y < newMax.y);
        assert(min.z < newMax.z);
        max = newMax;
    }
    void addPoint(const glm::vec3& p);
    void move(const glm::vec3& move);

    bool hasIntersection(const SveRay& ray) const;
    float getIntersection(const SveRay& ray) const;
    const glm::vec3& getMin() const {return min;}
    const glm::vec3& getMax() const {return max;}
};