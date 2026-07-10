#pragma once
#include <stdint.h>
#include <array>
#include <vector>
#include "Types.hpp"

#include <SGF/Core/Flags.hpp>

namespace SGF {
	namespace GPU {
		class CommandList {
		public:
			// -------------------------------------------------------------------------
			// Lifecycle
			// -------------------------------------------------------------------------
			void Reset();
			void Begin(Flags<CommandBufferUsage> usage = CommandBufferUsage::NONE);
			void BeginSecondary(RenderPass renderPass, uint32_t subpassIndex, Framebuffer framebuffer, Flags<CommandBufferUsage> usage);
			void End();

			// -------------------------------------------------------------------------
			// Render Pass
			// -------------------------------------------------------------------------
			void BeginRenderPass(RenderPass renderPass);
			void EndRenderPass();
			void NextSubpass();

			// -------------------------------------------------------------------------
			// Draw commands
			// -------------------------------------------------------------------------
			void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
			void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
			void DrawIndirect(Buffer indirectBuffer, size_t offset, uint32_t drawCount, uint32_t stride);
			void DrawIndexedIndirect(Buffer indirectBuffer, size_t offset, uint32_t drawCount, uint32_t stride);

			// Draw indirect with count (requires VK_KHR_draw_indirect_count / Vulkan 1.2)
			void DrawIndirectCount(Buffer indirectBuffer, size_t offset, Buffer countBuffer, size_t countBufferOffset, uint32_t maxDrawCount, uint32_t stride);
			void DrawIndexedIndirectCount(Buffer indirectBuffer, size_t offset, Buffer countBuffer, size_t countBufferOffset, uint32_t maxDrawCount, uint32_t stride);

			// Mesh shading draw commands (requires VK_EXT_mesh_shader)
			void DrawMeshTasks(uint32_t groupCountX, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
			void DrawMeshTasksIndirect(Buffer indirectBuffer, size_t offset, uint32_t drawCount, uint32_t stride);
			void DrawMeshTasksIndirectCount(Buffer indirectBuffer, size_t offset, Buffer countBuffer, size_t countBufferOffset, uint32_t maxDrawCount, uint32_t stride);

			// -------------------------------------------------------------------------
			// Compute / Ray Tracing
			// -------------------------------------------------------------------------
			void Dispatch(uint32_t groupCountX, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
			void DispatchIndirect(Buffer indirectBuffer, size_t offset);
			void DispatchBase(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

			// Ray tracing (requires VK_KHR_ray_tracing_pipeline)
			void TraceRays(const StridedDeviceAddressRegion& raygenSBT, const StridedDeviceAddressRegion& missSBT, const StridedDeviceAddressRegion& hitSBT, const StridedDeviceAddressRegion& callableSBT, uint32_t width, uint32_t height, uint32_t depth = 1);
			void TraceRaysIndirect(const StridedDeviceAddressRegion& raygenSBT, const StridedDeviceAddressRegion& missSBT, const StridedDeviceAddressRegion& hitSBT, const StridedDeviceAddressRegion& callableSBT, uint64_t indirectDeviceAddress);

			// -------------------------------------------------------------------------
			// Pipeline & descriptor binding
			// -------------------------------------------------------------------------
			void BindVertexBuffer(uint32_t binding, Buffer vertexBuffer, size_t offset = 0);
			void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const Buffer* vertexBuffers, const size_t* offsets);
			void BindIndexBuffer32Bit(Buffer indexBuffer, size_t offset);
			void BindIndexBuffer16Bit(Buffer indexBuffer, size_t offset);
			void BindIndexBuffer8Bit(Buffer indexBuffer, size_t offset);   // requires VK_KHR_index_type_uint8
			void BindPipeline(ComputePipeline pipeline);
			void BindPipeline(GraphicsPipeline pipeline);
			void BindPipeline(RayTracingPipeline pipeline);                // requires VK_KHR_ray_tracing_pipeline

			// Push constants
			void PushConstants(PipelineLayout layout, Flags<ShaderStage> stageFlags, uint32_t offset, uint32_t size, const void* pValues);
			template<typename T>
			inline void PushConstants(PipelineLayout layout, Flags<ShaderStage> stageFlags, uint32_t offset, const T& value) {
				PushConstants(layout, stageFlags, offset, sizeof(T), &value);
			}

#ifdef SGF_GPU_EXTENDED_FUNCTIONS
			// Push descriptor sets (requires VK_KHR_push_descriptor)
			void PushDescriptorSet(PipelineBindPoint bindPoint, PipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const WriteDescriptorSet* descriptorWrites);
#endif
			// Descriptor set binding
			void BindDescriptorSet(PipelineType type, PipelineLayout layout, uint32_t setIndex, DescriptorSet set);
			void BindDescriptorSets(PipelineType type, PipelineLayout layout, const DescriptorSet* sets, uint32_t count, uint32_t firstSet);
			template<uint32_t COUNT>
			inline void BindDescriptorSets(PipelineType type, PipelineLayout layout, const DescriptorSet(&descriptorSets)[COUNT], uint32_t firstSet) { BindDescriptorSets(type, layout, descriptorSets, COUNT, firstSet); }
			template<uint32_t COUNT>
			inline void BindDescriptorSets(PipelineType type, PipelineLayout layout, const std::array<DescriptorSet, COUNT>& descriptorSets, uint32_t firstSet) { BindDescriptorSets(type, layout, descriptorSets.data(), COUNT, firstSet); }
			inline void BindDescriptorSets(PipelineType type, PipelineLayout layout, const std::vector<DescriptorSet>& descriptorSets, uint32_t firstSet) { BindDescriptorSets(type, layout, descriptorSets.data(), (uint32_t)descriptorSets.size(), firstSet); }

			// -------------------------------------------------------------------------
			// Dynamic state
			// -------------------------------------------------------------------------
			void SetViewport(SGF::GPU::Viewport viewport);
			void SetViewport(float width, float height, float xoffset, float yoffset, float minDepth, float maxDepth);
			void SetViewports(const SGF::GPU::Viewport* viewport, uint32_t count);
			template<uint32_t COUNT>
			inline void SetViewports(std::array<SGF::GPU::Viewport, COUNT> viewports) { SetViewports(viewports.data(), COUNT); }
			template<uint32_t COUNT>
			inline void SetViewports(const SGF::GPU::Viewport(&viewports)[COUNT]) { SetViewports(viewports, COUNT); }
			inline void SetViewports(const std::vector<SGF::GPU::Viewport>& viewports) { SetViewports(viewports.data(), (uint32_t)viewports.size()); }

			void SetScissor(const Rect2D& scissor);
			inline void SetScissor(uint32_t width, uint32_t height, int32_t xOffset, int32_t yOffset) { SetScissor({ {xOffset, yOffset}, {width, height} }); }
			void SetScissors(const Rect2D* scissors, uint32_t count);
			template<uint32_t COUNT>
			inline void SetScissors(const std::array<Rect2D, COUNT>& scissors) { SetScissors(scissors.data(), COUNT); }
			inline void SetScissors(const std::vector<Rect2D>& scissors) { SetScissors(scissors.data(), (uint32_t)scissors.size()); }

			void SetLineWidth(float lineWidth);
			void SetDepthBias(float constantFactor, float clamp, float slopeFactor);
			void SetDepthBiasEnable(bool enable);                          // requires VK_EXT_extended_dynamic_state2
			void SetDepthBounds(float minDepthBounds, float maxDepthBounds);
			void SetDepthBoundsTestEnable(bool enable);                    // requires VK_EXT_extended_dynamic_state
			void SetDepthTestEnable(bool enable);                          // requires VK_EXT_extended_dynamic_state
			void SetDepthWriteEnable(bool enable);                         // requires VK_EXT_extended_dynamic_state
			void SetDepthCompareOp(CompareOp compareOp);                   // requires VK_EXT_extended_dynamic_state
			void SetStencilTestEnable(bool enable);                        // requires VK_EXT_extended_dynamic_state
			void SetStencilOp(Flags<StencilFace> faceMask, StencilOp failOp, StencilOp passOp, StencilOp depthFailOp, CompareOp compareOp);  // requires VK_EXT_extended_dynamic_state
			void SetStencilCompareMask(Flags<StencilFace> faceMask, uint32_t compareMask);
			void SetStencilWriteMask(Flags<StencilFace> faceMask, uint32_t writeMask);
			void SetStencilReference(Flags<StencilFace> faceMask, uint32_t reference);
			void SetBlendConstants(float r, float g, float b, float a);
			void SetCullMode(Flags<CullMode> cullMode);                    // requires VK_EXT_extended_dynamic_state
			void SetFrontFace(FrontFace frontFace);                        // requires VK_EXT_extended_dynamic_state
			void SetPrimitiveTopology(PrimitiveTopology topology);         // requires VK_EXT_extended_dynamic_state
			void SetPrimitiveRestartEnable(bool enable);                   // requires VK_EXT_extended_dynamic_state2
			void SetRasterizerDiscardEnable(bool enable);                  // requires VK_EXT_extended_dynamic_state2
			void SetVertexInputBindingStride(uint32_t firstBinding, uint32_t bindingCount, const uint32_t* strides); // requires VK_EXT_extended_dynamic_state

#ifdef SGF_GPU_EXTENDED_FUNCTIONS
			void SetSampleLocations(const SampleLocationsInfo& sampleLocationsInfo); // requires VK_EXT_sample_locations
			void SetFragmentShadingRate(Extent2D fragmentSize, FragmentShadingRateCombinerOp combinerOps[2]); // requires VK_KHR_fragment_shading_rate
#endif

			// -------------------------------------------------------------------------
			// Copy / Blit / Resolve (Buffer & Image)
			// -------------------------------------------------------------------------

			// Buffer copies
			void CopyBuffer(Buffer srcBuffer, Buffer dstBuffer, uint32_t regionCount, const BufferCopy* regions);
			inline void CopyBuffer(Buffer srcBuffer, Buffer dstBuffer, size_t size, size_t srcOffset = 0, size_t dstOffset = 0) {
				BufferCopy region{ srcOffset, dstOffset, size };
				CopyBuffer(srcBuffer, dstBuffer, 1, &region);
			}
			void FillBuffer(Buffer dstBuffer, size_t dstOffset, size_t size, uint32_t data);
			void UpdateBuffer(Buffer dstBuffer, size_t dstOffset, size_t dataSize, const void* pData);

			// Image copies
			void CopyImage(Image srcImage, ImageLayout srcLayout, Image dstImage, ImageLayout dstLayout, uint32_t regionCount, const ImageCopy* regions);
			void BlitImage(Image srcImage, ImageLayout srcLayout, Image dstImage, ImageLayout dstLayout, uint32_t regionCount, const ImageBlit* regions, FilterType filter);
			void ResolveImage(Image srcImage, ImageLayout srcLayout, Image dstImage, ImageLayout dstLayout, uint32_t regionCount, const ImageResolve* regions);

			// Buffer <-> Image
			void CopyBufferToImage(Buffer srcBuffer, Image dstImage, ImageLayout dstImageLayout, uint32_t regionCount, const BufferImageCopy* regions);
			void CopyImageToBuffer(Image srcImage, ImageLayout srcImageLayout, Buffer dstBuffer, uint32_t regionCount, const BufferImageCopy* regions);

			// Clear operations
			void ClearColorImage(Image image, ImageLayout imageLayout, const ClearColorValue& color, uint32_t rangeCount, const ImageSubresourceRange* ranges);
			void ClearDepthStencilImage(Image image, ImageLayout imageLayout, float depth, uint32_t stencil, uint32_t rangeCount, const ImageSubresourceRange* ranges);
			void ClearAttachments(uint32_t attachmentCount, const ClearAttachment* attachments, uint32_t rectCount, const ClearRect* rects);

			// -------------------------------------------------------------------------
			// Synchronisation — Barriers & Events
			// -------------------------------------------------------------------------

			// Pipeline barrier (legacy, pre-Vulkan 1.3)
			void PipelineBarrier(Flags<PipelineStage> srcStageMask, Flags<PipelineStage> dstStageMask, Flags<Dependency> dependencyFlags, uint32_t memoryBarrierCount, const MemoryBarrier* memoryBarriers, uint32_t bufferMemoryBarrierCount, const BufferMemoryBarrier* bufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const ImageMemoryBarrier* imageMemoryBarriers);

			// Synchronisation 2 (requires VK_KHR_synchronization2 / Vulkan 1.3)
			//void PipelineBarrier2(const DependencyInfo& dependencyInfo);

			// Convenience wrappers
			void MemoryBarrier(Flags<PipelineStage> srcStage, Flags<PipelineStage> dstStage, Flags<Access> srcAccess, Flags<Access> dstAccess);
			void BufferBarrier(Buffer buffer, size_t offset, size_t size, Flags<PipelineStage> srcStage, Flags<PipelineStage> dstStage, Flags<Access> srcAccess, Flags<Access> dstAccess, QueueType srcQueueType = QueueType::NONE, QueueType dstQueueType = QueueType::NONE);
			void ImageBarrier(Image image, const ImageSubresourceRange& subresourceRange, ImageLayout oldLayout, ImageLayout newLayout, Flags<PipelineStage> srcStage, Flags<PipelineStage> dstStage, Flags<Access> srcAccess, Flags<Access> dstAccess, QueueType srcQueueType = QueueType::NONE, QueueType dstQueueType = QueueType::NONE);

#ifdef SGF_GPU_EXTENDED_FUNCTIONS
			// Events
			void SetEvent(Event event, Flags<PipelineStage> stageMask);
			void SetEvent2(Event event, const DependencyInfo& dependencyInfo);   // requires VK_KHR_synchronization2
			void ResetEvent(Event event, Flags<PipelineStage> stageMask);
			void ResetEvent2(Event event, Flags<PipelineStage2> stageMask);      // requires VK_KHR_synchronization2
			void WaitEvents(uint32_t eventCount, const Event* events, Flags<PipelineStage> srcStageMask, Flags<PipelineStage> dstStageMask, uint32_t memoryBarrierCount, const MemoryBarrier* memoryBarriers, uint32_t bufferMemoryBarrierCount, const BufferMemoryBarrier* bufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const ImageMemoryBarrier* imageMemoryBarriers);
			void WaitEvents2(uint32_t eventCount, const Event* events, const DependencyInfo* dependencyInfos); // requires VK_KHR_synchronization2
#endif

			// -------------------------------------------------------------------------
			// Queries
			// -------------------------------------------------------------------------
			void BeginQuery(QueryPool queryPool, uint32_t query, Flags<QueryControl> flags = QueryControl::NONE);
			void EndQuery(QueryPool queryPool, uint32_t query);
			void ResetQueryPool(QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
			void WriteTimestamp(Flags<PipelineStage> pipelineStage, QueryPool queryPool, uint32_t query);
			//void WriteTimestamp2(Flags<PipelineStage2> stage, QueryPool queryPool, uint32_t query); // requires VK_KHR_synchronization2
			void CopyQueryPoolResults(QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, Buffer dstBuffer, size_t dstOffset, size_t stride, Flags<QueryResult> flags);
#ifdef SGF_GPU_EXTENDED_FUNCTIONS
			void BeginConditionalRendering(Buffer buffer, size_t offset, Flags<ConditionalRenderingFlag> flags = ConditionalRenderingFlag::NONE); // requires VK_EXT_conditional_rendering
			void EndConditionalRendering();                                // requires VK_EXT_conditional_rendering
			// -------------------------------------------------------------------------
			// Acceleration Structures (requires VK_KHR_acceleration_structure)
			// -------------------------------------------------------------------------
			void BuildAccelerationStructures(uint32_t infoCount, const AccelerationStructureBuildGeometryInfo* infos, const AccelerationStructureBuildRangeInfo* const* rangeInfos);
			void BuildAccelerationStructuresIndirect(uint32_t infoCount, const AccelerationStructureBuildGeometryInfo* infos, const uint64_t* indirectDeviceAddresses, const uint32_t* indirectStrides, const uint32_t* const* maxPrimitiveCounts);
			void CopyAccelerationStructure(const CopyAccelerationStructureInfo& info);
			void CopyAccelerationStructureToMemory(const CopyAccelerationStructureToMemoryInfo& info);
			void CopyMemoryToAccelerationStructure(const CopyMemoryToAccelerationStructureInfo& info);
			void WriteAccelerationStructuresProperties(uint32_t accelerationStructureCount, const AccelerationStructure* accelerationStructures, QueryType queryType, QueryPool queryPool, uint32_t firstQuery);
			// -------------------------------------------------------------------------
			// Debug utilities (requires VK_EXT_debug_utils)
			// -------------------------------------------------------------------------
			void BeginDebugLabel(const char* labelName, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
			void EndDebugLabel();
			void InsertDebugLabel(const char* labelName, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
#endif 

			// -------------------------------------------------------------------------
			// Secondary command buffer execution
			// -------------------------------------------------------------------------
			void ExecuteCommands(uint32_t commandBufferCount, const CommandList* commandLists);
			template<uint32_t COUNT>
			inline void ExecuteCommands(const std::array<CommandList, COUNT>& commandLists) { ExecuteCommands(COUNT, commandLists.data()); }
			inline void ExecuteCommands(const std::vector<CommandList>& commandLists) { ExecuteCommands((uint32_t)commandLists.size(), commandLists.data()); }

			// -------------------------------------------------------------------------
			// Transform feedback (requires VK_EXT_transform_feedback)
			// -------------------------------------------------------------------------
			void BeginTransformFeedback(uint32_t firstCounterBuffer, uint32_t counterBufferCount, const Buffer* counterBuffers, const size_t* counterBufferOffsets);
			void EndTransformFeedback(uint32_t firstCounterBuffer, uint32_t counterBufferCount, const Buffer* counterBuffers, const size_t* counterBufferOffsets);
			void BeginQueryIndexed(QueryPool queryPool, uint32_t query, Flags<QueryControl> flags, uint32_t index);
			void EndQueryIndexed(QueryPool queryPool, uint32_t query, uint32_t index);
			void BindTransformFeedbackBuffers(uint32_t firstBinding, uint32_t bindingCount, const Buffer* buffers, const size_t* offsets, const size_t* sizes);
			void DrawIndirectByteCount(uint32_t instanceCount, uint32_t firstInstance, Buffer counterBuffer, size_t counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride);

			// -------------------------------------------------------------------------
			// Handle access
			// -------------------------------------------------------------------------
			inline void* GetHandle() const { return m_Handle; }

		private:
			void* m_Handle = nullptr;
		};
	}
}