#pragma once

#include <SGF.hpp>
#include "Model.hpp"
#include "Renderer/GridRenderer.hpp"
#include "Renderer/ModelRenderer.hpp"
#include "Renderer/EditorRenderer.hpp"
#include "CameraController.hpp"
#include "Viewport.hpp"
#include "DebugWindow.hpp"
#include "ModelSelectionCPU.hpp"
#include "AnimationController.hpp"
#include <future>

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
        std::vector<AnimationController> animationControllers;
        std::vector<std::unique_ptr<GenericModel>> models;
        //std::set<uint32_t> selectionIndices;
        uint32_t selectedModelIndex = UINT32_MAX; 
        uint32_t selectedNodeIndex = UINT32_MAX;
        glm::dvec2 cursorPos;
        glm::dvec2 cursorMove;
        ImVec2 relativeCursor;
        CameraController cameraController;
        std::future<std::unique_ptr<GenericModel>> loadingModel;
        
        float viewSize = 0.0f;
        float cameraZoom = 0.0f;
        bool isOrthographic = false;
        bool doCPUModelIntersection = false;
        uint32_t inputMode = 0;
        SelectionMode selectionMode = SelectionMode::MODEL;
		Profiler profiler;
        DebugWindow debugPanel;
        EditorRenderer editorRenderer;
		DebugRenderer debugRenderer;
        HitInfo hitInfo;

    private:
		void ImportModel(const char* filename);
        void CheckModelImportStatus();
	    void DrawTreeNode(uint32_t model, const GenericModel::Node& node);
	    void DrawModelNodeExcludeSelectedHierarchy(const GenericModel& model, const GenericModel::Node& node) const;
	    void DrawModelNodeRecursive(const GenericModel& model, const GenericModel::Node& node) const;
        void ShowSelectionInformation();
        void ShowModelHierarchy();
        void UpdateAnimations(const UpdateEvent& event);
        void BindPipeline(VkPipeline pipeline, VkPipelineLayout layout);
        void ClearSelection();
        void UseGuizmo();
        void TestSelectionAlgorithms();
	};
}