#pragma once

#include "core.h"

#include "render/SVE_ImageBuffer.h"
#include "render/SVE_VertexBuffer.h"
#include "render/SVE_RenderPipeline.h"
#include "render/SVE_StagingBuffer.h"
#include "SVE_AABB.h"
#include "SVE_Ray.h"

struct SveModelVertex {
	alignas(16) glm::vec3 position;
	uint32_t imageIndex;
	alignas(16) glm::vec3 normal;
	uint32_t padding2;
	alignas(16) glm::vec2 uvCoord;
	uint32_t padding3;
	uint32_t padding4;
};

struct Image {
	uint32_t width;
	uint32_t height;
	std::vector<uint8_t> pixels;
};


inline constexpr VkVertexInputBindingDescription SVE_MODEL_VERTEX_BINDINGS[] = {
		{0, sizeof(SveModelVertex), VK_VERTEX_INPUT_RATE_VERTEX}
};
inline constexpr VkVertexInputAttributeDescription SVE_MODEL_VERTEX_ATTRIBUTES[] = {
	{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SveModelVertex, position)},
	{1, 0, VK_FORMAT_R32_UINT, offsetof(SveModelVertex, imageIndex)},
	{2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SveModelVertex, normal)},
	{3, 0, VK_FORMAT_R32_UINT, offsetof(SveModelVertex, padding2)},
	{4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SveModelVertex, uvCoord)},
	{5, 0, VK_FORMAT_R32_UINT, offsetof(SveModelVertex, padding3)},
	{6, 0, VK_FORMAT_R32_UINT, offsetof(SveModelVertex, padding4)}
};
inline const VkPipelineVertexInputStateCreateInfo SVE_MODEL_VERTEX_INPUT_INFO = vkl::createPipelineVertexInputStateInfo(ARRAY_SIZE(SVE_MODEL_VERTEX_BINDINGS), SVE_MODEL_VERTEX_BINDINGS,
	ARRAY_SIZE(SVE_MODEL_VERTEX_ATTRIBUTES), SVE_MODEL_VERTEX_ATTRIBUTES);
	
struct SveImage {
	std::vector<uint8_t> pixels;
	uint32_t height = 0;
	uint32_t width = 0;
};

class SveModel {
	//std::vector<SveModelVertex> vertices;
	//std::vector<uint32_t> indices;
	//std::vector<SveImage> images;
public:
	struct BVHNode {
		SveAABB box;
		uint32_t childIndex;
		uint32_t startIndex;
		uint32_t indexCount;
	};

	class Mesh {
	public:
		std::vector<SveModelVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<BVHNode> volumeHierarchy;
		std::vector<glm::mat4> instanceTransforms;
		uint32_t imageIndex;
		uint32_t nodeCount;
		void buildBVH();
		float getIntersection(const SveRay& ray, float closest);
	private:
		float getNodeIntersection(const BVHNode& node, const SveRay& ray, float closest);
		float getTriangleIntersection(uint32_t startIndex, const SveRay& ray);
		void buildBVHChildren(const BVHNode& parent, uint32_t& nodeCount);
	};


	std::vector<Mesh> meshes;
	std::vector<Image> images;
	SveAABB boundingBox;

	SveModel(const char* filename);
	float getIntersection(const SveRay& ray);
};
