#include <SGF.hpp>
#include "Model.hpp"

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
        typedef size_t ModelHandle;
        struct ModelDrawData {
            uint32_t indexOffset;
            uint32_t vertexOffset;
            uint32_t instanceOffset;
        };
    public:
        void Initialize(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout);
        inline ModelRenderer(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout)
        { Initialize(renderPass, subpass, descriptorPool, uniformLayout); }
        inline ModelRenderer() {}
        ~ModelRenderer();

        ModelHandle UploadModel(const GenericModel& model);

        void PrepareDrawing(uint32_t frameIndex);
        void BindBuffersToModel(VkCommandBuffer commands, ModelRenderer::ModelHandle handle) const;

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
        //inline VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; } 
        inline VkDescriptorSet GetDescriptorSet(size_t index) const { return descriptorSets[index]; }
        inline VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorLayout; }
        static constexpr VkPipelineVertexInputStateCreateInfo GetPipelineVertexInput() { return MODEL_VERTEX_INPUT_INFO; }
        
    private:
        static constexpr VkVertexInputBindingDescription MODEL_VERTEX_BINDINGS[] = {
            {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX},
            {1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE},
        };

        static constexpr VkVertexInputAttributeDescription MODEL_VERTEX_ATTRIBUTES[] = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) }, // Position
            {1, 0, VK_FORMAT_A2B10G10R10_UNORM_PACK32, offsetof(Vertex, normal) }, // Normal
            {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) }, // UV
            {3, 0, VK_FORMAT_R8G8B8A8_SRGB, offsetof(Vertex, color) }, // Vertex Color 
            {4, 0, VK_FORMAT_R32_UINT, offsetof(Vertex, textureIndex) }, // Texture Index
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
        std::vector<ModelDrawData> modelDrawData;
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
        //VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        uint32_t totalVertexCount = 0;
        uint32_t totalIndexCount = 0;
        uint32_t totalInstanceCount = 0;
        bool descriptorInvalidated[SGF_FRAMES_IN_FLIGHT] = {};
    private:
        void InvalidateDescriptors();
        void CheckTransferStatus();

        void UpdateTextureDescriptors(uint32_t imageCount);
        void BeginTransfer(const GenericModel& model);
        void FinalizeTransfer();

        size_t GetRequiredIndexMemorySize(const GenericModel& model) const;
        size_t GetRequiredInstanceMemorySize(const GenericModel& model) const;
        size_t GetRequiredVertexMemorySize(const GenericModel& model) const;
        size_t GetRequiredTextureMemorySize(const GenericModel& model) const;
        inline size_t GetTotalRequiredMemorySize(const GenericModel& model) const 
        { return GetRequiredIndexMemorySize(model) + GetRequiredVertexMemorySize(model) + GetRequiredInstanceMemorySize(model) + GetRequiredTextureMemorySize(model); }

        size_t UploadTextures(const GenericModel& model, size_t startOffset);
        size_t PrepareVertexUpload(const GenericModel& model, size_t startOffset, VkBufferCopy* pRegion);
        size_t PrepareIndexUpload(const GenericModel& model, size_t startOffset, VkBufferCopy* pRegion);
        size_t PrepareInstanceUpload(const GenericModel& model, size_t startOffset, VkBufferCopy* pRegion);
        size_t UploadTexture(const TextureImage& image, const Texture& texture, size_t offset);
    };
}