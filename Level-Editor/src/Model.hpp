#pragma once

#include "SGF_Core.hpp"
#include "Geometry/AABB.hpp"
#include "Render/Texture.hpp"

namespace SGF {
	struct ModelVertex {
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

	class Model {
	public:
		struct BVHNode {
			AABB box;
			uint32_t childIndex;
			uint32_t startIndex;
			uint32_t indexCount;
		};

		class Mesh {
		public:
			std::vector<ModelVertex> vertices;
			std::vector<uint32_t> indices;
			std::vector<BVHNode> volumeHierarchy;
			std::vector<glm::mat4> instanceTransforms;
			uint32_t imageIndex;
			uint32_t nodeCount;
			void buildBVH();
			float getIntersection(const Ray& ray, float closest);
		private:
			float getNodeIntersection(const BVHNode& node, const Ray& ray, float closest);
			float getTriangleIntersection(uint32_t startIndex, const Ray& ray);
			void buildBVHChildren(const BVHNode& parent, uint32_t& nodeCount);
		};


		std::vector<Mesh> meshes;
		std::vector<Image> images;
		AABB boundingBox;

		Model(const char* filename);
		inline Model() {}
		void LoadFromFile(const char* filename);
		void Clear();
		float getIntersection(const Ray& ray);
	};
}