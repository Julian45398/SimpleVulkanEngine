#include "SVE_ModelRenderer.h"


constexpr uint32_t MAX_INSTANCE_COUNT = 2048;
constexpr uint32_t VERTEX_START_BYTE_OFFSET = MAX_INSTANCE_COUNT * sizeof(glm::mat4);
constexpr VkDeviceSize PAGE_SIZE = 2 << 27;
constexpr uint32_t MAX_VERTICES = (PAGE_SIZE - MAX_INSTANCE_COUNT * sizeof(glm::mat4)) / sizeof(SveModelVertex);
constexpr char MODEL_VERTEX_SHADER_FILE[] = "resources/shaders/model.vert";
constexpr char MODEL_FRAGMENT_SHADER_FILE[] = "resources/shaders/model.frag";
constexpr VkVertexInputBindingDescription MODEL_VERTEX_BINDINGS[] = {
	{0, sizeof(SveModelVertex), VK_VERTEX_INPUT_RATE_VERTEX},
	{1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE}
};
constexpr VkVertexInputAttributeDescription MODEL_VERTEX_ATTRIBUTES[] = {
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
const VkPipelineVertexInputStateCreateInfo MODEL_VERTEX_INPUT_INFO = vkl::createPipelineVertexInputStateInfo(ARRAY_SIZE(MODEL_VERTEX_BINDINGS), MODEL_VERTEX_BINDINGS,
	ARRAY_SIZE(MODEL_VERTEX_ATTRIBUTES), MODEL_VERTEX_ATTRIBUTES);



SveModelRenderer::SveModelRenderer(VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout) {
	vertexBuffer = VK_NULL_HANDLE;
	vertexDeviceMemory = VK_NULL_HANDLE;
	sampler = VK_NULL_HANDLE;
	fence = VK_NULL_HANDLE;
	commandPool = VK_NULL_HANDLE;
	commandBuffer = VK_NULL_HANDLE;
	stagingBuffer = VK_NULL_HANDLE;
	stagingMemory = VK_NULL_HANDLE;
	stagingMapped = nullptr;
	descriptorLayout = VK_NULL_HANDLE;
	pipelineLayout = VK_NULL_HANDLE;
	pipeline = VK_NULL_HANDLE;
	transformCount = 0;
	vertexCount = 0;
	indexCount = 0;
	freeVertexBufferMemory = PAGE_SIZE - MAX_INSTANCE_COUNT * sizeof(glm::mat4);


	// Vertex and Index Buffers:
	vertexBuffer = vkl::createBuffer(SVE::getDevice(), PAGE_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, SVE::getGraphicsFamily());
	vertexDeviceMemory = vkl::allocateForBuffer(SVE::getDevice(), SVE::getPhysicalDevice(), vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Sampler:
	sampler = vkl::createSampler(SVE::getDevice(), VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

	// Descriptor Set Layout:
	{
		VkDescriptorSetLayoutBinding uniform_bindings[] = {
			//{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
			{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_TEXTURE_COUNT, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
		};
		descriptorLayout = SVE::createDescriptorSetLayout(ARRAY_SIZE(uniform_bindings), uniform_bindings);
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
			VkWriteDescriptorSet writes[] = {
				vkl::createDescriptorWrite(descriptorSets[i].set, VK_DESCRIPTOR_TYPE_SAMPLER, 0, 0, 1, &sampler_info)
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
			uniformLayout,
			descriptorLayout
		};
		pipelineLayout = vkl::createPipelineLayout(SVE::getDevice(), ARRAY_SIZE(descriptor_layouts), descriptor_layouts, ARRAY_SIZE(push_constant_ranges), push_constant_ranges);
		// Pipeline:
		createPipeline(VK_POLYGON_MODE_FILL);
	}

	// Transfer Objects:
	commandPool = SVE::createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	commandBuffer = SVE::createCommandBuffer(commandPool);
	fence = SVE::createFence();
}

SveModelRenderer::~SveModelRenderer() {
	if (stagingMapped != nullptr) {
		SVE::waitForFence(fence);
		SVE::freeMemory(stagingMemory);
		SVE::destroyBuffer(stagingBuffer);
	}
	SVE::destroyFence(fence);
	SVE::destroyCommandBuffer(commandPool, commandBuffer);
	SVE::destroyCommandPool(commandPool);
	
	SVE::destroyPipelineLayout(pipelineLayout);
	SVE::destroyPipeline(pipeline);

	SVE::destroyDescriptorSetLayout(descriptorLayout);

	SVE::destroySampler(sampler);
	SVE::destroyBuffer(vertexBuffer);
	SVE::freeMemory(vertexDeviceMemory);
}

void SveModelRenderer::addModel(const SveModel& model) {
	shl::logInfo("adding Model");
	//const Model& model = *modelPtr;
	size_t total_size = 0;

	// get total allocation size:
	for (size_t i = 0; i < model.images.size(); ++i) {
		shl::logInfo("pixel size: ", model.images[i].pixels.size());
		total_size += model.images[i].pixels.size();
	}
	size_t vertex_size = 0;
	for (size_t i = 0; i < model.meshes.size(); ++i) {
		vertex_size += model.meshes[i].indices.size() * sizeof(uint32_t);
		vertex_size += model.meshes[i].vertices.size() * sizeof(SveModelVertex);
		total_size += model.meshes[i].instanceTransforms.size() * sizeof(glm::mat4);

		//freeVertexBufferMemory -= model.meshes[i].indices.size() * sizeof(uint32_t);
		//freeVertexBufferMemory -= model.meshes[i].vertices.size() * sizeof(SveModelVertex);
	}
	if (freeVertexBufferMemory < vertex_size) {
		shl::logError("not enough free vertex memory for model!");
		shl::logError("failed to load model");
		return;
	}
	else {
		freeVertexBufferMemory -= vertex_size;
	}
	total_size += vertex_size;

	stagingBuffer = SVE::createBuffer(total_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	stagingMemory = SVE::allocateForBuffer(stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingMapped = (uint8_t*)SVE::mapMemory(stagingMemory, VK_WHOLE_SIZE, 0);

	vkl::resetCommandPool(SVE::getDevice(), commandPool);
	vkl::beginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	
	size_t offset = 0;
	// Copy image data:
	for (size_t i = 0; i < model.images.size(); ++i) {
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

	// Copy mesh data:
	for (size_t i = 0; i < model.meshes.size(); ++i) {
		const Mesh& mesh = model.meshes[i];

		// Instance transforms:
		VkBufferCopy instance_region;
		instance_region.dstOffset = transformCount * sizeof(glm::mat4);
		instance_region.size = mesh.instanceTransforms.size() * sizeof(glm::mat4);
		instance_region.srcOffset = offset;
		transformCount += (uint32_t)mesh.instanceTransforms.size();
		//shl::logDebug("instance region offset: ", offset, " size: ", instance_region.size, " instance count: ", mesh.instanceTransforms.size(), " dst offset: ", instance_region.dstOffset);
		memcpy(stagingMapped + offset, mesh.instanceTransforms.data(), instance_region.size);
		offset += instance_region.size;
		
		// Indices:
		VkBufferCopy index_region;
		index_region.size = mesh.indices.size() * sizeof(uint32_t);
		index_region.dstOffset = PAGE_SIZE - indexCount * sizeof(uint32_t) - index_region.size;
		index_region.srcOffset = offset;
		indexCount += (uint32_t)mesh.indices.size();
		//shl::logDebug("index region offset: ", offset, " size: ", index_region.size, " index count: ", mesh.indices.size(), " dst offset: ", index_region.dstOffset);
		memcpy(stagingMapped + offset, mesh.indices.data(), index_region.size);
		offset += index_region.size;

		// Vertices:
		VkBufferCopy vertex_region;
		vertex_region.dstOffset = VERTEX_START_BYTE_OFFSET + vertexCount * sizeof(SveModelVertex);
		vertex_region.size = mesh.vertices.size() * sizeof(SveModelVertex);
		vertex_region.srcOffset = offset;
		vertexCount += (uint32_t)mesh.vertices.size();
		//shl::logDebug("vertex region offset: ", offset, " size: ", vertex_region.size, " vertex count: ", mesh.vertices.size(), " dst offset: ", vertex_region.dstOffset);
		memcpy(stagingMapped + offset, mesh.vertices.data(), vertex_region.size);
		offset += vertex_region.size;

		VkBufferCopy regions[] = {
			instance_region, index_region, vertex_region
		};

		shl::logInfo("index ",i ," size: ", index_region.size, " index size offset : ", index_region.dstOffset," vertex size: ",vertex_region.size , " vertex offset : ", vertex_region.dstOffset);
		vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, ARRAY_SIZE(regions), regions);
	}

	// Submitting Commands:
	vkl::endCommandBuffer(commandBuffer);
	vkl::submitCommands(SVE::getGraphicsQueue(), commandBuffer, fence);

	ModelDrawData drawData;
	drawData.imageCount = model.images.size();
	drawData.instanceCount = 1;
	drawData.meshes.resize(model.meshes.size());
	for(size_t i = 0; i < model.meshes.size(); ++i) {
		drawData.meshes[i].imageIndex = model.meshes[i].imageIndex;
		drawData.meshes[i].indexCount = (uint32_t)model.meshes[i].indices.size();
		drawData.meshes[i].vertexCount = (uint32_t)model.meshes[i].vertices.size();
		drawData.meshes[i].instanceCount = (uint32_t)model.meshes[i].instanceTransforms.size();
	}
	models.push_back(drawData);
	//models.push_back(&model);
	shl::logInfo("Model added. Free vertex memory: ", freeVertexBufferMemory);
	uint32_t vertex_count = 0;
	uint32_t index_count = 0;
	for (size_t i = 0; i < models.size(); ++i) {
		for(size_t j = 0; j < models[i].meshes.size(); ++j) {
			vertex_count += models[i].meshes[j].vertexCount;
			index_count += models[i].meshes[j].indexCount;
		}
		
	}
	shl::logInfo("Total vertex count: ", vertex_count, " triangle count: ", index_count/3);
	shl::logInfo("Total allocated image memory: ", textureAllocator.getAllocatedSize());
	shl::logInfo("Total image count: ", textures.size());
	shl::logWarn("HelloWorld");
}

void SveModelRenderer::draw(VkCommandBuffer commands, VkDescriptorSet uniformSet) {
	checkTransferStatus();
	updateTextureDescriptors();

	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	VkViewport viewport = { (float)SVE::getViewportOffsetX(), (float)SVE::getViewportOffsetY(), (float)SVE::getViewportWidth(), (float)SVE::getViewportHeight(), 0.0f, 1.0f };
	VkRect2D scissor = { {SVE::getViewportOffsetX(), SVE::getViewportOffsetY()}, {SVE::getViewportWidth(), SVE::getViewportHeight()} };
	vkCmdSetViewport(commands, 0, 1, &viewport);
	vkCmdSetScissor(commands, 0, 1, &scissor);
	VkDescriptorSet sets[] = {
		uniformSet,
		descriptorSets[SVE::getImageIndex()].set
	};

	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, ARRAY_SIZE(sets), sets, 0, nullptr);
	VkBuffer buffers[] = { vertexBuffer, vertexBuffer };
	VkDeviceSize offsets[] = { VERTEX_START_BYTE_OFFSET, 0 };
	static_assert(ARRAY_SIZE(buffers) == ARRAY_SIZE(offsets));
	vkCmdBindVertexBuffers(commands, 0, ARRAY_SIZE(buffers), buffers, offsets);
	vkCmdBindIndexBuffer(commands, vertexBuffer, 0, VK_INDEX_TYPE_UINT32);
	int32_t vertex_offset = 0;
	uint32_t instance_offset = 0;
	constexpr uint32_t max_index_offset = PAGE_SIZE / sizeof(uint32_t);
	uint32_t first_index = max_index_offset;
	uint32_t image_count = 0;
	for (size_t i = 0; i < models.size(); ++i) {
		//const SveModel& model = models[i][0];
		const ModelDrawData& drawData = models[i];
		for (size_t j = 0; j < drawData.meshes.size(); ++j) {
			const auto& mesh = drawData.meshes[j];
			first_index -= mesh.indexCount;
			uint32_t image_offset = image_count + mesh.imageIndex;
			vkCmdPushConstants(commands, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(image_offset), &image_offset);
			vkCmdDrawIndexed(commands, mesh.indexCount, mesh.instanceCount, first_index, vertex_offset, instance_offset);
			vertex_offset += mesh.vertexCount;
			instance_offset += mesh.instanceCount;
		}
		image_count += (uint32_t)drawData.imageCount;
	}
}

void SveModelRenderer::changePipelineSettings(VkPolygonMode polygonMode) {
	vkl::destroyPipeline(SVE::getDevice(), pipeline);
	createPipeline(polygonMode);
}
void SveModelRenderer::invalidateDescriptors() {
	for (size_t i = 0; i < descriptorSets.size(); ++i) {
		descriptorSets[i].invalidated = true;
	}
	shl::logInfo("descriptors invalidated!");
}

void SveModelRenderer::checkTransferStatus() {
	if (stagingMapped != nullptr) {
		if (vkGetFenceStatus(SVE::getDevice(), fence) == VK_SUCCESS) {
			invalidateDescriptors();
			SVE::resetFence(fence);
			SVE::destroyBuffer(stagingBuffer);
			SVE::freeMemory(stagingMemory);
			stagingMapped = nullptr;
		}
	}
}

void SveModelRenderer::createPipeline(VkPolygonMode polygonMode) {
	pipeline = SveRenderPipelineBuilder(MODEL_VERTEX_SHADER_FILE, MODEL_FRAGMENT_SHADER_FILE, pipelineLayout, MODEL_VERTEX_INPUT_INFO).setPolygonMode(polygonMode).build();
}

void SveModelRenderer::updateTextureDescriptors() {
	Descriptor& descriptor = descriptorSets[SVE::getImageIndex()];
	if (descriptor.invalidated) {
		std::vector<VkDescriptorImageInfo> texture_info(textures.size());
		for (size_t i = 0; i < texture_info.size(); ++i) {
			texture_info[i] = { VK_NULL_HANDLE, textures[i].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		}
		VkWriteDescriptorSet descriptor_write = vkl::createDescriptorWrite(descriptor.set, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, 0, (uint32_t)texture_info.size(), texture_info.data());
		vkUpdateDescriptorSets(SVE::getDevice(), 1, &descriptor_write, 0, nullptr);
		descriptor.invalidated = false;
		shl::logInfo("Descriptor updated");
	}
}