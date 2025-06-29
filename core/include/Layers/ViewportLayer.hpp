#pragma once

#include "Layer.hpp"
#include "ImGuiLayer.hpp"
#include "Render/CommandList.hpp"
#include "Render/CameraController.hpp"
#include "Window.hpp"
#include "Render/ModelRenderer.hpp"
#include "Render/GridRenderer.hpp"
#include "Render/UniformBuffer.hpp"

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
	    void UpdateStatusWindow(const UpdateEvent& event);

        inline VkRenderPass GetRenderPass() { return renderPass; }

        virtual bool OnEvent(const KeyPressedEvent& event) override;
        //virtual bool onKeyRepeat(const KeyRepeatEvent& event) override;
        virtual bool OnEvent(const MouseMovedEvent& event) override;
        virtual bool OnEvent(const MousePressedEvent& event) override;
        virtual bool OnEvent(const MouseReleasedEvent& event) override;
        //virtual bool onMouseRelease(const MouseReleasedEvent& event) override;
        //virtual bool onMouseScroll(const MouseScrollEvent& event) override;
        //virtual bool onEvent(const KeyTypedEvent& event) override;
    private:
        enum InputMode : uint32_t {
            INPUT_NONE = 0,
            INPUT_HOVERED = BIT(0),
            INPUT_SELECTED = BIT(1),
            INPUT_CAPTURED = BIT(2)
        };
        void ResizeFramebuffer(uint32_t width, uint32_t height);
        void CreateFramebuffer();
        void DestroyFramebuffer();
        CommandList commands[FRAMES_IN_FLIGHT];
		VkRenderPass renderPass = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        VkImage colorImage = VK_NULL_HANDLE;
        VkImage depthImage = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        ImTextureID imGuiImageID = 0;
        VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
        VkImageView colorImageView = VK_NULL_HANDLE;
        VkImageView depthImageView = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        VkSemaphore signalSemaphore = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout uniformLayout = VK_NULL_HANDLE;
        UniformArray<glm::mat4> uniformBuffer;
        VkDescriptorSet uniformDescriptors[FRAMES_IN_FLIGHT];
        std::vector<Model> models;
        glm::dvec2 cursorPos;
        glm::dvec2 cursorMove;
        uint32_t width = 0;
        uint32_t height = 0;
        VkFormat imageFormat;
        uint32_t imageIndex = 0;
        CameraController cameraController;
        Cursor cursor;
        ModelRenderer modelRenderer;
        GridRenderer gridRenderer;
        float viewSize = 0.0f;
        float cameraZoom = 0.0f;
        float aspectRatio = 0.0f;
        bool isOrthographic = false;
        uint32_t inputMode = 0;
	};
}