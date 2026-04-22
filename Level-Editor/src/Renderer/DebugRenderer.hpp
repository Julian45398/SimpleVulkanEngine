#pragma once

#include <SGF/Render.hpp>

namespace SGF {
	class DebugRenderer {
	public:
		struct LineVertex {
			glm::vec3 position;
			SGF::Color::RGBA8 color;
		};
	public:
		DebugRenderer(VkRenderPass renderPass, uint32_t subpass, uint32_t initialLineCapacity = 5000);
		~DebugRenderer();
		void AddLine(const glm::vec3& start, const glm::vec3& end, SGF::Color::RGBA8 color);
		void AddAABB(const glm::vec3& center, const glm::vec3& extents, SGF::Color::RGBA8 color);
		void AddTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, SGF::Color::RGBA8 color);
		void AddMesh(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, const glm::mat4& transform, SGF::Color::RGBA8 color);
		void AddSphere(const glm::vec3& center, float radius, SGF::Color::RGBA8 color);
		void AddFrustum(const glm::mat4& inverseProjection, const glm::mat4& inverseView, SGF::Color::RGBA8 color);
		void Draw(VkCommandBuffer commandBuffer, const glm::mat4& viewProj, uint32_t viewportWidth, uint32_t viewportHeight);
		void Clear();
		inline uint32_t GetLineCount() const { return (uint32_t)lineVertices.size() / 2; }
	private:
		std::vector<LineVertex> lineVertices;
		HostCoherentRingBuffer<SGF_FRAMES_IN_FLIGHT> vertexRingBuffer;
		HostCoherentRingBuffer<SGF_FRAMES_IN_FLIGHT> cameraBuffer;
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSets[SGF_FRAMES_IN_FLIGHT];
	};
}