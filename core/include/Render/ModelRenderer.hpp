#include "SGF_Core.hpp"
#include "Model.hpp"
#include "Render/ImageMemoryAllocator.hpp"

namespace SGF {
    class ModelRenderer {
    public:
        static constexpr uint32_t MAX_TEXTURE_COUNT = 128;
        ModelRenderer(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout);
        ~ModelRenderer();
        void addModel(const Model& model);
        void draw(VkCommandBuffer commands, VkDescriptorSet uniformSet, uint32_t viewportWidth, uint32_t viewportHeight);
        void changePipelineSettings(VkPolygonMode polgyonMode);
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
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexDeviceMemory;
        // Images:
        VkSampler sampler;
        std::vector<TextureImage> textures;
        ImageMemoryAllocator textureAllocator;
        // TransferResources:
        VkFence fence;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;
        uint8_t* stagingMapped;
        // Descriptors:
        struct Descriptor {
            VkDescriptorSet set;
            bool invalidated;
        };
        VkDescriptorSetLayout descriptorLayout;
        std::vector<Descriptor> descriptorSets;
        // Pipeline:
        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;
        uint32_t transformCount;
        uint32_t vertexCount;
        uint32_t indexCount;
        size_t freeVertexBufferMemory;

        void invalidateDescriptors();
        void checkTransferStatus();
        void createPipeline(VkRenderPass renderPass, uint32_t subpass, VkPolygonMode polygonMode);
        void updateTextureDescriptors();
    };
}