#pragma once

#include "SGF_Core.hpp"
#include "Geometry/AABB.hpp"
#include "Render/Texture.hpp"

#include <glm/gtc/quaternion.hpp>

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
	class GenericModel {
	public:
		struct Bone {
			glm::mat4 offsetMatrix;
			glm::mat4 currentTransform;
			glm::mat4 nodeTransform;
			glm::mat4 globalTransform;
			uint32_t parent;
			uint32_t index;
			std::string name;
		};

		struct VertexWeight {
			uint32_t boneIndices[4];
			float boneWeights[4];
		};

		struct KeyFrame {
			float time;
			glm::vec3 value;
		};
		
		struct RotationKeyFrame {
			float time;
			glm::quat value;
		};

		struct AnimationChannel {
			uint32_t boneIndex;
			std::vector<KeyFrame> positionKeys;
			std::vector<RotationKeyFrame> rotationKeys;
			std::vector<KeyFrame> scaleKeys;
		};

		struct Animation {
			std::string name;
			float duration;
			float ticksPerSecond;
			std::vector<AnimationChannel> channels;
		};

	public:
		struct Vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec4 color;
		};

		struct Node {
			//glm::mat4 localTransform;
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

		// Animation Data:
		std::vector<Bone> bones;
		std::vector<VertexWeight> vertexWeights;
		std::vector<Animation> animations;


		std::string name;

		inline GenericModel() = default;
		inline GenericModel(const char* filename) { ImportModel(filename); }


        inline const std::string& GetName() const { return name; }
		inline const std::vector<uint32_t>& GetIndices() const { return indices; }
		inline const std::vector<Vertex>& GetVertices() const { return vertices; }
		inline const std::vector<Node>& GetNodes() const { return nodes; }
		inline Node& GetNode(size_t index) { return nodes[index]; }
		inline const Node& GetNode(size_t index) const { return nodes[index]; }
		inline const std::vector<Mesh>& GetMeshes() const { return meshes; }
		inline size_t GetVertexCount() const { return vertices.size(); }
		inline size_t GetIndexCount() const { return indices.size(); }
		inline size_t GetTextureCount() const { return textures.size(); }
		inline size_t GetNodeCount() const { return nodes.size(); }
		inline size_t GetMeshCount() const { return meshes.size(); }
		inline size_t GetTotalInstanceCount() const { size_t c = 0; for (size_t i = 0; i < nodes.size(); ++i) { c += nodes[i].meshes.size(); } return c; }

		inline bool HasAnimations() const { return !animations.empty(); }
		inline bool HasSkeletalAnimation() const { return !bones.empty() && !animations.empty(); }

		const Node* ImportModel(const char* filename);
		void RemoveModel(const char* name);
        const Node& Duplicate(const Node& node);
		const Node& AddChild(const Node& node, const std::string& name, const glm::mat4& transform);
        void Remove(const Node& node);

		inline void TransformNode(Node& node, const glm::mat4& deltaTransform) { node.globalTransform = deltaTransform * node.globalTransform; }
		void TransformNodeRecursive(Node& node, const glm::mat4& deltaTransform);

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