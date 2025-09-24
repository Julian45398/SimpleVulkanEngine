#include "ModelRenderer.hpp"

namespace SGF {
    constexpr size_t PAGE_SIZE = 2 << 27;
    constexpr uint32_t MAX_TEXTURE_COUNT = 128;
    constexpr uint32_t MAX_INSTANCE_COUNT = 2048;
    constexpr uint32_t MAX_INDEX_COUNT = 2 << 22;

    constexpr size_t INDEX_BUFFER_SIZE = MAX_INDEX_COUNT * sizeof(uint32_t);
    constexpr size_t INSTANCE_TRANSFORMS_SIZE = MAX_INSTANCE_COUNT * sizeof(glm::mat4);
    constexpr size_t INSTANCE_TEXTURE_INDEX_SIZE = MAX_INSTANCE_COUNT * sizeof(uint32_t);
    constexpr uint32_t MAX_VERTEX_COUNT = (PAGE_SIZE - INDEX_BUFFER_SIZE - INSTANCE_TRANSFORMS_SIZE - INSTANCE_TEXTURE_INDEX_SIZE) / (sizeof(glm::vec4) + sizeof(glm::vec4) + sizeof(glm::vec2));

    constexpr size_t VERTEX_POSITIONS_SIZE = MAX_VERTEX_COUNT * sizeof(glm::vec4);
    constexpr size_t VERTEX_NORMALS_SIZE = MAX_VERTEX_COUNT * sizeof(glm::vec4);
    constexpr size_t VERTEX_UV_COORDS_SIZE = MAX_VERTEX_COUNT * sizeof(glm::vec2);
    constexpr size_t TOTAL_VERTEX_BUFFER_SIZE = VERTEX_POSITIONS_SIZE + VERTEX_NORMALS_SIZE + VERTEX_UV_COORDS_SIZE;

    constexpr size_t INSTANCE_TRANSFORMS_BYTE_OFFSET = 0;
    constexpr size_t INSTANCE_TEXTURE_INDEX_BYTE_OFFSET = INSTANCE_TRANSFORMS_SIZE;
    constexpr size_t VERTEX_POSITIONS_BYTE_OFFSET = INSTANCE_TEXTURE_INDEX_BYTE_OFFSET + INSTANCE_TEXTURE_INDEX_SIZE;
    constexpr size_t VERTEX_NORMALS_BYTE_OFFSET = VERTEX_POSITIONS_BYTE_OFFSET + VERTEX_POSITIONS_SIZE;
    constexpr size_t VERTEX_UV_COORDS_BYTE_OFFSET = VERTEX_NORMALS_BYTE_OFFSET + VERTEX_NORMALS_SIZE;
    constexpr size_t INDEX_BUFFER_BYTE_OFFSET = VERTEX_UV_COORDS_BYTE_OFFSET + VERTEX_UV_COORDS_SIZE;

    constexpr size_t VERTEX_BUFFER_OFFSETS[] = { 
        VERTEX_POSITIONS_BYTE_OFFSET,
        VERTEX_NORMALS_BYTE_OFFSET,
        VERTEX_UV_COORDS_BYTE_OFFSET,
        INSTANCE_TRANSFORMS_BYTE_OFFSET,
        INSTANCE_TEXTURE_INDEX_BYTE_OFFSET,
    };
    constexpr size_t VERTEX_ITEM_SIZES[] = {
        sizeof(glm::vec4),
        sizeof(glm::vec4),
        sizeof(glm::vec2),
        sizeof(glm::mat4),
        sizeof(uint32_t),
    };

    static_assert((INDEX_BUFFER_BYTE_OFFSET + INDEX_BUFFER_SIZE) <= PAGE_SIZE);

    constexpr char MODEL_VERTEX_SHADER_FILE[] = "shaders/model.vert";
    constexpr char MODEL_FRAGMENT_SHADER_FILE[] = "shaders/model.frag";

    void ModelRenderer::Initialize(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout) {
        auto& device = Device::Get();
        totalInstanceCount = 0;
        totalVertexCount = 0;
        totalIndexCount = 0;
        totalTextureCount = 0;
        info("Max Renderer vertex count: ", MAX_VERTEX_COUNT);

        // Vertex and Index Buffers:
        vertexBuffer = device.CreateBuffer(PAGE_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        vertexDeviceMemory = device.AllocateMemory(vertexBuffer);

        // Sampler:
        sampler = device.CreateImageSampler();

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
        device.Destroy(fence, commandPool, pipelineLayout, descriptorLayout, sampler, vertexBuffer, vertexDeviceMemory);
    }

    size_t ModelRenderer::UploadTextures(const GenericModel& model) {
        auto& device = Device::Get();
        size_t offset = 0;
        // Copy image data:
        for (size_t i = 0; i < model.textures.size(); ++i) {
            textures.push_back(textureAllocator.createImage(model.textures[i].GetWidth(), model.textures[i].GetHeight()));
            auto& texture = textures.back();

            VkBufferImageCopy region{};
            region.bufferOffset = offset;
            region.imageSubresource = {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.layerCount = 1;
            region.imageExtent = { model.textures[i].GetWidth(), model.textures[i].GetHeight(), 1 };

            size_t mem_size = model.textures[i].GetMemorySize();
            offset = stagingBuffer.CopyData(model.textures[i].GetData(), mem_size, offset);

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.image = texture.image;
            barrier.srcQueueFamilyIndex = device.GetGraphicsFamily();
            barrier.dstQueueFamilyIndex = device.GetGraphicsFamily();
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
        return offset;
    }
    size_t CalculateTotalRequiredStagingMemorySize(const GenericModel& model) {
        size_t total_size = 0;
        for (size_t i = 0; i < model.textures.size(); ++i) {
            total_size += model.textures[i].GetMemorySize();
        }
        total_size += model.indices.size() * sizeof(uint32_t);
        total_size += model.vertexPositions.size() * sizeof(model.vertexPositions[0]);
        total_size += model.vertexNormals.size() * sizeof(model.vertexNormals[0]);
        total_size += model.uvCoordinates.size() * sizeof(model.vertexPositions[0]);
            
        for (size_t i = 0; i < model.meshInfos.size(); ++i) {
            total_size += model.meshInfos[i].instanceTransforms.size() * (sizeof(glm::mat4) + sizeof(uint32_t));
        }
        return total_size;
    }

    ModelRenderer::ModelDrawData ModelRenderer::AddModel(const GenericModel& model) {
        //const Model& model = *modelPtr;
        size_t uploadMemorySize = CalculateTotalRequiredStagingMemorySize(model);

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

        size_t total_added_instances = model.GetTotalInstanceCount();
        size_t offset = UploadTextures(model);
        // Copy mesh data:
        VkBufferCopy indexRegion;
        indexRegion.size = model.indices.size() * sizeof(uint32_t);
        indexRegion.srcOffset = offset;
        indexRegion.dstOffset = INDEX_BUFFER_BYTE_OFFSET + totalIndexCount * sizeof(uint32_t);
        offset = stagingBuffer.CopyData(model.indices.data(), indexRegion);

        VkBufferCopy posRegion;
        posRegion.size = model.vertexPositions.size() * sizeof(model.vertexPositions[0]);
        posRegion.srcOffset = offset;
        posRegion.dstOffset = VERTEX_POSITIONS_BYTE_OFFSET + totalVertexCount * sizeof(model.vertexPositions[0]);
        offset = stagingBuffer.CopyData(model.vertexPositions.data(), posRegion);

        VkBufferCopy normalRegion;
        normalRegion.size = model.vertexNormals.size() * sizeof(model.vertexNormals[0]);
        normalRegion.srcOffset = offset;
        normalRegion.dstOffset = VERTEX_NORMALS_BYTE_OFFSET + totalVertexCount * sizeof(model.vertexNormals[0]);
        offset = stagingBuffer.CopyData(model.vertexNormals.data(), normalRegion);

        VkBufferCopy uvRegion;
        uvRegion.size = model.uvCoordinates.size() * sizeof(model.uvCoordinates[0]);
        uvRegion.srcOffset = offset;
        uvRegion.dstOffset = VERTEX_UV_COORDS_BYTE_OFFSET + totalVertexCount * sizeof(model.uvCoordinates[0]);
        offset = stagingBuffer.CopyData(model.uvCoordinates.data(), uvRegion);

        VkBufferCopy transformsRegion;
        transformsRegion.size = total_added_instances * sizeof(glm::mat4);
        transformsRegion.srcOffset = offset;
        transformsRegion.dstOffset = INSTANCE_TRANSFORMS_BYTE_OFFSET + totalInstanceCount * sizeof(glm::mat4);
        offset += transformsRegion.size;

        VkBufferCopy imageIndexRegion;
        imageIndexRegion.size = total_added_instances * sizeof(uint32_t);
        imageIndexRegion.srcOffset = offset;
        imageIndexRegion.dstOffset = INSTANCE_TEXTURE_INDEX_BYTE_OFFSET + totalInstanceCount * sizeof(uint32_t);
        // Copy instances
        {
            size_t transformOffset = transformsRegion.srcOffset;
            size_t imageIndexOffset = imageIndexRegion.srcOffset;
            for (size_t i = 0; i < model.meshInfos.size(); ++i) {
                const GenericModel::MeshInfo& meshInfo = model.meshInfos[i];

                size_t transforms_size = meshInfo.instanceTransforms.size() * sizeof(meshInfo.instanceTransforms[0]);
                transformOffset = stagingBuffer.CopyData(meshInfo.instanceTransforms.data(), transforms_size, transformOffset);
                // TextureIndices:
                uint32_t texture_index = meshInfo.textureIndex + totalTextureCount;
                for (size_t j = 0; j < meshInfo.instanceTransforms.size(); ++j) {
                    imageIndexOffset = stagingBuffer.CopyData(&texture_index, sizeof(texture_index), imageIndexOffset);
                }
            }
        }
        
        VkBufferCopy regions[] = {
            indexRegion, posRegion, normalRegion, uvRegion, transformsRegion, imageIndexRegion 
        };

        //info("index ",i ," size: ", index_region.size, " index size offset : ", index_region.dstOffset," vertex size: ",vertex_region.size , " vertex offset : ", vertex_region.dstOffset);
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, ARRAY_SIZE(regions), regions);

        // Submitting Commands:
        vkEndCommandBuffer(commandBuffer);
        Vk::SubmitCommands(device.GetGraphicsQueue(0), commandBuffer, fence);

        //totalInstanceCount += total_added_instances;
        //totalVertexCount += model.vertexPositions.size();
        //totalIndexCount += model.indices.size();
        totalTextureCount += model.textures.size();

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
            vertexBuffer, 
            vertexBuffer, 
            vertexBuffer 
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