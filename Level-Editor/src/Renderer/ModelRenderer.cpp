#include "ModelRenderer.hpp"

namespace SGF {
    constexpr size_t PAGE_SIZE = 2 << 27;
    constexpr uint32_t MAX_TEXTURE_COUNT = 128;
    constexpr uint32_t MAX_INSTANCE_COUNT = 2048;
    constexpr uint32_t MAX_INDEX_COUNT = 2 << 22;

    constexpr size_t INDEX_BUFFER_SIZE = MAX_INDEX_COUNT * sizeof(uint32_t);
    constexpr size_t INSTANCE_BUFFER_SIZE = MAX_INSTANCE_COUNT * sizeof(glm::mat4);
    constexpr uint32_t MAX_VERTEX_COUNT = (PAGE_SIZE - INDEX_BUFFER_SIZE - INSTANCE_BUFFER_SIZE) / sizeof(ModelRenderer::ModelVertex);

    constexpr size_t VERTEX_BUFFER_SIZE = MAX_VERTEX_COUNT * sizeof(ModelRenderer::ModelVertex);

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

    ModelRenderer::ModelVertex::ModelVertex(const glm::vec3& p, const glm::vec3& n, const glm::vec2& u, const glm::vec4& c, uint32_t i) : position(p), uv(u), textureIndex(i) {
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
        totalInstanceCount = 0;
        totalVertexCount = 0;
        totalIndexCount = 0;

        // Vertex and Index Buffers:
        vertexBuffer = device.CreateBuffer(PAGE_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        vertexDeviceMemory = device.AllocateMemory(vertexBuffer);

        // Sampler:
        sampler = device.CreateImageSampler(VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 0.f, VK_FALSE, 0.f, 0, VK_COMPARE_OP_ALWAYS, 0.f, 0.f, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

        // Descriptor Set Layout:
        {
            VkDescriptorSetLayoutBinding uniform_bindings[] = {
                //{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
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
        
        // Pipeline:
        {
            // Layout:
            VkDescriptorSetLayout descriptor_layouts[] = {
                uniformLayout,
                descriptorLayout
            };
            VkPushConstantRange range;
            range.offset = 0;
            range.size = sizeof(glm::vec4);
            range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; 
            VkPushConstantRange push_constant_ranges[] = {
                range
            };
            pipelineLayout = device.CreatePipelineLayout(descriptor_layouts, push_constant_ranges);
            // Pipeline:
            //CreatePipeline(renderPass, subpass, VK_POLYGON_MODE_FILL);
        }
    }

    ModelRenderer::~ModelRenderer() {
        auto& device = Device::Get();
        if (stagingBuffer.GetSize() != 0) {
            device.WaitFence(fence);
        }
        device.Destroy(fence, commandPool, pipelineLayout, descriptorLayout, sampler, vertexBuffer, vertexDeviceMemory);
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

    size_t ModelRenderer::UploadTextures(const GenericModel& model) {
        auto& device = Device::Get();
        size_t offset = 0;
        if (model.textures.size() == 0 && textures.size() == 0) {
            // Create empty 1x1 texture
            // Create default texture
            textures.push_back(textureAllocator.CreateImage(1, 1));
            Texture texture(1, 1, (uint8_t*)&DEFAULT_COLOR);
            UploadTexture(textures.back(), texture, 0);

        }
        // Copy image data:
        for (size_t i = 0; i < model.textures.size(); ++i) {
            textures.push_back(textureAllocator.CreateImage(model.textures[i].GetWidth(), model.textures[i].GetHeight()));
            auto& texture = textures.back();
            offset = UploadTexture(texture, model.textures[i], offset);
        }
        return offset;
    }

    size_t CalculateTotalRequiredStagingMemorySize(const GenericModel& model, const ModelRenderer& renderer) {
        size_t total_size = 0;
        for (size_t i = 0; i < model.textures.size(); ++i) {
            total_size += model.textures[i].GetMemorySize();
        }
        if (model.textures.size() == 0 && renderer.GetTextureCount() == 0) {
            total_size += sizeof(uint32_t); // add default 1x1 - texture
        }

        total_size += model.indices.size() * sizeof(uint32_t) + model.vertices.size() * sizeof(ModelRenderer::ModelVertex);
            
        for (size_t i = 0; i < model.meshInfos.size(); ++i) {
            total_size += model.meshInfos[i].instanceTransforms.size() * sizeof(glm::mat4);
        }
        return total_size;
    }

    void ModelRenderer::BeginTransfer(size_t uploadMemorySize) {
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

    ModelRenderer::ModelDrawData ModelRenderer::AddModel(const GenericModel& model) {
        //const Model& model = *modelPtr;
        size_t uploadMemorySize = CalculateTotalRequiredStagingMemorySize(model, *this);

        auto& device = Device::Get();
        BeginTransfer(uploadMemorySize);

        uint32_t textureIndexOffset = textures.size();
        size_t total_added_instances = model.GetTotalInstanceCount();
        size_t offset = UploadTextures(model);
        // Copy mesh data:
        VkBufferCopy indexRegion;
        indexRegion.size = model.indices.size() * sizeof(uint32_t);
        indexRegion.srcOffset = offset;
        indexRegion.dstOffset = INDEX_BUFFER_BYTE_OFFSET + totalIndexCount * sizeof(uint32_t);
        offset = stagingBuffer.CopyData(model.indices.data(), indexRegion);

        VkBufferCopy vertexRegion;
        vertexRegion.size = model.vertices.size() * sizeof(ModelRenderer::ModelVertex);
        vertexRegion.dstOffset = VERTEX_BYTE_OFFSET + totalVertexCount * sizeof(ModelRenderer::ModelVertex);
        vertexRegion.srcOffset = offset;
        for (size_t j = 0; j < model.meshInfos.size(); ++j) {
            for (size_t k = 0; k < model.meshInfos[j].vertexCount; ++k) {
                size_t i = model.meshInfos[j].vertexOffset + k;
                uint32_t renderTextureIndex = model.meshInfos[j].textureIndex == UINT32_MAX ? 0 : model.meshInfos[j].textureIndex + textureIndexOffset;
                ModelRenderer::ModelVertex modelVertex(model.vertices[i].position, model.vertices[i].normal, model.vertices[i].uv, 
                    model.vertices[i].color, renderTextureIndex);
                offset = stagingBuffer.CopyData(&modelVertex, sizeof(modelVertex), offset);
            }
        }

        VkBufferCopy instanceRegion;
        instanceRegion.size = total_added_instances * sizeof(glm::mat4);
        instanceRegion.srcOffset = offset;
        instanceRegion.dstOffset = INSTANCE_BYTE_OFFSET + totalInstanceCount * sizeof(glm::mat4);
        // Copy instances
        {
            size_t transformOffset = instanceRegion.srcOffset;
            for (size_t i = 0; i < model.meshInfos.size(); ++i) {
                const GenericModel::MeshInfo& meshInfo = model.meshInfos[i];
                size_t transforms_size = meshInfo.instanceTransforms.size() * sizeof(meshInfo.instanceTransforms[0]);
                transformOffset = stagingBuffer.CopyData(meshInfo.instanceTransforms.data(), transforms_size, transformOffset);
            }
        }

        VkBufferCopy regions[] = {
            indexRegion, vertexRegion, instanceRegion 
        };

        vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, ARRAY_SIZE(regions), regions);

        // Submitting Commands:
        FinalizeTransfer();

        ModelDrawData drawData;
        drawData.meshes.resize(model.meshInfos.size());
        for(size_t i = 0; i < model.meshInfos.size(); ++i) {
            drawData.meshes[i].indexCount = (uint32_t)model.meshInfos[i].indexCount;
            drawData.meshes[i].vertexCount = (uint32_t)model.meshInfos[i].vertexCount;
            drawData.meshes[i].instanceCount = (uint32_t)model.meshInfos[i].instanceTransforms.size();

            drawData.meshes[i].instanceOffset = totalInstanceCount; 
            drawData.meshes[i].vertexOffset = totalVertexCount; 
            drawData.meshes[i].indexOffset = totalIndexCount;

            totalIndexCount += (uint32_t)model.meshInfos[i].indexCount;
            totalVertexCount += (uint32_t)model.meshInfos[i].vertexCount;
            totalInstanceCount += (uint32_t)model.meshInfos[i].instanceTransforms.size();
        }
        return drawData;
        //models.push_back(drawData);
    }

    void ModelRenderer::PrepareDrawing(VkCommandBuffer commands, VkPipeline pipeline, VkDescriptorSet uniformSet, glm::uvec2 viewportSize, uint32_t frameIndex, const glm::vec4& colorModifier) {
        CheckTransferStatus();
        UpdateTextureDescriptors(frameIndex);

        vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdPushConstants(commands, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec4), &colorModifier);
        VkViewport viewport = { (float)0.0f, (float)0.0f, (float)viewportSize.x, (float)viewportSize.y, 0.0f, 1.0f };
        VkRect2D scissor = { {0, 0}, { viewportSize.x, viewportSize.y } };
        vkCmdSetViewport(commands, 0, 1, &viewport);
        vkCmdSetScissor(commands, 0, 1, &scissor);
        VkDescriptorSet sets[] = {
            uniformSet,
            descriptorSets[frameIndex]
        };

        vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, ARRAY_SIZE(sets), sets, 0, nullptr);
        const VkBuffer buffers[] = {
            vertexBuffer, 
            vertexBuffer, 
        };
        static_assert(ARRAY_SIZE(buffers) == ARRAY_SIZE(VERTEX_BUFFER_OFFSETS));
        vkCmdBindVertexBuffers(commands, 0, ARRAY_SIZE(buffers), buffers, VERTEX_BUFFER_OFFSETS);
        vkCmdBindIndexBuffer(commands, vertexBuffer, INDEX_BUFFER_BYTE_OFFSET, VK_INDEX_TYPE_UINT32);
    }
    void ModelRenderer::BindPipeline(VkCommandBuffer commands, VkPipeline pipeline) const {
        vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
    void ModelRenderer::SetColorModifier(VkCommandBuffer commands, const glm::vec4& color) const {
        vkCmdPushConstants(commands, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec4), &color);
    }

    size_t ModelRenderer::GetTotalDeviceMemoryUsed() const {
        return totalIndexCount * sizeof(uint32_t) + totalVertexCount * sizeof(ModelVertex) + totalInstanceCount * sizeof(glm::mat4) + textureAllocator.GetUsedMemorySize();
    }
    size_t ModelRenderer::GetTotalDeviceMemoryAllocated() const {
        return PAGE_SIZE + textureAllocator.GetAllocatedSize();
    }
        
    void ModelRenderer::DrawMesh(VkCommandBuffer commands, const ModelRenderer::MeshDrawData& m) const {
        vkCmdDrawIndexed(commands, m.indexCount, m.instanceCount, m.indexOffset, m.vertexOffset, m.instanceOffset);
    }
    void ModelRenderer::DrawModel(VkCommandBuffer commands, const ModelRenderer::ModelDrawData& m) const {
        for (const auto& mesh : m.meshes) {
            DrawMesh(commands, mesh);
        }
    }

    /*
    void ModelRenderer::DrawAll(VkCommandBuffer commands) {
        uint32_t vertex_offset = 0;
        uint32_t instance_offset = 0;
        uint32_t index_offset = 0;
        for (size_t i = 0; i < models.size(); ++i) {
            //const Model& model = models[i][0];
            const ModelDrawData& drawData = models[i];
            for (size_t j = 0; j < drawData.meshes.size(); ++j) {
                const auto& mesh = drawData.meshes[j];
                //info("Drawing mesh: ", j, " indexCount: ", mesh.indexCount, " vertices: ", mesh.vertexCount);
                vkCmdDrawIndexed(commands, mesh.indexCount, mesh.instanceCount, index_offset, vertex_offset, instance_offset);
                vertex_offset += mesh.vertexCount;
                instance_offset += mesh.instanceCount;
                index_offset += mesh.indexCount;
            }
        }
    }
    */
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