#pragma once


#include "core.h"

#include "render/SVE_ImageBuffer.h"
#include "render/SVE_StagingBuffer.h"
#include "render/SVE_UniformBuffer.h"
#include "render/Camera.h"
#include "SVE_Model.h"

class SveModelBuffer {
private:
	SveVertexBuffer<SveModelVertex> vertexBuffer;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptor;
	SveImageBuffer image;
	VkSampler sampler;
	//VkDeviceMemory imageMemory;
	SveModel& modelRef;
public:
	SveModelBuffer(SveModel& model, VkDescriptorSetLayout descriptorLayout) : modelRef(model) {
		//images.resize(model.images.size());
		//imageViews.resize(model.images.size());
		//descriptors.resize(model.images.size());
		sampler = vkl::createSampler(SVE::getDevice(), VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);
		image.allocate(model.images[0].width, model.images[0].height, 1, 1);
		vertexBuffer.allocate((uint32_t)model.vertices.size(), (uint32_t)model.indices.size());

		VkCommandPool pool = SVE::createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		VkCommandBuffer commands = SVE::createCommandBuffer(pool);
		VkFence fence = SVE::createFence();
		//VkBufferImageCopy region = { 0, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {}, {model.images[0].width, model.images[0].height, 1}};
		
		VkBufferCopy vertex_copy;
		vertex_copy.size = model.vertices.size() * sizeof(SveModelVertex);
		vertex_copy.srcOffset = 0;
		vertex_copy.dstOffset = 0;

		VkBufferCopy index_copy;
		index_copy.size = model.indices.size() * sizeof(uint32_t);
		index_copy.srcOffset = vertex_copy.srcOffset + vertex_copy.size;
		index_copy.dstOffset = 0;

		VkBufferImageCopy image_copy;
		image_copy.bufferImageHeight = 0;
		image_copy.bufferRowLength = 0;
		image_copy.imageExtent = { model.images[0].width, model.images[0].height, 1 };
		image_copy.imageOffset = { 0, 0, 0 };
		image_copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		image_copy.bufferOffset = index_copy.srcOffset + index_copy.size;

		VkDeviceSize staging_buf_size = image_copy.bufferOffset + model.images[0].pixels.size();
		VkBuffer buffer = SVE::createBuffer(staging_buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		VkDeviceMemory memory = SVE::allocateForBuffer(buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		void* mapped = SVE::mapMemory(memory);
		memcpy((uint8_t*)mapped + vertex_copy.srcOffset, model.vertices.data(), vertex_copy.size);
		memcpy((uint8_t*)mapped + index_copy.srcOffset, model.indices.data(), index_copy.size);
		memcpy((uint8_t*)mapped + image_copy.bufferOffset, model.images[0].pixels.data(), model.images[0].pixels.size());

		vkl::beginCommandBuffer(commands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		image.uploadChanges(commands, buffer, 1, &image_copy, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
		vertexBuffer.uploadVertexData(commands, buffer, 1, &vertex_copy, 1, &index_copy);

		vkl::endCommandBuffer(commands);
		SVE::submitCommands(commands, fence);

		VkDescriptorPoolSize pool_sizes[] = { vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)model.images.size()) };
		descriptorPool = vkl::createDescriptorPool(SVE::getDevice(), 1, ARRAY_SIZE(pool_sizes), pool_sizes);
		descriptor = vkl::allocateDescriptorSet(SVE::getDevice(), descriptorPool, 1, &descriptorLayout);

		VkDescriptorImageInfo image_info = { sampler, image.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		VkWriteDescriptorSet writes[] = {
			vkl::createDescriptorWrite(descriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 0, 1, &image_info)
		};
		vkUpdateDescriptorSets(SVE::getDevice(), ARRAY_SIZE(writes), writes, 0, nullptr);

		SVE::waitForFence(fence);
	}
	inline void bindAndDraw(VkCommandBuffer commands, VkPipelineLayout layout) {
		//shl::logDebug("drawing model");
		vertexBuffer.bind(commands);
		shl::logDebug("ImageView address: ", image.getImageView());
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &descriptor, 0, nullptr);
		vkCmdDrawIndexed(commands, (uint32_t)modelRef.indices.size(), 1, 0, 0, 0);
	}
};

struct Mesh {
	std::vector<SveModelVertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<glm::mat4> instanceTransforms;
	size_t imageIndex;
};

struct Image {
	uint32_t width;
	uint32_t height;
	std::vector<uint8_t> pixels;
};

struct Model {
	//std::vector<Instance> instances;
	std::vector<Mesh> meshes;
	std::vector<Image> images;
};

struct ModelInformation {
	uint32_t indexOffset;
	uint32_t bertexOffset;
	uint32_t indexCount;
};


inline constexpr VkVertexInputBindingDescription MODEL_VERTEX_BINDINGS[] = {
	{0, sizeof(SveModelVertex), VK_VERTEX_INPUT_RATE_VERTEX},
	{0, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE}
};
inline constexpr VkVertexInputAttributeDescription MODEL_VERTEX_ATTRIBUTES[] = {
	{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SveModelVertex, position)},
	{1, 0, VK_FORMAT_R32_UINT, offsetof(SveModelVertex, imageIndex)},
	{2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SveModelVertex, normal)},
	{3, 0, VK_FORMAT_R32_UINT, offsetof(SveModelVertex, padding2)},
	{4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SveModelVertex, uvCoord)},
	{5, 0, VK_FORMAT_R32_UINT, offsetof(SveModelVertex, padding3)},
	{6, 0, VK_FORMAT_R32_UINT, offsetof(SveModelVertex, padding4)},
	{7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0}, 
	{8, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)},
	{9, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 2},
	{10, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 3}
};
inline const VkPipelineVertexInputStateCreateInfo MODEL_VERTEX_INPUT_INFO = vkl::createPipelineVertexInputStateInfo(ARRAY_SIZE(MODEL_VERTEX_BINDINGS), MODEL_VERTEX_BINDINGS,
	ARRAY_SIZE(MODEL_VERTEX_ATTRIBUTES), MODEL_VERTEX_ATTRIBUTES);
	
class SceneRenderer {
private:
	std::vector<Model> models;

	// Vulkan stuff:
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	// Buffers:
	static constexpr uint32_t MAX_INSTANCE_COUNT = 2048;
	static constexpr VkDeviceSize PAGE_SIZE = 2 << 27;
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkBuffer instanceBuffer = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexDeviceMemory;

	// Images:
	VkImage images = VK_NULL_HANDLE;
	VkDeviceMemory imageMemory = VK_NULL_HANDLE;

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;


public:
	inline SceneRenderer() {
		//VkPhysicalDeviceMemoryProperties mem_properties = vkl::getPhysicalDeviceMemoryProperties(SVE::getPhysicalDevice());
		//vertexDeviceMemory = vkl::allocateMemory(SVE::getDevice(), SVE::getPhysicalDevice(), )
		vertexBuffer = vkl::createBuffer(SVE::getDevice(), PAGE_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, SVE::getGraphicsFamily());
		vertexDeviceMemory = vkl::allocateForBuffer(SVE::getDevice(), SVE::getPhysicalDevice(), vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		//images = vkl::createImage2D(SVE::getDevice(), VK_FORMAT_R8G8B8A8_SRGB, );
		// TODO: get memory type bits:
		VkMemoryRequirements mem_req = {PAGE_SIZE * 2, 2 << 16, 0xFFFF};
		imageMemory = vkl::allocateMemory(SVE::getDevice(), SVE::getPhysicalDevice(), mem_req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}
	inline void draw(VkCommandBuffer commands) {
		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		VkBuffer buffers[] = { vertexBuffer, vertexBuffer };
		VkDeviceSize offsets[] = { sizeof(glm::mat4) * MAX_INSTANCE_COUNT, 0};
		static_assert(ARRAY_SIZE(buffers) == ARRAY_SIZE(offsets));
		vkCmdBindVertexBuffers(commands, 0, ARRAY_SIZE(buffers), buffers, offsets);
		int32_t vertex_offset = 0;
		uint32_t instance_offset = 0;
		constexpr uint32_t max_index_offset = PAGE_SIZE / sizeof(uint32_t);
		uint32_t index_offset = max_index_offset;
		for (size_t i = 0; i < models.size(); ++i) {
			Model& model = models[i];
			for (size_t j = 0; j < model.meshes.size(); ++j) {
				Mesh& mesh = model.meshes[j];
				index_offset -= (uint32_t)mesh.indices.size();
				//vkCmdPushConstants(commands, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mesh.imageIndex), &mesh.imageIndex);
				vkCmdDrawIndexed(commands, (uint32_t)mesh.indices.size(), (uint32_t)mesh.instanceTransforms.size(), index_offset, vertex_offset, instance_offset);
				vertex_offset += (uint32_t)mesh.indices.size();
				instance_offset += (uint32_t)mesh.instanceTransforms.size();
			}

		}
	}
};

class SveSceneRenderer {
	inline static const char* VERTEX_SHADER_FILE = "resources/shaders/model.vert";
	inline static const char* FRAGMENT_SHADER_FILE = "resources/shaders/model.frag";

	inline static const VkDescriptorSetLayoutBinding UNIFORM_BINDING = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
	inline static const VkDescriptorSetLayoutBinding IMAGE_BINDING = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

	SveUniformBuffer<glm::mat4> uniformBuffer;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout uniformDescriptorLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout imageDescriptorLayout = VK_NULL_HANDLE;
	VkSampler imageSampler = VK_NULL_HANDLE;
	SveRenderPipeline renderPipeline;
	std::vector<VkDescriptorSet> uniformSets;
	std::vector<SveModelBuffer> modelBuffers;
	//uint32_t modelCount;
public:
	inline void addModel(SveModel& model) {
		modelBuffers.emplace_back(model, imageDescriptorLayout);
	}
	inline SveSceneRenderer() : 
		descriptorPool(vkl::createDescriptorPool(SVE::getDevice(), SVE::getImageCount() + 1000, 1, &vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SVE::getImageCount()))),
		uniformDescriptorLayout(vkl::createDescriptorSetLayout(SVE::getDevice(), 1, &UNIFORM_BINDING)), imageDescriptorLayout(vkl::createDescriptorSetLayout(SVE::getDevice(), 1, &IMAGE_BINDING)),
		imageSampler(vkl::createSampler(SVE::getDevice(), VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT)),
		renderPipeline(VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE, SVE_MODEL_VERTEX_INPUT_INFO, 2, &uniformDescriptorLayout)
	{

		modelBuffers.reserve(512);
		VkDescriptorPoolSize pool_sizes[] = {
			vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SVE::getImageCount())
			//vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
		};
		descriptorPool = vkl::createDescriptorPool(SVE::getDevice(), SVE::getImageCount() + 1000, ARRAY_SIZE(pool_sizes), pool_sizes);
		//uniformDescriptorLayout = vkl::createDescriptorSetLayout();
		//imageDescriptorLayout = ;
		uniformSets.resize(SVE::getImageCount());
		for (uint32_t i = 0; i < SVE::getImageCount(); ++i) {
			uniformSets[i] = vkl::allocateDescriptorSet(SVE::getDevice(), descriptorPool, 1, &uniformDescriptorLayout);
			VkDescriptorBufferInfo buf_info = { uniformBuffer.getBuffer(i), 0, VK_WHOLE_SIZE};
			//VkDescriptorImageInfo image_info = { imageSampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
			VkWriteDescriptorSet writes[] = {
				vkl::createDescriptorWrite(uniformSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, 1, &buf_info)
			};
			vkUpdateDescriptorSets(SVE::getDevice(), ARRAY_SIZE(writes), writes, 0, nullptr);
		}
	}

	inline void renderScene(VkCommandBuffer commands, const Camera& camera) {
		renderPipeline.bindPipeline(commands);
		uniformBuffer.update(camera.getViewProj());
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPipeline.getLayout(), 0, 1, &uniformSets[SVE::getImageIndex()], 0, nullptr);
		shl::logInfo("Drawing ", modelBuffers.size(), " models");
		for (size_t i = 0; i < modelBuffers.size(); ++i) {
			shl::logInfo("Drawing model ", i);
			modelBuffers[i].bindAndDraw(commands, renderPipeline.getLayout());
			if (modelBuffers.size() > 1) {
				break;
			}
		}
		if (modelBuffers.size() > 1) {
			//modelCount = 1;
			//shl::logFatal("Hello world");
		}
	}
};