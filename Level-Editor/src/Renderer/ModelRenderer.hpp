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
        inline VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; } 
        static constexpr VkPipelineVertexInputStateCreateInfo GetPipelineVertexInput() { return MODEL_VERTEX_INPUT_INFO; }
        //void ChangePipelineSettings(VkPolygonMode polgyonMode);
    private:
        static constexpr VkVertexInputBindingDescription MODEL_VERTEX_BINDINGS[] = {
            {0, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX},
            {1, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX},
            {2, sizeof(glm::vec2), VK_VERTEX_INPUT_RATE_VERTEX},
            {3, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE},
            {4, sizeof(uint32_t), VK_VERTEX_INPUT_RATE_INSTANCE},
        };
        static constexpr VkVertexInputAttributeDescription MODEL_VERTEX_ATTRIBUTES[] = {
            {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 }, // Position
            {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0 }, // Normal
            {2, 2, VK_FORMAT_R32G32_SFLOAT, 0 }, // UV
            {3, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0}, // Transformation
            {4, 3, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4)},
            {5, 3, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 2},
            {6, 3, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 3},
            {7, 4, VK_FORMAT_R32_UINT, 0}, // Texture index
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
        uint32_t totalTextureCount = 0;
        bool descriptorInvalidated[SGF_FRAMES_IN_FLIGHT] = {};
        //size_t freeVertexBufferMemory = 0;

    private:
        void InvalidateDescriptors();
        void CheckTransferStatus();
        //void CreatePipeline(VkRenderPass renderPass, uint32_t subpass, VkPolygonMode polygonMode);
        void UpdateTextureDescriptors(uint32_t imageCount);
        size_t UploadTextures(const GenericModel& model);
    };
}