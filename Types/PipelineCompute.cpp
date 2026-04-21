#include "PipelineCompute.h"
#include "DeviceManager.h"
#include "vulkan/vulkan.hpp"

KE::VK::PipelineCompute::PipelineCompute(u32 deviceIndex, std::vector<uint32_t> spirv)
{
    vk::Device device = ContextManager::GetInstance().GetDevice(deviceIndex).GetVkDevice();

    vk::ShaderModuleCreateInfo shaderModuleInfo;
    shaderModuleInfo.codeSize     = spirv.size() * 4;
    shaderModuleInfo.pCode        = spirv.data();
    vk::ShaderModule shaderModule = device.createShaderModule(shaderModuleInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage  = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName  = "main";

    vk::PipelineCreateFlags2CreateInfo flags2Info;
    flags2Info.flags = vk::PipelineCreateFlagBits2::eDescriptorHeapEXT;

    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.pNext  = &flags2Info;
    pipelineInfo.stage  = shaderStageInfo;
    pipelineInfo.layout = VK_NULL_HANDLE; // pipelinelayout_;  // from previous step

    auto result = device.createComputePipeline(nullptr, pipelineInfo);

    // Clean up shader module (can be destroyed after pipeline creation)
    device.destroyShaderModule(shaderModule);

    pipeline_ = result.value;
}
KE::VK::PipelineCompute::PipelineCompute(u32 deviceIndex, SlangCompiledUnit shader)
{
    vk::Device                  device = ContextManager::GetInstance().GetDevice(deviceIndex).GetVkDevice();
    Slang::ComPtr<slang::IBlob> SpirV  = shader.getTargetCode();

    vk::ShaderModuleCreateInfo shaderModuleInfo;
    shaderModuleInfo.codeSize     = SpirV->getBufferSize();
    shaderModuleInfo.pCode        = reinterpret_cast<const uint32_t*>(SpirV->getBufferPointer());
    vk::ShaderModule shaderModule = device.createShaderModule(shaderModuleInfo);


    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage  = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName  = "main"; // entry point name in your shader

    vk::PipelineCreateFlags2CreateInfo flags2Info;
    flags2Info.flags = vk::PipelineCreateFlagBits2::eDescriptorHeapEXT;

    // MAPPINGS, SO I CAN USE DESCRIPTOR HEAPS WITH NEURAL INFERENCE
    vk::DescriptorSetAndBindingMappingEXT mapping;
    mapping.descriptorSet             = 0;
    mapping.firstBinding              = 0;
    mapping.bindingCount              = 4;
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


    //MAPPINGS END!

    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.pNext       = &flags2Info;
    pipelineInfo.stage       = shaderStageInfo;
    pipelineInfo.stage.pNext = &mappingInfo;
    pipelineInfo.layout      = VK_NULL_HANDLE; // from previous step



    auto result = device.createComputePipeline(nullptr, pipelineInfo);

    // Clean up shader module (can be destroyed after pipeline creation)
    device.destroyShaderModule(shaderModule);

    pipeline_ = result.value;
}
