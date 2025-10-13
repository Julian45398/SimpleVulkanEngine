#pragma once

#include "SGF_Core.hpp"
#include "Geometry/AABB.hpp"
#include "Render/Texture.hpp"

namespace SGF {
	class Level;
	class LevelRenderer {
		struct Vertex {
			glm::vec3 position;
			uint32_t normal;
			glm::vec2 uv;
			glm::vec<4,uint8_t> vertexColor;
			uint32_t textureIndex;
		};

		void LoadLevel(const Level& level);
		void UnloadCurrent();
		void AddIndices();
	};
	
	/*
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
*/

	class GenericModel {
	public:
		struct Vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec4 color;
		};

		struct Node {
			glm::mat4 localTransform;
			glm::mat4 globalTransform;
			uint32_t parent;
			uint32_t index;
			std::vector<uint32_t> children;
			std::vector<uint32_t> meshes;
			std::string name;
		};

		struct Mesh {
			AABB boundingBox;
			uint32_t indexOffset;
			uint32_t indexCount;
			uint32_t vertexOffset;
			uint32_t vertexCount;
			uint32_t textureIndex;
		};

		struct Model {
			uint32_t firstMesh;
			uint32_t meshCount;
		};

		std::vector<uint32_t> indices;
		std::vector<Vertex> vertices;
		std::vector<Texture> textures;
		std::vector<Node> nodes;
		std::vector<Mesh> meshes;
		std::string name;

		inline GenericModel() = default;
		inline GenericModel(const char* filename) { ImportModel(filename); }


        inline const std::string& GetName() const { return name; }
		inline const std::vector<uint32_t>& GetIndices() const { return indices; }
		inline const std::vector<Vertex>& GetVertices() const { return vertices; }
		inline const std::vector<Node>& GetNodes() const { return nodes; }
		inline const std::vector<Mesh>& GetMeshes() const { return meshes; }
		inline size_t GetVertexCount() const { return vertices.size(); }
		inline size_t GetIndexCount() const { return indices.size(); }
		inline size_t GetTextureCount() const { return textures.size(); }
		inline size_t GetNodeCount() const { return nodes.size(); }
		inline size_t GetMeshCount() const { return meshes.size(); }
		inline size_t GetTotalInstanceCount() const { size_t c = 0; for (size_t i = 0; i < nodes.size(); ++i) { c += nodes[i].meshes.size(); } return c; }

		const Node* ImportModel(const char* filename);
		void RemoveModel(const char* name);
        const Node& Duplicate(const Node& node);
		const Node& AddChild(const Node& node, const std::string& name, const glm::mat4& transform);
        void Remove(const Node& node);

		std::vector<Node> GetChildren(const Node& node) const;
		std::vector<Mesh> GetMeshes(const Node& node) const;
		
		inline const Node& GetRoot() const { return nodes[0]; }
		inline const Node& GetParent(const Node& node) const { return nodes[node.parent]; }
		inline const Node& GetChild(const Node& node, size_t index) const { return nodes[node.children[index]]; }
		inline const Mesh& GetMesh(const Node& node, size_t index) const { return meshes[node.meshes[index]]; }
		inline size_t GetMeshCount(const Node& node) const { return node.meshes.size(); }
		inline size_t GetChildCount(const Node& node) const { return node.children.size(); }
	};

}