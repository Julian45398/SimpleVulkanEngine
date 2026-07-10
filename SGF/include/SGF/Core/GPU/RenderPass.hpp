#pragma once

#include "Types.hpp"

namespace SGF::GPU {
	constexpr uint32_t ATTACHMENT_UNUSED = ~0U;
	constexpr uint32_t SUBPASS_EXTERNAL = ~0U;
	class RenderPassBuilder {
	public:
		RenderPass Build();

		RenderPassBuilder& AddAttachment(
			Format format,
			SampleCount samples,
			ImageLayout initialLayout,
			ImageLayout finalLayout,
			AttachmentLoad loadOp = AttachmentLoad::CLEAR,
			AttachmentStore storeOp = AttachmentStore::STORE,
			AttachmentLoad stencilLoadOp = AttachmentLoad::DONT_CARE,
			AttachmentStore stencilStoreOp = AttachmentStore::DONT_CARE);

		RenderPassBuilder& AddColorAttachment(
			Format format,
			SampleCount samples,
			ImageLayout initialLayout,
			ImageLayout finalLayout,
			AttachmentLoad loadOp = AttachmentLoad::CLEAR,
			AttachmentStore storeOp = AttachmentStore::STORE);

		RenderPassBuilder& AddDepthAttachment(
			Format format,
			SampleCount samples,
			ImageLayout initialLayout = ImageLayout::UNDEFINED,
			ImageLayout finalLayout = ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			AttachmentLoad loadOp = AttachmentLoad::CLEAR,
			AttachmentStore storeOp = AttachmentStore::STORE,
			AttachmentLoad stencilLoadOp = AttachmentLoad::DONT_CARE,
			AttachmentStore stencilStoreOp = AttachmentStore::DONT_CARE);

		RenderPassBuilder& AddSubpass();

		RenderPassBuilder& AddSubpass(
			Flags<PipelineStage> pipelineStages,
			Flags<Access> accessTypes,
			Flags<PipelineStage> nextPipelineStages,
			Flags<Access> nextAccessTypes,
			Flags<Dependency> dependencyFlags = Dependency::NONE);

		RenderPassBuilder& AddColorReference(uint32_t attachmentIndex, ImageLayout layout);
		RenderPassBuilder& SetColorReference(uint32_t referenceIndex, uint32_t attachmentIndex, ImageLayout layout);
		RenderPassBuilder& ClearColorReferences();
		RenderPassBuilder& ClearColorReferences(uint32_t subpassIndex);

		RenderPassBuilder& AddInputReference(uint32_t attachmentIndex, ImageLayout layout);
		RenderPassBuilder& SetInputReference(uint32_t referenceIndex, uint32_t attachmentIndex, ImageLayout layout);
		RenderPassBuilder& ClearInputReferences();
		RenderPassBuilder& ClearInputReferences(uint32_t subpassIndex);

		RenderPassBuilder& DepthStencilReference(uint32_t attachmentIndex, ImageLayout layout);
		RenderPassBuilder& SetDepthStencilReference(uint32_t referenceIndex, uint32_t attachmentIndex, ImageLayout layout);
		RenderPassBuilder& ClearDepthStencilReference();
		RenderPassBuilder& ClearDepthStencilReference(uint32_t subpassIndex);

		RenderPassBuilder& AddResolveReference(uint32_t attachmentIndex, ImageLayout layout);
		RenderPassBuilder& SetResolveReference(uint32_t referenceIndex, uint32_t attachmentIndex, ImageLayout layout);
		RenderPassBuilder& ClearResolveReferences();
		RenderPassBuilder& ClearResolveReferences(uint32_t subpassIndex);

		RenderPassBuilder& AddPreserveReference(uint32_t attachmentIndex);
		RenderPassBuilder& ClearPreserveReferences();
		RenderPassBuilder& ClearPreserveReferences(uint32_t subpassIndex);

		RenderPassBuilder& ClearAttachments();
		RenderPassBuilder& ClearReferences();
		RenderPassBuilder& ClearSubpasses();

		/**
		* Subpass Dependencies:
		*  - Used for synchronization between subpasses and external operations.
		*  - Each dependency specifies a source and destination subpass, along with the stages and access types involved.
		*  - For external dependencies, use SUBPASS_EXTERNAL as the source or destination subpass index.
		*/

		RenderPassBuilder& AddSubpassDependency(
			uint32_t srcSubpass,
			uint32_t dstSubpass,
			Flags<PipelineStage> srcStages,
			Flags<PipelineStage> dstStages,
			Flags<Access> srcAccesses,
			Flags<Access> dstAccesses,
			Flags<Dependency> dependencyFlags = Dependency::NONE);

		/**
		* Getters:
		*/
		uint32_t GetAttachmentCount() const;
		uint32_t GetSubpassCount() const;
		uint32_t GetDependencyCount() const;
		uint32_t GetColorReferenceCount() const;
		uint32_t GetColorReferenceCount(uint32_t subpassIndex) const;
		uint32_t GetInputReferenceCount() const;
		uint32_t GetInputReferenceCount(uint32_t subpassIndex) const;
		uint32_t GetResolveReferenceCount() const;
		uint32_t GetResolveReferenceCount(uint32_t subpassIndex) const;
		bool HasDepthStencilReference() const;
		bool HasDepthStencilReference(uint32_t subpassIndex) const;

		RenderPassBuilder(const RenderPassBuilder& other);
		RenderPassBuilder(RenderPassBuilder&& other);
		~RenderPassBuilder();
	private:
		inline RenderPassBuilder(void* handle) : m_Handle(handle) {}
		void FinalizeBuildData();
	private:
		void* m_Handle = nullptr;
	};
}