#pragma once

#include "engine_core.h"

#include "render/VertexIndexBuffer.h"
#include "render/SVE_StagingBuffer.h"

class SceneBuffer {
private:
	struct ModelInfo {
		glm::mat4 transform;
		uint32_t vertexOffset;
		uint32_t indexOffset;
		SveModel& modelReference;
	};
	VertexIndexBuffer deviceLocalBuffer;
	SveStagingBuffer stagingBuffer;
	std::vector<ModelInfo> models;
	std::vector<VkBufferCopy> vertexRegions;
	std::vector<VkBufferCopy> indexRegions;

public:
	inline void init() {
		stagingBuffer.allocate(0xFFFF * sizeof(models[0].modelReference.vertices[0]));
		deviceLocalBuffer.allocate(0xFFFFFF, 0xFFFFFF * 2);
	}
	inline void render(VkCommandBuffer commands) {

		deviceLocalBuffer.bind(commands);
		uint32_t count = models.back().indexOffset + models.back().modelReference.indices.size();
		vkCmdDrawIndexed(commands, count, 1, 0, 0, 0);
	}
	inline bool hasChanges() {
		return vertexRegions.size() != 0 || indexRegions.size() != 0;
	}
	inline void uploadChanges(VkCommandBuffer commands) {
		assert(hasChanges());
		deviceLocalBuffer.uploadVertexData(commands, stagingBuffer, vertexRegions.size(), vertexRegions.data(), indexRegions.size(), indexRegions.data());
		vertexRegions.clear();
		indexRegions.clear();
		stagingBuffer.clear();
	}
	inline uint32_t addModel(SveModel& model) {
		VkBufferCopy vertex_copy;
		VkBufferCopy index_copy;
		vertex_copy.size = model.vertices.size() * sizeof(SveModelVertex);
		if (models.size() != 0) {
			size_t index = models.size() - 1;
			vertex_copy.dstOffset = models[index].vertexOffset + sizeof(model.vertices[0]) * models[index].modelReference.vertices.size();
			index_copy.dstOffset = models[index].indexOffset + sizeof(model.indices[0]) * models[index].modelReference.indices.size();
		}
		else {
			vertex_copy.dstOffset = 0;
			index_copy.dstOffset = 0;
		}
		vertex_copy.size = model.vertices.size() * sizeof(model.vertices[0]);
		vertex_copy.srcOffset = stagingBuffer.addToTransfer(vertex_copy.size, model.vertices.data());
		index_copy.size = model.indices.size() * sizeof(model.indices[0]);
		index_copy.srcOffset = stagingBuffer.addToTransfer(index_copy.size, model.indices.data());
		uint32_t model_index = models.size();
		models.push_back({ glm::mat4(1.f), 0, 0, model });
		shl::logDebug("added model with ", model.vertices.size(), " vertices and ", model.indices.size(), " indices");
		for (uint32_t i = 0; i < model.vertices.size(); ++i) {
			auto position = model.vertices[i].position;
			shl::logDebug("Pos ", i, " { ", position.x, ", ", position.y, ", ", position.z, " }");
		}
		for (uint32_t i = 0; i < model.indices.size(); ++i) {
			auto index = model.indices[i];
			shl::logDebug("index: ", i, " value: ", index);
		}
		vertexRegions.push_back(vertex_copy);
		indexRegions.push_back(index_copy);
		shl::logInfo("Vertex copy created: { dstOffset: ", vertex_copy.dstOffset, ", srcOffset: ", vertex_copy.srcOffset, ", size: ", vertex_copy.size, " }");
		shl::logInfo("Index copy created: { dstOffset: ", index_copy.dstOffset, ", srcOffset: ", index_copy.srcOffset, ", size: ", index_copy.size, " }");
		return model_index;
	}
};