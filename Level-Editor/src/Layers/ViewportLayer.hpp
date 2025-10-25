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
	    void UpdateDebugWindow(const UpdateEvent& event);
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
        enum class SelectionMode {
            NODE,
            MODEL,
            NO_SELECTION
        };
        struct CursorHover {
            inline CursorHover(uint32_t integer) { memcpy(this, &integer, sizeof(uint32_t)); }
            inline CursorHover(uint32_t modelIndex, uint32_t nodeIndex) : model(modelIndex), node(nodeIndex) {}
            inline bool operator==(CursorHover other) { return node == other.node && model == other.model; }
            inline bool IsValid() const { return UINT32_MAX != ToInt(); }
            inline uint32_t ToInt() const { return *(uint32_t*)this; }
            uint32_t node : 20;
            uint16_t model : 12;
        };
        static_assert(sizeof(CursorHover) == sizeof(uint32_t));

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
        //std::set<uint32_t> selectionIndices;
        uint32_t selectedModelIndex = UINT32_MAX; 
        uint32_t selectedNodeIndex = UINT32_MAX;
        glm::dvec2 cursorPos;
        glm::dvec2 cursorMove;
        ImVec2 relativeCursor;
        //uint32_t cursorValue = 3320;
        CursorHover hoverValue = CursorHover(UINT32_MAX);
        uint32_t imageIndex = 0;
        CameraController cameraController;
        VkPipeline renderPipeline;
        VkPipelineLayout pipelineLayout;
        VkPipeline outlinePipeline;
        VkPipelineLayout outlineLayout;
        Cursor cursor;
        VkBuffer modelPickBuffer;
        VkDeviceMemory modelPickMemory;
        CursorHover* modelPickMapped;
        ModelRenderer modelRenderer;
        GridRenderer gridRenderer;
        float viewSize = 0.0f;
        float cameraZoom = 0.0f;
        bool isOrthographic = false;
        uint32_t inputMode = 0;
        SelectionMode selectionMode = SelectionMode::MODEL;

    private:
        void ResizeFramebuffer(uint32_t width, uint32_t height);
	    void BuildNodeTree(const GenericModel& model, const GenericModel::Node& node);
	    void DrawTreeNode(const GenericModel& model, const GenericModel::Node& node);
	    void DrawModelNodeExcludeSelectedHierarchy(const GenericModel& model, const GenericModel::Node& node) const;
	    void DrawModelNodeRecursive(const GenericModel& model, const GenericModel::Node& node) const;
        void RenderWireframe(RenderEvent& event);
	    void RenderModel(RenderEvent& event, uint32_t modelIndex);
        void RenderModelSelection(RenderEvent& event);
        void RenderNodeSelection(RenderEvent& event);
        void BindPipeline(VkPipeline pipeline, VkPipelineLayout layout);
        void ClearSelection();
	};
}