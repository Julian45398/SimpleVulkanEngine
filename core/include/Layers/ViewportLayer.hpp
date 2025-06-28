#pragma once

#include "Layer.hpp"
#include "ImGuiLayer.hpp"
#include "Render/CommandList.hpp"
#include "Render/CameraController.hpp"

#define SGF_FRAMES_IN_FLIGHT 2


namespace SGF {
    inline const uint32_t FRAMES_IN_FLIGHT = SGF_FRAMES_IN_FLIGHT;
	class ViewportLayer : public Layer {
    public:
        ViewportLayer(VkFormat imageFormat);
        ~ViewportLayer();
		virtual void OnAttach() override;
		virtual void OnDetach() override;
        virtual void OnEvent(RenderEvent& event) override;
        virtual void OnEvent(const UpdateEvent& event) override;
        virtual void OnEvent(const WindowResizeEvent& event) override;

        void RenderViewport(RenderEvent& event);
	    void UpdateViewport(const UpdateEvent& event);

        inline VkRenderPass GetRenderPass() { return renderPass; }
        //virtual bool onKeyPress(const KeyPressedEvent& event) override;
        //virtual bool onKeyRelease(const KeyReleasedEvent& event) override;
        //virtual bool onKeyRepeat(const KeyRepeatEvent& event) override;
        //virtual bool onMouseMove(const MouseMovedEvent& event) override;
        //virtual bool onMousePress(const MousePressedEvent& event) override;
        //virtual bool onMouseRelease(const MouseReleasedEvent& event) override;
        //virtual bool onMouseScroll(const MouseScrollEvent& event) override;
        //virtual bool onEvent(const KeyTypedEvent& event) override;
    private:
        void ResizeFramebuffer(uint32_t width, uint32_t height);
        void CreateFramebuffer();
        void DestroyFramebuffer();
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
        CameraController cameraController;
	};
}