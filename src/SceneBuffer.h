#pragma once

#include "core.h"

#include "render/SVE_VertexBuffer.h"
#include "render/SVE_StagingBuffer.h"
#include "render/SVE_ImageBuffer.h"
#include "render/SVE_Descriptor.h"
#include "SVE_Model.h"

class SceneBuffer {
public:
	struct ModelInfo {
		glm::mat4 transform;
		uint32_t vertexOffset;
		uint32_t indexOffset;
		SveModel& modelReference;
	};
	SveVertexBuffer deviceLocalBuffer;
	SveStagingBuffer stagingBuffer;
	SveImageBuffer imageBuffer;
	std::vector<ModelInfo> models;
	std::vector<VkBufferCopy> vertexRegions;
	std::vector<VkBufferCopy> indexRegions;
	std::vector<VkBufferImageCopy> imageRegions;

public:
	inline SceneBuffer() {
		stagingBuffer.allocate(0xFFFF * sizeof(models[0].modelReference.vertices[0]));
		deviceLocalBuffer.allocate(0xFFFFFF, 0xFFFFFF * 2);
		imageBuffer.allocate(256, 256, 256);
	}
	inline void render(VkCommandBuffer commands) {
		deviceLocalBuffer.bind(commands);
		uint32_t count = models.back().indexOffset + models.back().modelReference.indices.size();
		vkCmdDrawIndexed(commands, count, 1, 0, 0, 0);
	}
	inline bool hasChanges() {
		return vertexRegions.size() != 0 || indexRegions.size() != 0 || imageRegions.size();
	}
	inline void uploadChanges(VkCommandBuffer commands) {
		assert(hasChanges());
		deviceLocalBuffer.uploadVertexData(commands, stagingBuffer, vertexRegions.size(), vertexRegions.data(), indexRegions.size(), indexRegions.data());
		imageBuffer.uploadChanges(commands, stagingBuffer, imageRegions.size(), imageRegions.data(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
		vertexRegions.clear();
		indexRegions.clear();
		imageRegions.clear();
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
		VkBufferImageCopy image_copy;
		image_copy.bufferImageHeight = 0;
		image_copy.bufferRowLength = 0;
		image_copy.imageExtent = { model.imageWidth, model.imageHeight , 1 };
		image_copy.imageOffset = { 0, 0, 0 };
		image_copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		image_copy.bufferOffset = stagingBuffer.addToTransfer(model.pixelData.size() * sizeof(model.pixelData[0]), model.pixelData.data());
		models.push_back({ glm::mat4(1.f), 0, 0, model });
		shl::logDebug("added model with ", model.vertices.size(), " vertices and ", model.indices.size(), " indices");
		shl::logDebug("image width: ", model.imageWidth, ", height: ", model.imageHeight, "pixel count: ", model.pixelData.size());
		/*
		for (uint32_t i = 0; i < model.vertices.size(); ++i) {
			auto position = model.vertices[i].position;
			shl::logDebug("Pos ", i, " { ", position.x, ", ", position.y, ", ", position.z, " }");
		}
		for (uint32_t i = 0; i < model.indices.size(); ++i) {
			auto index = model.indices[i];
			shl::logDebug("index: ", i, " value: ", index);
		}
		*/
		vertexRegions.push_back(vertex_copy);
		indexRegions.push_back(index_copy);
		imageRegions.push_back(image_copy);
		shl::logInfo("Vertex copy created: { dstOffset: ", vertex_copy.dstOffset, ", srcOffset: ", vertex_copy.srcOffset, ", size: ", vertex_copy.size, " }");
		shl::logInfo("Index copy created: { dstOffset: ", index_copy.dstOffset, ", srcOffset: ", index_copy.srcOffset, ", size: ", index_copy.size, " }");
		return model_index;
	}
};