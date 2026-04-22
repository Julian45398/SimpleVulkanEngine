#pragma once

#include "SGF_Core.hpp"
#include <numeric>

class Ray {
private:
    glm::vec3 o;
    glm::vec3 dir;
    glm::vec3 invDir;
public:
    inline void SetDirection(const glm::vec3& direction) {
        dir = glm::normalize(direction);
        invDir = 1.f / (dir + glm::vec3(std::numeric_limits<float>::min()));
    }
    inline Ray() : o(0), dir(1.f, 0.f, 0.f), invDir(1.f / dir) {}
    inline Ray(const glm::vec3& origin, const glm::vec3& direction) : o(origin) {
        SetDirection(direction);
    }
    inline void OffsetOrigin(float epsilon) { o += dir * epsilon; }
    inline void SetOrigin(const glm::vec3& origin) { o = origin; }
    inline glm::vec3 GetTransformedDirection(const glm::mat4& transformationMatrix) const {
        return glm::vec3(transformationMatrix * glm::vec4(dir, 0.f));
    }
    inline glm::vec3 GetTransformedOrigin(const glm::mat4& transformationMatrix) const {
        return glm::vec3(transformationMatrix * glm::vec4(o, 1.f));
    }
    inline Ray GetTransformedRay(const glm::mat4& transformationMatrix) const {
        return Ray(GetTransformedOrigin(transformationMatrix), GetTransformedDirection(transformationMatrix));
    }
    inline glm::vec3 GetPoint(float t) const { return o + dir * t; }
    
    inline const glm::vec3& GetOrigin() const { return o; }
    inline const glm::vec3& GetDirection() const { return dir; }
    inline const glm::vec3& GetInvDirection() const { return invDir; }
};