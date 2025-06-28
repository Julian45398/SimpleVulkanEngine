#include "Render/GraphicsPipeline.hpp"
#include "Render/Device.hpp"

namespace SGF {

    VkPipeline GraphicsPipelineBuilder::Build() {
        return Device::Get().CreatePipeline(info);
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::GeometryShader(const char* filename) {
        auto& device = Device::Get();
        assert(device.HasFeatureEnabled(DEVICE_FEATURE_GEOMETRY_SHADER));
        AddShaderStage(filename, VK_SHADER_STAGE_GEOMETRY_BIT);
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::FragmentShader(const char* filename) {
        AddShaderStage(filename, VK_SHADER_STAGE_FRAGMENT_BIT);
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::VertexShader(const char* filename) {
        AddShaderStage(filename, VK_SHADER_STAGE_VERTEX_BIT);
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::TesselationEvaluationShader(const char* filename) {
        auto& device = Device::Get();
        assert(device.HasFeatureEnabled(DEVICE_FEATURE_TESSELLATION_SHADER));
        AddShaderStage(filename, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::TesselationControlShader(const char* filename) {
        auto& device = Device::Get();
        assert(device.HasFeatureEnabled(DEVICE_FEATURE_TESSELLATION_SHADER));
        AddShaderStage(filename, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        return *this;
    }

    GraphicsPipelineBuilder::~GraphicsPipelineBuilder() {
        auto& device = Device::Get();
        for (uint32_t i = 0; i < info.stageCount; ++i) {
            device.Destroy(pipelineStages[i].module);
        }
    }
    GraphicsPipelineBuilder::GraphicsPipelineBuilder(const Device* device, VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass)
    {
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = FLAG_NONE;
        info.stageCount = 0;
        info.pStages = pipelineStages;
        info.pVertexInputState = &VERTEX_INPUT_NONE;
        info.pInputAssemblyState = &inputAssemblyState;
        info.pTessellationState = nullptr; // &tessellationState;
        info.pViewportState = &viewportState;
        info.pRasterizationState = &rasterizationState;
        info.pMultisampleState = &multisampleState;
        info.pDepthStencilState = &depthStencilState;
        info.pColorBlendState = &colorBlendState;
        info.pDynamicState = &dynamicStateInfo;
        info.layout = layout;
        info.renderPass = renderPass;
        info.subpass = subpass;
        info.basePipelineHandle = VK_NULL_HANDLE;
        info.basePipelineIndex = 0;

        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.pNext = nullptr;
        inputAssemblyState.flags = FLAG_NONE;
        inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyState.primitiveRestartEnable = VK_FALSE;

        tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationState.pNext = nullptr;
        tessellationState.flags = FLAG_NONE;
        tessellationState.patchControlPoints = 1;

        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.pNext = nullptr;
        viewportState.flags = FLAG_NONE;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &stViewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &stScissor;

        stViewport.width = 0.0f;
        stViewport.height = 0.0f;
        stViewport.x = 0.0f;
        stViewport.y = 0.0f;
        stViewport.maxDepth = 1.0f;
        stViewport.minDepth = 0.0f;

        stScissor.extent = { 0, 0 };
        stScissor.offset = { 0, 0 };

        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.pNext = nullptr;
        rasterizationState.flags = FLAG_NONE;
        rasterizationState.depthClampEnable = VK_FALSE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationState.depthBiasEnable = VK_FALSE;
        rasterizationState.depthBiasConstantFactor = 0.0f;
        rasterizationState.depthBiasClamp = 0.0f;
        rasterizationState.depthBiasSlopeFactor = 0.0f;
        rasterizationState.lineWidth = 1.0f;

        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.pNext = nullptr;
        multisampleState.flags = FLAG_NONE;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.minSampleShading = 0.0f;
		multisampleState.pSampleMask = nullptr;
		multisampleState.alphaToCoverageEnable = VK_FALSE;
		multisampleState.alphaToOneEnable = VK_FALSE;

		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.pNext = nullptr;
		depthStencilState.flags = FLAG_NONE;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = {};
        depthStencilState.back = {};
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;

		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.pNext = nullptr;
		colorBlendState.flags = FLAG_NONE;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.logicOp = VK_LOGIC_OP_COPY;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachmentState;
        colorBlendState.blendConstants[0] = 0.f;
        colorBlendState.blendConstants[1] = 0.f;
        colorBlendState.blendConstants[2] = 0.f;
        colorBlendState.blendConstants[3] = 0.f;

		colorBlendAttachmentState.blendEnable = VK_TRUE;
		colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.pNext = nullptr;
        dynamicStateInfo.flags = FLAG_NONE;
        dynamicStateInfo.dynamicStateCount = 0;
        dynamicStateInfo.pDynamicStates = dynamicStates;
    }

    void GraphicsPipelineBuilder::AddShaderStage(const char* filename, VkShaderStageFlagBits stage) {
        assert(info.stageCount < SGF_PIPELINE_MAX_PIPELINE_STAGES);

        auto& device = Device::Get();
        VkShaderModule shader = device.CreateShaderModule(filename);
        pipelineStages[info.stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineStages[info.stageCount].stage = stage;
        pipelineStages[info.stageCount].pName = "main";
        pipelineStages[info.stageCount].module = shader;
        pipelineStages[info.stageCount].pNext = nullptr;
        pipelineStages[info.stageCount].pSpecializationInfo = nullptr;
        pipelineStages[info.stageCount].flags = FLAG_NONE;
        info.stageCount++;
    }
}