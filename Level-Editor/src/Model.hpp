#pragma once

#include "SGF_Core.hpp"
#include "Geometry/AABB.hpp"
#include "Render/Texture.hpp"

namespace SGF {
	class GenericModel {
	public:
		struct MeshInfo {
			uint32_t vertexCount = 0;
			uint32_t vertexOffset = 0;
			uint32_t indexCount = 0;
			uint32_t indexOffset = 0;
			AABB boundingBox;
			uint32_t textureIndex = 0;
			std::vector<glm::mat4> instanceTransforms;
		};
		std::vector<MeshInfo> meshInfos;
		std::vector<glm::vec4> vertexPositions;
		std::vector<glm::vec4> vertexNormals;
		std::vector<glm::vec2> uvCoordinates;
		std::vector<uint32_t> indices;
		std::vector<Texture> textures;
		std::string debugName;

		inline GenericModel() = default;
		inline GenericModel(const char* filename) { LoadFromFile(filename); }
		inline void Clear() {
			meshInfos.clear();
			vertexPositions.clear();
			vertexNormals.clear();
			uvCoordinates.clear();
			indices.clear();
			textures.clear();
		}
		inline size_t GetTotalVertexCount() const { return vertexPositions.size(); }
		inline size_t GetTotalIndexCount() const { return indices.size(); }
		inline size_t GetTotalTextureCount() const { return textures.size(); }
		inline size_t GetTotalInstanceCount() const { size_t count = 0; for (const auto& mesh : meshInfos) { count += mesh.instanceTransforms.size(); } return count; }

		void LoadFromFile(const char* filename);
	};
}