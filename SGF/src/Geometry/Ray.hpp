#pragma once

#include "SGF_Core.hpp"
#include <numeric>

class Ray {
private:
    glm::vec3 o;
    glm::vec3 dir;
    glm::vec3 invDir;
public:
    inline Ray(const glm::vec3& origin, const glm::vec3 direction) {
        o = origin;
        dir = glm::normalize(direction);
        invDir = 1.f / (dir + glm::vec3(std::numeric_limits<float>::min()));
    }
    inline void setOrigin(const glm::vec3& origin) {
        o = origin;
    }
    inline void setDir(const glm::vec3& direction) {
        dir = glm::normalize(direction);
        invDir = 1.f / (dir + glm::vec3(std::numeric_limits<float>::min()));
    }
    
    inline const glm::vec3& getOrigin() const {return o;}
    inline const glm::vec3& getDir() const {return dir;}
    inline const glm::vec3& getInvDir() const {return invDir;}
};