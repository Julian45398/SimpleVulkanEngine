#include <SGF.hpp>
#include "Model.hpp"

namespace SGF {
    class ModelRenderer {
    public:
        void Initialize(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout);
        inline ModelRenderer(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout)
        { Initialize(renderPass, subpass, descriptorPool, uniformLayout); }
        inline ModelRenderer() {}
        ~ModelRenderer();
        void AddModel(const GenericModel& model);
        void Draw(VkCommandBuffer commands, VkDescriptorSet uniformSet, uint32_t viewportWidth, uint32_t viewportHeight, uint32_t frameIndex);
        void ChangePipelineSettings(VkPolygonMode polgyonMode);
    private:
        struct ModelDrawData {
            struct MeshDrawData {
                uint32_t vertexCount;
                uint32_t indexCount;
                uint32_t instanceCount;
            };
            std::vector<MeshDrawData> meshes;
        };
        // Models:
        std::vector<ModelDrawData> models;
        // Vertex buffers:
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexDeviceMemory = VK_NULL_HANDLE;
        // Images:
        VkSampler sampler = VK_NULL_HANDLE;
        std::vector<TextureImage> textures;
        ImageMemoryAllocator textureAllocator;
        // TransferResources:
        VkFence fence = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        StagingBuffer stagingBuffer;
        // Descriptors:
        VkDescriptorSet descriptorSets[SGF_FRAMES_IN_FLIGHT];
        bool descriptorInvalidated[SGF_FRAMES_IN_FLIGHT] = {};
        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        // Pipeline:
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        uint32_t totalInstanceCount = 0;
        uint32_t totalVertexCount = 0;
        uint32_t totalIndexCount = 0;
        uint32_t totalTextureCount = 0;
        //size_t freeVertexBufferMemory = 0;

    private:
        void InvalidateDescriptors();
        void CheckTransferStatus();
        void CreatePipeline(VkRenderPass renderPass, uint32_t subpass, VkPolygonMode polygonMode);
        void UpdateTextureDescriptors(uint32_t imageCount);
        size_t UploadTextures(const GenericModel& model);
    };
}