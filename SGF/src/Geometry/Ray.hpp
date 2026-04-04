#pragma once

#include "SGF_Core.hpp"
#include <numeric>

class Ray {
private:
    glm::vec3 o;
    glm::vec3 dir;
    glm::vec3 invDir;
public:
    inline Ray() : o(0), dir(1.f, 0.f, 0.f), invDir(1.f / dir) {}

    inline Ray(const glm::vec3& origin, const glm::vec3 direction) {
        o = origin;
        dir = glm::normalize(direction);
        invDir = 1.f / (dir + glm::vec3(std::numeric_limits<float>::min()));
    }
    inline void SetOrigin(const glm::vec3& origin) {
        o = origin;
    }
    inline void SetDir(const glm::vec3& direction) {
        dir = glm::normalize(direction);
        invDir = 1.f / (dir + glm::vec3(std::numeric_limits<float>::min()));
    }
    
    inline const glm::vec3& GetOrigin() const {return o;}
    inline const glm::vec3& GetDir() const {return dir;}
    inline const glm::vec3& GetInvDir() const {return invDir;}
};