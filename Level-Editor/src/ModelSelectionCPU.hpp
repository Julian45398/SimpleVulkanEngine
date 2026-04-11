#pragma once

#include "SGF_Core.hpp"
#include "Geometry/Ray.hpp"
#include "Model.hpp"
#include <limits>

namespace SGF {
    struct HitInfo {
        float t = std::numeric_limits<float>::max();
        glm::vec3 position;
        glm::vec3 normal;
        uint32_t triangleIndex;
		uint32_t meshIndex;
		uint32_t nodeIndex;
	};

    // Create a world-space ray from pixel coordinates.
    // Uses Vulkan conventions: framebuffer origin top-left, NDC z in [0,1].
    inline Ray CreateRayFromPixel(
        uint32_t px,
        uint32_t py,
        uint32_t screenWidth,
        uint32_t screenHeight,
        const glm::mat4& view, const glm::mat4& projection)
    {
        glm::vec4 viewport(0.0f, 0.0f,
            static_cast<float>(screenWidth),
            static_cast<float>(screenHeight));

        // Vulkan framebuffer origin is top-left
        float flippedY = static_cast<float>(py);

        //const auto& view = camera.GetView();
        //const auto& projection = camera.GetProj(90.f, (float)screenWidth / (float)screenHeight);
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

        glm::vec3 pvec = glm::cross(ray.GetDirection(), edge2);
        float det = glm::dot(edge1, pvec);

        if (fabs(det) < EPSILON)
            return false; // Ray parallel to triangle

        float invDet = 1.0f / det;

        glm::vec3 tvec = ray.GetOrigin() - v0;
        float u = glm::dot(tvec, pvec) * invDet;
        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 qvec = glm::cross(tvec, edge1);
        float v = glm::dot(ray.GetDirection(), qvec) * invDet;
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

    inline bool GetMeshIntersection(
        const Ray& ray,
        const GenericModel& model,
        const GenericModel::Mesh& mesh,
        const glm::mat4& transform,
        HitInfo& outHit) {
        bool hit = false;
        const auto& vertices = model.GetVertices();
        const auto& indices = model.GetIndices();
        for (size_t i = 0; i < mesh.indexCount / 3; ++i) {
            uint32_t i0 = indices[mesh.indexOffset + i * 3 + 0];
            uint32_t i1 = indices[mesh.indexOffset + i * 3 + 1];
            uint32_t i2 = indices[mesh.indexOffset + i * 3 + 2];

            glm::vec3 v0 = vertices[i0].position;
            glm::vec3 v1 = vertices[i1].position;
            glm::vec3 v2 = vertices[i2].position;

            v0 = glm::vec3(transform * glm::vec4(v0, 1.f));
            v1 = glm::vec3(transform * glm::vec4(v1, 1.f));
            v2 = glm::vec3(transform * glm::vec4(v2, 1.f));

            float t, u, v;
            if (IntersectTriangle(ray, v0, v1, v2, t, u, v)) {
                if (t < outHit.t) {
                    hit = true;
                    outHit.t = t;
                    outHit.position = ray.GetOrigin() + t * ray.GetDirection();
                    outHit.normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
                    outHit.triangleIndex = static_cast<uint32_t>(i);
                    outHit.meshIndex = 0;
					outHit.nodeIndex = 0;
                }
            }
        }
        return hit;
    }

    inline bool GetNodeIntersection(
        const Ray& ray,
        const GenericModel& model,
        const GenericModel::Node& node,
        HitInfo& outHit) {
        bool hit = false;
        for (size_t i = 0; i < node.meshes.size(); ++i) {
            auto& m = model.GetMesh(node, i);
            if (GetMeshIntersection(ray, model, m, node.globalTransform, outHit)) {
				// fixed: meshIndex is the mesh id, nodeIndex is the node id
				outHit.meshIndex = node.meshes[i];
				outHit.nodeIndex = node.index;
                hit = true;
            }
        }
        return hit;
    }

    inline bool GetNodeIntersectionRecursive(
        const Ray& ray,
        const GenericModel& model,
        const GenericModel::Node& node,
        HitInfo& outHit, std::vector<uint32_t>& debugCheckedNodes) {
		debugCheckedNodes.push_back(node.index);
        bool hit = GetNodeIntersection(ray, model, node, outHit);
        for (size_t i = 0; i < node.children.size(); ++i) {
            auto& n = model.nodes[node.children[i]];
            if (GetNodeIntersectionRecursive(ray, model, n, outHit, debugCheckedNodes)) {
                hit = true;
            }
        }
        return hit;
    }

    inline bool GetModelIntersection(
        const Ray& ray,
        const GenericModel& model,
        HitInfo& outHit, std::vector<uint32_t>& debugCheckedNodes) {
        return GetNodeIntersectionRecursive(ray, model, model.GetRoot(), outHit, debugCheckedNodes);
    }

    /*
    bool IntersectIndexedTriangles(
        const Ray& ray,
        const GenericModel& model,
        HitInfo& outHit)
    {
        bool hit = false;
        float closestT = std::numeric_limits<float>::max();

		const auto& vertices = model.GetVertices();
		const auto& indices = model.GetIndices();
        const size_t triangleCount = indices.size() / 3;

        auto& root = model.GetRoot();
        auto& child = model.GetChild(root, 0);

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
                    outHit.position = ray.GetOrigin() + t * ray.GetDirection();
                    outHit.normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
                    outHit.triangleIndex = static_cast<uint32_t>(i);
                }
            }
        }
        return hit;
    }
    */
}