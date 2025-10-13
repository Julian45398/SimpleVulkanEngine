#pragma once

#include <SGF.hpp>
#include "Model.hpp"
#include "Renderer/GridRenderer.hpp"
#include "Renderer/ModelRenderer.hpp"
#include "CameraController.hpp"
#include "Viewport.hpp"

namespace SGF {
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
	    void UpdateModelWindow(const UpdateEvent& event);

        inline VkRenderPass GetRenderPass() { return viewport.GetRenderPass(); }

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

        CommandList commands[SGF_FRAMES_IN_FLIGHT];
        Viewport viewport;
        VkSampler sampler = VK_NULL_HANDLE;
        ImTextureID imGuiImageID = 0;
        VkSemaphore signalSemaphore = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout uniformLayout = VK_NULL_HANDLE;
        UniformArray<glm::mat4> uniformBuffer;
        VkDescriptorSet uniformDescriptors[SGF_FRAMES_IN_FLIGHT];
        std::vector<GenericModel> models;
        std::vector<ModelRenderer::ModelHandle> modelBindOffsets;
        glm::dvec2 cursorPos;
        glm::dvec2 cursorMove;
        ImVec2 relativeCursor;
        uint32_t cursorValue = 3320;
        const GenericModel* selectedModel = nullptr;
        const GenericModel::Node* selectedNode = nullptr;
        const uint32_t* selectedMesh = nullptr;
        uint32_t imageIndex = 0;
        CameraController cameraController;
        VkPipeline renderPipeline;
        VkPipeline selectionPipeline;
        Cursor cursor;
        VkBuffer modelPickBuffer;
        VkDeviceMemory modelPickMemory;
        uint32_t* modelPickMapped;
        ModelRenderer modelRenderer;
        GridRenderer gridRenderer;
        float viewSize = 0.0f;
        float cameraZoom = 0.0f;
        bool isOrthographic = false;
        uint32_t inputMode = 0;

    private:
        void ResizeFramebuffer(uint32_t width, uint32_t height);
	    void BuildNodeTree(const GenericModel& model, const GenericModel::Node& node);
	    void DrawNode(const GenericModel& model, const GenericModel::Node& node);
	    void DrawModelNodeExcludeSelected(VkCommandBuffer commands, const GenericModel& model, const GenericModel::Node& node) const;
	};
}