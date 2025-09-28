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
			uint32_t textureIndex = 0;
			AABB boundingBox;
			std::vector<glm::mat4> instanceTransforms;
		};
		struct Vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec4 color;
		};
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Texture> textures;
		std::vector<MeshInfo> meshInfos;
		std::vector<glm::mat4> modelTransforms;
		std::string debugName;

		inline GenericModel() = default;
		inline GenericModel(const char* filename) { LoadFromFile(filename); }
		inline void Clear() {
			meshInfos.clear();
			vertices.clear();
			//vertexPositions.clear();
			//vertexNormals.clear();
			//uvCoordinates.clear();
			indices.clear();
			textures.clear();
		}
		inline size_t GetTotalVertexCount() const { return vertices.size(); }
		inline size_t GetTotalIndexCount() const { return indices.size(); }
		inline size_t GetTotalTextureCount() const { return textures.size(); }
		inline size_t GetTotalInstanceCount() const { size_t count = 0; for (const auto& mesh : meshInfos) { count += mesh.instanceTransforms.size(); } return count; }

		void LoadFromFile(const char* filename);
	};
}