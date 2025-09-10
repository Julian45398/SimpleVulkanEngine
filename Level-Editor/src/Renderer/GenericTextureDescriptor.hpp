#pragma once

#include <SGF.hpp>

namespace SGF {
    class GenericTextureResource {
    public:
        class GenericTextureDescriptor {
        public:
            inline operator VkDescriptorSet() const { return set; }
            inline VkDescriptorSet Get() const { return set; }
            void UpdateSampler(VkSampler sampler);
            void UpdateTexture(VkImageView imageView, uint32_t position = 0);
            void UpdateTextures(const VkImageView* imageViews, uint32_t textureCount, uint32_t textureOffset = 0);
        private:
            VkDescriptorSet set;
        };
        GenericTextureResource(VkDescriptorPool descriptorPool, VkSampler sampler, uint32_t maxTextureCount);
        ~GenericTextureResource();
        inline VkDescriptorSetLayout GetLayout() const { return descriptorSetLayout; }
        inline GenericTextureDescriptor Get(size_t index) const { return descriptorSets[index]; }
        inline GenericTextureDescriptor operator[](size_t index) const { return descriptorSets[index]; }
    private:
        GenericTextureDescriptor descriptorSets[SGF_FRAMES_IN_FLIGHT];
        VkDescriptorSetLayout descriptorSetLayout;
    };
}