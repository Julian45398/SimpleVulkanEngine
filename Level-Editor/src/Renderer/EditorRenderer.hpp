#pragma once

#include <SGF.hpp>
#include "Model.hpp"
#include "Renderer/GridRenderer.hpp"
#include "Renderer/ModelRenderer.hpp"
#include "CameraController.hpp"
#include "Viewport.hpp"
#include "DebugWindow.hpp"
#include "ModelSelectionCPU.hpp"
#include "AnimationController.hpp"

namespace SGF {
	class EditorRenderer {
    public:
		EditorRenderer(VkFormat imageFormat);
        ~EditorRenderer();


		inline uint32_t GetHoveredModelIndex() const { return hoverValue.GetModelIndex(); }
		inline uint32_t GetHoveredNodeIndex() const { return hoverValue.GetNodeIndex(); }
        inline VkRenderPass GetRenderPass() const { return viewport.GetRenderPass(); }
		inline uint32_t GetSubpass() const { return 0; }
		inline ImTextureID GetImGuiTextureID() const { return imGuiImageID; }
		inline const Viewport& GetViewport() const { return viewport; }
		inline float GetAspectRatio() const { return viewport.GetAspectRatio(); }
		inline float GetWidth() const { return viewport.GetWidth(); }
        inline float GetHeight() const { return viewport.GetHeight(); }
		inline bool IsCursorHoveringItem() const { return hoverValue.IsValid(); }
		inline const CommandList& GetCurrentCommandBuffer() const { return commands[imageIndex]; }

		inline void AddModel(const GenericModel& model) { modelRenderer.UploadModel(model); }
		inline void UpdateInstanceTransforms(const GenericModel& model) { modelRenderer.UpdateInstanceTransforms(model); }
	    inline void UpdateBoneTransforms(const GenericModel& model, const std::vector<glm::mat4>& boneTransforms) { modelRenderer.UpdateBoneTransforms(model, boneTransforms); }
        void BeginFrame(RenderEvent& event, const glm::mat4& viewProj);
		void EndFrame(RenderEvent& event, glm::uvec2 pixelPos);
        inline void BindStaticRenderPipeline() { BindStaticPipeline(staticRenderPipeline, staticRenderPipelineLayout); };
        inline void BindOutlinePipeline() { BindStaticPipeline(outlinePipeline, outlineLayout); }
        inline void BindSkeletalRenderPipeline() { BindSkeletalPipeline(skeletalRenderPipeline, skeletalRenderPipelineLayout); }
        //inline void BindSkeletalOutlinePipeline() { BindSkeletalPipeline(outlinePipeline, outlineLayout); }
        void SetColorModifier(const glm::vec4& colorModifier) const;
        void SetModelTransparency(float transparency) const;
        void SetModifiers(const glm::vec4& colorModifer, float transparency) const;
        void DrawModel(const GenericModel& model, uint32_t modelIndex);
        void DrawModelNodeRecursive(const GenericModel& model, uint32_t modelIndex, const GenericModel::Node& node) const;
        void DrawModelExcludeNode(const GenericModel& model, uint32_t modelIndex, const GenericModel::Node& node) const;
        void DrawNodeRecursiveExcludeNode(const GenericModel& model, uint32_t modelIndex, const GenericModel::Node& currentNode, const GenericModel::Node& excludedNode) const;
        void DrawNodeOutline(const GenericModel& model, const GenericModel::Node& node);
        void DrawModelOutline(const GenericModel& model);
		void DrawGrid();
		void ResizeFramebuffer(uint32_t w, uint32_t h);
    private:
        struct CursorHover {
            inline CursorHover(uint32_t integer) { memcpy(this, &integer, sizeof(uint32_t)); }
            inline CursorHover(uint32_t modelIndex, uint32_t nodeIndex) : model(modelIndex), node(nodeIndex) {}
            inline bool operator==(CursorHover other) { return node == other.node && model == other.model; }
            inline bool IsValid() const { return UINT32_MAX != ToInt(); }
            inline uint32_t ToInt() const { return *(uint32_t*)this; }
			inline uint32_t GetModelIndex() const { return (uint32_t)model; }
			inline uint32_t GetNodeIndex() const { return (uint32_t)node; }
            uint32_t node : 20;
            uint32_t model : 12;
        };
        static_assert(sizeof(CursorHover) == sizeof(uint32_t));

        void DrawNodeRecursiveExcludeNodePrivate(const GenericModel& model, uint32_t modelIndex, const GenericModel::Node& currentNode, const GenericModel::Node& excludedNode) const;
        void SetCurrentID(CursorHover currentID) const;
		void BindStaticPipeline(VkPipeline pipeline, VkPipelineLayout pipelineLayout);
        void BindSkeletalPipeline(VkPipeline pipeline, VkPipelineLayout layout);
        CommandList commands[SGF_FRAMES_IN_FLIGHT];
        Viewport viewport;
        VkSampler sampler = VK_NULL_HANDLE;
        ImTextureID imGuiImageID = 0;
        VkSemaphore signalSemaphore = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout uniformLayout = VK_NULL_HANDLE;
        UniformArray<glm::mat4> uniformBuffer;
        VkDescriptorSet uniformDescriptors[SGF_FRAMES_IN_FLIGHT];

        VkPipeline staticRenderPipeline;
        VkPipelineLayout staticRenderPipelineLayout;
		VkPipeline skeletalRenderPipeline;
        VkPipelineLayout skeletalRenderPipelineLayout;
        VkPipeline outlinePipeline;
        VkPipelineLayout outlineLayout;
        //Cursor cursor;
        VkBuffer modelPickBuffer;
        VkDeviceMemory modelPickMemory;
        CursorHover* modelPickMapped;
        ModelRenderer modelRenderer;
        GridRenderer gridRenderer;

        CursorHover hoverValue;
        uint32_t imageIndex = 0;
	};
}