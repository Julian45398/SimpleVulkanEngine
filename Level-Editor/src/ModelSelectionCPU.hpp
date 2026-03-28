#pragma once

#include "SGF_Core.hpp"
#include "Geometry/Ray.hpp"
#include "Model.hpp"

namespace SGF {
    struct HitInfo {
        float t;
        glm::vec3 position;
        glm::vec3 normal;
        uint32_t triangleIndex;
	};

    Ray CreateRayFromPixel(
        uint32_t px,
        uint32_t py,
        uint32_t screenWidth,
        uint32_t screenHeight,
        const glm::mat4& view,
        const glm::mat4& projection)
    {
        glm::vec4 viewport(0.0f, 0.0f,
            static_cast<float>(screenWidth),
            static_cast<float>(screenHeight));

        // Vulkan framebuffer origin is top-left
        float flippedY = static_cast<float>(py);

        // Near plane (depth = 0 in Vulkan)
        glm::vec3 nearPoint = glm::unProject(
            glm::vec3(static_cast<float>(px), flippedY, 0.0f),
            view,
            projection,
            viewport);

        // Far plane (depth = 1 in Vulkan)
        glm::vec3 farPoint = glm::unProject(
            glm::vec3(static_cast<float>(px), flippedY, 1.0f),
            view,
            projection,
            viewport);

        glm::vec3 direction = glm::normalize(farPoint - nearPoint);

        return Ray{ nearPoint, direction };
    }

    inline bool IntersectTriangle(
        const Ray& ray,
        const glm::vec3& v0,
        const glm::vec3& v1,
        const glm::vec3& v2,
        float& outT,
        float& outU,
        float& outV)
    {
        const float EPSILON = 1e-8f;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;

        glm::vec3 pvec = glm::cross(ray.direction, edge2);
        float det = glm::dot(edge1, pvec);

        if (fabs(det) < EPSILON)
            return false; // Ray parallel to triangle

        float invDet = 1.0f / det;

        glm::vec3 tvec = ray.origin - v0;
        float u = glm::dot(tvec, pvec) * invDet;
        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 qvec = glm::cross(tvec, edge1);
        float v = glm::dot(ray.direction, qvec) * invDet;
        if (v < 0.0f || u + v > 1.0f)
            return false;

        float t = glm::dot(edge2, qvec) * invDet;
        if (t < EPSILON)
            return false; // Intersection behind origin

        outT = t;
        outU = u;
        outV = v;

        return true;
    }

    bool IntersectIndexedTriangles(
        const Ray& ray,
        const GenericModel& model,
        HitInfo& outHit)
    {
        bool hit = false;
        float closestT = std::numeric_limits<float>::max();

        const size_t triangleCount = indices.size() / 3;
		const auto& vertices = model.GetVertices();
		const auto& indices = model.GetIndices();

        for (size_t i = 0; i < triangleCount; ++i)
        {
            uint32_t i0 = indices[i * 3 + 0];
            uint32_t i1 = indices[i * 3 + 1];
            uint32_t i2 = indices[i * 3 + 2];

            const glm::vec3& v0 = vertices[i0].position;
            const glm::vec3& v1 = vertices[i1].position;
            const glm::vec3& v2 = vertices[i2].position;

            float t, u, v;
            if (IntersectTriangle(ray, v0, v1, v2, t, u, v))
            {
                if (t < closestT)
                {
                    closestT = t;
                    hit = true;

                    outHit.t = t;
                    outHit.position = ray.origin + t * ray.direction;
                    outHit.normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
                    outHit.triangleIndex = static_cast<uint32_t>(i);
                }
            }
        }
        return hit;
    }
}