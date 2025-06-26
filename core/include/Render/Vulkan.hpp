#pragma once


namespace SGF {
    namespace Vk {
        inline VkClearValue CreateDepthClearValue(float depth, uint32_t stencil) { VkClearValue value; value.depthStencil.depth = depth; value.depthStencil.stencil = stencil; return value; }
        inline VkClearValue CreateColorClearValue(float r, float g, float b, float a) { return { r, g, b , a }; }
    } // namespace VK
} // namespace SGF 