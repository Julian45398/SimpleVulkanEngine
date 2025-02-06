#pragma once


#include "core.h"

#include "render/SVE_ImageBuffer.h"
#include "render/SVE_StagingBuffer.h"
#include "render/SVE_UniformBuffer.h"
#include "render/Camera.h"
#include "SVE_Model.h"
#include <unordered_map>

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
		vertexBuffer.bind(commands);
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &descriptor, 0, nullptr);
		vkCmdDrawIndexed(commands, (uint32_t)modelRef.indices.size(), 1, 0, 0, 0);
	}
};

struct Mesh {
	std::vector<SveModelVertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<glm::mat4> instanceTransforms;
	uint32_t imageIndex;
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
	{1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE}
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
	

struct TextureImage {
	VkImage image;
	VkImageView view;
};
class ImageMemoryAllocator {
public:
	static const VkDeviceSize REGION_SIZE = 16384 * 8192;
private:
	struct MemRegion {
		uint32_t size;
		uint32_t offset;
		uint32_t regionIndex;
	};
	struct ImageRegion {
		TextureImage image;
		MemRegion region;
	};
	std::vector<VkDeviceMemory> allocatedRegions;
	std::vector<MemRegion> freeRegions;
	std::vector<ImageRegion> imageRegions;
public:
	inline const TextureImage createImage(uint32_t width, uint32_t height) {
		TextureImage texture;
		MemRegion textureRegion;
		texture.image = SVE::createImage2D(width, height, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		auto memreq = SVE::getImageMemoryRequirements(texture.image);
		if (memreq.size > REGION_SIZE) {
			shl::logFatal("image memory requirement exceeds max byts size of: ", REGION_SIZE);
		}
		bool region_found = false;
		for (size_t i = 0; i < freeRegions.size(); ++i) {
			MemRegion region = freeRegions[i];
			if (memreq.size <= region.size) {
				region_found = true;
				region.size = (uint32_t)memreq.size;
				SVE::bindImageMemory(texture.image, allocatedRegions[region.regionIndex], region.offset);
				textureRegion = region;
				if (memreq.size != region.size) {
					freeRegions[i].offset += (uint32_t)memreq.size;
					freeRegions[i].size -= (uint32_t)memreq.size;
				}
				else {
					freeRegions.erase(freeRegions.begin() + i);
				}
				break;
			}
		}
		if (!region_found) {
			uint32_t size = (uint32_t)memreq.size;
			memreq.size = REGION_SIZE;
			VkDeviceMemory memory = SVE::allocateMemory(memreq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			allocatedRegions.push_back(memory);
			MemRegion region = { size, 0, (uint32_t)allocatedRegions.size() - 1 };
			SVE::bindImageMemory(texture.image, memory, region.offset);
			textureRegion = region;
			region.offset = size;
			region.size = (uint32_t)REGION_SIZE - size;
			freeRegions.push_back(region);
		}
		texture.view = SVE::createImageView2D(texture.image, VK_FORMAT_R8G8B8A8_SRGB);
		imageRegions.push_back({texture, textureRegion});

		return imageRegions.back().image;
	}

	inline void destroyImage(const TextureImage& texture) {
		MemRegion region;
		for (uint32_t i = 0; i < imageRegions.size(); ++i) {
			if (imageRegions[i].image.image ==  texture.image) {
				region = imageRegions[i].region;
				imageRegions.erase(imageRegions.begin() + i);
				break;
			}
		}
		freeRegions.push_back(region);
		SVE::destroyImage(texture.image);
		SVE::destroyImageView(texture.view);
	}

	inline void defragmentMemory() {
		shl::logWarn("Defragmentation of image allocator memory not implemented yet!");
	}

	inline ~ImageMemoryAllocator() {
		for (const auto& region : imageRegions) {
			SVE::destroyImage(region.image.image);
			SVE::destroyImageView(region.image.view);
		}
		for (size_t i = 0; i < allocatedRegions.size(); ++i) {
			SVE::freeMemory(allocatedRegions[i]);
		}
	}
};

class SceneRenderer {
private:
	// Models:
	std::vector<Model> models;

	// Buffers:
	static constexpr uint32_t MAX_INSTANCE_COUNT = 2048;
	static constexpr uint32_t MAX_TEXTURE_COUNT = 128;
	static constexpr uint32_t VERTEX_START_BYTE_OFFSET = MAX_INSTANCE_COUNT * sizeof(glm::mat4);
	static constexpr VkDeviceSize PAGE_SIZE = 2 << 27;
	static constexpr uint32_t MAX_VERTICES = (PAGE_SIZE - MAX_INSTANCE_COUNT * sizeof(glm::mat4)) / sizeof(SveModelVertex);
	static constexpr char VERTEX_SHADER_FILE[] = "resources/shaders/model_2.vert";
	static constexpr char FRAGMENT_SHADER_FILE[] = "resources/shaders/model_2.frag";
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexDeviceMemory;

	// Images:
	VkSampler sampler = VK_NULL_HANDLE;
	std::vector<TextureImage> textures;
	ImageMemoryAllocator textureAllocator;

	// UniformBuffer:
	SveUniformBuffer<glm::mat4> uniformBuffer;

	// TransferResources:
	VkFence fence = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
	uint8_t* stagingMapped = nullptr;

	// Descriptors:
	struct Descriptor {
		VkDescriptorSet set;
		bool invalidated;
	};
	VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
	//VkDescriptorSetLayout textureLayout = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::vector<Descriptor> descriptorSets;
	//VkDescriptorSet textureSet = VK_NULL_HANDLE;
	// Pipeline:
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
	VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
	VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	uint32_t transformCount = 0;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

public:
	inline SceneRenderer() {
		// Vertex and Index Buffers:
		vertexBuffer = vkl::createBuffer(SVE::getDevice(), PAGE_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, SVE::getGraphicsFamily());
		vertexDeviceMemory = vkl::allocateForBuffer(SVE::getDevice(), SVE::getPhysicalDevice(), vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// Sampler:
		sampler = vkl::createSampler(SVE::getDevice(), VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

		// Descriptor Pool:
		{
			VkDescriptorPoolSize pool_sizes[] = {
				vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SVE::getImageCount()),
				vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, SVE::getImageCount()),
				vkl::createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, SVE::getImageCount() * MAX_TEXTURE_COUNT),
			};
			descriptorPool = vkl::createDescriptorPool(SVE::getDevice(), SVE::getImageCount() + 1, ARRAY_SIZE(pool_sizes), pool_sizes);
		}

		// Descriptor Set Layout:
		{
			VkDescriptorSetLayoutBinding uniform_bindings[] = {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
				{1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
				{2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_TEXTURE_COUNT, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
			};
			descriptorLayout = vkl::createDescriptorSetLayout(SVE::getDevice(), ARRAY_SIZE(uniform_bindings), uniform_bindings);
		}

		// DescriptorSets:
		{
			// Allocating Descriptor Sets:
			descriptorSets.resize(SVE::getImageCount());
			for (auto& set : descriptorSets) {
				set.set = vkl::allocateDescriptorSet(SVE::getDevice(), descriptorPool, 1, &descriptorLayout);
				// descriptors only invalidated after model was added
				set.invalidated = false;
			}

			// Writing Descriptor Sets:
			std::vector<VkWriteDescriptorSet> descriptor_writes(descriptorSets.size() * 2);
			VkDescriptorImageInfo sampler_info = { sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
			VkDescriptorBufferInfo uniform_info = {VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
			for (size_t i = 0; i < descriptorSets.size(); ++i) {
				uniform_info.buffer = uniformBuffer.getBuffer((uint32_t)i);
				VkWriteDescriptorSet writes[] = {
					vkl::createDescriptorWrite(descriptorSets[i].set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 0, 1, &uniform_info),
					vkl::createDescriptorWrite(descriptorSets[i].set, VK_DESCRIPTOR_TYPE_SAMPLER, 1, 0, 1, &sampler_info)
				};
				vkUpdateDescriptorSets(SVE::getDevice(), ARRAY_SIZE(writes), writes, 0, nullptr);
			}
		}

		// Pipeline:
		{
			// Layout:
			VkPushConstantRange push_constant_ranges[] = {
				{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Mesh::imageIndex)}
			};
			VkDescriptorSetLayout descriptor_layouts[] = {
				descriptorLayout
			};
			pipelineLayout = vkl::createPipelineLayout(SVE::getDevice(), ARRAY_SIZE(descriptor_layouts), descriptor_layouts, ARRAY_SIZE(push_constant_ranges), push_constant_ranges);

			// Pipeline:
			createPipeline();
		}

		// Transfer Objects:
		commandPool = SVE::createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		commandBuffer = SVE::createCommandBuffer(commandPool);
		fence = SVE::createFence();
		VkResult status = vkGetFenceStatus(SVE::getDevice(), fence);
		if (status == VK_SUCCESS) {
			shl::logInfo("FENCE is signaled");
		}
		else if (status == VK_NOT_READY) {
			shl::logInfo("FENCE is not signaled");
		}
	}

	inline ~SceneRenderer() {
		SVE::waitForFence(fence);
		SVE::destroyFence(fence);
		SVE::destroyCommandPool(commandPool);
		if (stagingMapped != nullptr) {
			SVE::freeMemory(stagingMemory);
			SVE::destroyBuffer(stagingBuffer);
		}
		SVE::destroyPipelineLayout(pipelineLayout);
		SVE::destroyPipeline(pipeline);

		SVE::destroyDescriptorSetLayout(descriptorLayout);
		SVE::destroyDescriptorPool(descriptorPool);

		SVE::destroyBuffer(vertexBuffer);
		SVE::freeMemory(vertexDeviceMemory);
	}

	inline void addModel(const Model& model) {
		shl::logInfo("adding Model");
		models.push_back(model);
		shl::logInfo("pushed back");
		size_t total_size = 0;

		// get total allocation size:
		for (size_t i = 0; i < model.images.size(); ++i) {
			shl::logInfo("pixel size: ", model.images[i].pixels.size());
			total_size += model.images[i].pixels.size();
		}
		for (size_t i = 0; i < model.meshes.size(); ++i) {
			total_size += model.meshes[i].indices.size() * sizeof(uint32_t);
			total_size += model.meshes[i].vertices.size() * sizeof(SveModelVertex);
			total_size += model.meshes[i].instanceTransforms.size() * sizeof(glm::mat4);
			shl::logInfo("instances: ", model.meshes[i].instanceTransforms.size());
			shl::logInfo("indices: ", model.meshes[i].indices.size());
			shl::logInfo("vertices: ", model.meshes[i].vertices.size());
		}
		shl::logDebug("total size: ", total_size);

		stagingBuffer = SVE::createBuffer(total_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		stagingMemory = SVE::allocateForBuffer(stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingMapped = (uint8_t*)SVE::mapMemory(stagingMemory, VK_WHOLE_SIZE, 0);

		vkl::resetCommandPool(SVE::getDevice(), commandPool);
		vkl::beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		
		size_t offset = 0;
		// Copy image data:
		for (size_t i = 0; i < model.images.size(); ++i) {
			shl::logDebug("copy image...");
			total_size += model.images[i].pixels.size();
			textures.push_back(textureAllocator.createImage(model.images[i].width, model.images[i].height));
			auto& texture = textures.back();

			VkBufferImageCopy region{};
			region.bufferOffset = offset;
			region.imageSubresource = {};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = { model.images[i].width, model.images[i].height, 1 };

			size_t mem_size = model.images[i].pixels.size();
			memcpy(stagingMapped + offset, model.images[i].pixels.data(), mem_size);
			offset += mem_size;

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.image = texture.image;
			barrier.srcQueueFamilyIndex = SVE::getGraphicsFamily();
			barrier.dstQueueFamilyIndex = SVE::getGraphicsFamily();
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			barrier.srcAccessMask = barrier.dstAccessMask;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.oldLayout = barrier.newLayout;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
		}
		shl::logDebug("images copied...");

		// Copy mesh data:
		for (size_t i = 0; i < model.meshes.size(); ++i) {
			const Mesh& mesh = model.meshes[i];

			// Instance transforms:
			VkBufferCopy instance_region;
			instance_region.dstOffset = transformCount * sizeof(glm::mat4);
			instance_region.size = mesh.instanceTransforms.size() * sizeof(glm::mat4);
			instance_region.srcOffset = offset;
			transformCount += (uint32_t)mesh.instanceTransforms.size();
			shl::logDebug("instance region offset: ", offset, " size: ", instance_region.size, " dst offset: ", instance_region.dstOffset);
			memcpy(stagingMapped + offset, mesh.instanceTransforms.data(), instance_region.size);
			shl::logDebug("instances copied...");
			offset += instance_region.size;
			
			// Indices:
			VkBufferCopy index_region;
			index_region.size = mesh.indices.size() * sizeof(uint32_t);
			index_region.dstOffset = PAGE_SIZE - indexCount * sizeof(uint32_t) - index_region.size;
			index_region.srcOffset = offset;
			indexCount += (uint32_t)mesh.indices.size();
			shl::logDebug("instance region offset: ", offset, " size: ", index_region.size, " dst offset: ", index_region.dstOffset);
			memcpy(stagingMapped + offset, mesh.indices.data(), index_region.size);
			shl::logDebug("indices copied...");
			offset += index_region.size;

			// Vertices:
			VkBufferCopy vertex_region;
			vertex_region.dstOffset = VERTEX_START_BYTE_OFFSET + vertexCount * sizeof(SveModelVertex);
			vertex_region.size = mesh.vertices.size() * sizeof(SveModelVertex);
			vertex_region.srcOffset = offset;
			vertexCount += (uint32_t)mesh.vertices.size();
			shl::logDebug("vertex region offset: ", offset, " size: ", vertex_region.size, " dst offset: ", vertex_region.dstOffset);
			memcpy(stagingMapped + offset, mesh.vertices.data(), vertex_region.size);
			offset += vertex_region.size;
			shl::logDebug("vertices copied...");

			VkBufferCopy regions[] = {
				instance_region, index_region, vertex_region
			};
			vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, ARRAY_SIZE(regions), regions);
		}

		// Submitting Commands:
		vkl::endCommandBuffer(commandBuffer);
		vkl::submitCommands(SVE::getGraphicsQueue(), commandBuffer, fence);
		VkResult status = vkGetFenceStatus(SVE::getDevice(), fence);
		if (status == VK_SUCCESS) {
			shl::logInfo("FENCE is signaled");
		}
		else if (status == VK_NOT_READY) {
			shl::logInfo("FENCE is not signaled");
		}
		//shl::logFatal("test");

		shl::logInfo("Model added");
		
	}

	inline void draw(VkCommandBuffer commands, const glm::mat4& viewMatrix) {
		//shl::logDebug("drawing scene...");
		uniformBuffer.update(viewMatrix);
		checkTransferStatus();
		updateTextureDescriptors();

		vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		VkViewport viewport = { (float)SVE::getViewportOffsetX(), (float)SVE::getViewportOffsetY(), (float)SVE::getViewportWidth(), (float)SVE::getViewportHeight(), 0.0f, 1.0f};
		VkRect2D scissor = { {SVE::getViewportOffsetX(), SVE::getViewportOffsetY()}, {SVE::getViewportWidth(), SVE::getViewportHeight()}};
		vkCmdSetViewport(commands, 0, 1, &viewport);
		vkCmdSetScissor(commands, 0, 1, &scissor);
	
		vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[SVE::getImageIndex()].set, 0, nullptr);
		VkBuffer buffers[] = { vertexBuffer, vertexBuffer };
		VkDeviceSize offsets[] = { VERTEX_START_BYTE_OFFSET, 0};
		static_assert(ARRAY_SIZE(buffers) == ARRAY_SIZE(offsets));
		vkCmdBindVertexBuffers(commands, 0, ARRAY_SIZE(buffers), buffers, offsets);
		vkCmdBindIndexBuffer(commands, vertexBuffer, 0, VK_INDEX_TYPE_UINT32);
		int32_t vertex_offset = 0;
		uint32_t instance_offset = 0;
		constexpr uint32_t max_index_offset = PAGE_SIZE / sizeof(uint32_t);
		uint32_t index_offset = max_index_offset;
		for (size_t i = 0; i < models.size(); ++i) {
			Model& model = models[i];
			for (size_t j = 0; j < model.meshes.size(); ++j) {
				Mesh& mesh = model.meshes[j];
				index_offset -= (uint32_t)mesh.indices.size();
				vkCmdPushConstants(commands, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mesh.imageIndex), &mesh.imageIndex);
				vkCmdDrawIndexed(commands, (uint32_t)mesh.indices.size(), (uint32_t)mesh.instanceTransforms.size(), index_offset, vertex_offset, instance_offset);
				//shl::logInfo("index count: ", mesh.indices.size(), " index offset: ", index_offset, " vertex offset: ", vertex_offset, " instances: ", mesh.instanceTransforms.size(), " instance_offset: ", instance_offset);
				vertex_offset += (uint32_t)mesh.indices.size();
				instance_offset += (uint32_t)mesh.instanceTransforms.size();
			}

		}
	}

	inline void changePipelineSettings(VkPolygonMode mode, VkCullModeFlags cull) {
		polygonMode = mode;
		cullMode = cull;
		vkl::destroyPipeline(SVE::getDevice(), pipeline);
		createPipeline();
	}
private:
	inline void invalidateDescriptors() {
		for (size_t i = 0; i < descriptorSets.size(); ++i) {
			descriptorSets[i].invalidated = true;
		}
	}

	inline void checkTransferStatus() {
		if (stagingMapped != nullptr) {
			if (vkGetFenceStatus(SVE::getDevice(), fence) == VK_SUCCESS) {
				invalidateDescriptors();
				SVE::waitForFence(fence);
				SVE::destroyBuffer(stagingBuffer);
				SVE::freeMemory(stagingMemory);
				stagingMapped = nullptr;
			}
		}
	}

	inline void createPipeline() {
		auto vertex_data = util::readBinaryFile(VERTEX_SHADER_FILE);
		auto fragment_data = util::readBinaryFile(FRAGMENT_SHADER_FILE);
		VkShaderModule vertex_module = vkl::createShaderModule(SVE::getDevice(), vertex_data.size(), (const uint32_t*)vertex_data.data());
		VkShaderModule fragment_module = vkl::createShaderModule(SVE::getDevice(), fragment_data.size(), (const uint32_t*)fragment_data.data());
		VkPipelineShaderStageCreateInfo stages[] = {
			vkl::createPipelineShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, vertex_module),
			vkl::createPipelineShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_module)
		};
		VkPipelineInputAssemblyStateCreateInfo assembly_info = vkl::createPipelineInputAssemblyInfo(primitiveTopology, VK_FALSE);
		VkViewport viewport = { (float)SVE::getViewportOffsetX(), (float)SVE::getViewportOffsetY(), (float)SVE::getViewportWidth(), (float)SVE::getViewportHeight(), 0.0f, 1.0f };
		VkRect2D scissor = { {SVE::getViewportOffsetX(), SVE::getViewportOffsetY()}, {SVE::getViewportWidth(), SVE::getViewportHeight()} };
		VkPipelineViewportStateCreateInfo viewport_info = vkl::createPipelineViewportStateInfo(1, nullptr, 1, nullptr);
		VkPipelineRasterizationStateCreateInfo rasterization = vkl::createPipelineRasterizationStateInfo(VK_FALSE, VK_FALSE, polygonMode, cullMode, VK_FRONT_FACE_CLOCKWISE, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);
		VkPipelineMultisampleStateCreateInfo multisample = vkl::createPipelineMultisampleStateInfo(VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0.0f, nullptr, VK_FALSE, VK_FALSE);
		VkPipelineDepthStencilStateCreateInfo depth_stencil = vkl::createPipelineDepthStencilStateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE, VK_FALSE);
		VkPipelineColorBlendAttachmentState color_blend_attachement{};
		color_blend_attachement.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
#ifdef DISABLE_COLOR_BLEND
		color_blend_attachement.blendEnable = VK_FALSE;
#else
		color_blend_attachement.blendEnable = VK_TRUE;
		color_blend_attachement.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachement.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachement.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachement.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachement.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachement.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
#endif
		float blend_constants[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		VkPipelineColorBlendStateCreateInfo color_blend = vkl::createPipelineColorBlendStateInfo(VK_FALSE, VK_LOGIC_OP_COPY, 1, &color_blend_attachement, blend_constants);
		VkDynamicState dynamic_states[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamic_state = vkl::createPipelineDynamiceStateCreateInfo(ARRAY_SIZE(dynamic_states), dynamic_states);
		VkGraphicsPipelineCreateInfo info = vkl::createGraphicsPipelineInfo(ARRAY_SIZE(stages), stages, &MODEL_VERTEX_INPUT_INFO, &assembly_info, nullptr, &viewport_info, &rasterization, &multisample, &depth_stencil, &color_blend, &dynamic_state, pipelineLayout, SVE::getRenderPass(), 0);

		pipeline = vkl::createGraphicsPipeline(SVE::getDevice(), info);
		vkl::destroyShaderModule(SVE::getDevice(), vertex_module);
		vkl::destroyShaderModule(SVE::getDevice(), fragment_module);

	}

	inline void updateTextureDescriptors() {
		Descriptor& descriptor = descriptorSets[SVE::getImageIndex()];
		if (descriptor.invalidated) {
			std::vector<VkDescriptorImageInfo> texture_info(textures.size());
			for (size_t i = 0; i < texture_info.size(); ++i) {
				texture_info[i] = { VK_NULL_HANDLE, textures[i].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			}
			VkWriteDescriptorSet descriptor_write = vkl::createDescriptorWrite(descriptor.set, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, 0, (uint32_t)texture_info.size(), texture_info.data());
			vkUpdateDescriptorSets(SVE::getDevice(), 1, &descriptor_write, 0, nullptr);
			descriptor.invalidated = false;
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
		for (size_t i = 0; i < modelBuffers.size(); ++i) {
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