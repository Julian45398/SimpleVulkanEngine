#include "DebugRenderer.hpp"
#include "Device.hpp"
#include "CommandList.hpp"
#include <cmath>
#include <algorithm>
#include "Vulkan.hpp"

namespace SGF {
	namespace {
		static constexpr VkVertexInputBindingDescription LINE_VERTEX_BINDINGS[] = {
			{0, sizeof(DebugRenderer::LineVertex), VK_VERTEX_INPUT_RATE_VERTEX}
			//{1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE},
		};

		static constexpr VkVertexInputAttributeDescription LINE_VERTEX_ATTRIBUTES[] = {
			{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(DebugRenderer::LineVertex, position) }, // Position
			{1, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(DebugRenderer::LineVertex, color) }, // Vertex Color (UNORM statt sRGB)
		};

		static constexpr VkPipelineVertexInputStateCreateInfo LINE_VERTEX_INPUT_INFO = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = FLAG_NONE,
			.vertexBindingDescriptionCount = ARRAY_SIZE(LINE_VERTEX_BINDINGS),
			.pVertexBindingDescriptions = LINE_VERTEX_BINDINGS,
			.vertexAttributeDescriptionCount = ARRAY_SIZE(LINE_VERTEX_ATTRIBUTES),
			.pVertexAttributeDescriptions = LINE_VERTEX_ATTRIBUTES,
		};
	}
	
	DebugRenderer::DebugRenderer(VkRenderPass renderPass, uint32_t subpass, uint32_t initialLineCapacity)
		: vertexRingBuffer(sizeof(LineVertex) * initialLineCapacity * 2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), 
		  cameraBuffer(sizeof(glm::mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
		  pipeline(VK_NULL_HANDLE),
		  pipelineLayout(VK_NULL_HANDLE),
		  descriptorPool(VK_NULL_HANDLE),
		  descriptorSetLayout(VK_NULL_HANDLE)
	{
		lineVertices.reserve(initialLineCapacity * 2);
		const auto& device = Device::Get();

		// Create Descriptor Set Layout
		VkDescriptorSetLayoutBinding layoutBindings[] = {
			Vk::CreateDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
		};
		descriptorSetLayout = device.CreateDescriptorSetLayout(layoutBindings);

		// Create Descriptor Pool
		VkDescriptorPoolSize poolSizes[] = {
			Vk::CreateDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SGF_FRAMES_IN_FLIGHT)
		};
		descriptorPool = device.CreateDescriptorPool(SGF_FRAMES_IN_FLIGHT, poolSizes);

		// Allocate Descriptor Sets (one per frame)
		VkDescriptorSetLayout layouts[SGF_FRAMES_IN_FLIGHT];
		for (uint32_t i = 0; i < SGF_FRAMES_IN_FLIGHT; ++i) {
			layouts[i] = descriptorSetLayout;
		}
		device.AllocateDescriptorSets(descriptorPool, layouts, SGF_FRAMES_IN_FLIGHT, descriptorSets);

		// Update Descriptor Sets with Uniform Buffer info
		for (uint32_t i = 0; i < SGF_FRAMES_IN_FLIGHT; ++i) {
			VkDescriptorBufferInfo bufferInfo = {
				cameraBuffer.GetBuffer(),
				cameraBuffer.GetCurrentBufferOffset() + (i * sizeof(glm::mat4)),
				sizeof(glm::mat4)
			};
			VkWriteDescriptorSet write = Vk::CreateDescriptorWrite(
				descriptorSets[i],
				0,
				0,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				&bufferInfo,
				1
			);
			device.UpdateDescriptors(&write, 1);
		}

		// Create Pipeline Layout with Descriptor Set Layout
		pipelineLayout = device.CreatePipelineLayout(descriptorSetLayout);

		// Create Graphics Pipeline
		pipeline = device.CreateGraphicsPipeline(pipelineLayout, renderPass, subpass)
			.FragmentShader("shaders/debug_line.frag")
			.VertexShader("shaders/debug_line.vert")
			.VertexInput(LINE_VERTEX_INPUT_INFO)
			.Topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
            .DynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.DynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.Depth(false, true)
			.AddColorBlendAttachment(false, 0)
			.Build();
	}

	DebugRenderer::~DebugRenderer() {
		const auto& device = Device::Get();
		if (pipeline != VK_NULL_HANDLE) {
			device.Destroy(pipeline);
		}
		if (pipelineLayout != VK_NULL_HANDLE) {
			device.Destroy(pipelineLayout);
		}
		if (descriptorPool != VK_NULL_HANDLE) {
			device.Destroy(descriptorPool);
		}
		if (descriptorSetLayout != VK_NULL_HANDLE) {
			device.Destroy(descriptorSetLayout);
		}
	}

	void DebugRenderer::AddLine(const glm::vec3& start, const glm::vec3& end, SGF::Color::RGBA8 color) {
		lineVertices.push_back({ start, color });
		lineVertices.push_back({ end, color });
	}

	void DebugRenderer::AddBox(const glm::vec3& center, const glm::vec3& extents, SGF::Color::RGBA8 color) {
		glm::vec3 h = extents * 0.5f;
		glm::vec3 corners[8] = {
			center + glm::vec3(-h.x, -h.y, -h.z),
			center + glm::vec3( h.x, -h.y, -h.z),
			center + glm::vec3( h.x,  h.y, -h.z),
			center + glm::vec3(-h.x,  h.y, -h.z),
			center + glm::vec3(-h.x, -h.y,  h.z),
			center + glm::vec3( h.x, -h.y,  h.z),
			center + glm::vec3( h.x,  h.y,  h.z),
			center + glm::vec3(-h.x,  h.y,  h.z)
		};
		const int edges[12][2] = {
			{0,1},{1,2},{2,3},{3,0}, // bottom
			{4,5},{5,6},{6,7},{7,4}, // top
			{0,4},{1,5},{2,6},{3,7}  // sides
		};
		for (int i = 0; i < 12; ++i) {
			AddLine(corners[edges[i][0]], corners[edges[i][1]], color);
		}
	}

	void DebugRenderer::AddSphere(const glm::vec3& center, float radius, SGF::Color::RGBA8 color) {
		const int segments = 24;
		for (int circle = 0; circle < 3; ++circle) {
			for (int i = 0; i < segments; ++i) {
				float a0 = (static_cast<float>(i) / segments) * glm::two_pi<float>();
				float a1 = (static_cast<float>(i + 1) / segments) * glm::two_pi<float>();
				glm::vec3 p0, p1;
				switch (circle) {
				case 0: // XY
					p0 = center + radius * glm::vec3(glm::cos(a0), glm::sin(a0), 0.0f);
					p1 = center + radius * glm::vec3(glm::cos(a1), glm::sin(a1), 0.0f);
					break;
				case 1: // XZ
					p0 = center + radius * glm::vec3(glm::cos(a0), 0.0f, glm::sin(a0));
					p1 = center + radius * glm::vec3(glm::cos(a1), 0.0f, glm::sin(a1));
					break;
				default: // YZ
					p0 = center + radius * glm::vec3(0.0f, glm::cos(a0), glm::sin(a0));
					p1 = center + radius * glm::vec3(0.0f, glm::cos(a1), glm::sin(a1));
					break;
				}
				AddLine(p0, p1, color);
			}
		}
	}

	void DebugRenderer::AddFrustum(const glm::mat4& inverseProjection, const glm::mat4& inverseView, SGF::Color::RGBA8 color) {
		// NDC corners für Vulkan: x,y in [-1,1], z in [0,1]
		glm::vec4 ndc[8] = {
			// near plane (z = 0)
			{ -1.0f, -1.0f, 0.0f, 1.0f },
			{  1.0f, -1.0f, 0.0f, 1.0f },
			{  1.0f,  1.0f, 0.0f, 1.0f },
			{ -1.0f,  1.0f, 0.0f, 1.0f },
			// far plane (z = 1)
			{ -1.0f, -1.0f, 1.0f, 1.0f },
			{  1.0f, -1.0f, 1.0f, 1.0f },
			{  1.0f,  1.0f, 1.0f, 1.0f },
			{ -1.0f,  1.0f, 1.0f, 1.0f }
		};
		glm::vec3 world[8];
		for (int i = 0; i < 8; ++i) {
			glm::vec4 v = inverseProjection * ndc[i];
			if (v.w != 0.0f) v /= v.w;
			v = inverseView * v;
			if (v.w != 0.0f) v /= v.w;
			world[i] = glm::vec3(v);
		}
		const int edges[12][2] = {
			{0,1},{1,2},{2,3},{3,0}, // near
			{4,5},{5,6},{6,7},{7,4}, // far
			{0,4},{1,5},{2,6},{3,7}  // sides
		};
		for (int i = 0; i < 12; ++i) {
			AddLine(world[edges[i][0]], world[edges[i][1]], color);
		}
	}

	void DebugRenderer::Draw(VkCommandBuffer commandBuffer, const glm::mat4& viewProj, uint32_t viewportWidth, uint32_t viewportHeight) {
		if (lineVertices.empty() || pipeline == VK_NULL_HANDLE) {
			return;
		}

		// Write camera matrix to uniform buffer
		cameraBuffer.Write(&viewProj, sizeof(glm::mat4));

		// Write vertex data to ring buffer: 
		size_t vertexDataSize = lineVertices.size() * sizeof(LineVertex);
		if (vertexDataSize > vertexRingBuffer.GetPageSize()) {
			Log::Warn("DebugRenderer: vertex data exceeds ring buffer page size!");
			vertexRingBuffer.Resize(vertexDataSize, 0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
			return;
		}

		void* mapped = vertexRingBuffer.GetCurrentPagePointer();
		memcpy(mapped, lineVertices.data(), vertexDataSize);

		// Setup viewport and scissor
		VkViewport viewport;
		viewport.height = static_cast<float>(viewportHeight);
		viewport.width = static_cast<float>(viewportWidth);
		viewport.maxDepth = 1.0f;
		viewport.minDepth = 0.0f;
		viewport.x = 0.0f;
		viewport.y = 0.0f;

		VkRect2D scissor;
		scissor.extent.width = viewportWidth;
		scissor.extent.height = viewportHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		// Bind pipeline and descriptor sets
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[cameraBuffer.GetCurrentPageIndex()], 0, nullptr);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// Bind vertex buffer and draw
		vertexRingBuffer.BindCurrentAsVertexBuffer(commandBuffer, 0);
		vkCmdDraw(commandBuffer, static_cast<uint32_t>(lineVertices.size()), 1, 0, 0);

		vertexRingBuffer.NextPage();
		cameraBuffer.NextPage();
	}

	void DebugRenderer::Clear() {
		lineVertices.clear();
	}
}