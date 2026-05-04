#include "PipelineGraphics.h"

#include "DeviceManager.h"


using namespace KE::VK;
PipelineGraphics::PipelineGraphics(u32 deviceIndex, SlangCompiledUnit shader)
{
    vk::Device device = ContextManager::GetInstance().GetDevice(deviceIndex).GetVkDevice();
    vk::ShaderModuleCreateInfo shaderModuleInfo;

    Slang::ComPtr<slang::IBlob> SpirV = shader.getTargetCode();
    shaderModuleInfo.codeSize = SpirV->getBufferSize();
    shaderModuleInfo.pCode = reinterpret_cast<const uint32_t *>(SpirV->getBufferPointer());

    vk::ShaderModule shaderModule = device.createShaderModule(shaderModuleInfo);

    vk::PipelineShaderStageCreateInfo vertexStageInfo;
    vertexStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertexStageInfo.module = shaderModule;
    vertexStageInfo.pName = "vertMain";
    vk::PipelineShaderStageCreateInfo fragmentStageInfo;
    fragmentStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragmentStageInfo.module = shaderModule;
    fragmentStageInfo.pName = "fragMain";

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertexStageInfo, fragmentStageInfo};

    vk::Format format = vk::Format::eR8G8B8A8Unorm;
    
    vk::PipelineCreateFlags2CreateInfo flags2Info;
    flags2Info.flags = vk::PipelineCreateFlagBits2::eDescriptorHeapEXT;

    vk::PipelineRenderingCreateInfo renderingCreateInfo;
    renderingCreateInfo.pNext = &flags2Info;
    renderingCreateInfo.colorAttachmentCount = 1;
    renderingCreateInfo.pColorAttachmentFormats = &format;
    

    vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo;
    pipelineInputAssemblyStateCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;

    vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo;

    vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo;
    pipelineRasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eNone;

    vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo;
    pipelineViewportStateCreateInfo.scissorCount = 1;
    pipelineViewportStateCreateInfo.viewportCount = 1;

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
        vk::DynamicState::eVertexInputEXT};

    vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo;
    pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
    pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();

    vk::PipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                     vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    blendAttachment.blendEnable    = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable   = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments    = &blendAttachment;


    vk::PipelineVertexInputStateCreateInfo emptyVertexInput{};

    // MAPPINGS, SO I CAN USE DESCRIPTOR HEAPS WITH NEURAL INFERENCE
    vk::DescriptorSetAndBindingMappingEXT mapping;
    mapping.descriptorSet             = 0;
    mapping.firstBinding              = 0;
    mapping.bindingCount              = 10;
    mapping.resourceMask              = vk::SpirvResourceTypeFlagBitsEXT::eAll;
    mapping.source                    = vk::DescriptorMappingSourceEXT::eHeapWithConstantOffset;
    mapping.sourceData.constantOffset = {
        .heapOffset      = 3024 * 32, // byte offset of binding 0 in the heap
        .heapArrayStride = 32         // each subsequent binding steps by this
    };

    vk::ShaderDescriptorSetAndBindingMappingInfoEXT mappingInfo;
    mappingInfo.pNext        = nullptr;
    mappingInfo.mappingCount = 1;
    mappingInfo.pMappings    = &mapping;

    shaderStages[0].pNext = &mappingInfo;
    shaderStages[1].pNext = &mappingInfo;

    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.pNext = &renderingCreateInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.layout = VK_NULL_HANDLE;
    pipelineInfo.pVertexInputState = &emptyVertexInput;
    pipelineInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
    pipelineInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
    pipelineInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
    pipelineInfo.pViewportState = &pipelineViewportStateCreateInfo;
    pipelineInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
    pipelineInfo.pColorBlendState = &colorBlending;

    auto result = device.createGraphicsPipeline(nullptr, pipelineInfo);

    pipeline_ = result.value;
}

PipelineGraphics::PipelineGraphics(u32 deviceIndex, std::vector<uint32_t> vertSpirv, std::vector<uint32_t> fragSpirv)
{
    vk::Device device = ContextManager::GetInstance().GetDevice(deviceIndex).GetVkDevice();

    vk::ShaderModuleCreateInfo vertModuleInfo;
    vertModuleInfo.codeSize = vertSpirv.size() * 4;
    vertModuleInfo.pCode    = vertSpirv.data();
    vk::ShaderModule vertModule = device.createShaderModule(vertModuleInfo);

    vk::ShaderModuleCreateInfo fragModuleInfo;
    fragModuleInfo.codeSize = fragSpirv.size() * 4;
    fragModuleInfo.pCode    = fragSpirv.data();
    vk::ShaderModule fragModule = device.createShaderModule(fragModuleInfo);

    vk::PipelineShaderStageCreateInfo vertexStageInfo;
    vertexStageInfo.stage  = vk::ShaderStageFlagBits::eVertex;
    vertexStageInfo.module = vertModule;
    vertexStageInfo.pName  = "main";

    vk::PipelineShaderStageCreateInfo fragmentStageInfo;
    fragmentStageInfo.stage  = vk::ShaderStageFlagBits::eFragment;
    fragmentStageInfo.module = fragModule;
    fragmentStageInfo.pName  = "main";

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertexStageInfo, fragmentStageInfo};

    vk::Format format = vk::Format::eR8G8B8A8Unorm;

    vk::PipelineCreateFlags2CreateInfo flags2Info;
    flags2Info.flags = vk::PipelineCreateFlagBits2::eDescriptorHeapEXT;

    vk::PipelineRenderingCreateInfo renderingCreateInfo;
    renderingCreateInfo.pNext                   = &flags2Info;
    renderingCreateInfo.colorAttachmentCount    = 1;
    renderingCreateInfo.pColorAttachmentFormats = &format;

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

    vk::PipelineMultisampleStateCreateInfo multisample;

    vk::PipelineRasterizationStateCreateInfo rasterization;
    rasterization.polygonMode = vk::PolygonMode::eFill;
    rasterization.lineWidth   = 1.0f;
    rasterization.cullMode    = vk::CullModeFlagBits::eNone;

    vk::PipelineViewportStateCreateInfo viewport;
    viewport.scissorCount  = 1;
    viewport.viewportCount = 1;

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicState;
    dynamicState.pDynamicStates    = dynamicStates.data();
    dynamicState.dynamicStateCount = dynamicStates.size();

    vk::PipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                     vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    blendAttachment.blendEnable    = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable   = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments    = &blendAttachment;

    // MAPPINGS, SO I CAN USE DESCRIPTOR HEAPS WITH NEURAL INFERENCE
    vk::DescriptorSetAndBindingMappingEXT mapping;
    mapping.descriptorSet             = 0;
    mapping.firstBinding              = 0;
    mapping.bindingCount              = 5;
    mapping.resourceMask              = vk::SpirvResourceTypeFlagBitsEXT::eAll;
    mapping.source                    = vk::DescriptorMappingSourceEXT::eHeapWithConstantOffset;
    mapping.sourceData.constantOffset = {
        .heapOffset      = 3024 * 32, // byte offset of binding 0 in the heap
        .heapArrayStride = 32         // each subsequent binding steps by this
    };

    vk::ShaderDescriptorSetAndBindingMappingInfoEXT mappingInfo;
    mappingInfo.pNext        = nullptr;
    mappingInfo.mappingCount = 1;
    mappingInfo.pMappings    = &mapping;

    shaderStages[0].pNext = &mappingInfo;
    shaderStages[1].pNext = &mappingInfo;

    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.pNext               = &renderingCreateInfo;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages.data();
    pipelineInfo.layout              = VK_NULL_HANDLE;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pMultisampleState   = &multisample;
    pipelineInfo.pRasterizationState = &rasterization;
    pipelineInfo.pViewportState      = &viewport;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.pColorBlendState    = &colorBlending;

    auto result = device.createGraphicsPipeline(nullptr, pipelineInfo);
    pipeline_   = result.value;

    device.destroyShaderModule(vertModule);
    device.destroyShaderModule(fragModule);
}