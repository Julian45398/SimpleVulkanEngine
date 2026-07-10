#pragma once
#pragma once

#include <vector>
#include "Types.hpp"

namespace SGF::GPU
{
    class GraphicsPipelineBuilder
    {
    public:
        GraphicsPipelineBuilder();

        GraphicsPipeline Build();
        //Pipeline Build(Flags<GraphicsPipelineOptions> options = GraphicsPipelineOption)

        //=========================================================
        // Pipeline Layout
        //=========================================================

        GraphicsPipelineBuilder& Layout(PipelineLayout layout);

        GraphicsPipelineBuilder& RenderPass(
            RenderPass renderPass,
            uint32_t subpass = 0);

        //=========================================================
        // Shader Stages
        //=========================================================

        GraphicsPipelineBuilder& VertexShader(const char* filename);
        GraphicsPipelineBuilder& FragmentShader(const char* filename);
        GraphicsPipelineBuilder& GeometryShader(const char* filename);
        GraphicsPipelineBuilder& TessellationControlShader(const char* filename);
        GraphicsPipelineBuilder& TessellationEvaluationShader(const char* filename);

        GraphicsPipelineBuilder& VertexShader(ShaderModule module, const char* entry = "main");

        GraphicsPipelineBuilder& FragmentShader(ShaderModule module, const char* entry = "main");

        GraphicsPipelineBuilder& GeometryShader(ShaderModule module, const char* entry = "main");

        GraphicsPipelineBuilder& TessellationControlShader(ShaderModule module, const char* entrypoint = "main");

        GraphicsPipelineBuilder& TessellationEvaluationShader(ShaderModule module, const char* entrypoint = "main");

        //=========================================================
        // Vertex Input
        //=========================================================

        GraphicsPipelineBuilder& ClearVertexInput();

        GraphicsPipelineBuilder& AddVertexBinding(uint32_t stride, VertexInputRate rate = VertexInputRate::VERTEX);
        GraphicsPipelineBuilder& SetVertexBinding(uint32_t stride, VertexInputRate rate = VertexInputRate::VERTEX);

        GraphicsPipelineBuilder& AddVertexAttribute(Format format, uint32_t offset);
        template<typename T>
        inline GraphicsPipelineBuilder& AddVertexAttribute(uint32_t offset) {
            if constexpr (std::is_same_v<T, uint32_t>)
                return AddVertexAttribute(Format::R32_UINT, offset);

            else if constexpr (std::is_same_v<T, int32_t>)
                return AddVertexAttribute(Format::R32_SINT, offset);

            else if constexpr (std::is_same_v<T, float>)
                return AddVertexAttribute(Format::R32_SFLOAT, offset);

            else if constexpr (std::is_same_v<T, glm::vec2>)
                return AddVertexAttribute(Format::RG32_SFLOAT, offset);

            else if constexpr (std::is_same_v<T, glm::uvec2>)
                return AddVertexAttribute(Format::RG32_UINT, offset);

            else if constexpr (std::is_same_v<T, glm::ivec2>)
                return AddVertexAttribute(Format::RG32_SINT, offset);

            else if constexpr (std::is_same_v<T, glm::vec3>)
                return AddVertexAttribute(Format::RGB32_SFLOAT, offset);

            else if constexpr (std::is_same_v<T, glm::uvec3>)
                return AddVertexAttribute(Format::RGB32_UINT, offset);

            else if constexpr (std::is_same_v<T, glm::ivec3>)
                return AddVertexAttribute(Format::RGB32_SINT, offset);

            else if constexpr (std::is_same_v<T, glm::vec4>)
                return AddVertexAttribute(Format::RGBA32_SFLOAT, offset);

            else if constexpr (std::is_same_v<T, glm::uvec4>)
                return AddVertexAttribute(Format::RGBA32_UINT, offset);

            else if constexpr (std::is_same_v<T, glm::ivec4>)
                return AddVertexAttribute(Format::RGBA32_SINT, offset);

            else if constexpr (std::is_same_v<T, glm::mat4>)
            {
                AddVertexAttribute(Format::RGBA32_SFLOAT, offset);
                AddVertexAttribute(Format::RGBA32_SFLOAT, offset + sizeof(glm::vec4));
                AddVertexAttribute(Format::RGBA32_SFLOAT, offset + 2 * sizeof(glm::vec4));
                return AddVertexAttribute(Format::RGBA32_SFLOAT, offset + 3 * sizeof(glm::vec4));
            }

            else
            {
                static_assert(sizeof(T) == 0, "Unsupported vertex attribute type.");
            }
        }
        template<typename INTEGER_TYPE>
        inline static constexpr INTEGER_TYPE AlignOffset(INTEGER_TYPE offset, INTEGER_TYPE alignment) {
            return (offset + alignment - 1) & ~(alignment - 1); // Align up
        }
        template<typename... Types>
        GraphicsPipelineBuilder& AddVertexInput() {
            uint32_t offset = 0;
            (
                (offset = AlignOffset(offset, Alignment<Types, GLSLLayout::STD140>()),
                AddVertexAttribute<Types>(offset),
                offset += sizeof(Types)),
            ...);
            return *this;
        }
        inline GraphicsPipelineBuilder& AddVertexInput() { return *this; }

        //=========================================================
        // Input Assembly
        //=========================================================

        GraphicsPipelineBuilder& Topology(PrimitiveTopology topology = PrimitiveTopology::TRIANGLE_LIST);

        GraphicsPipelineBuilder& PrimitiveRestart(bool enable = true);

        //=========================================================
        // Tessellation
        //=========================================================

        GraphicsPipelineBuilder& PatchControlPoints(uint32_t count);

        //=========================================================
        // Viewport
        //=========================================================

        GraphicsPipelineBuilder& Viewport(
            float width,
            float height,
            float x = 0.0f,
            float y = 0.0f,
            float minDepth = 0.0f,
            float maxDepth = 1.0f);

        GraphicsPipelineBuilder& Viewport(const SGF::GPU::Viewport& viewport);

        GraphicsPipelineBuilder& Scissor(
            uint32_t width,
            uint32_t height,
            int32_t x = 0,
            int32_t y = 0);

        GraphicsPipelineBuilder& Scissor(const Rect2D& scissor);

        //=========================================================
        // Rasterizer
        //=========================================================

        GraphicsPipelineBuilder& PolygonMode(
            PolygonMode mode = PolygonMode::FILL);

        GraphicsPipelineBuilder& CullMode(
            CullMode mode =
            CullMode::BACK);

        GraphicsPipelineBuilder& FrontFace(
            FrontFace face =
            FrontFace::COUNTER_CLOCKWISE);

        GraphicsPipelineBuilder& LineWidth(
            float width);

        GraphicsPipelineBuilder& DepthClamp(
            bool enable = true);

        GraphicsPipelineBuilder& RasterizerDiscard(
            bool enable = true);

        GraphicsPipelineBuilder& DepthBias(
            bool enable,
            float constantFactor,
            float clamp,
            float slopeFactor);

        //=========================================================
        // Multisampling
        //=========================================================

        GraphicsPipelineBuilder& SampleCount(
            SampleCount samples =
            SampleCount::B1);

        GraphicsPipelineBuilder& SampleShading(
            bool enable,
            float minimum = 1.0f);

        GraphicsPipelineBuilder& AlphaToCoverage(
            bool enable = true);

        GraphicsPipelineBuilder& AlphaToOne(
            bool enable = true);

        //=========================================================
        // Depth / Stencil
        //=========================================================

        GraphicsPipelineBuilder& Depth(
            bool test = true,
            bool write = true,
            CompareOp compare =
            CompareOp::LESS);

        GraphicsPipelineBuilder& DepthBounds(
            bool enable,
            float minimum,
            float maximum);

        GraphicsPipelineBuilder& StencilTest(
            bool enable = true);

        GraphicsPipelineBuilder& FrontStencil(const StencilOpState& state);

        GraphicsPipelineBuilder& FrontStencil(
			StencilOp failOp,
			StencilOp passOp,
			StencilOp depthFailOp,
			CompareOp compareOp,
			uint32_t compareMask,
			uint32_t writeMask,
			uint32_t reference);

        GraphicsPipelineBuilder& BackStencil(const StencilOpState& state);

        GraphicsPipelineBuilder& BackStencil(
			StencilOp failOp,
			StencilOp passOp,
			StencilOp depthFailOp,
			CompareOp compareOp,
			uint32_t compareMask,
			uint32_t writeMask,
			uint32_t reference);

        //=========================================================
        // Color Blend
        //=========================================================

        GraphicsPipelineBuilder& AddColorBlendAttachment(
            bool blendEnable = true,
            Flags<ColorComponent> colorMask = ColorComponent::RGBA,
            BlendOp alphaBlendOp = BlendOp::ADD,
            BlendFactor srcAlpha = BlendFactor::ONE,
            BlendFactor dstAlpha = BlendFactor::ZERO,
            BlendOp colorBlendOp = BlendOp::ADD,
            BlendFactor srcColor = BlendFactor::SRC_ALPHA,
            BlendFactor dstColor = BlendFactor::ONE_MINUS_SRC_ALPHA);

        GraphicsPipelineBuilder& SetColorBlendAttachment(
            uint32_t index,
            bool blendEnable = true,
            Flags<ColorComponent> colorMask = ColorComponent::RGBA,
            BlendOp alphaBlendOp = BlendOp::ADD,
            BlendFactor srcAlpha = BlendFactor::ONE,
            BlendFactor dstAlpha = BlendFactor::ZERO,
            BlendOp colorBlendOp = BlendOp::ADD,
            BlendFactor srcColor = BlendFactor::SRC_ALPHA,
            BlendFactor dstColor = BlendFactor::ONE_MINUS_SRC_ALPHA);

        GraphicsPipelineBuilder& LogicOp(bool enable, LogicOp op);

        GraphicsPipelineBuilder& BlendConstants(float r, float g, float b, float a);

        //=========================================================
        // Dynamic State
        //=========================================================

        GraphicsPipelineBuilder& DynamicState(
            DynamicState state);

        GraphicsPipelineBuilder& ClearDynamicStates();

        //=========================================================
        // Convenience Helpers
        //=========================================================

        GraphicsPipelineBuilder& Opaque();

        GraphicsPipelineBuilder& AlphaBlending();

        GraphicsPipelineBuilder& AdditiveBlending();

        GraphicsPipelineBuilder& PremultipliedAlpha();

        GraphicsPipelineBuilder& NoCulling();

        GraphicsPipelineBuilder& BackFaceCulling();

        GraphicsPipelineBuilder& FrontFaceCulling();

        GraphicsPipelineBuilder& Wireframe();

        GraphicsPipelineBuilder& Fill();

        GraphicsPipelineBuilder& DisableDepth();

        GraphicsPipelineBuilder& DepthReadOnly();

        GraphicsPipelineBuilder& DynamicViewport();

        GraphicsPipelineBuilder& DynamicScissor();

        GraphicsPipelineBuilder& DynamicViewportScissor();

        GraphicsPipelineBuilder& TriangleList();

        GraphicsPipelineBuilder& TriangleStrip();

        GraphicsPipelineBuilder& LineList();

        GraphicsPipelineBuilder& LineStrip();

        GraphicsPipelineBuilder& PointList();

        ~GraphicsPipelineBuilder();

    private:
        void AddShaderStage(
            const char* filename,
            ShaderStage stage);

        void AddShaderStage(
            ShaderModule module,
            ShaderStage stage,
            const char* entry);
    private:
        void* m_Handle = nullptr;
    };

}