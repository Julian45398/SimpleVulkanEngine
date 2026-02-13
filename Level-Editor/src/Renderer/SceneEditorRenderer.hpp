#pragma once

#include <SGF.hpp>
#include "Model.hpp"
#include "Renderer/ModelRenderer.hpp"
#include "Renderer/GridRenderer.hpp"
#include "CameraController.hpp"
#include "Viewport.hpp"
#include "Editor/Scene.hpp"
#include "Editor/SceneHierarchyPanel.hpp"

namespace SGF {
    class SceneEditorRenderer {
        struct SceneBindings {
            std::vector<ModelRenderer::ModelHandle> modelHandles;
        };
    public:
        void Initialize(VkRenderPass renderPass, uint32_t subpass, VkDescriptorPool descriptorPool, VkDescriptorSetLayout uniformLayout);
        void OnResize(uint32_t width, uint32_t height);
        void PrepareDrawing(uint32_t imageIndex);
        void RenderScene(VkCommandBuffer commands, const Scene& scene, const SceneHierarchyPanel& hierarchyPanel, const SceneBindings& bindings, uint32_t imageIndex);
        SceneBindings UploadSceneModels(const Scene& scene);
    private:
        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_BasePipeline;
        VkPipeline m_PickPipeline;
        ModelRenderer m_ModelRenderer;
        GridRenderer m_GridRenderer;
        Viewport m_Viewport;
    private:
        void BindPipeline(VkCommandBuffer commands, VkPipeline pipeline) const;
    };
}