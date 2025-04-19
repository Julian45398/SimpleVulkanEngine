#include "SVE_GeometryRenderer.h"
#include "SVE_Backend.h"
#include "SVE_RenderPipeline.h"

struct GeometryVertex {
	glm::vec3 position;
	uint32_t colorIndex;
};

constexpr VkVertexInputBindingDescription GEOMETRY_VERTEX_BINDINGS[] = {
	{0, sizeof(GeometryVertex), VK_VERTEX_INPUT_RATE_VERTEX}
	//{1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE}
};
constexpr VkVertexInputAttributeDescription GEOMETRY_VERTEX_ATTRIBUTES[] = {
	{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GeometryVertex, position)},
	{1, 0, VK_FORMAT_R32_UINT, offsetof(GeometryVertex, colorIndex)}
};

constexpr VkPipelineVertexInputStateCreateInfo GEOMETRY_VERTEX_INPUT = {
	VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, VKL_FLAG_NONE,
	sizeof(GEOMETRY_VERTEX_BINDINGS), GEOMETRY_VERTEX_BINDINGS,
	sizeof(GEOMETRY_VERTEX_ATTRIBUTES), GEOMETRY_VERTEX_ATTRIBUTES
};

SveGeometryRenderer::SveGeometryRenderer(const VkDescriptorSetLayout uniformLayout) {
	pipelineLayout = SVE::createPipelineLayout(1, &uniformLayout);
	pipeline = SveRenderPipelineBuilder("shaders/geometry.vert", "shaders/geometry.frag", pipelineLayout, GEOMETRY_VERTEX_INPUT).setPolygonMode(VK_POLYGON_MODE_LINE)
		.setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST).build();
}

SveGeometryRenderer::~SveGeometryRenderer() {
	SVE::destroyPipeline(pipeline);
	SVE::destroyPipelineLayout(pipelineLayout);
	SVE::destroyBuffer(vertexBuffer);
}

void SveGeometryRenderer::draw(VkCommandBuffer commands, VkDescriptorSet uniformSet) {
	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &uniformSet, 0, nullptr);
	vkCmdBindVertexBuffers(commands, 0, 1, &vertexBuffer, { nullptr });
	vkCmdBindIndexBuffer(commands, vertexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commands, indexCount, 1, 0, 0, 0);
}
