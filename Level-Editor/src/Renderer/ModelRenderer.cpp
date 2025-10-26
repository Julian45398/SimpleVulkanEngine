#include "ModelRenderer.hpp"

namespace SGF {
    constexpr size_t PAGE_SIZE = 2 << 27;
    constexpr uint32_t MAX_TEXTURE_COUNT = 128;
    constexpr uint32_t MAX_INSTANCE_COUNT = 2048;
    constexpr uint32_t MAX_INDEX_COUNT = 2 << 22;

    constexpr size_t INDEX_BUFFER_SIZE = MAX_INDEX_COUNT * sizeof(uint32_t);
    constexpr size_t INSTANCE_BUFFER_SIZE = MAX_INSTANCE_COUNT * sizeof(glm::mat4);
    constexpr size_t VERTEX_BUFFER_SIZE = PAGE_SIZE - INDEX_BUFFER_SIZE - INSTANCE_BUFFER_SIZE;

    constexpr uint32_t MAX_VERTEX_COUNT = VERTEX_BUFFER_SIZE / sizeof(ModelRenderer::Vertex);

    constexpr size_t INSTANCE_BYTE_OFFSET = 0;
    constexpr size_t VERTEX_BYTE_OFFSET = INSTANCE_BYTE_OFFSET + INSTANCE_BUFFER_SIZE;
    constexpr size_t INDEX_BUFFER_BYTE_OFFSET = VERTEX_BYTE_OFFSET + VERTEX_BUFFER_SIZE;

    constexpr size_t VERTEX_BUFFER_OFFSETS[]{
        VERTEX_BYTE_OFFSET,
        INSTANCE_BYTE_OFFSET,
    };

    inline uint32_t PackNormalA2B10G10R10(const glm::vec3& n) {
        glm::vec3 v = glm::clamp(n * 0.5f + 0.5f, 0.0f, 1.0f);
        uint32_t x = static_cast<uint32_t>(v.x * 1023.0f) & 0x3FF;
        uint32_t y = static_cast<uint32_t>(v.y * 1023.0f) & 0x3FF;
        uint32_t z = static_cast<uint32_t>(v.z * 1023.0f) & 0x3FF;
        return (z << 20) | (y << 10) | x;
    }

    inline uint32_t PackColorRGBA8(const glm::vec4& c) {
        uint32_t r = static_cast<uint32_t>(glm::clamp(c.r, 0.0f, 1.0f) * 255.0f) & 0xFF;
        uint32_t g = static_cast<uint32_t>(glm::clamp(c.g, 0.0f, 1.0f) * 255.0f) & 0xFF;
        uint32_t b = static_cast<uint32_t>(glm::clamp(c.b, 0.0f, 1.0f) * 255.0f) & 0xFF;
        uint32_t a = static_cast<uint32_t>(glm::clamp(c.a, 0.0f, 1.0f) * 255.0f) & 0xFF;
        return (a << 24) | (b << 16) | (g << 8) | r;
    }

    ModelRenderer::Vertex::Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec2& u, const glm::vec4& c, uint32_t i) : position(p), uv(u), textureIndex(i) {
        normal = PackNormalA2B10G10R10(n);
        color.r = static_cast<uint32_t>(glm::clamp(c.r, 0.0f, 1.0f) * 255.0f) & 0xFF;
        color.g = static_cast<uint32_t>(glm::clamp(c.g, 0.0f, 1.0f) * 255.0f) & 0xFF;
        color.b = static_cast<uint32_t>(glm::clamp(c.b, 0.0f, 1.0f) * 255.0f) & 0xFF;
        color.a = static_cast<uint32_t>(glm::clamp(c.a, 0.0f, 1.0f) * 255.0f) & 0xFF;
    }

    static_assert((INDEX_BUFFER_BYTE_OFFSET + INDEX_BUFFER_SIZE) <= PAGE_SIZE);

    constexpr char MODEL_VERTEX_SHADER_FILE[] = "shaders/model.vert";
    constexpr char MODEL_FRAGMENT_SHADER_FILE[] = "shaders/model.frag";
    const uint32_t DEFAULT_COLOR = 0xFFFFFFFF; // White

    void ModelRenderer::Initialize(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout) {
        auto& device = Device::Get();
        //totalInstanceCount = 0;
        totalVertexCount = 0;
        totalIndexCount = 0;
        totalInstanceCount = 0;

        // Vertex and Index Buffers:
        vertexBuffer = device.CreateBuffer(PAGE_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        vertexDeviceMemory = device.AllocateMemory(vertexBuffer);

        // Sampler:
        sampler = device.CreateImageSampler(VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 0.f, VK_FALSE, 0.f, 0, VK_COMPARE_OP_ALWAYS, 0.f, 0.f, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

        // Descriptor Set Layout:
        {
            VkDescriptorSetLayoutBinding uniform_bindings[] = {
                {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_TEXTURE_COUNT, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
            };
            descriptorLayout = device.CreateDescriptorSetLayout(uniform_bindings);
        }

        // DescriptorSets:
        {
            // Allocating Descriptor Sets:
            VkDescriptorSetLayout layouts[] = {descriptorLayout, descriptorLayout};
            device.AllocateDescriptorSets(descriptorPool, layouts, descriptorSets);
            descriptorInvalidated[0] = false;
            descriptorInvalidated[1] = false;

            // Writing Descriptor Sets:
            VkDescriptorImageInfo sampler_info = { sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
            VkWriteDescriptorSet writes[] = {
                Vk::CreateDescriptorWrite(descriptorSets[0], 0, 0, VK_DESCRIPTOR_TYPE_SAMPLER, &sampler_info, 1),
                Vk::CreateDescriptorWrite(descriptorSets[1], 0, 0, VK_DESCRIPTOR_TYPE_SAMPLER, &sampler_info, 1)
            };
            device.UpdateDescriptors(writes);
        }

        // Transfer Objects:
        commandPool = device.CreateGraphicsCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
        commandBuffer = device.AllocateCommandBuffer(commandPool);
        fence = device.CreateFence();
    }

    ModelRenderer::~ModelRenderer() {
        auto& device = Device::Get();
        if (stagingBuffer.GetSize() != 0) {
            device.WaitFence(fence);
        }
        device.Destroy(fence, commandPool, descriptorLayout, sampler, vertexBuffer, vertexDeviceMemory);
    }

    size_t ModelRenderer::UploadTexture(const TextureImage& image, const Texture& texture, size_t offset) {
        auto& device = Device::Get();

        VkBufferImageCopy region{};
        region.bufferOffset = offset;
        region.imageSubresource = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = { texture.GetWidth(), texture.GetHeight(), 1 };

        size_t mem_size = texture.GetMemorySize();
        offset = stagingBuffer.CopyData(texture.GetData(), mem_size, offset);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.image = image.image;
        barrier.srcQueueFamilyIndex = device.GetGraphicsFamily();
        barrier.dstQueueFamilyIndex = device.GetGraphicsFamily();
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        barrier.srcAccessMask = barrier.dstAccessMask;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = barrier.newLayout;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
        return offset;
    }

    size_t ModelRenderer::UploadTextures(const GenericModel& model, size_t startOffset) {
        auto& device = Device::Get();
        size_t offset = startOffset;
        if (model.textures.size() == 0 && textures.size() == 0) {
            // Create empty 1x1 default texture
            textures.push_back(textureAllocator.CreateImage(1, 1));
            Texture texture(1, 1, (uint8_t*)&DEFAULT_COLOR);
            UploadTexture(textures.back(), texture, 0);
            info("Uploading Dummy Texture!");
        }
        // Copy image data:
        for (size_t i = 0; i < model.textures.size(); ++i) {
            textures.push_back(textureAllocator.CreateImage(model.textures[i].GetWidth(), model.textures[i].GetHeight()));
            auto& texture = textures.back();
            offset = UploadTexture(texture, model.textures[i], offset);
        }
        return offset;
    }

    size_t ModelRenderer::GetRequiredIndexMemorySize(const GenericModel& model) const {
        return model.indices.size() * sizeof(model.indices[0]);
    }
    size_t ModelRenderer::GetRequiredInstanceMemorySize(const GenericModel& model) const {
        return model.nodes.size() * sizeof(model.nodes[0].globalTransform);
    }
    size_t ModelRenderer::GetRequiredVertexMemorySize(const GenericModel& model) const {
        return model.vertices.size() * sizeof(ModelRenderer::Vertex);
    }
    size_t ModelRenderer::GetRequiredTextureMemorySize(const GenericModel& model) const {
        if (model.textures.size() == 0 && textures.size() == 0) return 4; // the size of the buffer texture
        size_t size = 0;
        for (const auto& texture : model.textures) {
            size += texture.GetMemorySize();
        }
        return size;
    }

    void ModelRenderer::BeginTransfer(const GenericModel& model) {
        size_t uploadMemorySize = GetTotalRequiredMemorySize(model);
        auto& device = Device::Get();

        if (stagingBuffer.IsInitialized()) {
            device.WaitFence(fence);
            device.Reset(fence);
            if (stagingBuffer.GetSize() < uploadMemorySize) 
                stagingBuffer.Resize(uploadMemorySize);
        } else {
            stagingBuffer.Allocate(uploadMemorySize);
        }
        device.Reset(commandPool);
        Vk::BeginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    }

    void ModelRenderer::FinalizeTransfer() {
        vkEndCommandBuffer(commandBuffer);
        Vk::SubmitCommands(Device::Get().GetGraphicsQueue(0), commandBuffer, fence);
    }

    size_t ModelRenderer::PrepareIndexUpload(const GenericModel& model, size_t startOffset, VkBufferCopy* pRegion) {
        VkBufferCopy& indexRegion = *pRegion;
        indexRegion.size = model.indices.size() * sizeof(uint32_t);
        indexRegion.srcOffset = startOffset;
        indexRegion.dstOffset = INDEX_BUFFER_BYTE_OFFSET + totalIndexCount * sizeof(uint32_t);
        return stagingBuffer.CopyData(model.indices.data(), indexRegion);
    }

    size_t ModelRenderer::PrepareVertexUpload(const GenericModel& model, size_t startOffset, VkBufferCopy* pRegion) {
        uint32_t textureIndexOffset = textures.size();
        VkBufferCopy& vertexRegion = *pRegion;
        vertexRegion.size = model.vertices.size() * sizeof(ModelRenderer::Vertex);
        vertexRegion.dstOffset = VERTEX_BYTE_OFFSET + totalVertexCount * sizeof(ModelRenderer::Vertex);
        vertexRegion.srcOffset = startOffset;
        size_t meshVertexCount = 0;
        for (size_t j = 0; j < model.meshes.size(); ++j) {
            meshVertexCount += model.meshes[j].vertexCount;
            for (size_t k = 0; k < model.meshes[j].vertexCount; ++k) {
                size_t i = model.meshes[j].vertexOffset + k;
                uint32_t renderTextureIndex = model.meshes[j].textureIndex == UINT32_MAX ? 0 : model.meshes[j].textureIndex + textureIndexOffset;
                ModelRenderer::Vertex modelVertex(model.vertices[i].position, model.vertices[i].normal, model.vertices[i].uv, 
                    model.vertices[i].color, renderTextureIndex);
                startOffset = stagingBuffer.CopyData(&modelVertex, sizeof(modelVertex), startOffset);
            }
        }
        assert(meshVertexCount == model.vertices.size());
        return startOffset;
    }

    size_t ModelRenderer::PrepareInstanceUpload(const GenericModel& model, size_t offset, VkBufferCopy* pRegion) {
        auto& region = *pRegion;
        region.srcOffset = offset;
        region.size = model.nodes.size() * sizeof(glm::mat4);
        region.dstOffset = totalInstanceCount * sizeof(glm::mat4);
        for (size_t i = 0; i < model.nodes.size(); ++i) {
            offset = stagingBuffer.CopyData(&model.nodes[i].globalTransform, sizeof(glm::mat4), offset);
        }
        return offset;
    }

    ModelRenderer::ModelHandle ModelRenderer::UploadModel(const GenericModel& model) {
        size_t uploadMemorySize = GetTotalRequiredMemorySize(model);

        auto& device = Device::Get();
        BeginTransfer(model);

        VkBufferCopy indexRegion;
        size_t offset = PrepareIndexUpload(model, 0, &indexRegion);

        // Copy mesh data:
        VkBufferCopy vertexRegion;
        offset = PrepareVertexUpload(model, offset, &vertexRegion);
        
        VkBufferCopy instanceRegion;
        offset = PrepareInstanceUpload(model, offset, &instanceRegion);
        
        VkBufferCopy regions[] = {
            indexRegion, vertexRegion, instanceRegion
        };

        vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, ARRAY_SIZE(regions), regions);

        offset = UploadTextures(model, offset);
        assert(offset == uploadMemorySize);

        // Submitting Commands:
        FinalizeTransfer();

        ModelDrawData drawData;
        drawData.indexOffset = totalIndexCount;
        drawData.vertexOffset = totalVertexCount;
        drawData.instanceOffset = totalInstanceCount;
        totalIndexCount += model.indices.size();
        totalVertexCount += model.vertices.size();
        totalInstanceCount += model.nodes.size();
        modelDrawData.push_back(drawData);
        return modelDrawData.size()-1;
    }

    // New: update already-uploaded instance transforms for a model handle
    void ModelRenderer::UpdateInstanceTransforms(ModelHandle handle, const GenericModel& model) {
        if (handle >= modelDrawData.size()) return;

        auto& device = Device::Get();
        const size_t transformCount = model.nodes.size();
        if (transformCount == 0) return;
        const size_t uploadSize = transformCount * sizeof(glm::mat4);

        // Ensure staging buffer is ready and large enough
        if (stagingBuffer.IsInitialized()) {
            device.WaitFence(fence);
            device.Reset(fence);
            if (stagingBuffer.GetSize() < uploadSize) {
                stagingBuffer.Resize(uploadSize);
            }
        } else {
            stagingBuffer.Allocate(uploadSize);
        }

        // Reset command pool and begin one-time command buffer
        device.Reset(commandPool);
        Vk::BeginCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        // Copy transforms into staging buffer
        size_t offset = 0;
        for (size_t i = 0; i < transformCount; ++i) {
            offset = stagingBuffer.CopyData(&model.nodes[i].globalTransform, sizeof(glm::mat4), offset);
        }

        // Prepare buffer copy from staging -> device-local instance region
        VkBufferCopy region{};
        region.srcOffset = 0;
        region.dstOffset = INSTANCE_BYTE_OFFSET + modelDrawData[handle].instanceOffset * sizeof(glm::mat4);
        region.size = uploadSize;

        vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, 1, &region);

        VkBufferMemoryBarrier bufferBarrier{};
        bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        bufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
        bufferBarrier.srcQueueFamilyIndex = Device::Get().GetGraphicsFamily();
        bufferBarrier.dstQueueFamilyIndex = Device::Get().GetGraphicsFamily();
        bufferBarrier.buffer = vertexBuffer;
        bufferBarrier.offset = 0; // or restrict to the dstOffset/size for tighter scope
        bufferBarrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0,
            0, nullptr,
            1, &bufferBarrier,
            0, nullptr
        );

        // Submit and signal fence
        FinalizeTransfer();
    }

    void ModelRenderer::PrepareDrawing(uint32_t frameIndex) {
        CheckTransferStatus();
        UpdateTextureDescriptors(frameIndex);

    }

    size_t ModelRenderer::GetTotalDeviceMemoryUsed() const {
        return totalIndexCount * sizeof(uint32_t) + totalVertexCount * sizeof(Vertex) + /*totalInstanceCount * sizeof(glm::mat4) +*/ textureAllocator.GetUsedMemorySize();
    }

    size_t ModelRenderer::GetTotalDeviceMemoryAllocated() const {
        return PAGE_SIZE + textureAllocator.GetAllocatedSize();
    }
        
    void ModelRenderer::BindBuffersToModel(VkCommandBuffer commands, ModelRenderer::ModelHandle handle) const {
        VkDeviceSize offsets[] = {
            modelDrawData[handle].vertexOffset * sizeof(ModelRenderer::Vertex) + VERTEX_BYTE_OFFSET,
            modelDrawData[handle].instanceOffset * sizeof(glm::mat4) + INSTANCE_BYTE_OFFSET,
        };
        VkBuffer buffers[] = {
            vertexBuffer, vertexBuffer
        };
        vkCmdBindVertexBuffers(commands, 0, ARRAY_SIZE(buffers), buffers, offsets);
        vkCmdBindIndexBuffer(commands, vertexBuffer, INDEX_BUFFER_BYTE_OFFSET + modelDrawData[handle].indexOffset * sizeof(uint32_t), VK_INDEX_TYPE_UINT32);
    }
    void ModelRenderer::DrawModel(VkCommandBuffer commands, const GenericModel& model) const {
        DrawNodeRecursive(commands, model, model.GetRoot());
    }
    void ModelRenderer::DrawNode(VkCommandBuffer commands, const GenericModel& model, const GenericModel::Node& node) const {
        if (node.meshes.size() == 0) return;
        for (size_t i = 0; i < node.meshes.size(); ++i) {
            auto& m = model.GetMesh(node, i);
            vkCmdDrawIndexed(commands, m.indexCount, 1, m.indexOffset, m.vertexOffset, node.index);
        }
    }
    void ModelRenderer::DrawNodeRecursive(VkCommandBuffer commands, const GenericModel& model, const GenericModel::Node& node) const {
        DrawNode(commands, model, node);
        for (size_t i = 0; i < node.children.size(); ++i) {
            auto& n = model.nodes[node.children[i]];
            DrawNodeRecursive(commands, model, n);
        }
    }
    void ModelRenderer::DrawMesh(VkCommandBuffer commands, const GenericModel::Node& node, const GenericModel::Mesh& m) const {
        vkCmdDrawIndexed(commands, m.indexCount, 1, m.indexOffset, m.vertexOffset, node.index);
    }

    void ModelRenderer::InvalidateDescriptors() {
        descriptorInvalidated[0] = true;
        descriptorInvalidated[1] = true;
    }

    void ModelRenderer::CheckTransferStatus() {
        if (stagingBuffer.IsInitialized()) {
            auto& device = Device::Get();
            if (device.IsFenceSignaled(fence)) {
                InvalidateDescriptors();
                device.Reset(fence);
                stagingBuffer.Clear();
            }
        }
    }

    void ModelRenderer::UpdateTextureDescriptors(uint32_t frameIndex) {
        auto& device = Device::Get();
        if (descriptorInvalidated[frameIndex]) {
            std::vector<VkDescriptorImageInfo> texture_info(textures.size());
            for (size_t i = 0; i < texture_info.size(); ++i) {
                texture_info[i] = { VK_NULL_HANDLE, textures[i].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
            }
            device.UpdateDescriptor(descriptorSets[frameIndex], 1, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, texture_info.data(), (uint32_t)texture_info.size());
            descriptorInvalidated[frameIndex] = false;
        }
    }
}