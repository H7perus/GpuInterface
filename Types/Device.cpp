
#include "Device.h"

KE::VK::Device::Device() {};

KE::VK::Device::Device(vkb::Device vkboot_device, u32 deviceIndex)
{
    deviceIndex_    = deviceIndex;
    device_         = vk::Device(vkboot_device.device);
    physicalDevice_ = vk::PhysicalDevice(vkboot_device.physical_device);
    vkboot_device_  = vkboot_device;

    vk::PhysicalDeviceProperties2 props2;
    props2.pNext = &heapProperties_;

    physicalDevice_.getProperties2(&props2);


    queueFamilyIndices_ = FindQueueFamilies();
}

void KE::VK::Device::InitDescriptorHeaps()
{
    resourceHeap_ = DescriptorHeapBuffer(deviceIndex_, 64, DescriptorHeapType::ResourceHeap);
    samplerHeap_  = DescriptorHeapBuffer(deviceIndex_, 64, DescriptorHeapType::SamplerHeap);
}

vk::Device KE::VK::Device::GetVkDevice()
{
    return device_;
}

vk::PhysicalDevice KE::VK::Device::GetVkPhysicalDevice()
{
    return physicalDevice_;
}

u32 KE::VK::Device::FindMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties) const
{

    vk::PhysicalDevice                 physicalDevice = vk::PhysicalDevice(physicalDevice_);
    vk::PhysicalDeviceMemoryProperties memProperties;

    physicalDevice.getMemoryProperties(&memProperties);

    for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

vk::Queue KE::VK::Device::GetGraphicsQueue() const
{
    auto graphics_queue_ret = device_.getQueue(queueFamilyIndices_.graphicsFamily.value(), 0);
    if (!graphics_queue_ret)
    { /* report */
    }
    return graphics_queue_ret;
}

vk::Queue KE::VK::Device::GetComputeQueue() const
{
    auto compute_queue_ret = device_.getQueue(queueFamilyIndices_.computeFamily.value(), 0);
    if (!compute_queue_ret)
    { /* report */
    }
    return compute_queue_ret;
}

vk::CommandPool KE::VK::Device::CreateGraphicsCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags            = vk::CommandPoolCreateFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    poolInfo.queueFamilyIndex = queueFamilyIndices_.graphicsFamily.value();

    vk::CommandPool pool;

    try
    {
        pool = device_.createCommandPool(poolInfo, nullptr);
    }
    catch (const vk::SystemError& err)
    {
        std::cerr << "Failed to create command pool: " << err.what() << std::endl;
    }

    return pool;
}

vk::CommandPool KE::VK::Device::CreateComputeCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags            = vk::CommandPoolCreateFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    poolInfo.queueFamilyIndex = queueFamilyIndices_.computeFamily.value();

    vk::CommandPool pool;

    try
    {
        pool = device_.createCommandPool(poolInfo, nullptr);
    }
    catch (const vk::SystemError& err)
    {
        std::cerr << "Failed to create command pool: " << err.what() << std::endl;
    }

    return pool;
}
i32 KE::VK::Device::EnterResourceDescriptor(vk::ResourceDescriptorInfoEXT* descriptor)
{
    return resourceHeap_.EnterResourceDescriptor(descriptor);
}

i32 KE::VK::Device::EnterSamplerDescriptor(vk::SamplerCreateInfo* descriptor)
{
    return samplerHeap_.EnterSamplerDescriptor(descriptor);
};
;

KE::VK::QueueFamilyIndices KE::VK::Device::FindQueueFamilies()
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    physicalDevice_.getQueueFamilyProperties(&queueFamilyCount, nullptr);

    std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
    physicalDevice_.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        // TODO: Present family must also be supported.

        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
        {
            indices.computeFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }
        i++;
    }
    return indices;
}
