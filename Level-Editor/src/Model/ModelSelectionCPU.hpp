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


    // Source - https://stackoverflow.com/a/69185265
	// Posted by paulytools, modified by community. See post 'Timeline' for change history
	// Retrieved 2026-04-13, License - CC BY-SA 4.0

	// must normalize direction of ray
    inline bool intersectRayTri(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, glm::vec3 o, glm::vec3 n) {
        glm::vec3 e1, e2, pvec, qvec, tvec;

        e1 = v2 - v1;
        e2 = v3 - v1;
        pvec = glm::cross(n, e2);

        n = glm::normalize(n);
        //NORMALIZE(pvec);
        float det = glm::dot(pvec, e1);

        if (det != 0)
        {
            float invDet = 1.0f / det;
            tvec = o - v1;
            // NORMALIZE(tvec);
            float u = invDet * glm::dot(tvec, pvec);
            if (u < 0.0f || u > 1.0f)
            {

                return false;
            }
            qvec = glm::cross(tvec, e1);
            // NORMALIZE(qvec);
            float v = invDet * glm::dot(qvec, n);
            if (v < 0.0f || u + v > 1.0f)
            {

                return false;
            }
        }
        else return false;
        return true; // det != 0 and all tests for false intersection fail
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
    inline bool GetMeshIntersection(const Ray& ray, const GenericModel& model, const GenericModel::Mesh& mesh, const glm::mat4& transform, HitInfo& outHit) {
        bool hit = false;
        const auto& vertices = model.GetVertices();
        const auto& indices = model.GetIndices();
        for (size_t i = 0; i < mesh.indexCount / 3; ++i) {
            uint32_t i0 = indices[mesh.indexOffset + i * 3 + 0];
            uint32_t i1 = indices[mesh.indexOffset + i * 3 + 1];
            uint32_t i2 = indices[mesh.indexOffset + i * 3 + 2];

            glm::vec3 v0 = vertices[mesh.vertexOffset + i0].position;
            glm::vec3 v1 = vertices[mesh.vertexOffset + i1].position;
            glm::vec3 v2 = vertices[mesh.vertexOffset + i2].position;

            v0 = glm::vec3(transform * glm::vec4(v0, 1.f));
            v1 = glm::vec3(transform * glm::vec4(v1, 1.f));
            v2 = glm::vec3(transform * glm::vec4(v2, 1.f));

            float t, u, v;
            if (IntersectTriangle(ray, v0, v1, v2, t, u, v)) {
                if (t <= outHit.t) {
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

    inline bool GetMeshIntersection2(const Ray& ray, const GenericModel& model, const GenericModel::Mesh& mesh, const glm::mat4& transform, HitInfo& outHit) {
        bool hit = false;
        const auto& vertices = model.GetVertices();
        const auto& indices = model.GetIndices();
        auto invTransform = glm::inverse(transform);
        glm::vec3 transformedDir = ray.GetTransformedDirection(invTransform);
        float scale = glm::length(transformedDir);
        Ray invRay(ray.GetTransformedDirection(invTransform), transformedDir);
        for (size_t i = 0; i < mesh.indexCount / 3; ++i) {
            uint32_t i0 = indices[mesh.indexOffset + i * 3 + 0];
            uint32_t i1 = indices[mesh.indexOffset + i * 3 + 1];
            uint32_t i2 = indices[mesh.indexOffset + i * 3 + 2];

            glm::vec3 v0 = vertices[mesh.vertexOffset + i0].position;
            glm::vec3 v1 = vertices[mesh.vertexOffset + i1].position;
            glm::vec3 v2 = vertices[mesh.vertexOffset + i2].position;
            float t, u, v;

            if (IntersectTriangle(invRay, v0, v1, v2, t, u, v)) {
                t = t * scale;
                if (t <= outHit.t) {
                    hit = true;
                    outHit.t = t;
                    outHit.position = transform * glm::vec4((invRay.GetOrigin() + t * invRay.GetDirection()), 1.f);
                    outHit.normal = glm::transpose(invTransform) * glm::vec4(glm::normalize(glm::cross(v1 - v0, v2 - v0)), 1.f);
                    outHit.triangleIndex = static_cast<uint32_t>(i);
                    outHit.meshIndex = 0;
					outHit.nodeIndex = 0;
                }
            }
        }
        return hit;
    }
    inline bool GetNodeIntersection2(const Ray& ray, const GenericModel& model, const GenericModel::Node& node, HitInfo& outHit) {
        bool hit = false;
        for (size_t i = 0; i < node.meshes.size(); ++i) {
            auto& m = model.GetMesh(node, i);
            if (GetMeshIntersection2(ray, model, m, node.globalTransform, outHit)) {
				// fixed: meshIndex is the mesh id, nodeIndex is the node id
				outHit.meshIndex = node.meshes[i];
				outHit.nodeIndex = node.index;
                hit = true;
            }
        }
        return hit;
    }
    inline bool GetNodeIntersectionRecursive2(const Ray& ray, const GenericModel& model, const GenericModel::Node& node, HitInfo& outHit) {
        bool hit = GetNodeIntersection2(ray, model, node, outHit);
        for (size_t i = 0; i < node.children.size(); ++i) {
            auto& n = model.nodes[node.children[i]];
            if (GetNodeIntersectionRecursive2(ray, model, n, outHit)) {
                hit = true;
            }
        }
        return hit;
    }
    inline bool GetModelIntersection2(const Ray& ray, const GenericModel& model, HitInfo& outHit) {
        bool hit = false;
		size_t nodeCount = model.GetNodeCount();
        for (size_t i = 0; i < nodeCount; ++i) {
            auto& node = model.GetNode(i);
            if (GetNodeIntersection2(ray, model, node, outHit)) {
				hit = true;
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
        HitInfo& outHit) {
        bool hit = GetNodeIntersection(ray, model, node, outHit);
        for (size_t i = 0; i < node.children.size(); ++i) {
            auto& n = model.nodes[node.children[i]];
            if (GetNodeIntersectionRecursive(ray, model, n, outHit)) {
                hit = true;
            }
        }
        return hit;
    }
    inline bool GetModelIntersection(
        const Ray& ray,
        const GenericModel& model,
        HitInfo& outHit) {
        bool hit = false;
		size_t nodeCount = model.GetNodeCount();
        for (size_t i = 0; i < nodeCount; ++i) {
            auto& node = model.GetNode(i);
            if (GetNodeIntersection(ray, model, node, outHit)) {
				hit = true;
            }
        }
        return hit;
    }
}