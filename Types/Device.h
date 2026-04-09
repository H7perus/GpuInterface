#pragma once

// #include "../VkHelpers.h"

#include "DescriptorHeapBuffer.h"
#include "GpuInterface/GpuInterface.h"
#include "ResourceSet.h"


#include "VkBootstrap.h"
#include "vulkan/vulkan.hpp"


#include <iostream>
#include <map>
#include <optional>

// TODO: Think about where to store named buffer references etc. Its a bit of an annoyance.


#include "GpuInterface/BasicTypeAliases.h"

using std::shared_ptr;

namespace KE::VK
{
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

class GPUI_DLL_API Device
{
    friend class DeviceManager;
    // reserved ranges and element size for sampler and resource heaps

  protected:
    u32                                           deviceIndex_;
    vk::Device                                    device_;
    vk::PhysicalDevice                            physicalDevice_;
    vkb::Device                                   vkboot_device_;
    vk::PhysicalDeviceDescriptorHeapPropertiesEXT heapProperties_;
    QueueFamilyIndices                            queueFamilyIndices_;

    std::map<std::string, shared_ptr<KE::VK::ResourceSet>> namedResources_;

    DescriptorHeapBuffer resourceHeap_ = DescriptorHeapBuffer();
    DescriptorHeapBuffer samplerHeap_  = DescriptorHeapBuffer();

  public:
    Device();

    Device(vkb::Device vkboot_device, u32 deviceIndex);

    void InitDescriptorHeaps();

    vk::Device GetVkDevice();

    vk::PhysicalDevice GetVkPhysicalDevice();


    template <std::derived_from<IResource> T> ResourceSet CreateResource(auto&&... args)
    {
        return ResourceSet::Create<T>(deviceIndex_, std::forward<decltype(args)>(args)...);
    }

    u32 FindMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties) const;


    vk::Queue GetGraphicsQueue() const;

    vk::Queue GetComputeQueue() const;


    vk::CommandPool CreateGraphicsCommandPool();

    vk::CommandPool CreateComputeCommandPool();


    i64 GetResourceHeapReservedRange() const
    {
        return heapProperties_.minResourceHeapReservedRange;
    }

    i64 GetResourceDescriptorStride() const
    {
        return (heapProperties_.bufferDescriptorAlignment > heapProperties_.imageDescriptorAlignment) ? heapProperties_.bufferDescriptorAlignment : heapProperties_.imageDescriptorAlignment;
    }

    i64 GetBufferDescriptorSize() const
    {
        i64 size = heapProperties_.bufferDescriptorSize;

        return size;
    }

    i64 GetImageDescriptorSize()
    {
        return heapProperties_.imageDescriptorSize;
    }

    i64 GetSamplerHeapReservedRange() const
    {
        return heapProperties_.minSamplerHeapReservedRange;
    }

    i64 GetSamplerDescriptorSize() const
    {
        return heapProperties_.samplerDescriptorSize;
    }

    vk::BindHeapInfoEXT* GetResourceHeapBindInfoPtr()
    {
        return resourceHeap_.GetBindInfo();
    }

    vk::BindHeapInfoEXT* GetSamplerHeapBindInfoPtr()
    {
        return samplerHeap_.GetBindInfo();
    }

    i32 EnterResourceDescriptor(vk::ResourceDescriptorInfoEXT* descriptor);

    i32 EnterSamplerDescriptor(vk::SamplerCreateInfo* descriptor);


    Device(Device&&) noexcept = default;

    operator vk::Device() const
    {
        return device_;
    }

    operator vk::Device()
    {
        return device_;
    }

    operator vk::PhysicalDevice() const
    {
        return physicalDevice_;
    }

    operator vk::PhysicalDevice()
    {
        return physicalDevice_;
    }

    operator vkb::Device() const
    {
        return vkboot_device_;
    }

  private:
    QueueFamilyIndices FindQueueFamilies();
};

} // namespace KE::VK