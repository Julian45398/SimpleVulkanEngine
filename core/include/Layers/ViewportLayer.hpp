#pragma once

#include "Layer.hpp"
#include "ImGuiLayer.hpp"
#include "Render/CommandList.hpp"

#define SGF_FRAMES_IN_FLIGHT 2


namespace SGF {
    inline const uint32_t FRAMES_IN_FLIGHT = SGF_FRAMES_IN_FLIGHT;
	class ViewportLayer : public Layer {
    public:
        ViewportLayer(VkFormat imageFormat);
        ~ViewportLayer();
		virtual void onAttach() override;
		virtual void onDetach() override;
        virtual void onRender(RenderEvent& event) override;
        virtual void onUpdate(const UpdateEvent& event) override;
        inline VkRenderPass getRenderPass() { return renderPass; }
        //virtual bool onKeyPress(const KeyPressedEvent& event) override;
        //virtual bool onKeyRelease(const KeyReleasedEvent& event) override;
        //virtual bool onKeyRepeat(const KeyRepeatEvent& event) override;
        //virtual bool onMouseMove(const MouseMovedEvent& event) override;
        //virtual bool onMousePress(const MousePressedEvent& event) override;
        //virtual bool onMouseRelease(const MouseReleasedEvent& event) override;
        //virtual bool onMouseScroll(const MouseScrollEvent& event) override;
        //virtual bool onKeyTyped(const KeyTypedEvent& event) override;
        
    private:
        void resizeFramebuffer(uint32_t width, uint32_t height);
        void createFramebuffer();
        void destroyFramebuffer();
        CommandList commands;
		VkRenderPass renderPass = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        VkImage colorImage = VK_NULL_HANDLE;
        VkImage depthImage = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        ImTextureID imGuiImageID = 0;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
        VkImageView colorImageView = VK_NULL_HANDLE;
        VkImageView depthImageView = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkSemaphore signalSemaphore = VK_NULL_HANDLE;
        uint32_t width = 0;
        uint32_t height = 0;
        VkFormat imageFormat;
	};
}