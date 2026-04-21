#pragma once

#include <iostream>
#include <optional>

#include "VkBootstrap.h"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>


#include "GpuInterface/BasicTypeAliases.h"
#include "SDL3/SDL_vulkan.h"


#include "Types/Device.h"

#include "GpuInterfaceDLL.h"


void GPUI_DLL_API InitDispatcherForDLL(vk::Instance instance);


inline vkb::Instance createInstance()
{
    u32 count;
    // SDL_Vulkan_GetInstanceExtensions(&count);
    vkb::Instance        vkb_inst_;
    vkb::InstanceBuilder builder;
    try
    {
        auto inst_ret =
            builder.set_app_name("Example Vulkan Application")
                .request_validation_layers()
                .require_api_version(1, 4, 341)
                .enable_validation_layers()
                //.add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT)
                //.add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT)
                //.use_default_debug_messenger()
                .build();
        if (!inst_ret)
        {
            throw std::runtime_error("Failed to create instance: " + inst_ret.error().message());
        }
        vkb_inst_ = inst_ret.value();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }


    printf("API Version: %d.%d.%d\n", VK_VERSION_MAJOR(vkb_inst_.api_version), VK_VERSION_MINOR(vkb_inst_.api_version),
           VK_VERSION_PATCH(vkb_inst_.api_version));

    VULKAN_HPP_DEFAULT_DISPATCHER.init();
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vk::Instance(vkb_inst_.instance));

    return vkb_inst_;
}

inline void createSurface(vk::SurfaceKHR& surface, SDL_Window* window, vk::Instance instance)
{

    // SUS: weirded out by instance having to be created with the SDL extensions when it works fine without. Weird...
    bool err = SDL_Vulkan_CreateSurface(window, instance, NULL, (VkSurfaceKHR*)&surface);

    if (!err)
    { /* handle error */
    }
}


// TODO: We need to be able to pass requirements for the device.
inline vkb::Device createDevice(vkb::Instance vkb_inst, vk::SurfaceKHR surface)
{
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    auto                        phys_ret = selector //.set_surface(NULL)
                                               .add_required_extension("VK_EXT_descriptor_heap")
                                               .add_required_extension("VK_KHR_shader_untyped_pointers")
                                               .add_required_extension("VK_EXT_vertex_input_dynamic_state")
                                               .add_required_extension("VK_NV_cooperative_vector")
                                               .set_minimum_version(1, 4)
                                               .require_present(true)
                                               .set_surface(surface)
                                               .select();
    if (!phys_ret)
    {
        std::cerr << "Error code: " << phys_ret.vk_result() << std::endl;
        std::cerr << "Error message: " << phys_ret.error().message() << std::endl;
    }

    vk::PhysicalDeviceVulkan13Features features13;
    features13.synchronization2 = VK_TRUE;
    features13.dynamicRendering = VK_TRUE;


    vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT vertexInputDynamicStateFeat;
    vertexInputDynamicStateFeat.vertexInputDynamicState = VK_TRUE;
    vk::PhysicalDeviceDescriptorHeapFeaturesEXT descHeapFeatures;
    descHeapFeatures.descriptorHeap = VK_TRUE;

    vk::PhysicalDeviceShaderUntypedPointersFeaturesKHR untypedPtrFeatures;
    untypedPtrFeatures.shaderUntypedPointers = VK_TRUE;

    vk::PhysicalDeviceVulkan12Features features12;
    features12.bufferDeviceAddress    = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.shaderInt8             = VK_TRUE;

    vk::PhysicalDeviceCooperativeVectorFeaturesNV cooperativeVectorFeatures;
    cooperativeVectorFeatures.cooperativeVector = VK_TRUE;

    vkb::DeviceBuilder device_builder{phys_ret.value()};
    auto               dev_ret = device_builder.add_pNext(&features12)
                                     .add_pNext(&features13)
                                     .add_pNext(&descHeapFeatures)
                                     .add_pNext(&untypedPtrFeatures) // also need this one
                                     .add_pNext(&vertexInputDynamicStateFeat)
                                     .add_pNext(&cooperativeVectorFeatures)
                                     //.enable_extension("VK_EXT_descriptor_heap")
                       //.enable_extension("VK_KHR_shader_untyped_pointers")
                       //.add_pNext(&DynamicStateFeat)
                       .build();
    if (!dev_ret)
    { /* report */
    }
    vkb::Device vkb_device = dev_ret.value();

    if (!dev_ret)
    {
        std::cerr << "Failed to create logical device: " << dev_ret.error().message() << std::endl;
    }
    return vkb_device;
}

inline vkb::Device createDevice(vkb::Instance vkb_inst)
{
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    auto                        phys_ret = selector //.set_surface(NULL)
                                               .add_required_extension("VK_EXT_descriptor_heap")
                                               .add_required_extension("VK_KHR_shader_untyped_pointers")
                                               .add_required_extension("VK_EXT_vertex_input_dynamic_state")
                                               .add_required_extension("VK_NV_cooperative_vector")
                                               .set_minimum_version(1, 4)
                                               .defer_surface_initialization()
                                               .select();
    if (!phys_ret)
    {
        std::cerr << "Error code: " << phys_ret.vk_result() << std::endl;
        std::cerr << "Error message: " << phys_ret.error().message() << std::endl;
    }

    vk::PhysicalDeviceVulkan11Features features11;

    features11.uniformAndStorageBuffer16BitAccess = true;

    vk::PhysicalDeviceVulkan12Features features12;
    features12.bufferDeviceAddress    = true;
    features12.runtimeDescriptorArray = true;
    features12.shaderInt8             = true;
    features12.vulkanMemoryModel      = true;
    features12.shaderFloat16          = true;
    features12.uniformAndStorageBuffer8BitAccess = true;

    vk::PhysicalDeviceVulkan13Features features13;
    features13.synchronization2 = true;
    features13.dynamicRendering = true;


    vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT vertexInputDynamicStateFeat;
    vertexInputDynamicStateFeat.vertexInputDynamicState = true;
    vk::PhysicalDeviceDescriptorHeapFeaturesEXT descHeapFeatures;
    descHeapFeatures.descriptorHeap = true;

    vk::PhysicalDeviceShaderUntypedPointersFeaturesKHR untypedPtrFeatures;
    untypedPtrFeatures.shaderUntypedPointers = true;


    vk::PhysicalDeviceCooperativeVectorFeaturesNV cooperativeVectorFeatures;
    cooperativeVectorFeatures.cooperativeVector = true;

    vk::PhysicalDevice16BitStorageFeatures storageBuffer16BitStorageFeatures;
    storageBuffer16BitStorageFeatures.storageBuffer16BitAccess = true;

    vkb::DeviceBuilder device_builder{phys_ret.value()};
    auto               dev_ret = device_builder.add_pNext(&features11)
                                     .add_pNext(&features12)
                                     .add_pNext(&features13)
                                     .add_pNext(&descHeapFeatures)
                                     .add_pNext(&untypedPtrFeatures) // also need this one
                                     .add_pNext(&vertexInputDynamicStateFeat)
                                     .add_pNext(&cooperativeVectorFeatures)
                                     .add_pNext(&storageBuffer16BitStorageFeatures)
                                     //.enable_extension("VK_EXT_descriptor_heap")
                       //.enable_extension("VK_KHR_shader_untyped_pointers")
                       //.add_pNext(&DynamicStateFeat)
                       .build();
    if (!dev_ret)
    { /* report */
    }
    vkb::Device vkb_device = dev_ret.value();

    if (!dev_ret)
    {
        std::cerr << "Failed to create logical device: " << dev_ret.error().message() << std::endl;
    }
    return vkb_device;
}

// THESE NEXT THREE NEED WORKOVER, THIS CAN NOT PROPERLY SUPPORT ASYNC/DIFFERING PRIORITIES ON DIFFERENT QUEUES

inline auto getGraphicsQueue(vkb::Device device)
{
    auto graphics_queue_ret = device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_ret)
    { /* report */
    }
    return vk::Queue(graphics_queue_ret.value());
}

inline auto getComputeQueue(vkb::Device device)
{
    auto compute_queue_ret = device.get_queue(vkb::QueueType::compute);
    if (!compute_queue_ret)
    { /* report */
    }
    return vk::Queue(compute_queue_ret.value());
}

inline auto getPresentQueue(vkb::Device device)
{
    auto present_queue_ret = device.get_queue(vkb::QueueType::present);
    if (!present_queue_ret)
    {
        std::cout << present_queue_ret.error() << std::endl;
    }
    return vk::Queue(present_queue_ret.value());
}


struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value() && graphicsFamily.has_value();
    }
};

inline QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
    device.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {

        if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
        {
            indices.computeFamily = i;
        }
        // VkBool32 presentSupport = false;
        // presentSupport = device.getSurfaceSupportKHR(i, VKS.surface);

        // if (presentSupport)
        //	indices.presentFamily = i;

        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}


inline vk::CommandPool createComputeCommandPool(vkb::Device device)
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vk::PhysicalDevice(device.physical_device));

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags            = vk::CommandPoolCreateFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();


    vk::CommandPool pool;

    try
    {
        pool = vk::Device(device.device).createCommandPool(poolInfo, nullptr);
    }
    catch (const vk::SystemError& err)
    {
        std::cerr << "Failed to create command pool: " << err.what() << std::endl;
    }

    return pool;
}

inline vk::DescriptorSetLayout createDescriptorSetLayout(vkb::Device vkboot_device)
{
    vk::DescriptorSetLayoutBinding binding;
    binding.binding            = 0; // matches binding = 0 in shader
    binding.descriptorType     = vk::DescriptorType::eStorageBuffer;
    binding.descriptorCount    = 1;                                 // number of descriptors (1 buffer)
    binding.stageFlags         = vk::ShaderStageFlagBits::eCompute; // used in compute shader
    binding.pImmutableSamplers = nullptr;                           // only for samplers


    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings    = &binding;

    return vk::Device(vkboot_device.device).createDescriptorSetLayout(layoutInfo);
}

inline vk::PipelineLayout createPipelineLayout(vkb::Device vkboot_device, vk::DescriptorSetLayout& DSL)
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount         = 1;
    pipelineLayoutInfo.pSetLayouts            = &DSL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;

    return vk::Device(vkboot_device).createPipelineLayout(pipelineLayoutInfo);
}

inline vk::ShaderModule createShaderModule(KE::VK::Device device, const char* code, int size)
{
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = size;
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(code);

    vk::ShaderModule shaderModule;

    shaderModule = vk::Device(device).createShaderModule(createInfo);
    return shaderModule;
}

inline vk::Pipeline createComputePipeline(vk::PipelineLayout pipelineLayout, std::vector<char> spirvCode,
                                          vkb::Device vkboot_device)
{

    vk::Device device = vk::Device(vkboot_device.device);

    vk::ShaderModuleCreateInfo shaderModuleInfo;
    shaderModuleInfo.codeSize = spirvCode.size();
    shaderModuleInfo.pCode    = reinterpret_cast<const u32*>(spirvCode.data());
    std::cout << "Magic number: 0x" << std::hex << reinterpret_cast<const u32*>(spirvCode.data())[0] << std::dec
              << std::endl;
    vk::ShaderModule shaderModule = device.createShaderModule(shaderModuleInfo);

    // Create the compute pipeline
    vk::PipelineShaderStageCreateInfo shaderStageInfo;
    shaderStageInfo.stage  = vk::ShaderStageFlagBits::eCompute;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName  = "main"; // entry point name in your shader

    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.stage  = shaderStageInfo;
    pipelineInfo.layout = pipelineLayout; // from previous step


    auto result = device.createComputePipeline(nullptr, pipelineInfo);


    // Clean up shader module (can be destroyed after pipeline creation)
    device.destroyShaderModule(shaderModule);

    return result.value;
}

inline vk::DescriptorPool createDescriptorPool(vkb::Device vkboot_device)
{
    vk::DescriptorPoolSize poolSize;
    poolSize.type            = vk::DescriptorType::eStorageBuffer;
    poolSize.descriptorCount = 1; // one storage buffer

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
    poolInfo.maxSets       = 1;

    return vk::Device(vkboot_device.device).createDescriptorPool(poolInfo);
}

inline vk::DescriptorSet createDescriptorSet(vkb::Device vkboot_device, vk::DescriptorPool descriptorPool,
                                             vk::DescriptorSetLayout descriptorSetLayout)
{
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool     = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &descriptorSetLayout; // from earlier

    std::vector<vk::DescriptorSet> descriptorSets = vk::Device(vkboot_device.device).allocateDescriptorSets(allocInfo);
    return descriptorSets[0];
}

inline void updateDescriptorSet(vkb::Device vkboot_device, vk::Buffer buffer, vk::DescriptorSet descriptorSet)
{
    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = buffer; // the vk::Buffer you created
    bufferInfo.offset = 0;
    bufferInfo.range  = VK_WHOLE_SIZE; // or specific size in bytes

    // Write the buffer to the descriptor set
    vk::WriteDescriptorSet descriptorWrite;
    descriptorWrite.dstSet          = descriptorSet;
    descriptorWrite.dstBinding      = 0; // matches binding = 0 in shader/layout
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType  = vk::DescriptorType::eStorageBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo     = &bufferInfo;

    vk::Device(vkboot_device.device).updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

inline vk::CommandBuffer allocateCommandBuffer(vkb::Device vkboot_device, vk::CommandPool commandPool)
{
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool        = commandPool; // compute command pool from earlier
    allocInfo.level              = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    std::vector<vk::CommandBuffer> commandBuffers = vk::Device(vkboot_device.device).allocateCommandBuffers(allocInfo);
    return commandBuffers[0];
}
