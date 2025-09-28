#include <SGF.hpp>
#include "Model.hpp"

namespace SGF {
    class ModelRenderer {
    public:
        struct MeshDrawData {
            uint32_t vertexCount;
            uint32_t indexCount;
            uint32_t instanceCount;
            uint32_t vertexOffset;
            uint32_t indexOffset;
            uint32_t instanceOffset;
        };
        struct ModelDrawData {
            std::vector<MeshDrawData> meshes;
        };
    public:
        void Initialize(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout);
        inline ModelRenderer(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout)
        { Initialize(renderPass, subpass, descriptorPool, uniformLayout); }
        inline ModelRenderer() {}
        ~ModelRenderer();
        ModelDrawData AddModel(const GenericModel& model);
        void DrawModel(VkCommandBuffer commands, const ModelDrawData& modelDrawData) const;
        void DrawMesh(VkCommandBuffer commands, const MeshDrawData& meshDrawData) const;
        void PrepareDrawing(VkCommandBuffer commands, VkPipeline pipeline, VkDescriptorSet uniformSet, glm::uvec2 viewportSize, uint32_t frameIndex, const glm::vec4& colorModifier = {1.f, 1.f, 1.f, 0.f});
        void BindPipeline(VkCommandBuffer commands, VkPipeline pipeline) const;
        void SetColorModifier(VkCommandBuffer commands, const glm::vec4& color) const;
        size_t GetTotalDeviceMemoryUsed() const;
        size_t GetTotalDeviceMemoryAllocated() const;
        inline size_t GetTextureCount() const { return textures.size(); }
        inline uint32_t GetTotalVertexCount() const { return totalVertexCount; }
        inline uint32_t GetTotalIndexCount() const { return totalIndexCount; }
        inline uint32_t GetTotalInstanceCount() const { return totalInstanceCount; }
        inline VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; } 
        static constexpr VkPipelineVertexInputStateCreateInfo GetPipelineVertexInput() { return MODEL_VERTEX_INPUT_INFO; }
        //void ChangePipelineSettings(VkPolygonMode polgyonMode);
        struct ModelVertex {
            ModelVertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv, const glm::vec4& color, uint32_t textureIndex);
            alignas(16) glm::vec3 position;
            uint32_t normal;
            alignas(8) glm::vec2 uv;
            glm::vec<4, uint8_t> color;
            uint32_t textureIndex;
        };
    private:
        static constexpr VkVertexInputBindingDescription MODEL_VERTEX_BINDINGS[] = {
            {0, sizeof(ModelVertex), VK_VERTEX_INPUT_RATE_VERTEX},
            {1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE},
        };
        static constexpr VkVertexInputAttributeDescription MODEL_VERTEX_ATTRIBUTES[] = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ModelVertex, position) }, // Position
            {1, 0, VK_FORMAT_A2B10G10R10_UNORM_PACK32, offsetof(ModelVertex, normal) }, // Normal
            {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ModelVertex, uv) }, // UV
            {3, 0, VK_FORMAT_R8G8B8A8_SRGB, offsetof(ModelVertex, color) }, // Vertex Color 
            {4, 0, VK_FORMAT_R32_UINT, offsetof(ModelVertex, textureIndex) }, // Texture Index
            {5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0}, // Transformation
            {6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)},
            {7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 2},
            {8, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 3},
        };

        static constexpr VkPipelineVertexInputStateCreateInfo MODEL_VERTEX_INPUT_INFO = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = FLAG_NONE,
            .vertexBindingDescriptionCount = ARRAY_SIZE(MODEL_VERTEX_BINDINGS),
            .pVertexBindingDescriptions = MODEL_VERTEX_BINDINGS,
            .vertexAttributeDescriptionCount = ARRAY_SIZE(MODEL_VERTEX_ATTRIBUTES),
            .pVertexAttributeDescriptions = MODEL_VERTEX_ATTRIBUTES,
        };

        // Images:
        std::vector<TextureImage> textures;
        // Vertex buffers:
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexDeviceMemory = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        ImageMemoryAllocator textureAllocator;
        // TransferResources:
        VkFence fence = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        StagingBuffer stagingBuffer;
        // Descriptors:
        VkDescriptorSet descriptorSets[SGF_FRAMES_IN_FLIGHT];
        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        // Pipeline:
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        //VkPipeline pipeline = VK_NULL_HANDLE;
        uint32_t totalInstanceCount = 0;
        uint32_t totalVertexCount = 0;
        uint32_t totalIndexCount = 0;
        bool descriptorInvalidated[SGF_FRAMES_IN_FLIGHT] = {};
        //size_t freeVertexBufferMemory = 0;

    private:
        void InvalidateDescriptors();
        void CheckTransferStatus();
        //void CreatePipeline(VkRenderPass renderPass, uint32_t subpass, VkPolygonMode polygonMode);
        void UpdateTextureDescriptors(uint32_t imageCount);
        void BeginTransfer(size_t uploadMemorySize);
        void FinalizeTransfer();
        size_t UploadTextures(const GenericModel& model);
        size_t UploadTexture(const TextureImage& image, const Texture& texture, size_t offset);
    };
}