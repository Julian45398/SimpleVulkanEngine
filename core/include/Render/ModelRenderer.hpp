#include "SGF_Core.hpp"
#include "Model.hpp"
#include "Render/ImageMemoryAllocator.hpp"

namespace SGF {
    class ModelRenderer {
    public:
        static constexpr uint32_t MAX_TEXTURE_COUNT = 128;
        void Initialize(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout, uint32_t imageCount);
        inline ModelRenderer(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout, uint32_t imageCount)
        { Initialize(renderPass, subpass, descriptorPool, uniformLayout, imageCount); }
        inline ModelRenderer() {}
        ~ModelRenderer();
        void AddModel(const Model& model);
        void Draw(VkCommandBuffer commands, VkDescriptorSet uniformSet, uint32_t viewportWidth, uint32_t viewportHeight, uint32_t imageIndex);
        void ChangePipelineSettings(VkPolygonMode polgyonMode);
    private:
        struct ModelDrawData {
            struct MeshDrawData {
                uint32_t vertexCount;
                //uint32_t vertexOffset;
                uint32_t indexCount;
                //uint32_t indexOffset;
                uint32_t imageIndex;
                //uint32_t imageOffset;
                uint32_t instanceCount;
                //uint32_t instanceOffset;
            };
            std::vector<MeshDrawData> meshes;
            uint32_t instanceCount;
            uint32_t imageCount;
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
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
        uint8_t* stagingMapped = nullptr;
        // Descriptors:
        struct Descriptor {
            VkDescriptorSet set;
            bool invalidated;
        };
        VkDescriptorSetLayout descriptorLayout = VK_NULL_HANDLE;
        std::vector<Descriptor> descriptorSets;
        // Pipeline:
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        uint32_t transformCount = 0;
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
        uint32_t imageCount = 0;
        size_t freeVertexBufferMemory = 0;

        void InvalidateDescriptors();
        void CheckTransferStatus();
        void CreatePipeline(VkRenderPass renderPass, uint32_t subpass, VkPolygonMode polygonMode);
        void UpdateTextureDescriptors(uint32_t imageCount);
    };
}