#pragma once

#include "core.h"

#include "render/SVE_ImageBuffer.h"
#include "render/SVE_VertexBuffer.h"
#include "render/SVE_RenderPipeline.h"
#include "render/SVE_StagingBuffer.h"

struct SveModelVertex {
	alignas(16) glm::vec3 position;
	uint32_t imageIndex;
	alignas(16) glm::vec3 normal;
	uint32_t padding2;
	alignas(16) glm::vec2 uvCoord;
	uint32_t padding3;
	uint32_t padding4;
};

struct Ray {
	inline Ray(float xOrig, float yOrig, float zOrig, float xDir, float yDir, float zDir)
	: origin(xOrig, yOrig, zOrig), direction(xDir, yDir, zDir) {}

	inline Ray(const glm::vec3& orig, const glm::vec3& dir)
	: origin(orig), direction(dir) {}
	glm::vec3 origin;
	glm::vec3 direction;
};

struct AABB {
	glm::vec3 min = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 max = glm::vec3(0.f, 0.f, 0.f);
	void includePoint(const glm::vec3& point);
	void setNewStartingPoint(const glm::vec3& point);
	glm::vec2 getIntersection(const Ray& ray);
};

struct BVHNode {
	AABB box;
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
	void buildBVH();
private:
	void buildBVHChildren(BVHNode parent);
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
	std::vector<Mesh> meshes;
	std::vector<Image> images;
	AABB boundingBox;

	SveModel(const char* filename);
	glm::vec2 getIntersection(const Ray& ray);
};
