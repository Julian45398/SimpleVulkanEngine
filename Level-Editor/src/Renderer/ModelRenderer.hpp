#pragma once

#include <SGF.hpp>
#include "Model/Model.hpp"


namespace SGF {
    class ModelRenderer {
    public:
        struct Vertex {
            Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color, uint32_t textureIndex);
            alignas(16) glm::vec3 position;
            uint32_t normal;
            alignas(8) glm::vec2 uv;
            glm::vec<4, uint8_t> color;
            uint32_t textureIndex;
        };
        struct ModelDrawData {
            uint32_t indexOffset;
            uint32_t vertexOffset;
            uint32_t instanceOffset;
            uint32_t vertexWeightOffset;
            uint32_t boneTransformsOffset;
        };
    public:
        void Initialize(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout);
        inline ModelRenderer(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout) : boneTransformsRingBuffer(MemorySize::KB_64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        { Initialize(renderPass, subpass, descriptorPool, uniformLayout); }
        inline ModelRenderer() : boneTransformsRingBuffer(MemorySize::KB_64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {}
        ~ModelRenderer();

        void UploadModel(const GenericModel& model);
        void UpdateInstanceTransforms(const GenericModel& model);
        void UpdateBoneTransforms(const GenericModel& model, const glm::mat4* pBoneTransforms, size_t count);
        inline void UpdateBoneTransforms(const GenericModel& model, const std::vector<glm::mat4>& boneTransforms) { UpdateBoneTransforms(model, boneTransforms.data(), boneTransforms.size()); }

        void PrepareDrawing(uint32_t frameIndex);
        bool BindBuffersToModel(VkCommandBuffer commands, const GenericModel& model) const;
        void BindPipeline(VkCommandBuffer commands, VkPipeline pipeline) const;

        void DrawModel(VkCommandBuffer commands, const GenericModel& model) const;
        void DrawNodeRecursive(VkCommandBuffer commands, const GenericModel& model, const GenericModel::Node& node) const;
        void DrawNode(VkCommandBuffer commands, const GenericModel& model, const GenericModel::Node& node) const;
        void DrawMesh(VkCommandBuffer commands, const GenericModel::Node& node, const GenericModel::Mesh& mesh) const;

        //void SetColorModifier(VkCommandBuffer commands, const glm::vec4& color = { 1.f, 1.f, 1.f, 1.f}) const;
        //void SetMeshTransform(VkCommandBuffer commands, const glm::mat4& transform) const;

        size_t GetTotalDeviceMemoryUsed() const;
        size_t GetTotalDeviceMemoryAllocated() const;
        inline size_t GetTextureCount() const { return textures.size(); }
        inline uint32_t GetTotalVertexCount() const { return totalVertexCount; }
        inline uint32_t GetTotalIndexCount() const { return totalIndexCount; }
        size_t GetBoneTransformsOffset(const GenericModel& model) const;
        size_t GetBoneVertexWeightsOffset(const GenericModel& model) const;
        //inline VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; } 
        inline VkDescriptorSet GetTextureDescriptorSet(size_t index) const {
            assert(index < (sizeof(descriptorSets) / sizeof(descriptorSets[0])));
            return descriptorSets[index]; 
        }
        inline VkDescriptorSet GetBoneDescriptorSet(size_t index) const { 
            assert(index < (sizeof(boneTransformsDescriptors) / sizeof(boneTransformsDescriptors[0])));
            return boneTransformsDescriptors[index]; 
        }
        inline VkDescriptorSetLayout GetTextureDescriptorSetLayout() const { return textureDescriptorLayout; }
        inline VkDescriptorSetLayout GetBoneDescriptorSetLayout() const { return boneDescriptorLayout; }
        static const VkPipelineVertexInputStateCreateInfo GetStaticModelVertexInput();
        static const VkPipelineVertexInputStateCreateInfo GetSkeletalModelVertexInput();

        const ModelDrawData& GetDrawData(const GenericModel& model) const;
    private:
        // Images:
        std::vector<TextureImage> textures;
        //std::vector<ModelDrawData> modelDrawData;
		std::unordered_map<const GenericModel*, ModelDrawData> modelDrawData;
		HostCoherentRingBuffer<SGF_FRAMES_IN_FLIGHT> boneTransformsRingBuffer;
        // Vertex buffers:
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexDeviceMemory = VK_NULL_HANDLE;
        VkBuffer vertexWeightsBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexWeightsMemory = VK_NULL_HANDLE;
        size_t allocatedVertexWeightsSize = 0;
        VkSampler sampler = VK_NULL_HANDLE;
        ImageMemoryAllocator textureAllocator;
        // TransferResources:
        VkFence fence = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        StagingBuffer stagingBuffer;
        const GenericModel* uploadingModel = nullptr;
        // Descriptors:
        VkDescriptorSet descriptorSets[SGF_FRAMES_IN_FLIGHT];
        VkDescriptorSet boneTransformsDescriptors[SGF_FRAMES_IN_FLIGHT];
        VkDescriptorSetLayout textureDescriptorLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout boneDescriptorLayout = VK_NULL_HANDLE;
        // Pipeline:
        //VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        uint32_t totalVertexCount = 0;
        uint32_t totalIndexCount = 0;
        uint32_t totalInstanceCount = 0;
        uint32_t totalWeightCount = 0;
        uint32_t totalBoneCount = 0;
        bool descriptorInvalidated[SGF_FRAMES_IN_FLIGHT] = {};
    private:
        void InvalidateDescriptors();
        void CheckTransferStatus();

        void UpdateTextureDescriptors(uint32_t imageCount);
        void BeginTransfer(const GenericModel& model);
        void FinalizeTransfer();

        size_t UploadTextures(const GenericModel& model, size_t startOffset);
        size_t PrepareVertexUpload(const GenericModel& model, size_t startOffset, VkBufferCopy* pRegion);
        size_t PrepareIndexUpload(const GenericModel& model, size_t startOffset, VkBufferCopy* pRegion);
        size_t PrepareInstanceUpload(const GenericModel& model, size_t offset, VkBufferCopy* pRegion);
        size_t UploadVertexWeights(const GenericModel& model, size_t startOffset);
        size_t UploadTexture(const TextureImage& image, const Texture& texture, size_t offset);
    };
}